#include "GsfAlgorithm.h"
#include "GsfComponent.h"
#include "GsfTrackInitializer.h"
#include "GsfMixture.h"
#include "BetheHeitlerSplitter.h"
#include "FullMixtureModeStatus.h"

#include "DD4hep/Detector.h"
#include "DD4hep/DD4hepUnits.h"
#include "DDRec/MaterialManager.h"

#include "kaltest/TKalDetCradle.h"
#include "kaltest/TKalTrackState.h"
#include "kaltest/TKalTrackSite.h"
#include "kaltest/THelicalTrack.h"
#include "kaltest/KalTrackDim.h"
#include "kaltest/TVSurface.h"

#include "DDKalTest/DDVMeasLayer.h"
#include "DDKalTest/DDVTrackHit.h"
#include "DDKalTest/DDCylinderMeasLayer.h"
#include "DDKalTest/DDCylinderHit.h"
#include "DDKalTest/DDPlanarHit.h"

#include "DDKalTest/DDKalDetector.h"
#include "TrackSystemSvc/IMarlinTrack.h"
#include "TrackSystemSvc/ITrackSystemSvc.h"
#include "TrackSystemSvc/MarlinTrkUtils.h"

#include "edm4hep/TrackerHit.h"
#include "edm4hep/MCParticle.h"

#include "TDecompChol.h"
#include "TMatrixDSym.h"
#include "TVectorD.h"

#include <boost/format.hpp>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <exception>
#include <memory>
#include <limits>
#include <map>
#include <set>
#include <utility>
#include <functional>

DECLARE_COMPONENT(RecGsfTracking)

using DH = edm4hep::TrackState;

// ============================================================================
// Helpers
// ============================================================================

/// LCIO → KalTest parameter conversion
struct LcioSeed {
  double omega, d0, z0, phi, tanl;
  double refX, refY, refZ;
};

static LcioSeed extractSeed(const edm4hep::Track& trk) {
  for (auto loc : {DH::AtFirstHit, DH::AtIP}) {
    for (const auto& ts : trk.getTrackStates())
      if (ts.location == loc)
        return {ts.omega, ts.D0, ts.Z0, ts.phi, ts.tanLambda,
                ts.referencePoint.x, ts.referencePoint.y, ts.referencePoint.z};
  }
  return {};
}

/// Hit matched to a measurement layer
struct MatchedHit {
  edm4hep::TrackerHit lcioHit;
  const DDVMeasLayer* layer;
  DDVTrackHit*        kalHit;
  double              radius;
  std::size_t         inputOrder;
  int                 surfaceIndex;
  double              surfaceOrder;
};


/// Find the measurement layer for a TrackerHit, first by cellID-indexed lookup
/// (O(1) via IsOnSurface), then fall back to the legacy radius-based scan.
static const DDVMeasLayer* findLayer(
    const edm4hep::TrackerHit& th,
    const std::multimap<int, const DDVMeasLayer*>& cellIDIndex,
    const TKalDetCradle* cradle) {

  TVector3 pos(th.getPosition().x, th.getPosition().y, th.getPosition().z);

  // ── primary: cellID-based lookup ──
  auto range = cellIDIndex.equal_range(th.getCellID());
  for (auto it = range.first; it != range.second; ++it) {
    auto* ml = it->second;
    auto* sf = dynamic_cast<const TVSurface*>(ml);
    if (sf && sf->IsOnSurface(pos)) return ml;
  }

  // ── fallback: radius-based scan over all layers ──
  double hr = pos.Perp();
  const DDVMeasLayer* best = nullptr;
  double bestDist = 1e18;

  for (int i = 0; i < cradle->GetEntriesFast(); i++) {
    auto* ml = dynamic_cast<DDVMeasLayer*>(cradle->At(i));
    if (!ml) continue;
    auto* sf = dynamic_cast<const TVSurface*>(ml);
    if (!sf) continue;

    if (sf->IsOnSurface(pos)) return ml;

    double s = sf->GetSortingPolicy();
    double d2 = (s - hr) * (s - hr);
    if (d2 < bestDist && d2 < 25.0) {
      bestDist = d2;
      best = ml;
    }
  }
  return best;
}

static int covIndex5(int row, int col) {
  if (row < col) std::swap(row, col);
  return row * (row + 1) / 2 + col;
}

static TKalTrackSite* makeInitialSiteFromTrackState(
    const DH& ts, const MatchedHit& firstHit, double bz,
    double covarianceScale = 1.0);

static double normalizePhi(double phi) {
  while (phi >= M_PI) phi -= 2.0 * M_PI;
  while (phi < -M_PI) phi += 2.0 * M_PI;
  return phi;
}

static edm4hep::TrackState trackStateFromComponent(
    const GsfComponent& comp, double bz, int location) {
  if (comp.continuationValid) {
    edm4hep::TrackState ts = comp.continuationState;
    ts.location = location;
    return ts;
  }
  edm4hep::TrackState ts;
  ts.location = location;
  if (!comp.kaltrack || comp.kaltrack->GetEntriesFast() == 0 || bz == 0.0) return ts;

  auto* site = dynamic_cast<TKalTrackSite*>(comp.kaltrack->Last());
  if (!site) return ts;
  auto& state = dynamic_cast<TKalTrackState&>(site->GetCurState());
  auto helix = state.GetHelix();
  const auto& pivot = helix.GetPivot();
  const double alpha = bz * 2.99792458e-4;

  ts.D0 = -helix.GetDrho();
  ts.phi = normalizePhi(helix.GetPhi0() + M_PI / 2.0);
  ts.omega = helix.GetKappa() * alpha;
  ts.Z0 = helix.GetDz();
  ts.tanLambda = helix.GetTanLambda();
  ts.referencePoint = {pivot.X(), pivot.Y(), pivot.Z()};

  auto& cov = state.GetCovMat();
  for (auto& v : ts.covMatrix) v = 0.0;
  const double scale[5] = {-1.0, 1.0, alpha, 1.0, 1.0};
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j <= i; ++j) {
      ts.covMatrix[covIndex5(i, j)] = scale[i] * scale[j] * cov(i, j);
    }
  }
  return ts;
}


static double ptFromTrackState(const edm4hep::TrackState& ts, double bz) {
  const double alpha = bz * 2.99792458e-4;
  const double kappa = (alpha != 0.0) ? ts.omega / alpha : 0.0;
  return (kappa != 0.0) ? 1.0 / std::abs(kappa) : 0.0;
}

static std::string compactTrackState(const edm4hep::TrackState& ts, double bz) {
  std::ostringstream os;
  os << std::fixed << std::setprecision(4)
     << "ref=(" << ts.referencePoint.x << "," << ts.referencePoint.y << "," << ts.referencePoint.z << ")"
     << " pT=" << ptFromTrackState(ts, bz)
     << " d0=" << ts.D0
     << " z0=" << ts.Z0
     << " phi=" << ts.phi
     << " tanL=" << ts.tanLambda
     << " omega=" << ts.omega;
  return os.str();
}

static std::string compactMatrix(const MarlinTrk::MeasurementUpdate::Matrix& matrix) {
  std::ostringstream os;
  os << matrix.rows << "x" << matrix.cols << "[";
  for (int row = 0; row < matrix.rows; ++row) {
    if (row) os << "; ";
    for (int col = 0; col < matrix.cols; ++col) {
      if (col) os << ", ";
      os << std::setprecision(8) << matrix.values[row * matrix.cols + col];
    }
  }
  os << "]";
  return os.str();
}

static std::string surfacePredictionResidual(
    const GsfComponent& comp, const MatchedHit& hit, double bz) {
  std::ostringstream os;
  if (!comp.kaltrack || !hit.layer || !hit.kalHit) return "unavailable";

  auto helix = comp.helixAtLastSite(bz);
  TVector3 crossing;
  double dphi = 0.0;
  int cellId = 0;
  const TVector3 measured = hit.layer->HitToXv(*hit.kalHit);
  double bestResidual2 = std::numeric_limits<double>::infinity();
  for (int mode : {MarlinTrk::IMarlinTrack::modeBackward,
                   MarlinTrk::IMarlinTrack::modeClosest,
                   MarlinTrk::IMarlinTrack::modeForward}) {
    TVector3 candidate;
    double candidateDphi = 0.0;
    int candidateCellId = 0;
    if (hit.layer->getIntersectionAndCellID(
            helix, candidate, candidateDphi, candidateCellId, mode) == 0) continue;
    const double residual2 = (candidate - measured).Mag2();
    if (residual2 < bestResidual2) {
      bestResidual2 = residual2;
      crossing = candidate;
      dphi = candidateDphi;
      cellId = candidateCellId;
    }
  }
  if (!std::isfinite(bestResidual2)) return "no surface intersection";

  const TKalMatrix predicted = hit.layer->XvToMv(*hit.kalHit, crossing);
  const int dim = std::min(hit.kalHit->GetDimension(), predicted.GetNrows());
  const TVector3 globalResidual = measured - crossing;

  os << std::fixed << std::setprecision(6)
     << "predXYZ=(" << crossing.X() << "," << crossing.Y() << "," << crossing.Z() << ") mm"
     << " measXYZ=(" << measured.X() << "," << measured.Y() << "," << measured.Z() << ") mm"
     << " resXYZ(meas-pred)=(" << globalResidual.X() << ","
     << globalResidual.Y() << "," << globalResidual.Z() << ") mm"
     << " |cross-hit|=" << std::sqrt(bestResidual2) << " mm"
     << " local=[";
  for (int i = 0; i < dim; ++i) {
    if (i) os << ", ";
    const double measurement = hit.kalHit->GetX(i);
    const double sigma = hit.kalHit->GetDX(i);
    const double residual = measurement - predicted(i, 0);
    os << "m" << i << " pred=" << predicted(i, 0)
       << " meas=" << measurement << " res=" << residual
       << " sigma=" << sigma;
    if (sigma > 0.0) os << " measPull=" << residual / sigma;
  }
  os << "] dphi=" << dphi << " cell=" << cellId;
  return os.str();
}

static double componentFitChi2(const GsfComponent& comp) {
  if (comp.fitChi2 > 0.0) return comp.fitChi2;
  return comp.kaltrack ? comp.kaltrack->GetChi2() : 0.0;
}

static TMatrixD updateMatrix5(const MarlinTrk::MeasurementUpdate::Matrix& source) {
  TMatrixD result(5, 5);
  result.Zero();
  if (source.rows < 5 || source.cols < 5 ||
      source.values.size() < static_cast<size_t>(source.rows * source.cols))
    return result;
  for (int row = 0; row < 5; ++row)
    for (int col = 0; col < 5; ++col)
      result(row, col) = source.values[row * source.cols + col];
  return result;
}

static TMatrixD updateVector5(const MarlinTrk::MeasurementUpdate::Matrix& source) {
  TMatrixD result(5, 1);
  result.Zero();
  if (source.rows < 5 || source.cols != 1 ||
      source.values.size() < static_cast<size_t>(source.rows))
    return result;
  for (int row = 0; row < 5; ++row) result(row, 0) = source.values[row];
  return result;
}

static void trackStateToKalTest5(const edm4hep::TrackState& ts, double bz,
                                 TMatrixD& mean, TMatrixD& covariance) {
  const double alpha = bz * 2.99792458e-4;
  mean.ResizeTo(5, 1);
  mean(0, 0) = -ts.D0;
  mean(1, 0) = normalizePhi(ts.phi - M_PI / 2.0);
  mean(2, 0) = ts.omega / alpha;
  mean(3, 0) = ts.Z0;
  mean(4, 0) = ts.tanLambda;
  covariance.ResizeTo(5, 5);
  const double scale[5] = {-1.0, 1.0, 1.0 / alpha, 1.0, 1.0};
  for (int row = 0; row < 5; ++row)
    for (int col = 0; col < 5; ++col)
      covariance(row, col) = scale[row] * scale[col] *
          ts.covMatrix[covIndex5(row, col)];
}

static double maxRelativeMatrixDifference(const TMatrixD& actual,
                                          const TMatrixD& expected) {
  double maximum = 0.0;
  for (int row = 0; row < actual.GetNrows(); ++row) {
    for (int col = 0; col < actual.GetNcols(); ++col) {
      const double scale = std::max({1.0e-30, std::abs(actual(row, col)),
                                     std::abs(expected(row, col))});
      maximum = std::max(maximum,
                         std::abs(actual(row, col) - expected(row, col)) / scale);
    }
  }
  return maximum;
}

static bool appendBaselineStateToComponent(
    GsfComponent& comp, const edm4hep::TrackState& ts,
    const MatchedHit& hit, double bz,
    const edm4hep::TrackState& processInput,
    const MarlinTrk::MeasurementUpdate& update) {
  TKalTrackSite* site = makeInitialSiteFromTrackState(ts, hit, bz);
  if (!site || !comp.kaltrack) {
    delete site;
    return false;
  }
  comp.kaltrack->Add(site);
  GsfSmoothingStep step;
  trackStateToKalTest5(ts, bz, step.filteredMean, step.filteredCovariance);
  trackStateToKalTest5(processInput, bz, step.processInputMean,
                       step.processInputCovariance);
  step.predictedMean = updateVector5(update.predictedState);
  step.predictedCovariance = updateMatrix5(update.predictedCovariance);
  step.processJacobian = comp.pendingProcessJacobian;
  step.transportJacobian = updateMatrix5(update.transportJacobian);
  step.transportProcessNoise = updateMatrix5(update.processNoiseCovariance);
  step.transitionJacobian = step.transportJacobian;
  step.transitionValid = update.transportJacobian.rows >= 5 &&
      update.transportJacobian.cols >= 5 &&
      update.processNoiseCovariance.rows >= 5 &&
      update.processNoiseCovariance.cols >= 5;
  if (comp.pendingProcessJacobian.GetNrows() == 5 &&
      comp.pendingProcessJacobian.GetNcols() == 5) {
    step.transitionJacobian *= comp.pendingProcessJacobian;
  }
  if (step.transitionValid) {
    TMatrixD transportT(TMatrixD::kTransposed, step.transportJacobian);
    const TMatrixD reconstructedPrediction = step.transportJacobian *
        step.processInputCovariance * transportT + step.transportProcessNoise;
    step.transportCovarianceClosure = maxRelativeMatrixDifference(
        step.predictedCovariance, reconstructedPrediction);
    if (!comp.smoothingSteps.empty()) {
      const auto& previous = comp.smoothingSteps.back();
      TMatrixD processT(TMatrixD::kTransposed, step.processJacobian);
      const TMatrixD deterministicProcessCovariance = step.processJacobian *
          previous.filteredCovariance * processT;
      step.bhAddedKappaVariance = step.processInputCovariance(2, 2) -
          deterministicProcessCovariance(2, 2);
    }
  }
  comp.smoothingSteps.push_back(step);
  comp.pendingProcessJacobian.ResizeTo(5, 5);
  comp.pendingProcessJacobian.UnitMatrix();
  comp.continuationState = ts;
  comp.continuationValid = true;
  return true;
}

struct GsfSmootherNode {
  int surface = -1;
  int parent = -1;
  std::map<int, double> reductionSources;
  bool transitionValid = false;
  TMatrixD filteredMean{5, 1};
  TMatrixD filteredCovariance{5, 5};
  TMatrixD predictedMean{5, 1};
  TMatrixD predictedCovariance{5, 5};
  TMatrixD transitionJacobian{5, 5};
  double smoothedWeight = 0.0;
  bool smoothedValid = false;
  TMatrixD smoothedMean{5, 1};
  TMatrixD smoothedCovariance{5, 5};
};

struct GaussianAccumulator {
  double weight = 0.0;
  bool haveReference = false;
  double phiReference = 0.0;
  TMatrixD first{5, 1};
  TMatrixD second{5, 5};
};

struct GaussianMomentState {
  bool valid = false;
  TMatrixD mean{5, 1};
  TMatrixD covariance{5, 5};
};

struct GaussianComponentSnapshot {
  double weight = 0.0;
  int componentId = -1;
  int lineageNodeId = -1;
  bool noRadiationLineage = false;
  GaussianMomentState state;
};

/// Common transient result of the single outward GSF pass used by the
/// reverse workflow.  Filtered mixtures are captured after the
/// measurement posterior, cutoff, and KL reduction, but before the outgoing
/// material convolution.  The final live components are the shared inward
/// seed source; the snapshots remain immutable inputs to the two-filter
/// smoothed mixtures.  Their states never enter either filter, while the
/// explicit SmoothedMarginal mode may use their overlap weights to reweight
/// the live B_updated states.
class SharedForwardFilterResult {
public:
  SharedForwardFilterResult(std::size_t hitCount, double bz, bool enabled)
      : m_bz(bz), m_enabled(enabled), m_filtered(hitCount) {}

  void captureFiltered(std::size_t hitIndex,
                       const std::vector<GsfComponent*>& components) {
    if (!m_enabled || hitIndex >= m_filtered.size()) {
      return;
    }
    auto& snapshots = m_filtered[hitIndex];
    snapshots.clear();
    snapshots.reserve(components.size());
    for (const auto* component : components) {
      if (!component || !(component->weight > 0.0) ||
          !std::isfinite(component->weight)) {
        continue;
      }
      GaussianComponentSnapshot snapshot;
      snapshot.weight = component->weight;
      snapshot.componentId = component->debugId;
      snapshot.lineageNodeId = component->lineageNodeId;
      snapshot.noRadiationLineage = component->noRadiationLineage;
      component->helixAtLastSite(m_bz).PutInto(snapshot.state.mean);
      snapshot.state.covariance = component->covAtLastSite(m_bz);
      snapshot.state.valid = std::isfinite(snapshot.state.mean(2, 0)) &&
          std::isfinite(snapshot.state.covariance(2, 2));
      if (snapshot.state.valid) snapshots.push_back(std::move(snapshot));
    }
  }

  void complete(const std::vector<GsfComponent*>& components) {
    m_finalComponents.clear();
    if (!m_enabled) return;
    m_finalComponents.reserve(components.size());
    for (const auto* component : components)
      if (component) m_finalComponents.push_back(component);
  }

  const std::vector<GaussianComponentSnapshot>& filteredAt(
      std::size_t hitIndex) const {
    static const std::vector<GaussianComponentSnapshot> empty;
    return hitIndex < m_filtered.size() ? m_filtered[hitIndex] : empty;
  }

  const std::vector<const GsfComponent*>& finalComponents() const {
    return m_finalComponents;
  }

private:
  double m_bz = 0.0;
  bool m_enabled = false;
  std::vector<std::vector<GaussianComponentSnapshot>> m_filtered;
  std::vector<const GsfComponent*> m_finalComponents;
};

/// One reduced interior two-filter smoothed mixture
/// F_updated[i] x B_predicted[i], 0 < i < N-1.  The boundary smoothed states
/// are represented by the existing live mixtures: B_smoothed[0] is
/// B_updated[0], and B_smoothed[N-1] is F_updated[N-1].  Interior products are
/// retained independently of endpoint publication.  Their direct pre-pruning
/// pair weights are also marginalized by backward component so the explicit
/// SmoothedMarginal inward-weight mode can attach them to B_updated states.
struct GsfSmoothedSurfaceResult {
  std::vector<GsfComponent*> components;
  std::map<int, double> backwardMarginalWeights;
  int pairCandidates = 0;
  int pairFailures = 0;
};

/// Complete result of the inward GSF pass.  Reverse publication consumes
/// terminalBackward = B_updated[0] = B_smoothed[0].  Explicit smoothed-surface
/// results contain only the non-propagated interior products; the outer boundary
/// B_smoothed[N-1] is the final forward mixture already held by
/// SharedForwardFilterResult.
struct GsfInwardFilterResult {
  std::vector<GsfComponent*> terminalBackward;
  std::vector<GsfSmoothedSurfaceResult> smoothedSurfaces;
  int acceptedMeasurements = 0;
  int rejectedMeasurements = 0;
  int splits = 0;
  int reductions = 0;
  double seedCovarianceScale = 1.0;
  int seedMeasurementDimension = 0;
  bool freshSeedInitialization = false;

  explicit GsfInwardFilterResult(std::size_t hitCount)
      : smoothedSurfaces(std::max<std::size_t>(hitCount, 2)) {}
};

static void accumulateGaussian(GaussianAccumulator& accumulator,
                               double weight, TMatrixD mean,
                               const TMatrixD& covariance) {
  if (!(weight > 0.0) || !std::isfinite(weight)) return;
  if (!accumulator.haveReference) {
    accumulator.phiReference = mean(1, 0);
    accumulator.haveReference = true;
  } else {
    while (mean(1, 0) - accumulator.phiReference >= M_PI)
      mean(1, 0) -= 2.0 * M_PI;
    while (mean(1, 0) - accumulator.phiReference < -M_PI)
      mean(1, 0) += 2.0 * M_PI;
  }
  TMatrixD meanT(TMatrixD::kTransposed, mean);
  TMatrixD weightedMean = mean;
  weightedMean *= weight;
  accumulator.first += weightedMean;
  TMatrixD weightedSecond = covariance + mean * meanT;
  weightedSecond *= weight;
  accumulator.second += weightedSecond;
  accumulator.weight += weight;
}

static bool finishGaussian(const GaussianAccumulator& accumulator,
                           TMatrixD& mean, TMatrixD& covariance) {
  if (!(accumulator.weight > 0.0) || !accumulator.haveReference) return false;
  mean = accumulator.first;
  mean *= 1.0 / accumulator.weight;
  TMatrixD meanT(TMatrixD::kTransposed, mean);
  covariance = accumulator.second;
  covariance *= 1.0 / accumulator.weight;
  covariance -= mean * meanT;
  return true;
}

static bool finiteMatrix(const TMatrixD& matrix);

static bool formSmoothedGaussian(
    const GaussianMomentState& forwardUpdated,
    const GaussianMomentState& backwardPredicted,
    GaussianMomentState& smoothed, double* logOverlap = nullptr,
    double* overlapDChi2 = nullptr, double* overlapLogDet = nullptr) {
  if (!forwardUpdated.valid || !backwardPredicted.valid) return false;
  TMatrixD covarianceSum = forwardUpdated.covariance +
                           backwardPredicted.covariance;
  double determinant = 0.0;
  covarianceSum.Invert(&determinant);
  if (!(determinant > 0.0) || !std::isfinite(determinant)) return false;
  TMatrixD gain = forwardUpdated.covariance * covarianceSum;
  TMatrixD delta = backwardPredicted.mean - forwardUpdated.mean;
  while (delta(1, 0) >= M_PI) delta(1, 0) -= 2.0 * M_PI;
  while (delta(1, 0) < -M_PI) delta(1, 0) += 2.0 * M_PI;
  if (logOverlap || overlapDChi2 || overlapLogDet) {
    const TMatrixD scaledDelta = covarianceSum * delta;
    double quadratic = 0.0;
    for (int index = 0; index < 5; ++index)
      quadratic += delta(index, 0) * scaledDelta(index, 0);
    if (!(quadratic >= 0.0) || !std::isfinite(quadratic)) return false;
    const double logDet = std::log(determinant);
    if (overlapDChi2) *overlapDChi2 = quadratic;
    if (overlapLogDet) *overlapLogDet = logDet;
    if (logOverlap) {
      *logOverlap = -0.5 *
          (5.0 * std::log(2.0 * M_PI) + logDet + quadratic);
    }
  }
  smoothed.mean = forwardUpdated.mean + gain * delta;
  smoothed.covariance = gain * backwardPredicted.covariance;
  TMatrixD covarianceT(TMatrixD::kTransposed, smoothed.covariance);
  smoothed.covariance += covarianceT;
  smoothed.covariance *= 0.5;
  smoothed.valid = std::isfinite(smoothed.mean(2, 0)) &&
                   std::isfinite(smoothed.covariance(2, 2));
  return smoothed.valid;
}

static bool initializeSmoothedComponent(
    GsfComponent& component, const GaussianMomentState& state,
    const TVector3& pivot, double bz, double weight, int componentId,
    bool noRadiationLineage) {
  if (!state.valid || !(weight > 0.0) || !std::isfinite(weight) || bz == 0.0)
    return false;
  component.weight = weight;
  component.debugId = componentId;
  component.noRadiationLineage = noRadiationLineage;
  component.debugHistory = "two-filter-smoothed";
  component.continuationValid = true;
  component.continuationState.location = DH::AtOther;
  component.continuationState.referencePoint = {
      static_cast<float>(pivot.X()), static_cast<float>(pivot.Y()),
      static_cast<float>(pivot.Z())};
  return component.setContinuationSurfaceState(
      state.mean, state.covariance, bz);
}

static int appendMeasurementSmootherNode(
    GsfComponent& component, int surface,
    std::vector<GsfSmootherNode>& graph) {
  if (component.smoothingSteps.empty()) return -1;
  const auto& step = component.smoothingSteps.back();
  GsfSmootherNode node;
  node.surface = surface;
  node.parent = component.smoothingNodeId;
  node.transitionValid = node.parent < 0 || step.transitionValid;
  node.filteredMean = step.filteredMean;
  node.filteredCovariance = step.filteredCovariance;
  node.predictedMean = step.predictedMean;
  node.predictedCovariance = step.predictedCovariance;
  node.transitionJacobian = step.transitionJacobian;
  graph.push_back(node);
  component.smoothingNodeId = static_cast<int>(graph.size()) - 1;
  component.smoothingSourceFractions.clear();
  component.smoothingSourceFractions[component.smoothingNodeId] = 1.0;
  return component.smoothingNodeId;
}

static bool appendReductionSmootherNodes(
    std::vector<GsfComponent*>& components, int surface, double bz,
    std::vector<GsfSmootherNode>& graph) {
  for (auto* component : components) {
    if (!component || component->smoothingSourceFractions.empty()) return false;
    if (component->smoothingSourceFractions.size() == 1 &&
        component->smoothingSourceFractions.begin()->first ==
            component->smoothingNodeId) continue;
    GsfSmootherNode node;
    node.surface = surface;
    node.reductionSources = component->smoothingSourceFractions;
    if (!component->continuationValid) return false;
    trackStateToKalTest5(component->continuationState, bz, node.filteredMean,
                         node.filteredCovariance);
    graph.push_back(node);
    component->smoothingNodeId = static_cast<int>(graph.size()) - 1;
    component->smoothingSourceFractions.clear();
    component->smoothingSourceFractions[component->smoothingNodeId] = 1.0;
  }
  return true;
}

static bool smoothKlReductionGraph(
    std::vector<GsfComponent*>& components,
    std::vector<GsfSmootherNode>& graph,
    int& activeNodes, int& reductionNodes) {
  if (components.empty() || graph.empty()) return false;
  std::vector<GaussianAccumulator> accumulators(graph.size());
  GsfMixture::normalizeWeights(components);
  for (auto* component : components) {
    if (!component || component->smoothingNodeId < 0 ||
        component->smoothingNodeId >= static_cast<int>(graph.size())) return false;
    const auto& terminal = graph[component->smoothingNodeId];
    accumulateGaussian(accumulators[component->smoothingNodeId],
                       component->weight, terminal.filteredMean,
                       terminal.filteredCovariance);
  }

  activeNodes = 0;
  reductionNodes = 0;
  for (int id = static_cast<int>(graph.size()) - 1; id >= 0; --id) {
    auto& node = graph[id];
    if (!finishGaussian(accumulators[id], node.smoothedMean,
                        node.smoothedCovariance)) continue;
    node.smoothedWeight = accumulators[id].weight;
    node.smoothedValid = true;
    ++activeNodes;

    if (!node.reductionSources.empty()) {
      ++reductionNodes;
      TMatrixD reducedInverse = node.filteredCovariance;
      double determinant = 0.0;
      reducedInverse.Invert(&determinant);
      if (!(std::abs(determinant) > 0.0) || !std::isfinite(determinant))
        return false;
      for (const auto& source : node.reductionSources) {
        if (source.first < 0 || source.first >= id || source.second < 0.0)
          return false;
        const auto& sourceNode = graph[source.first];
        TMatrixD gain = sourceNode.filteredCovariance * reducedInverse;
        TMatrixD delta = node.smoothedMean - node.filteredMean;
        delta(1, 0) = normalizePhi(delta(1, 0));
        TMatrixD mean = sourceNode.filteredMean + gain * delta;
        TMatrixD gainT(TMatrixD::kTransposed, gain);
        TMatrixD covariance = sourceNode.filteredCovariance +
            gain * (node.smoothedCovariance - node.filteredCovariance) * gainT;
        accumulateGaussian(accumulators[source.first],
                           node.smoothedWeight * source.second,
                           mean, covariance);
      }
      continue;
    }

    if (node.parent >= 0) {
      if (!node.transitionValid || node.parent >= id) return false;
      const auto& parent = graph[node.parent];
      TMatrixD predictedInverse = node.predictedCovariance;
      double determinant = 0.0;
      predictedInverse.Invert(&determinant);
      if (!(std::abs(determinant) > 0.0) || !std::isfinite(determinant))
        return false;
      TMatrixD transitionT(TMatrixD::kTransposed, node.transitionJacobian);
      TMatrixD gain = parent.filteredCovariance * transitionT * predictedInverse;
      TMatrixD delta = node.smoothedMean - node.predictedMean;
      delta(1, 0) = normalizePhi(delta(1, 0));
      TMatrixD mean = parent.filteredMean + gain * delta;
      TMatrixD gainT(TMatrixD::kTransposed, gain);
      TMatrixD covariance = parent.filteredCovariance +
          gain * (node.smoothedCovariance - node.predictedCovariance) * gainT;
      accumulateGaussian(accumulators[node.parent], node.smoothedWeight,
                         mean, covariance);
    }
  }

  GaussianAccumulator inner;
  int firstSurface = std::numeric_limits<int>::max();
  for (const auto& node : graph)
    if (node.smoothedValid) firstSurface = std::min(firstSurface, node.surface);
  for (const auto& node : graph) {
    if (node.smoothedValid && node.surface == firstSurface &&
        node.reductionSources.empty())
      accumulateGaussian(inner, node.smoothedWeight, node.smoothedMean,
                         node.smoothedCovariance);
  }
  TMatrixD innerMean(5, 1), innerCovariance(5, 5);
  if (!finishGaussian(inner, innerMean, innerCovariance)) return false;
  for (auto* component : components) {
    component->smoothedInnerMean = innerMean;
    component->smoothedInnerCovariance = innerCovariance;
    component->smoothedInnerValid = true;
  }
  return true;
}

static TKalTrackSite* makeInitialSiteFromTrackState(
    const DH& ts, const MatchedHit& firstHit, double bz,
    double covarianceScale) {

  const double alpha = bz * 2.99792458e-4;
  if (alpha == 0.0) return nullptr;

  TVector3 tsPivot(ts.referencePoint.x, ts.referencePoint.y, ts.referencePoint.z);
  TVector3 hitPivot = firstHit.layer->HitToXv(*firstHit.kalHit);

  THelicalTrack helix(-ts.D0, ts.phi - M_PI / 2., ts.omega / alpha,
                      ts.Z0, ts.tanLambda,
                      tsPivot.X(), tsPivot.Y(), tsPivot.Z(), bz);

  TMatrixD cov5(5, 5);
  cov5.Zero();
  const double scale[5] = {-1.0, 1.0, 1.0 / alpha, 1.0, 1.0};
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 5; ++j) {
      cov5(i, j) = covarianceScale * scale[i] * scale[j] *
                   ts.covMatrix[covIndex5(i, j)];
    }
  }

  double dphi = 0.0;
  TMatrixD jac(5, 5);
  jac.UnitMatrix();
  helix.MoveTo(hitPivot, dphi, &jac, &cov5);

  TKalMatrix sv(kSdim, 1);
  sv(0, 0) = helix.GetDrho();
  sv(1, 0) = helix.GetPhi0();
  sv(2, 0) = helix.GetKappa();
  sv(3, 0) = helix.GetDz();
  sv(4, 0) = helix.GetTanLambda();
  sv(5, 0) = 0.0;

  TKalMatrix cv(kSdim, kSdim);
  cv.Zero();
  for (int i = 0; i < 5; ++i)
    for (int j = 0; j < 5; ++j)
      cv(i, j) = cov5(i, j);
  cv(5, 5) = 1e6;

  TVTrackHit* siteHit = nullptr;
  if (auto* ch = dynamic_cast<DDCylinderHit*>(firstHit.kalHit))
    siteHit = new DDCylinderHit(*ch);
  else if (auto* ph = dynamic_cast<DDPlanarHit*>(firstHit.kalHit))
    siteHit = new DDPlanarHit(*ph);
  if (!siteHit) return nullptr;

  auto& site = *new TKalTrackSite(*siteHit, kSdim);
  site.SetHitOwner();
  site.SetOwner();
  site.SetPivot(hitPivot);
  site.Add(new TKalTrackState(sv, cv, site, TVKalSite::kPredicted));
  site.Add(new TKalTrackState(sv, cv, site, TVKalSite::kFiltered));
  return &site;
}

/// Pure geometric extrapolation: innermost state → IP via helix move.
/// Covariance is propagated through the MoveTo Jacobian: C' = F·C·Fᵀ.
/// No material effects (MS / energy loss) applied.
static void extrapolateToIP_geometric(GsfComponent* comp,
                                      const DDCylinderMeasLayer* ipLayer,
                                      double bz,
                                      THelicalTrack& outHelix, TMatrixD& outCov) {
  auto& innerSite = *dynamic_cast<const TKalTrackSite*>(comp->kaltrack->At(1));
  auto& innerState = dynamic_cast<TKalTrackState&>(innerSite.GetCurState());

  if (comp->smoothedInnerValid) {
    outCov.ResizeTo(5, 5);
    outCov = comp->smoothedInnerCovariance;
    outHelix = THelicalTrack(comp->smoothedInnerMean, innerSite.GetPivot(), bz);
    double dphi = 0.0;
    TMatrixD jacobian(5, 5);
    jacobian.UnitMatrix();
    outHelix.MoveTo(TVector3(0, 0, 0), dphi, &jacobian, &outCov);
    return;
  }

  outCov.ResizeTo(5, 5);
  for (int i = 0; i < 5; i++)
    for (int j = 0; j < 5; j++)
      outCov(i, j) = innerState.GetCovMat()(i, j);

  outHelix = innerState.GetHelix();

  TVector3 ipPoint(0, 0, 0);
  if (ipLayer && ipLayer != &innerSite.GetHit().GetMeasLayer()) {
    double dphi;
    TMatrixD jac(5, 5);
    jac.UnitMatrix();
    outHelix.MoveTo(ipPoint, dphi, &jac, &outCov);  // jac=F, outCov=F·C·Fᵀ
  }
}

/// Extrapolate the current continuation state to the IP.  This is used by the
/// reverse pass, whose last stored site is the innermost filtered measurement
/// and whose continuation snapshot additionally contains the reversed process
/// transition on that surface.
static bool extrapolateContinuationToIP(const GsfComponent& comp, double bz,
                                        THelicalTrack& outHelix,
                                        TMatrixD& outCov) {
  if (!comp.continuationValid || bz == 0.0) return false;
  outHelix = comp.helixAtLastSite(bz);
  outCov = comp.covAtLastSite(bz);
  double dphi = 0.0;
  TMatrixD jacobian(5, 5);
  jacobian.UnitMatrix();
  outHelix.MoveTo(TVector3(0.0, 0.0, 0.0), dphi, &jacobian, &outCov);
  return std::isfinite(outHelix.GetKappa()) &&
         std::isfinite(outCov(2, 2));
}

/// Full material-aware extrapolation: innermost state → IP using cradle Transport.
/// Includes multiple scattering and energy-loss correction via tQ and tSv.
/// MSOn / ElossOn must be set on the cradle for effects to be active.
static void extrapolateToIP_material(GsfComponent* comp,
                                     TKalDetCradle* cradle,
                                     const DDCylinderMeasLayer* ipLayer,
                                     double bz,
                                     THelicalTrack& outHelix, TMatrixD& outCov) {
  auto& innerSite = *dynamic_cast<const TKalTrackSite*>(comp->kaltrack->At(1));
  auto& innerState = dynamic_cast<TKalTrackState&>(innerSite.GetCurState());

  TKalMatrix tF(kSdim, kSdim);
  tF.UnitMatrix();
  TKalMatrix tQ(kSdim, kSdim);
  tQ.Zero();
  TKalMatrix tSv(kSdim, 1);
  TVector3 tX0;

  cradle->Transport(innerSite, *ipLayer, tX0, tSv, tF, tQ);

  // Propagate covariance: C' = F·C·Fᵀ + Q
  TKalMatrix kCov = innerState.GetCovMat();
  TKalMatrix tFt(TMatrixD::kTransposed, tF);
  kCov = tF * kCov * tFt + tQ;

  outCov.ResizeTo(5, 5);
  for (int i = 0; i < 5; i++)
    for (int j = 0; j < 5; j++)
      outCov(i, j) = kCov(i, j);

  // tSv is the energy-loss-corrected state at IP (already a 5-d helix, +1)
  outHelix = THelicalTrack(tSv, TVector3(0, 0, 0), bz);
}

static void helixToMean(const THelicalTrack& h, TMatrixD& mean) {
  mean.ResizeTo(5, 1);
  mean(0, 0) = h.GetDrho();
  mean(1, 0) = h.GetPhi0();
  mean(2, 0) = h.GetKappa();
  mean(3, 0) = h.GetDz();
  mean(4, 0) = h.GetTanLambda();
}

static void wrapPhiNear(double& phi, double ref) {
  while (phi - ref > M_PI) phi -= 2.0 * M_PI;
  while (phi - ref < -M_PI) phi += 2.0 * M_PI;
}

static bool extrapolateToIP_component(GsfComponent* comp,
                                      bool materialIPExtrap,
                                      TKalDetCradle* cradle,
                                      const DDCylinderMeasLayer* ipLayer,
                                      double bz,
                                      THelicalTrack& outHelix,
                                      TMatrixD& outCov) {
  if (!comp || !comp->kaltrack || comp->kaltrack->GetEntriesFast() <= 1)
    return false;
  if (materialIPExtrap)
    extrapolateToIP_material(comp, cradle, ipLayer, bz, outHelix, outCov);
  else
    extrapolateToIP_geometric(comp, ipLayer, bz, outHelix, outCov);
  return true;
}

static bool weightedMixtureAtIP(const std::vector<GsfComponent*>& comps,
                                bool materialIPExtrap,
                                TKalDetCradle* cradle,
                                const DDCylinderMeasLayer* ipLayer,
                                double bz,
                                THelicalTrack& outHelix,
                                TMatrixD& outCov) {
  std::vector<double> weights;
  std::vector<TMatrixD> means;
  std::vector<TMatrixD> covs;
  weights.reserve(comps.size());
  means.reserve(comps.size());
  covs.reserve(comps.size());

  double sumW = 0.0;
  double phiRef = 0.0;
  bool havePhiRef = false;

  for (auto* comp : comps) {
    if (!comp || comp->weight <= 0.0) continue;
    THelicalTrack h(TMatrixD(5,1), TVector3(0, 0, 0), bz);
    TMatrixD cov(5, 5);
    if (!extrapolateToIP_component(comp, materialIPExtrap, cradle, ipLayer, bz, h, cov))
      continue;

    TMatrixD mean(5, 1);
    helixToMean(h, mean);
    if (!havePhiRef) {
      phiRef = mean(1, 0);
      havePhiRef = true;
    } else {
      wrapPhiNear(mean(1, 0), phiRef);
    }

    weights.push_back(comp->weight);
    means.push_back(mean);
    covs.push_back(cov);
    sumW += comp->weight;
  }

  if (weights.empty() || sumW <= 0.0) return false;

  TMatrixD mergedMean(5, 1);
  mergedMean.Zero();
  for (size_t i = 0; i < weights.size(); i++) {
    TMatrixD term = means[i];
    term *= weights[i] / sumW;
    mergedMean += term;
  }
  wrapPhiNear(mergedMean(1, 0), 0.0);

  TMatrixD mergedCov(5, 5);
  mergedCov.Zero();
  for (size_t i = 0; i < weights.size(); i++) {
    const double w = weights[i] / sumW;
    TMatrixD d = means[i] - mergedMean;
    wrapPhiNear(d(1, 0), 0.0);
    TMatrixD dT(TMatrixD::kTransposed, d);
    TMatrixD term = covs[i] + d * dT;
    term *= w;
    mergedCov += term;
  }

  outCov.ResizeTo(5, 5);
  outCov = mergedCov;
  outHelix = THelicalTrack(mergedMean, TVector3(0, 0, 0), bz);
  return true;
}

/// Moment-match the reverse-filter continuation mixture at the IP. Reverse
/// tracks are stored outer-to-inner, so the forward helper cannot obtain their
/// innermost state from kaltrack->At(1).
static bool weightedReverseMixtureAtIP(
    const std::vector<GsfComponent*>& comps, double bz,
    THelicalTrack& outHelix, TMatrixD& outCov) {
  std::vector<double> weights;
  std::vector<TMatrixD> means;
  std::vector<TMatrixD> covariances;
  double sumWeight = 0.0;
  double phiReference = 0.0;
  bool havePhiReference = false;

  for (const auto* comp : comps) {
    if (!comp || comp->weight <= 0.0) continue;
    THelicalTrack helix(TMatrixD(5, 1), TVector3(0, 0, 0), bz);
    TMatrixD covariance(5, 5);
    if (!extrapolateContinuationToIP(*comp, bz, helix, covariance)) continue;
    TMatrixD mean(5, 1);
    helixToMean(helix, mean);
    if (!havePhiReference) {
      phiReference = mean(1, 0);
      havePhiReference = true;
    } else {
      wrapPhiNear(mean(1, 0), phiReference);
    }
    weights.push_back(comp->weight);
    means.push_back(mean);
    covariances.push_back(covariance);
    sumWeight += comp->weight;
  }
  if (weights.empty() || sumWeight <= 0.0) return false;

  TMatrixD mergedMean(5, 1);
  mergedMean.Zero();
  for (size_t i = 0; i < weights.size(); ++i) {
    TMatrixD term = means[i];
    term *= weights[i] / sumWeight;
    mergedMean += term;
  }
  wrapPhiNear(mergedMean(1, 0), 0.0);

  TMatrixD mergedCovariance(5, 5);
  mergedCovariance.Zero();
  for (size_t i = 0; i < weights.size(); ++i) {
    const double weight = weights[i] / sumWeight;
    TMatrixD delta = means[i] - mergedMean;
    wrapPhiNear(delta(1, 0), 0.0);
    TMatrixD deltaT(TMatrixD::kTransposed, delta);
    TMatrixD term = covariances[i] + delta * deltaT;
    term *= weight;
    mergedCovariance += term;
  }

  outCov.ResizeTo(5, 5);
  outCov = mergedCovariance;
  outHelix = THelicalTrack(mergedMean, TVector3(0, 0, 0), bz);
  return true;
}

// The FullMixtureMode endpoint is the maximum of the complete five-dimensional
// Gaussian-mixture density at the IP. It is neither a component selector nor a
// one-dimensional kappa splice. The deterministic multistart search below uses
// every component mean, the global moment, and every pairwise weighted midpoint
// as seeds, then solves the Gaussian-mixture stationary-point equation and
// finishes with local Newton refinement.
struct PreparedModeComponent {
  double weight = 0.0;
  TMatrixD mean{5, 1};
  TMatrixD precision{5, 5};
  double logNormalizer = 0.0;
};

struct MixtureModeEvaluation {
  bool valid = false;
  double logDensity = -std::numeric_limits<double>::infinity();
  std::vector<double> responsibilities;
  TMatrixD gradient{5, 1};
  TMatrixD hessian{5, 5};
};

struct FullMixtureModeDiagnostics {
  int inputComponents = 0;
  int usableComponents = 0;
  int starts = 0;
  int maxima = 0;
  int iterations = 0;
  double logDensity = -std::numeric_limits<double>::infinity();
  double scaledGradient = std::numeric_limits<double>::infinity();
};

static bool finiteMatrix(const TMatrixD& matrix) {
  for (int row = 0; row < matrix.GetNrows(); ++row)
    for (int column = 0; column < matrix.GetNcols(); ++column)
      if (!std::isfinite(matrix(row, column))) return false;
  return true;
}

static bool invertPositiveDefinite(const TMatrixD& matrix,
                                   TMatrixD& inverse,
                                   double* logDeterminant = nullptr) {
  if (matrix.GetNrows() != matrix.GetNcols() || !finiteMatrix(matrix))
    return false;
  const int dimension = matrix.GetNrows();
  TMatrixDSym symmetric(dimension);
  for (int row = 0; row < dimension; ++row) {
    for (int column = 0; column <= row; ++column) {
      const double value =
          0.5 * (matrix(row, column) + matrix(column, row));
      symmetric(row, column) = value;
    }
  }
  TDecompChol decomposition(symmetric);
  if (!decomposition.Decompose()) return false;
  if (logDeterminant) {
    double value = 0.0;
    const auto& upper = decomposition.GetU();
    for (int index = 0; index < dimension; ++index) {
      const double diagonal = upper(index, index);
      if (!(diagonal > 0.0) || !std::isfinite(diagonal)) return false;
      value += 2.0 * std::log(diagonal);
    }
    *logDeterminant = value;
  }
  Bool_t inversionOk = false;
  const TMatrixDSym symmetricInverse = decomposition.Invert(inversionOk);
  if (!inversionOk) return false;
  inverse.ResizeTo(dimension, dimension);
  for (int row = 0; row < dimension; ++row)
    for (int column = 0; column < dimension; ++column)
      inverse(row, column) = symmetricInverse(row, column);
  return finiteMatrix(inverse);
}

enum class FinalMixtureComponentSource : std::int32_t {
  GaussianSumSmoother = 1,
  ReverseFiltering = 2,
};

enum class LineageNodeSource : std::int32_t {
  ForwardFiltering = 1,
  ReverseFiltering = 2,
  SmoothedMixture = 3,
};

enum class LineageNodeOperation : std::int32_t {
  Seed = 1,
  BetheHeitlerSplit = 2,
  Measurement = 3,
  KlMerge = 4,
  Smoothing = 5,
};

enum class LineageNodeFate : std::int32_t {
  Active = 0,
  Advanced = 1,
  MeasurementRejected = 2,
  WeightCutoff = 3,
  KlMerged = 4,
  FinalSurvivor = 5,
  TrackAbandoned = 6,
  InwardInternalMessage = 7,
};

enum class LineageEdgeOperation : std::int32_t {
  BetheHeitlerSplit = 1,
  Measurement = 2,
  KlMerge = 3,
  ReverseSeed = 4,
  Smoothing = 5,
};

struct LineageNodeRecord {
  std::int32_t nodeId = -1;
  std::int32_t source = 0;
  std::int32_t operation = 0;
  std::int32_t hitIndex = -1;
  std::int32_t surfaceIndex = -1;
  std::int32_t componentId = -1;
  std::int32_t generation = 0;
  std::int32_t bhComponentIndex = -1;
  std::int32_t measurementStatus = -1;
  std::int32_t fate = static_cast<std::int32_t>(LineageNodeFate::Active);
  std::int32_t noRadiationLineage = 0;
  std::int32_t bestBranch = 0;
  std::int32_t finalMixture = 0;
  std::int32_t valid = 0;
  double weight = std::numeric_limits<double>::quiet_NaN();
  double priorWeight = std::numeric_limits<double>::quiet_NaN();
  double bhWeight = std::numeric_limits<double>::quiet_NaN();
  double bhMean = std::numeric_limits<double>::quiet_NaN();
  double bhVariance = std::numeric_limits<double>::quiet_NaN();
  double materialTX0 = std::numeric_limits<double>::quiet_NaN();
  double dchi2 = std::numeric_limits<double>::quiet_NaN();
  double logDetInnovation = std::numeric_limits<double>::quiet_NaN();
  double logUnnormalizedPosterior =
      std::numeric_limits<double>::quiet_NaN();
  double normalizedPosterior = std::numeric_limits<double>::quiet_NaN();
  double predictedKappa = std::numeric_limits<double>::quiet_NaN();
  double predictedKappaVariance = std::numeric_limits<double>::quiet_NaN();
  double filteredKappa = std::numeric_limits<double>::quiet_NaN();
  double filteredKappaVariance = std::numeric_limits<double>::quiet_NaN();
  double dominantLineageFraction =
      std::numeric_limits<double>::quiet_NaN();
  double mergeCost = std::numeric_limits<double>::quiet_NaN();
};

struct LineageEdgeRecord {
  std::int32_t fromNodeId = -1;
  std::int32_t toNodeId = -1;
  std::int32_t operation = 0;
};

class LineageGraphRecorder {
public:
  LineageGraphRecorder(bool enabled, double bz) : m_enabled(enabled), m_bz(bz) {}

  bool enabled() const { return m_enabled; }
  const std::vector<LineageNodeRecord>& nodes() const { return m_nodes; }
  const std::vector<LineageEdgeRecord>& edges() const { return m_edges; }

  int seed(GsfComponent& component, LineageNodeSource source, int hitIndex,
           int surfaceIndex) {
    return appendNode(component, source, LineageNodeOperation::Seed,
                      hitIndex, surfaceIndex);
  }

  int split(GsfComponent& child, int parentNodeId, LineageNodeSource source,
            int hitIndex, int surfaceIndex, int bhComponentIndex,
            const BetheHeitlerMixtureComponent& bh, double materialTX0) {
    const int nodeId = appendNode(child, source,
        LineageNodeOperation::BetheHeitlerSplit, hitIndex, surfaceIndex);
    if (nodeId < 0) return nodeId;
    auto& node = m_nodes[static_cast<std::size_t>(nodeId)];
    node.priorWeight = validNode(parentNodeId)
        ? m_nodes[static_cast<std::size_t>(parentNodeId)].weight
        : std::numeric_limits<double>::quiet_NaN();
    node.bhComponentIndex = bhComponentIndex;
    node.bhWeight = bh.weight;
    node.bhMean = bh.mean;
    node.bhVariance = bh.variance;
    node.materialTX0 = materialTX0;
    addEdge(parentNodeId, nodeId, LineageEdgeOperation::BetheHeitlerSplit);
    return nodeId;
  }

  int measurement(GsfComponent& component, int parentNodeId,
                  LineageNodeSource source, int hitIndex, int surfaceIndex,
                  int status, double priorWeight, double dchi2,
                  double logDetInnovation, double logPosterior,
                  const MarlinTrk::MeasurementUpdate* update) {
    const int nodeId = appendNode(component, source,
        LineageNodeOperation::Measurement, hitIndex, surfaceIndex);
    if (nodeId < 0) return nodeId;
    auto& node = m_nodes[static_cast<std::size_t>(nodeId)];
    node.measurementStatus = status;
    node.priorWeight = priorWeight;
    node.dchi2 = dchi2;
    node.logDetInnovation = logDetInnovation;
    node.logUnnormalizedPosterior = logPosterior;
    if (update && update->predictedState.rows >= 5 &&
        update->predictedState.cols == 1 &&
        update->predictedState.values.size() >= 5) {
      node.predictedKappa = update->predictedState.values[2];
    }
    if (update && update->predictedCovariance.rows >= 5 &&
        update->predictedCovariance.cols >= 5 &&
        update->predictedCovariance.values.size() >=
            static_cast<std::size_t>(update->predictedCovariance.rows *
                                     update->predictedCovariance.cols)) {
      node.predictedKappaVariance =
          update->predictedCovariance.values[
              2 * update->predictedCovariance.cols + 2];
    }
    addEdge(parentNodeId, nodeId, LineageEdgeOperation::Measurement);
    if (status == 0) mark(nodeId, LineageNodeFate::MeasurementRejected);
    return nodeId;
  }

  int merge(GsfComponent& merged, int keepSourceNodeId,
            int dropSourceNodeId, LineageNodeSource source, int hitIndex,
            int surfaceIndex, double mergeCost) {
    mark(keepSourceNodeId, LineageNodeFate::KlMerged);
    mark(dropSourceNodeId, LineageNodeFate::KlMerged);
    const int nodeId = appendNode(merged, source,
        LineageNodeOperation::KlMerge, hitIndex, surfaceIndex);
    if (nodeId < 0) return nodeId;
    auto& node = m_nodes[static_cast<std::size_t>(nodeId)];
    node.mergeCost = mergeCost;
    addEdge(keepSourceNodeId, nodeId, LineageEdgeOperation::KlMerge, false);
    addEdge(dropSourceNodeId, nodeId, LineageEdgeOperation::KlMerge, false);
    return nodeId;
  }

  int reverseSeed(GsfComponent& component, int forwardNodeId, int hitIndex,
                  int surfaceIndex) {
    const int nodeId = seed(component, LineageNodeSource::ReverseFiltering,
                            hitIndex, surfaceIndex);
    addEdge(forwardNodeId, nodeId, LineageEdgeOperation::ReverseSeed);
    return nodeId;
  }

  int smoothedMixture(GsfComponent& component, int forwardNodeId,
                      int backwardNodeId, int hitIndex, int surfaceIndex,
                      double pairPriorWeight, double overlapDChi2,
                      double overlapLogDet, double logPosterior,
                      const GaussianMomentState& backwardPredicted) {
    const int nodeId = appendNode(
        component, LineageNodeSource::SmoothedMixture,
        LineageNodeOperation::Smoothing, hitIndex, surfaceIndex);
    if (nodeId < 0) return nodeId;
    auto& node = m_nodes[static_cast<std::size_t>(nodeId)];
    node.priorWeight = pairPriorWeight;
    node.dchi2 = overlapDChi2;
    node.logDetInnovation = overlapLogDet;
    node.logUnnormalizedPosterior = logPosterior;
    if (backwardPredicted.valid &&
        backwardPredicted.mean.GetNrows() >= 5 &&
        backwardPredicted.covariance.GetNrows() >= 5 &&
        backwardPredicted.covariance.GetNcols() >= 5) {
      node.predictedKappa = backwardPredicted.mean(2, 0);
      node.predictedKappaVariance = backwardPredicted.covariance(2, 2);
    }
    addEdge(
        forwardNodeId, nodeId, LineageEdgeOperation::Smoothing, false);
    addEdge(
        backwardNodeId, nodeId, LineageEdgeOperation::Smoothing, false);
    return nodeId;
  }

  void setNormalizedPosterior(int nodeId, double weight) {
    if (!validNode(nodeId)) return;
    auto& node = m_nodes[static_cast<std::size_t>(nodeId)];
    node.normalizedPosterior = weight;
    node.weight = weight;
  }

  void setWeight(int nodeId, double weight) {
    if (validNode(nodeId))
      m_nodes[static_cast<std::size_t>(nodeId)].weight = weight;
  }

  void mark(int nodeId, LineageNodeFate fate) {
    if (!validNode(nodeId)) return;
    m_nodes[static_cast<std::size_t>(nodeId)].fate =
        static_cast<std::int32_t>(fate);
  }

  void markFinal(const std::vector<GsfComponent*>& components,
                 const GsfComponent* best) {
    for (const auto* component : components) {
      if (!component || !validNode(component->lineageNodeId)) continue;
      auto& node = m_nodes[static_cast<std::size_t>(component->lineageNodeId)];
      node.fate = static_cast<std::int32_t>(
          LineageNodeFate::FinalSurvivor);
      node.finalMixture = 1;
      node.bestBranch = component == best ? 1 : 0;
      node.weight = component->weight;
    }
  }

  void markAbandoned(const std::vector<GsfComponent*>& components) {
    for (const auto* component : components)
      if (component) mark(component->lineageNodeId,
                          LineageNodeFate::TrackAbandoned);
  }

  void markInwardInternalMessage(
      const std::vector<GsfComponent*>& components) {
    for (const auto* component : components) {
      if (!component || !validNode(component->lineageNodeId)) continue;
      auto& node =
          m_nodes[static_cast<std::size_t>(component->lineageNodeId)];
      if (node.fate == static_cast<std::int32_t>(LineageNodeFate::Active))
        node.fate = static_cast<std::int32_t>(
            LineageNodeFate::InwardInternalMessage);
    }
  }

private:
  bool validNode(int nodeId) const {
    return m_enabled && nodeId >= 0 &&
        nodeId < static_cast<int>(m_nodes.size());
  }

  int appendNode(GsfComponent& component, LineageNodeSource source,
                 LineageNodeOperation operation, int hitIndex,
                 int surfaceIndex) {
    if (!m_enabled) {
      component.lineageNodeId = -1;
      return -1;
    }
    LineageNodeRecord node;
    node.nodeId = static_cast<std::int32_t>(m_nodes.size());
    node.source = static_cast<std::int32_t>(source);
    node.operation = static_cast<std::int32_t>(operation);
    node.hitIndex = hitIndex;
    node.surfaceIndex = surfaceIndex;
    node.componentId = component.debugId;
    node.generation = component.generation;
    node.noRadiationLineage = component.noRadiationLineage ? 1 : 0;
    node.weight = component.weight;
    node.dominantLineageFraction = component.dominantLineageFraction;
    const auto helix = component.helixAtLastSite(m_bz);
    const auto covariance = component.covAtLastSite(m_bz);
    node.filteredKappa = helix.GetKappa();
    if (covariance.GetNrows() >= 5 && covariance.GetNcols() >= 5)
      node.filteredKappaVariance = covariance(2, 2);
    node.valid = std::isfinite(node.filteredKappa) &&
        std::isfinite(node.filteredKappaVariance) ? 1 : 0;
    m_nodes.push_back(node);
    component.lineageNodeId = node.nodeId;
    return node.nodeId;
  }

  void addEdge(int fromNodeId, int toNodeId, LineageEdgeOperation operation,
               bool advanceSource = true) {
    if (!validNode(fromNodeId) || !validNode(toNodeId)) return;
    m_edges.push_back({fromNodeId, toNodeId,
                       static_cast<std::int32_t>(operation)});
    if (advanceSource &&
        m_nodes[static_cast<std::size_t>(fromNodeId)].fate ==
            static_cast<std::int32_t>(LineageNodeFate::Active)) {
      mark(fromNodeId, LineageNodeFate::Advanced);
    }
  }

  bool m_enabled = false;
  double m_bz = 0.0;
  std::vector<LineageNodeRecord> m_nodes;
  std::vector<LineageEdgeRecord> m_edges;
};

static GsfSmoothedSurfaceResult buildSmoothedSurfaceMixture(
    const std::vector<GaussianComponentSnapshot>& forwardUpdated,
    const std::vector<GaussianComponentSnapshot>& backwardPredicted,
    int hitIndex, int surfaceIndex, const TVector3& pivot, double bz,
    int reductionTarget, double weightCutoff, bool protectIdentityLineage,
    const std::string& reductionMergeCost,
    LineageGraphRecorder& lineageGraph) {
  GsfSmoothedSurfaceResult result;
  result.pairCandidates = static_cast<int>(
      forwardUpdated.size() * backwardPredicted.size());
  result.components.reserve(static_cast<std::size_t>(result.pairCandidates));
  std::vector<double> pairLogWeights;
  pairLogWeights.reserve(static_cast<std::size_t>(result.pairCandidates));
  std::vector<int> pairBackwardComponentIds;
  pairBackwardComponentIds.reserve(
      static_cast<std::size_t>(result.pairCandidates));

  int smoothedComponentId = 0;
  for (const auto& forward : forwardUpdated) {
    for (const auto& backward : backwardPredicted) {
      GaussianMomentState smoothed;
      double logOverlap = 0.0;
      double overlapDChi2 = 0.0;
      double overlapLogDet = 0.0;
      if (!formSmoothedGaussian(
              forward.state, backward.state, smoothed, &logOverlap,
              &overlapDChi2, &overlapLogDet)) {
        ++result.pairFailures;
        continue;
      }
      const double pairPriorWeight = forward.weight * backward.weight;
      // Preserve the established arithmetic ordering exactly: multiplying the
      // priors before taking the logarithm can perturb product KL tie-breaking
      // at roundoff scale.
      const double logWeight = std::log(forward.weight) +
          std::log(backward.weight) + logOverlap;
      if (!std::isfinite(logWeight)) {
        ++result.pairFailures;
        continue;
      }

      auto* component = new GsfComponent();
      if (!initializeSmoothedComponent(
              *component, smoothed, pivot, bz, 1.0,
              smoothedComponentId++,
              forward.noRadiationLineage &&
                  backward.noRadiationLineage)) {
        delete component;
        ++result.pairFailures;
        continue;
      }
      component->debugHistory =
          "smoothed(f=" + std::to_string(forward.componentId) +
          ",b=" + std::to_string(backward.componentId) + ")";
      component->lineageNodeId = lineageGraph.smoothedMixture(
          *component, forward.lineageNodeId, backward.lineageNodeId,
          hitIndex, surfaceIndex, pairPriorWeight, overlapDChi2,
          overlapLogDet, logWeight, backward.state);
      result.components.push_back(component);
      pairLogWeights.push_back(logWeight);
      pairBackwardComponentIds.push_back(backward.componentId);
    }
  }

  if (result.components.empty()) return result;

  const double maximumLogWeight = *std::max_element(
      pairLogWeights.begin(), pairLogWeights.end());
  for (std::size_t pairIndex = 0;
       pairIndex < result.components.size(); ++pairIndex) {
    result.components[pairIndex]->weight = std::exp(
        pairLogWeights[pairIndex] - maximumLogWeight);
  }
  GsfMixture::normalizeWeights(result.components);
  for (std::size_t pairIndex = 0;
       pairIndex < result.components.size(); ++pairIndex) {
    const auto* component = result.components[pairIndex];
    result.backwardMarginalWeights[
        pairBackwardComponentIds[pairIndex]] += component->weight;
    lineageGraph.setNormalizedPosterior(
        component->lineageNodeId, component->weight);
  }

  auto cutoffObserver = [&](const GsfComponent& component) {
    lineageGraph.mark(
        component.lineageNodeId, LineageNodeFate::WeightCutoff);
  };
  GsfMixture::removeLowWeight(
      result.components, weightCutoff, protectIdentityLineage,
      cutoffObserver);
  if (static_cast<int>(result.components.size()) > reductionTarget) {
    auto reductionObserver = [&](GsfComponent& merged,
                                 int keepSourceNodeId,
                                 int dropSourceNodeId,
                                 double mergeCost) {
      merged.lineageNodeId = lineageGraph.merge(
          merged, keepSourceNodeId, dropSourceNodeId,
          LineageNodeSource::SmoothedMixture, hitIndex,
          surfaceIndex, mergeCost);
    };
    GsfMixture::reduce(
        result.components, reductionTarget, bz, protectIdentityLineage, {},
        reductionMergeCost, reductionObserver);
  }
  GsfMixture::normalizeWeights(result.components);
  for (const auto* component : result.components)
    lineageGraph.setWeight(component->lineageNodeId, component->weight);
  return result;
}

struct FinalMixtureComponentRecord {
  std::int32_t componentIndex = -1;
  std::int32_t componentID = -1;
  std::int32_t source = 0;
  std::int32_t valid = 0;
  double weight = 0.0;
  double kappa = std::numeric_limits<double>::quiet_NaN();
  double kappaVariance = std::numeric_limits<double>::quiet_NaN();
};

static std::vector<FinalMixtureComponentRecord>
captureFinalMixtureComponentsAtIP(
    const std::vector<GsfComponent*>& components, double bz,
    FinalMixtureComponentSource source,
    const std::function<bool(GsfComponent&, THelicalTrack&,
                             TMatrixD&)>& extrapolate) {
  double sumWeight = 0.0;
  for (const auto* component : components) {
    if (component && component->weight > 0.0 &&
        std::isfinite(component->weight)) {
      sumWeight += component->weight;
    }
  }
  std::vector<FinalMixtureComponentRecord> records;
  if (!(sumWeight > 0.0) || !std::isfinite(sumWeight)) return records;
  records.reserve(components.size());
  for (std::size_t componentIndex = 0;
       componentIndex < components.size(); ++componentIndex) {
    auto* component = components[componentIndex];
    if (!component || !(component->weight > 0.0) ||
        !std::isfinite(component->weight)) {
      continue;
    }
    FinalMixtureComponentRecord record;
    record.componentIndex = static_cast<std::int32_t>(componentIndex);
    record.componentID = component->debugId;
    record.source = static_cast<std::int32_t>(source);
    record.weight = component->weight / sumWeight;

    THelicalTrack componentIp(TMatrixD(5, 1), TVector3(0, 0, 0), bz);
    TMatrixD componentIpCovariance(5, 5);
    if (extrapolate(*component, componentIp, componentIpCovariance)) {
      TMatrixD componentIpMean;
      helixToMean(componentIp, componentIpMean);
      TMatrixD componentIpPrecision;
      record.kappa = componentIp.GetKappa();
      record.kappaVariance = componentIpCovariance(2, 2);
      record.valid = finiteMatrix(componentIpMean) &&
                     record.kappaVariance > 0.0 &&
                     std::isfinite(record.kappaVariance) &&
                     invertPositiveDefinite(componentIpCovariance,
                                            componentIpPrecision)
                         ? 1
                         : 0;
    }
    records.push_back(record);
  }
  return records;
}

static bool solvePositiveDefinite(const TMatrixD& matrix,
                                  const TMatrixD& rightHandSide,
                                  TMatrixD& solution) {
  if (matrix.GetNrows() != matrix.GetNcols() ||
      rightHandSide.GetNcols() != 1 ||
      rightHandSide.GetNrows() != matrix.GetNrows() ||
      !finiteMatrix(matrix) || !finiteMatrix(rightHandSide)) {
    return false;
  }
  const int dimension = matrix.GetNrows();
  TMatrixDSym symmetric(dimension);
  for (int row = 0; row < dimension; ++row)
    for (int column = 0; column <= row; ++column)
      symmetric(row, column) =
          0.5 * (matrix(row, column) + matrix(column, row));
  TDecompChol decomposition(symmetric);
  if (!decomposition.Decompose()) return false;
  TVectorD vector(dimension);
  for (int index = 0; index < dimension; ++index)
    vector(index) = rightHandSide(index, 0);
  if (!decomposition.Solve(vector)) return false;
  solution.ResizeTo(dimension, 1);
  for (int index = 0; index < dimension; ++index)
    solution(index, 0) = vector(index);
  return finiteMatrix(solution);
}

static MixtureModeEvaluation evaluateMixtureModeDensity(
    const std::vector<PreparedModeComponent>& components,
    const TMatrixD& position) {
  MixtureModeEvaluation result;
  if (components.empty() || position.GetNrows() != 5 ||
      position.GetNcols() != 1 || !finiteMatrix(position)) {
    return result;
  }

  std::vector<double> logTerms;
  std::vector<TMatrixD> scores;
  logTerms.reserve(components.size());
  scores.reserve(components.size());
  double maximumLogTerm = -std::numeric_limits<double>::infinity();
  for (const auto& component : components) {
    const TMatrixD displacement = component.mean - position;
    const TMatrixD score = component.precision * displacement;
    double quadratic = 0.0;
    for (int index = 0; index < 5; ++index)
      quadratic += displacement(index, 0) * score(index, 0);
    if (!std::isfinite(quadratic)) return result;
    const double logTerm = component.logNormalizer - 0.5 * quadratic;
    if (!std::isfinite(logTerm)) return result;
    logTerms.push_back(logTerm);
    scores.push_back(score);
    maximumLogTerm = std::max(maximumLogTerm, logTerm);
  }

  double exponentialSum = 0.0;
  result.responsibilities.resize(components.size(), 0.0);
  for (std::size_t index = 0; index < components.size(); ++index) {
    const double value = std::exp(logTerms[index] - maximumLogTerm);
    result.responsibilities[index] = value;
    exponentialSum += value;
  }
  if (!(exponentialSum > 0.0) || !std::isfinite(exponentialSum))
    return result;
  result.logDensity = maximumLogTerm + std::log(exponentialSum);
  result.gradient.Zero();
  result.hessian.Zero();
  for (std::size_t componentIndex = 0;
       componentIndex < components.size(); ++componentIndex) {
    const double responsibility =
        result.responsibilities[componentIndex] / exponentialSum;
    result.responsibilities[componentIndex] = responsibility;
    const auto& score = scores[componentIndex];
    const auto& precision = components[componentIndex].precision;
    for (int row = 0; row < 5; ++row) {
      result.gradient(row, 0) += responsibility * score(row, 0);
      for (int column = 0; column < 5; ++column) {
        result.hessian(row, column) += responsibility *
            (score(row, 0) * score(column, 0) -
             precision(row, column));
      }
    }
  }
  for (int row = 0; row < 5; ++row)
    for (int column = 0; column < 5; ++column)
      result.hessian(row, column) -=
          result.gradient(row, 0) * result.gradient(column, 0);
  result.valid = finiteMatrix(result.gradient) &&
                 finiteMatrix(result.hessian) &&
                 std::isfinite(result.logDensity);
  return result;
}

static double scaledVectorNorm(const TMatrixD& vector,
                               const TMatrixD& scales,
                               bool gradientUnits) {
  double maximum = 0.0;
  for (int index = 0; index < 5; ++index) {
    const double value = gradientUnits
        ? std::abs(vector(index, 0) * scales(index, 0))
        : std::abs(vector(index, 0) / scales(index, 0));
    maximum = std::max(maximum, value);
  }
  return maximum;
}

static FullMixtureModeStatus findFullMixtureModeAtIP(
    const std::vector<GsfComponent*>& components, double bz,
    const std::function<bool(GsfComponent&, THelicalTrack&,
                             TMatrixD&)>& extrapolate,
    THelicalTrack& outputHelix, TMatrixD& outputCovariance,
    FullMixtureModeDiagnostics& diagnostics) {
  diagnostics = {};
  diagnostics.inputComponents = static_cast<int>(components.size());
  std::vector<PreparedModeComponent> prepared;
  prepared.reserve(components.size());
  double sumWeight = 0.0;
  double phiReference = 0.0;
  bool havePhiReference = false;
  int positiveWeightComponents = 0;

  for (auto* component : components) {
    if (!component || !(component->weight > 0.0) ||
        !std::isfinite(component->weight)) {
      continue;
    }
    ++positiveWeightComponents;
    THelicalTrack helix(TMatrixD(5, 1), TVector3(0, 0, 0), bz);
    TMatrixD covariance(5, 5);
    if (!extrapolate(*component, helix, covariance))
      return FullMixtureModeStatus::IncompleteComponentSet;
    PreparedModeComponent entry;
    helixToMean(helix, entry.mean);
    if (!havePhiReference) {
      phiReference = entry.mean(1, 0);
      havePhiReference = true;
    } else {
      wrapPhiNear(entry.mean(1, 0), phiReference);
    }
    double logDeterminant = 0.0;
    if (!finiteMatrix(entry.mean) ||
        !invertPositiveDefinite(covariance, entry.precision,
                                &logDeterminant)) {
      return FullMixtureModeStatus::IncompleteComponentSet;
    }
    entry.weight = component->weight;
    entry.logNormalizer = -0.5 *
        (5.0 * std::log(2.0 * M_PI) + logDeterminant);
    prepared.push_back(std::move(entry));
    sumWeight += component->weight;
  }
  diagnostics.usableComponents = static_cast<int>(prepared.size());
  if (positiveWeightComponents == 0 || prepared.empty() ||
      !(sumWeight > 0.0) || !std::isfinite(sumWeight)) {
    return FullMixtureModeStatus::IncompleteComponentSet;
  }
  for (auto& component : prepared) {
    component.weight /= sumWeight;
    component.logNormalizer += std::log(component.weight);
  }

  TMatrixD weightedMean(5, 1);
  weightedMean.Zero();
  for (const auto& component : prepared) {
    TMatrixD contribution = component.mean;
    contribution *= component.weight;
    weightedMean += contribution;
  }
  TMatrixD scales(5, 1);
  scales.Zero();
  for (const auto& component : prepared) {
    TMatrixD covariance;
    if (!invertPositiveDefinite(component.precision, covariance))
      return FullMixtureModeStatus::IncompleteComponentSet;
    const TMatrixD displacement = component.mean - weightedMean;
    for (int index = 0; index < 5; ++index) {
      scales(index, 0) += component.weight *
          (covariance(index, index) +
           displacement(index, 0) * displacement(index, 0));
    }
  }
  for (int index = 0; index < 5; ++index) {
    if (!(scales(index, 0) > 0.0) || !std::isfinite(scales(index, 0)))
      return FullMixtureModeStatus::IncompleteComponentSet;
    scales(index, 0) = std::sqrt(scales(index, 0));
  }

  std::vector<TMatrixD> starts;
  starts.reserve(1 + prepared.size() +
                 prepared.size() * (prepared.size() - 1) / 2);
  starts.push_back(weightedMean);
  for (const auto& component : prepared) starts.push_back(component.mean);
  for (std::size_t first = 0; first < prepared.size(); ++first) {
    for (std::size_t second = first + 1; second < prepared.size(); ++second) {
      const double pairWeight =
          prepared[first].weight + prepared[second].weight;
      TMatrixD midpoint = prepared[first].mean;
      midpoint *= prepared[first].weight / pairWeight;
      TMatrixD secondTerm = prepared[second].mean;
      secondTerm *= prepared[second].weight / pairWeight;
      midpoint += secondTerm;
      starts.push_back(std::move(midpoint));
    }
  }
  diagnostics.starts = static_cast<int>(starts.size());

  bool foundMaximum = false;
  TMatrixD bestPosition(5, 1);
  TMatrixD bestCovariance(5, 5);
  double bestLogDensity = -std::numeric_limits<double>::infinity();
  double bestScaledGradient = std::numeric_limits<double>::infinity();

  for (const auto& start : starts) {
    TMatrixD position = start;
    auto evaluation = evaluateMixtureModeDensity(prepared, position);
    if (!evaluation.valid) continue;
    int iterations = 0;

    // Generalized Gaussian mean-shift fixed point.
    for (; iterations < 200; ++iterations) {
      TMatrixD precisionSum(5, 5);
      TMatrixD weightedPrecisionMean(5, 1);
      precisionSum.Zero();
      weightedPrecisionMean.Zero();
      for (std::size_t componentIndex = 0;
           componentIndex < prepared.size(); ++componentIndex) {
        const double responsibility =
            evaluation.responsibilities[componentIndex];
        TMatrixD precisionTerm = prepared[componentIndex].precision;
        precisionTerm *= responsibility;
        precisionSum += precisionTerm;
        TMatrixD meanTerm =
            prepared[componentIndex].precision *
            prepared[componentIndex].mean;
        meanTerm *= responsibility;
        weightedPrecisionMean += meanTerm;
      }
      TMatrixD fixedPoint;
      if (!solvePositiveDefinite(precisionSum, weightedPrecisionMean,
                                 fixedPoint)) {
        break;
      }
      TMatrixD direction = fixedPoint - position;
      if (scaledVectorNorm(direction, scales, false) < 1.0e-10) {
        position = fixedPoint;
        evaluation = evaluateMixtureModeDensity(prepared, position);
        break;
      }
      bool accepted = false;
      double stepScale = 1.0;
      for (int lineSearch = 0; lineSearch < 24; ++lineSearch) {
        TMatrixD proposal = direction;
        proposal *= stepScale;
        proposal += position;
        auto proposalEvaluation =
            evaluateMixtureModeDensity(prepared, proposal);
        if (proposalEvaluation.valid &&
            proposalEvaluation.logDensity + 1.0e-13 >=
                evaluation.logDensity) {
          position = std::move(proposal);
          evaluation = std::move(proposalEvaluation);
          accepted = true;
          break;
        }
        stepScale *= 0.5;
      }
      if (!accepted) break;
    }

    // Local Newton refinement of log p(x).
    for (int newton = 0; newton < 40; ++newton, ++iterations) {
      if (!evaluation.valid) break;
      const double gradientNorm =
          scaledVectorNorm(evaluation.gradient, scales, true);
      if (gradientNorm < 1.0e-8) break;
      TMatrixD negativeHessian = evaluation.hessian;
      negativeHessian *= -1.0;
      TMatrixD step;
      if (!solvePositiveDefinite(negativeHessian, evaluation.gradient,
                                 step)) {
        break;
      }
      bool accepted = false;
      double stepScale = 1.0;
      for (int lineSearch = 0; lineSearch < 24; ++lineSearch) {
        TMatrixD proposal = step;
        proposal *= stepScale;
        proposal += position;
        auto proposalEvaluation =
            evaluateMixtureModeDensity(prepared, proposal);
        if (proposalEvaluation.valid &&
            proposalEvaluation.logDensity + 1.0e-13 >=
                evaluation.logDensity) {
          position = std::move(proposal);
          evaluation = std::move(proposalEvaluation);
          accepted = true;
          break;
        }
        stepScale *= 0.5;
      }
      if (!accepted) break;
    }
    diagnostics.iterations = std::max(diagnostics.iterations, iterations);
    if (!evaluation.valid) continue;
    const double gradientNorm =
        scaledVectorNorm(evaluation.gradient, scales, true);
    TMatrixD negativeHessian = evaluation.hessian;
    negativeHessian *= -1.0;
    TMatrixD localCovariance;
    if (gradientNorm > 1.0e-5 ||
        !invertPositiveDefinite(negativeHessian, localCovariance)) {
      continue;
    }
    ++diagnostics.maxima;
    if (!foundMaximum || evaluation.logDensity > bestLogDensity) {
      foundMaximum = true;
      bestPosition = position;
      bestCovariance = localCovariance;
      bestLogDensity = evaluation.logDensity;
      bestScaledGradient = gradientNorm;
    }
  }

  if (!foundMaximum) return FullMixtureModeStatus::OptimizationFailed;
  TMatrixD localPrecision;
  if (!invertPositiveDefinite(bestCovariance, localPrecision)) {
    return FullMixtureModeStatus::InvalidLocalCovariance;
  }
  diagnostics.logDensity = bestLogDensity;
  diagnostics.scaledGradient = bestScaledGradient;
  outputCovariance.ResizeTo(5, 5);
  outputCovariance = bestCovariance;
  outputHelix = THelicalTrack(bestPosition, TVector3(0, 0, 0), bz);
  return FullMixtureModeStatus::Success;
}

/// Fill an edm4hep TrackState from a THelicalTrack + 5x5 cov
static void fillTrackState(edm4hep::TrackState& ts,
                            const THelicalTrack& h,
                            const TMatrixD& cov, double bz) {

  double d0 = -h.GetDrho();
  double phi = h.GetPhi0() + M_PI / 2.;
  double omega = (bz != 0) ? (h.GetKappa() * bz * 2.99792458e-4) : 0.0;
  double z0 = h.GetDz();
  double tanl = h.GetTanLambda();

  while (phi > M_PI) phi -= 2 * M_PI;
  while (phi < -M_PI) phi += 2 * M_PI;

  double al = omega / h.GetKappa();

  ts.D0 = (float)d0;
  ts.phi = (float)phi;
  ts.omega = (float)omega;
  ts.Z0 = (float)z0;
  ts.tanLambda = (float)tanl;

  decltype(ts.covMatrix) cl{};
  cl[ 0] = cov(0, 0);   cl[ 1] = -cov(1, 0);  cl[ 2] = cov(1, 1);
  cl[ 3] = -cov(2, 0) * al;  cl[ 4] = cov(2, 1) * al;
  cl[ 5] = cov(2, 2) * al * al;
  cl[ 6] = -cov(3, 0);       cl[ 7] = cov(3, 1);
  cl[ 8] = cov(3, 2) * al;   cl[ 9] = cov(3, 3);
  cl[10] = -cov(4, 0);       cl[11] = cov(4, 1);
  cl[12] = cov(4, 2) * al;   cl[13] = cov(4, 3);
  cl[14] = cov(4, 4);
  ts.covMatrix = cl;

  float pv[3] = {(float)h.GetPivot().X(),
                 (float)h.GetPivot().Y(),
                 (float)h.GetPivot().Z()};
  ts.referencePoint = edm4hep::Vector3f(pv);
}

/// Compute t/X0 for this layer (inner + outer material)
static double thicknessInX0(const DDVMeasLayer* layer) {
  double tX0 = 0;
  if (!layer->surface()) return 0;

  auto& matIn = layer->GetMaterial(false);
  auto& matOut = layer->GetMaterial(true);

  if (matIn.GetRadLength() > 0)
    tX0 += layer->surface()->innerThickness() / matIn.GetRadLength();
  if (matOut.GetRadLength() > 0)
    tX0 += layer->surface()->outerThickness() / matOut.GetRadLength();

  return tX0;
}

struct ComponentMaterialPath {
  double normalTX0 = 0.0;
  double pathTX0 = 0.0;
  double absCosIncidence = 0.0;
  int layerCount = 0;
  std::string layerAudit;
  bool valid = false;
};

struct RuntimeMaterialSummary {
  int candidateCount = 0;
  int validCount = 0;
  int aboveThresholdCount = 0;
  double validWeight = 0.0;
  double weightedTX0Sum = 0.0;
  double minTX0 = std::numeric_limits<double>::infinity();
  double maxTX0 = -std::numeric_limits<double>::infinity();
  int leadingComponentID = -1;
  double leadingComponentWeight =
      -std::numeric_limits<double>::infinity();
  double leadingTX0 = std::numeric_limits<double>::quiet_NaN();

  void add(const ComponentMaterialPath& path, double parentWeight,
           int parentComponentID, double splitThreshold) {
    ++candidateCount;
    if (parentWeight > leadingComponentWeight) {
      leadingComponentID = parentComponentID;
      leadingComponentWeight = parentWeight;
      leadingTX0 = path.valid
          ? path.pathTX0 : std::numeric_limits<double>::quiet_NaN();
    }
    if (!path.valid) return;
    ++validCount;
    if (path.pathTX0 > splitThreshold) ++aboveThresholdCount;
    validWeight += parentWeight;
    weightedTX0Sum += parentWeight * path.pathTX0;
    minTX0 = std::min(minTX0, path.pathTX0);
    maxTX0 = std::max(maxTX0, path.pathTX0);
  }

  double weightedTX0() const {
    return validWeight > 0.0
        ? weightedTX0Sum / validWeight
        : std::numeric_limits<double>::quiet_NaN();
  }

  double minimumTX0() const {
    return validCount > 0
        ? minTX0 : std::numeric_limits<double>::quiet_NaN();
  }

  double maximumTX0() const {
    return validCount > 0
        ? maxTX0 : std::numeric_limits<double>::quiet_NaN();
  }

  double leadingWeight() const {
    return candidateCount > 0
        ? leadingComponentWeight
        : std::numeric_limits<double>::quiet_NaN();
  }
};

/// Material owned by the current measurement surface and traversed when
/// continuing away from that surface. The slab thickness is projected along
/// the component-local tangent, matching DDKalTest's surface-material
/// convention while retaining the separate inner/outer radiation lengths.
static ComponentMaterialPath componentMaterialPath(
    const DDVMeasLayer* layer, const GsfComponent& component, double bz) {
  ComponentMaterialPath result;
  if (!layer || !layer->surface()) return result;
  result.normalTX0 = thicknessInX0(layer);
  if (!(result.normalTX0 > 0.0)) return result;

  const THelicalTrack helix = component.helixAtMeasurementSite(bz);
  const double tanLambda = helix.GetTanLambda();
  dd4hep::rec::Vector3D direction(-std::sin(helix.GetPhi0()),
                                  std::cos(helix.GetPhi0()), tanLambda);
  direction = direction.unit();
  const TVector3& pivot = helix.GetPivot();
  const dd4hep::rec::Vector3D point(pivot.X() * dd4hep::mm,
                                    pivot.Y() * dd4hep::mm,
                                    pivot.Z() * dd4hep::mm);
  const dd4hep::rec::Vector3D normal = layer->surface()->normal(point).unit();
  result.absCosIncidence = std::abs(direction * normal);
  if (!(result.absCosIncidence > 1.0e-6) ||
      !std::isfinite(result.absCosIncidence)) {
    return result;
  }
  result.pathTX0 = result.normalTX0 / result.absCosIncidence;
  result.layerCount = 1;
  result.valid = std::isfinite(result.pathTX0) && result.pathTX0 > 0.0;
  return result;
}

/// Material owned by a destination measurement surface, evaluated at the
/// component's crossing of that surface. This is the inward-propagation
/// counterpart of componentMaterialPath(): the component is still on the
/// outer surface, so the target surface's incidence must be evaluated at its
/// crossing rather than at the component's current pivot.
static ComponentMaterialPath componentMaterialPathAtCrossing(
    const DDVMeasLayer* layer, const GsfComponent& component, double bz,
    int propagationDirection) {
  ComponentMaterialPath result;
  if (!layer || !layer->surface()) return result;
  result.normalTX0 = thicknessInX0(layer);
  if (!(result.normalTX0 > 0.0)) return result;

  const THelicalTrack helix = component.helixAtMeasurementSite(bz);
  const auto* surface = dynamic_cast<const TVSurface*>(layer);
  TVector3 crossing;
  double phi = 0.0;
  if (!surface ||
      !surface->CalcXingPointWith(
          helix, crossing, phi, propagationDirection)) {
    return result;
  }

  const dd4hep::rec::Vector3D ddCrossing(
      crossing.X() * dd4hep::mm, crossing.Y() * dd4hep::mm,
      crossing.Z() * dd4hep::mm);
  if (!layer->surface()->insideBounds(ddCrossing)) return result;

  const TMatrixD tangentMatrix = helix.CalcDxDphi(phi);
  TVector3 tangent(tangentMatrix(0, 0), tangentMatrix(1, 0),
                   tangentMatrix(2, 0));
  if (!(tangent.Mag2() > 0.0)) return result;
  tangent = tangent.Unit();
  TVector3 normal = surface->GetOutwardNormal(crossing);
  if (!(normal.Mag2() > 0.0)) return result;
  normal = normal.Unit();
  result.absCosIncidence = std::abs(tangent.Dot(normal));
  if (!(result.absCosIncidence > 1.0e-6) ||
      !std::isfinite(result.absCosIncidence)) {
    return result;
  }

  result.pathTX0 = result.normalTX0 / result.absCosIncidence;
  result.layerCount = 1;
  result.valid = std::isfinite(result.pathTX0) && result.pathTX0 > 0.0;
  return result;
}

static ComponentMaterialPath geometryTransitionMaterialPath(
    dd4hep::rec::MaterialManager* manager, const TVector3& from,
    const TVector3& to) {
  if (!manager || (to - from).Mag2() <= 0.0) return {};
  const dd4hep::rec::Vector3D p0(from.X() * dd4hep::mm,
                                  from.Y() * dd4hep::mm,
                                  from.Z() * dd4hep::mm);
  const dd4hep::rec::Vector3D p1(to.X() * dd4hep::mm,
                                  to.Y() * dd4hep::mm,
                                  to.Z() * dd4hep::mm);

  // MaterialManager can start exactly on a TGeo boundary.  In that case its
  // zero-step recovery can advance through the first volume without recording
  // it.  A valid segment list must cover the requested point-to-point length;
  // use that geometry-only invariant to detect the omission, then retry from a
  // point just inside the requested interval and restore the tiny leading cap.
  const auto scan = [&](const dd4hep::rec::Vector3D& begin,
                        const dd4hep::rec::Vector3D& end) {
    ComponentMaterialPath path;
    double coveredLength = 0.0;
    const auto& materials = manager->materialsBetween(begin, end);
    for (const auto& segment : materials) {
      if (!(segment.second > 0.0)) continue;
      coveredLength += segment.second;
      const double radLength = segment.first.radLength();
      if (!(radLength > 0.0)) continue;
      const double tx0 = segment.second / radLength;
      path.pathTX0 += tx0;
      ++path.layerCount;
      if (!path.layerAudit.empty()) path.layerAudit += '|';
      std::ostringstream audit;
      audit << segment.first.name() << ':' << segment.second / dd4hep::mm
            << ':' << tx0;
      path.layerAudit += audit.str();
    }
    path.normalTX0 = path.pathTX0;
    path.absCosIncidence = 1.0;
    path.valid = path.layerCount > 0 && std::isfinite(path.pathTX0) &&
        path.pathTX0 > 0.0;
    return std::make_pair(path, coveredLength);
  };

  const double requestedLength = (to - from).Mag() * dd4hep::mm;
  const double coverageTolerance = std::max(
      1.0e-3 * dd4hep::mm, 1.0e-6 * requestedLength);
  auto original = scan(p0, p1);
  if (original.second + coverageTolerance >= requestedLength)
    return original.first;

  const double nudgeLength = std::min(
      1.0e-3 * dd4hep::mm, 0.01 * requestedLength);
  const TVector3 direction = (to - from).Unit();
  const TVector3 nudgedFrom = from +
      (nudgeLength / dd4hep::mm) * direction;
  const dd4hep::rec::Vector3D nudgedP0(
      nudgedFrom.X() * dd4hep::mm, nudgedFrom.Y() * dd4hep::mm,
      nudgedFrom.Z() * dd4hep::mm);
  auto repaired = scan(nudgedP0, p1);
  const double repairedRequestedLength = requestedLength - nudgeLength;
  if (repaired.second + coverageTolerance < repairedRequestedLength) {
    original.first.valid = false;
    return original.first;
  }

  const TVector3 capMidpoint = from +
      (0.5 * nudgeLength / dd4hep::mm) * direction;
  const dd4hep::rec::Vector3D capPosition(
      capMidpoint.X() * dd4hep::mm, capMidpoint.Y() * dd4hep::mm,
      capMidpoint.Z() * dd4hep::mm);
  const auto& capMaterial = manager->materialAt(capPosition);
  const double capRadLength = capMaterial.radLength();
  if (!(capRadLength > 0.0)) {
    repaired.first.valid = false;
    return repaired.first;
  }
  const double capTX0 = nudgeLength / capRadLength;
  repaired.first.pathTX0 += capTX0;
  repaired.first.normalTX0 = repaired.first.pathTX0;
  ++repaired.first.layerCount;
  std::ostringstream capAudit;
  capAudit << capMaterial.name() << ':' << nudgeLength / dd4hep::mm
           << ':' << capTX0 << ":coverage-cap";
  repaired.first.layerAudit = capAudit.str() +
      (repaired.first.layerAudit.empty()
           ? std::string{} : '|' + repaired.first.layerAudit);
  repaired.first.valid = repaired.first.layerCount > 0 &&
      std::isfinite(repaired.first.pathTX0) &&
      repaired.first.pathTX0 > 0.0;
  return repaired.first;
}

static ComponentMaterialPath componentGeometryTransitionMaterialPath(
    dd4hep::rec::MaterialManager* manager, const DDVMeasLayer* toLayer,
    const TVector3& matchedDestination, const GsfComponent& component,
    double bz, int propagationDirection = 1,
    const TVector3* canonicalReverseOuterEndpoint = nullptr) {
  if (!manager || !toLayer || !toLayer->surface()) return {};
  if (!std::isfinite(matchedDestination.X()) ||
      !std::isfinite(matchedDestination.Y()) ||
      !std::isfinite(matchedDestination.Z())) {
    return {};
  }

  const THelicalTrack helix = component.helixAtMeasurementSite(bz);
  const auto* destination = dynamic_cast<const TVSurface*>(toLayer);
  if (!destination || !destination->IsOnSurface(matchedDestination)) {
    return {};
  }

  // The destination hit has already been matched to this measurement layer.
  // Use its recorded global point as the material-segment endpoint.  Do not
  // reconstruct it with HitToXv(): a one-dimensional hit does not constrain
  // both local coordinates, so that conversion can invent a remote endpoint.
  // Re-solving a bounded surface intersection here can instead reject an
  // otherwise accepted transition when the component crossing is only
  // microns outside a finite sensor patch.
  const TVector3& start = helix.GetPivot();
  const TVector3 displacement = matchedDestination - start;
  if (!(displacement.Mag2() > 0.0)) return {};

  TVector3 outwardTangent(-std::sin(helix.GetPhi0()),
                          std::cos(helix.GetPhi0()),
                          helix.GetTanLambda());
  if (!(outwardTangent.Mag2() > 0.0)) return {};
  outwardTangent = outwardTangent.Unit();
  const double signedProgress = displacement.Dot(outwardTangent);
  if ((propagationDirection > 0 && !(signedProgress > 0.0)) ||
      (propagationDirection < 0 && !(signedProgress < 0.0))) {
    return {};
  }

  if (propagationDirection < 0) {
    const TVector3& outwardEndpoint = canonicalReverseOuterEndpoint
        ? *canonicalReverseOuterEndpoint : start;
    return geometryTransitionMaterialPath(
        manager, matchedDestination, outwardEndpoint);
  }
  return geometryTransitionMaterialPath(manager, start, matchedDestination);
}

// ============================================================================
// Algorithm
// ============================================================================

RecGsfTracking::RecGsfTracking(const std::string& name,
                                         ISvcLocator* svc)
  : Algorithm(name, svc) {}

RecGsfTracking::~RecGsfTracking() = default;

StatusCode RecGsfTracking::initialize() {
  m_nEvt = 0;
  m_truthBHLossOverrideCalls = 0;
  m_truthBHLossPassthroughTracks = 0;
  m_truthBHLossDynamicTracks = 0;
  m_truthBHLossInvalidTruthEvents = 0;
  m_truthBHLossInvalidEndpointTracks = 0;
  m_truthBHLossInvalidIntervalTracks = 0;
  m_truthBHLossMaxObservedEndpointDistance = 0.0;

  if (m_maxComponents.value() < 1) {
    error() << "MaxComponents must be at least 1" << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_reductionTargetComponents.value() < 0 ||
      m_reductionTargetComponents.value() > m_maxComponents.value()) {
    error() << "ReductionTargetComponents must be 0 or in [1, MaxComponents]" << endmsg;
    return StatusCode::FAILURE;
  }
  std::string reductionMergeCost = m_reductionMergeCost.value();
  std::transform(reductionMergeCost.begin(), reductionMergeCost.end(),
                 reductionMergeCost.begin(), ::tolower);
  if (reductionMergeCost != "symmetrickl" &&
      reductionMergeCost != "runnalls") {
    error() << "ReductionMergeCost must be SymmetricKL or Runnalls" << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_componentWeightCutoff.value() < 0.0 ||
      m_componentWeightCutoff.value() >= 1.0) {
    error() << "ComponentWeightCutoff must be in [0, 1)" << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_bhSplitThresh.value() < 0.0) {
    error() << "BHSplitThreshold must be non-negative" << endmsg;
    return StatusCode::FAILURE;
  }
  std::string outputMode = m_outputMode.value();
  std::transform(outputMode.begin(), outputMode.end(), outputMode.begin(), ::tolower);
  if (outputMode != "bestbranch" && outputMode != "weightedmean") {
    error() << "GSFOutputMode must be BestBranch or WeightedMean" << endmsg;
    return StatusCode::FAILURE;
  }
  std::string reverseSelectionMode = m_reverseSelectionMode.value();
  std::transform(reverseSelectionMode.begin(), reverseSelectionMode.end(),
                 reverseSelectionMode.begin(), ::tolower);
  if (reverseSelectionMode != "aggregateweight" &&
      reverseSelectionMode != "dominantlineage" &&
      reverseSelectionMode != "surfaceconsistency") {
    error() << "ReverseSelectionMode must be AggregateWeight, DominantLineage, "
               "or SurfaceConsistency"
            << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_surfaceConsistencyUninformativeFloor.value() <= 0.0 ||
      m_surfaceConsistencyUninformativeFloor.value() > 1.0) {
    error() << "SurfaceConsistencyUninformativeFloor must be in (0, 1]"
            << endmsg;
    return StatusCode::FAILURE;
  }
  std::string materialPathMode = m_materialPathMode.value();
  std::transform(materialPathMode.begin(), materialPathMode.end(),
                 materialPathMode.begin(), ::tolower);
  if (materialPathMode != "currentsurface" &&
      materialPathMode != "dd4hepbetweensurfaces") {
    error() << "MaterialPathMode must be CurrentSurface or "
               "DD4hepBetweenSurfaces" << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_truthBHLossInputTrackIndex.value() < 0) {
    error() << "TruthBHLossInputTrackIndex must be nonnegative" << endmsg;
    return StatusCode::FAILURE;
  }
  if (!(m_truthBHLossMaxEndpointDistance.value() > 0.0) ||
      !std::isfinite(m_truthBHLossMaxEndpointDistance.value())) {
    error() << "TruthBHLossMaxEndpointDistance must be finite and positive"
            << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_truthBHLossOverride.value()) {
    if (!m_isElectron.value()) {
      error() << "TruthBHLossOverride requires ElectronHypothesis=True"
              << endmsg;
      return StatusCode::FAILURE;
    }
    if (materialPathMode != "dd4hepbetweensurfaces") {
      error() << "TruthBHLossOverride requires "
                 "MaterialPathMode=DD4hepBetweenSurfaces so the embedded "
                 "truth intervals "
                 "and runtime BH intervals have the same ownership"
              << endmsg;
      return StatusCode::FAILURE;
    }
    info() << "Truth BH-loss oracle: using GsfG4MaterialSteps and "
              "GsfSimTrackerHitG4StepLinks from the current event; runtime "
              "input track "
           << m_truthBHLossInputTrackIndex.value()
           << " will be joined through MCRecoTrackerAssociation"
           << endmsg;
  }
  if (m_gaussianSumSmoothing.value() && m_reverseFiltering.value()) {
    error() << "GaussianSumSmoothing and ReverseFiltering are alternative "
               "backward-information workflows; enable only one"
            << endmsg;
    return StatusCode::FAILURE;
  }
  if (!std::isfinite(m_inwardSeedCovarianceScale.value())) {
    error() << "InwardSeedCovarianceScale must be finite; positive values "
               "scale the copied forward mixture and values <= 0 select a "
               "fresh backward initialization" << endmsg;
    return StatusCode::FAILURE;
  }
  std::string inwardWeightMode = m_inwardWeightMode.value();
  std::transform(inwardWeightMode.begin(), inwardWeightMode.end(),
                 inwardWeightMode.begin(), ::tolower);
  if (inwardWeightMode != "localmeasurement" &&
      inwardWeightMode != "smoothedmarginal") {
    error() << "InwardWeightMode must be LocalMeasurement or "
               "SmoothedMarginal"
            << endmsg;
    return StatusCode::FAILURE;
  }
  if (!std::isfinite(m_kappaSeedCov.value())) {
    error() << "KappaSeedCov must be finite; values <= 0 select the standard "
               "KF curvature variance"
            << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_gaussianSumSmoothing.value() && m_materialIPExtrap.value()) {
    error() << "GaussianSumSmoothing currently requires "
               "MaterialIPExtrapolation=False"
            << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_ecalComponentConstraint.value()) {
    if (!m_reverseFiltering.value()) {
      error() << "EcalComponentConstraint currently requires "
                 "ReverseFiltering=True"
              << endmsg;
      return StatusCode::FAILURE;
    }
    if (!(m_ecalConstraintRatioThreshold.value() > 1.0) ||
        !std::isfinite(m_ecalConstraintRatioThreshold.value())) {
      error() << "EcalConstraintRatioThreshold must be finite and greater "
                 "than 1"
              << endmsg;
      return StatusCode::FAILURE;
    }
    if (!(m_ecalConstraintLogPSigma.value() > 0.0) ||
        !std::isfinite(m_ecalConstraintLogPSigma.value())) {
      error() << "EcalConstraintLogPSigma must be finite and positive"
              << endmsg;
      return StatusCode::FAILURE;
    }
    if (!(m_ecalConstraintLikelihoodFloor.value() > 0.0) ||
        m_ecalConstraintLikelihoodFloor.value() > 1.0 ||
        !std::isfinite(m_ecalConstraintLikelihoodFloor.value())) {
      error() << "EcalConstraintLikelihoodFloor must be finite and in (0, 1]"
              << endmsg;
      return StatusCode::FAILURE;
    }
    if (!(m_ecalConstraintPhiWindow.value() > 0.0) ||
        m_ecalConstraintPhiWindow.value() > M_PI ||
        !std::isfinite(m_ecalConstraintPhiWindow.value())) {
      error() << "EcalConstraintPhiWindow must be finite and in (0, pi]"
              << endmsg;
      return StatusCode::FAILURE;
    }
    if (!(m_ecalConstraintThetaWindow.value() > 0.0) ||
        m_ecalConstraintThetaWindow.value() > M_PI ||
        !std::isfinite(m_ecalConstraintThetaWindow.value())) {
      error() << "EcalConstraintThetaWindow must be finite and in (0, pi]"
              << endmsg;
      return StatusCode::FAILURE;
    }
  }

  m_geosvc = service<IGeomSvc>("GeomSvc");
  m_materialManager = new dd4hep::rec::MaterialManager(
      m_geosvc->lcdd()->world().volume());
  m_field = m_geosvc->lcdd()
                ->field()
                .magneticField(dd4hep::Position(0, 0, 0))
                .z() / dd4hep::tesla;
  info() << "B=" << m_field << " T" << endmsg;

  auto trackSystemSvc = service<ITrackSystemSvc>("TrackSystemSvc");
  if (!trackSystemSvc) {
    error() << "Failed to find TrackSystemSvc for GSF local baseline initialisation" << endmsg;
    return StatusCode::FAILURE;
  }
  m_gsfMarlinTrkSystem = trackSystemSvc->getTrackSystem(this, "KalTest");
  m_gsfMarlinTrkSystem->setOption(MarlinTrk::IMarlinTrkSystem::CFG::useQMS, m_doMS.value());
  m_gsfMarlinTrkSystem->setOption(MarlinTrk::IMarlinTrkSystem::CFG::usedEdx, m_doDEDX.value());
  m_gsfMarlinTrkSystem->setOption(MarlinTrk::IMarlinTrkSystem::CFG::useSmoothing, true);
  m_gsfMarlinTrkSystem->init();

  // Build DDKalTest geometry cradle (standalone, independent of tracking pipeline)
  m_cradle = new TKalDetCradle();
  m_cradle->SetOwner(true);

  auto det = m_geosvc->lcdd();
  std::vector<dd4hep::DetElement> detectors = det->detectors("tracker");
  for (auto& pd : det->detectors("passive")) detectors.push_back(pd);
  for (auto& c : det->detectors("calorimeter")) {
    std::string n = c.name();
    std::transform(n.begin(), n.end(), n.begin(), ::tolower);
    if (n.find("ecal") != std::string::npos) detectors.push_back(c);
  }

  double minS = 1e99;
  for (auto& de : detectors) {
    auto* kd = new DDKalDetector(de);
    m_detectors.push_back(kd);
    m_cradle->Install(*kd);
    for (int i = 0; i < kd->GetEntriesFast(); i++) {
      auto* s = dynamic_cast<const TVSurface*>(kd->At(i));
      if (!s) continue;
      if (s->GetSortingPolicy() < minS &&
          dynamic_cast<DDCylinderMeasLayer*>(kd->At(i))) {
        minS = s->GetSortingPolicy();
        m_ipLayer = dynamic_cast<DDCylinderMeasLayer*>(kd->At(i));
      }
    }
  }
  m_cradle->Close();
  if (m_doMS) m_cradle->SwitchOnMS(); else m_cradle->SwitchOffMS();
  if (m_doDEDX) m_cradle->SwitchOnDEDX(); else m_cradle->SwitchOffDEDX();

  // ── build cellID → layer index (same approach as MarlinDDKalTest) ──
  m_cellIDToLayer.clear();
  for (auto* kd : m_detectors) {
    int nLayers = kd->GetEntriesFast();
    for (int i = 0; i < nLayers; i++) {
      auto* ml = dynamic_cast<const DDVMeasLayer*>(kd->At(i));
      if (!ml) continue;
      if (!ml->IsActive()) continue;
      for (auto cid : ml->getCellIDs())
        m_cellIDToLayer.insert({cid, ml});
    }
  }

  info() << "Cradle: " << m_cradle->GetEntriesFast() << " layers"
         << "  cellID-index: " << m_cellIDToLayer.size() << " entries"
         << " IP r=" << (m_ipLayer ? m_ipLayer->GetR() : -1) << endmsg;

  try {
    const auto bhModel = BetheHeitlerSplitter::modelFromName(m_bhModel.value());
    info() << "Bethe-Heitler model: "
           << BetheHeitlerSplitter::modelName(bhModel) << endmsg;
  } catch (const std::exception& e) {
    error() << e.what() << endmsg;
    return StatusCode::FAILURE;
  }

  info() << "GSF configuration: maxComponents=" << m_maxComponents.value()
         << " reductionTarget=" << m_reductionTargetComponents.value()
         << " reductionMode=KL"
         << " protectIdentityLineage=" << m_protectIdentityLineage.value()
         << " forwardBHSplitting=" << m_forwardBHSplitting.value()
         << " inwardBHSplitting=" << m_inwardBHSplitting.value()
         << " outputMode=" << m_outputMode.value()
         << " inwardWeightMode=" << m_inwardWeightMode.value()
         << " verbose=" << m_verboseDump.value() << "/"
         << m_verboseSplitDump.value() << "/"
         << m_componentDebugDump.value()
         << " ecalConstraint=" << m_ecalComponentConstraint.value()
         << " truthBHLossOverride=" << m_truthBHLossOverride.value()
         << endmsg;
  if (m_gaussianSumSmoothing.value() || m_reverseFiltering.value()) {
    info() << "Three-view GSF publication: BestBranch -> GSFTracksBestBranch, "
              "WeightedMean -> GSFTracksWeightedMean, FullMixtureMode -> "
              "GSFTracksFullMixtureMode; GSFOutputMode does not select "
              "between these collections"
           << endmsg;
  }

  return StatusCode::SUCCESS;
}

// ---------------------------------------------------------------------------
StatusCode RecGsfTracking::execute() {
  m_nEvt++;
  const int eventIndex = m_nEvt - 1;
  const bool publishPairedEndpoints =
      m_gaussianSumSmoothing.value() || m_reverseFiltering.value();
  auto* out = publishPairedEndpoints
      ? m_bestBranchOutputTracks.createAndPut()
      : m_outputTracks.createAndPut();
  edm4hep::TrackCollection* weightedMeanOut = nullptr;
  edm4hep::TrackCollection* fullMixtureModeOut = nullptr;
  podio::UserDataCollection<std::int32_t>* fullMixtureModeStatusOut = nullptr;
  if (publishPairedEndpoints)
    weightedMeanOut = m_weightedMeanOutputTracks.createAndPut();
  if (publishPairedEndpoints) {
    fullMixtureModeOut = m_fullMixtureModeOutputTracks.createAndPut();
    fullMixtureModeStatusOut = m_fullMixtureModeStatus.createAndPut();
  }
  auto* finalMixtureComponentInputTrackIndex =
      m_finalMixtureComponentInputTrackIndex.createAndPut();
  auto* finalMixtureComponentOutputTrackIndex =
      m_finalMixtureComponentOutputTrackIndex.createAndPut();
  auto* finalMixtureComponentIndex =
      m_finalMixtureComponentIndex.createAndPut();
  auto* finalMixtureComponentID = m_finalMixtureComponentID.createAndPut();
  auto* finalMixtureComponentSource =
      m_finalMixtureComponentSource.createAndPut();
  auto* finalMixtureComponentValid =
      m_finalMixtureComponentValid.createAndPut();
  auto* finalMixtureComponentWeight =
      m_finalMixtureComponentWeight.createAndPut();
  auto* finalMixtureComponentKappa =
      m_finalMixtureComponentKappa.createAndPut();
  auto* finalMixtureComponentKappaVariance =
      m_finalMixtureComponentKappaVariance.createAndPut();
  auto* lineageNodeInputTrackIndex =
      m_lineageNodeInputTrackIndex.createAndPut();
  auto* lineageNodeOutputTrackIndex =
      m_lineageNodeOutputTrackIndex.createAndPut();
  auto* lineageNodeId = m_lineageNodeId.createAndPut();
  auto* lineageNodeSource = m_lineageNodeSource.createAndPut();
  auto* lineageNodeOperation = m_lineageNodeOperation.createAndPut();
  auto* lineageNodeHitIndex = m_lineageNodeHitIndex.createAndPut();
  auto* lineageNodeSurfaceIndex = m_lineageNodeSurfaceIndex.createAndPut();
  auto* lineageNodeComponentId = m_lineageNodeComponentId.createAndPut();
  auto* lineageNodeGeneration = m_lineageNodeGeneration.createAndPut();
  auto* lineageNodeBhComponentIndex =
      m_lineageNodeBhComponentIndex.createAndPut();
  auto* lineageNodeMeasurementStatus =
      m_lineageNodeMeasurementStatus.createAndPut();
  auto* lineageNodeFate = m_lineageNodeFate.createAndPut();
  auto* lineageNodeNoRadiation = m_lineageNodeNoRadiation.createAndPut();
  auto* lineageNodeBestBranch = m_lineageNodeBestBranch.createAndPut();
  auto* lineageNodeFinalMixture = m_lineageNodeFinalMixture.createAndPut();
  auto* lineageNodeValid = m_lineageNodeValid.createAndPut();
  auto* lineageNodeWeight = m_lineageNodeWeight.createAndPut();
  auto* lineageNodePriorWeight = m_lineageNodePriorWeight.createAndPut();
  auto* lineageNodeBhWeight = m_lineageNodeBhWeight.createAndPut();
  auto* lineageNodeBhMean = m_lineageNodeBhMean.createAndPut();
  auto* lineageNodeBhVariance = m_lineageNodeBhVariance.createAndPut();
  auto* lineageNodeMaterialTX0 = m_lineageNodeMaterialTX0.createAndPut();
  auto* lineageNodeDChi2 = m_lineageNodeDChi2.createAndPut();
  auto* lineageNodeLogDetInnovation =
      m_lineageNodeLogDetInnovation.createAndPut();
  auto* lineageNodeLogUnnormalizedPosterior =
      m_lineageNodeLogUnnormalizedPosterior.createAndPut();
  auto* lineageNodeNormalizedPosterior =
      m_lineageNodeNormalizedPosterior.createAndPut();
  auto* lineageNodePredictedKappa =
      m_lineageNodePredictedKappa.createAndPut();
  auto* lineageNodePredictedKappaVariance =
      m_lineageNodePredictedKappaVariance.createAndPut();
  auto* lineageNodeFilteredKappa =
      m_lineageNodeFilteredKappa.createAndPut();
  auto* lineageNodeFilteredKappaVariance =
      m_lineageNodeFilteredKappaVariance.createAndPut();
  auto* lineageNodeDominantLineageFraction =
      m_lineageNodeDominantLineageFraction.createAndPut();
  auto* lineageNodeMergeCost = m_lineageNodeMergeCost.createAndPut();
  auto* lineageEdgeInputTrackIndex =
      m_lineageEdgeInputTrackIndex.createAndPut();
  auto* lineageEdgeOutputTrackIndex =
      m_lineageEdgeOutputTrackIndex.createAndPut();
  auto* lineageEdgeFromNodeId = m_lineageEdgeFromNodeId.createAndPut();
  auto* lineageEdgeToNodeId = m_lineageEdgeToNodeId.createAndPut();
  auto* lineageEdgeOperation = m_lineageEdgeOperation.createAndPut();
  auto persistFinalMixtureComponents = [&](const auto& records,
                                           std::int32_t inputTrackIndex,
                                           std::int32_t outputTrackIndex) {
    for (const auto& record : records) {
      finalMixtureComponentInputTrackIndex->push_back(inputTrackIndex);
      finalMixtureComponentOutputTrackIndex->push_back(outputTrackIndex);
      finalMixtureComponentIndex->push_back(record.componentIndex);
      finalMixtureComponentID->push_back(record.componentID);
      finalMixtureComponentSource->push_back(record.source);
      finalMixtureComponentValid->push_back(record.valid);
      finalMixtureComponentWeight->push_back(record.weight);
      finalMixtureComponentKappa->push_back(record.kappa);
      finalMixtureComponentKappaVariance->push_back(
          record.kappaVariance);
    }
  };
  auto persistLineageGraph = [&](const LineageGraphRecorder& graph,
                                 std::int32_t inputTrackIndex,
                                 std::int32_t outputTrackIndex) {
    for (const auto& node : graph.nodes()) {
      lineageNodeInputTrackIndex->push_back(inputTrackIndex);
      lineageNodeOutputTrackIndex->push_back(outputTrackIndex);
      lineageNodeId->push_back(node.nodeId);
      lineageNodeSource->push_back(node.source);
      lineageNodeOperation->push_back(node.operation);
      lineageNodeHitIndex->push_back(node.hitIndex);
      lineageNodeSurfaceIndex->push_back(node.surfaceIndex);
      lineageNodeComponentId->push_back(node.componentId);
      lineageNodeGeneration->push_back(node.generation);
      lineageNodeBhComponentIndex->push_back(node.bhComponentIndex);
      lineageNodeMeasurementStatus->push_back(node.measurementStatus);
      lineageNodeFate->push_back(node.fate);
      lineageNodeNoRadiation->push_back(node.noRadiationLineage);
      lineageNodeBestBranch->push_back(node.bestBranch);
      lineageNodeFinalMixture->push_back(node.finalMixture);
      lineageNodeValid->push_back(node.valid);
      lineageNodeWeight->push_back(node.weight);
      lineageNodePriorWeight->push_back(node.priorWeight);
      lineageNodeBhWeight->push_back(node.bhWeight);
      lineageNodeBhMean->push_back(node.bhMean);
      lineageNodeBhVariance->push_back(node.bhVariance);
      lineageNodeMaterialTX0->push_back(node.materialTX0);
      lineageNodeDChi2->push_back(node.dchi2);
      lineageNodeLogDetInnovation->push_back(node.logDetInnovation);
      lineageNodeLogUnnormalizedPosterior->push_back(
          node.logUnnormalizedPosterior);
      lineageNodeNormalizedPosterior->push_back(node.normalizedPosterior);
      lineageNodePredictedKappa->push_back(node.predictedKappa);
      lineageNodePredictedKappaVariance->push_back(
          node.predictedKappaVariance);
      lineageNodeFilteredKappa->push_back(node.filteredKappa);
      lineageNodeFilteredKappaVariance->push_back(
          node.filteredKappaVariance);
      lineageNodeDominantLineageFraction->push_back(
          node.dominantLineageFraction);
      lineageNodeMergeCost->push_back(node.mergeCost);
    }
    for (const auto& edge : graph.edges()) {
      lineageEdgeInputTrackIndex->push_back(inputTrackIndex);
      lineageEdgeOutputTrackIndex->push_back(outputTrackIndex);
      lineageEdgeFromNodeId->push_back(edge.fromNodeId);
      lineageEdgeToNodeId->push_back(edge.toNodeId);
      lineageEdgeOperation->push_back(edge.operation);
    }
  };
  auto* truthBHLossStatus = m_truthBHLossStatus.createAndPut();
  auto* truthMaterialIntervals = m_truthMaterialIntervals.createAndPut();
  auto* truthMaterialStatus = m_truthMaterialStatus.createAndPut();
  edm4hep::TrackCollection* ecalOut = nullptr;
  if (m_ecalComponentConstraint.value())
    ecalOut = m_ecalConstrainedOutputTracks.createAndPut();

  const bool selectedOnly = !m_selectedEventIndices.value().empty();
  if (selectedOnly && std::find(m_selectedEventIndices.value().begin(),
                                m_selectedEventIndices.value().end(),
                                eventIndex) == m_selectedEventIndices.value().end()) {
    truthBHLossStatus->push_back(truthBHLossStatusValue(
        m_truthBHLossOverride.value()
            ? TruthBHLossScopeStatus::TrackNotProcessed
            : TruthBHLossScopeStatus::Disabled));
    truthMaterialStatus->push_back(truthBHLossStatusValue(
        m_recordTruthMaterialIntervals.value()
            ? TruthBHLossScopeStatus::TrackNotProcessed
            : TruthBHLossScopeStatus::Disabled));
    return StatusCode::SUCCESS;
  }

  // Summaries are an event-local diagnostic view.  Keeping every summary for
  // the full job makes memory usage grow with the number of processed tracks.
  m_summaries.clear();

  info() << "GSF event index " << eventIndex
         << " (event count " << m_nEvt << ")" << endmsg;
  const auto* in = m_inputTracks.get();
  if (!in) return StatusCode::SUCCESS;

  const auto initialTruthStatus = m_truthBHLossOverride.value()
      ? TruthBHLossScopeStatus::NotSelected
      : TruthBHLossScopeStatus::Disabled;
  truthBHLossStatus->resize(in->size());
  for (std::size_t index = 0; index < in->size(); ++index)
    (*truthBHLossStatus)[index] = truthBHLossStatusValue(initialTruthStatus);
  const auto initialMaterialStatus = m_recordTruthMaterialIntervals.value()
      ? TruthBHLossScopeStatus::NotSelected
      : TruthBHLossScopeStatus::Disabled;
  truthMaterialStatus->resize(in->size());
  for (std::size_t index = 0; index < in->size(); ++index)
    (*truthMaterialStatus)[index] =
        truthBHLossStatusValue(initialMaterialStatus);
  const int materialInputTrack = m_truthBHLossInputTrackIndex.value();
  if (m_recordTruthMaterialIntervals.value() && materialInputTrack >= 0 &&
      materialInputTrack < static_cast<int>(truthMaterialStatus->size())) {
    (*truthMaterialStatus)[static_cast<std::size_t>(materialInputTrack)] =
        truthBHLossStatusValue(TruthBHLossScopeStatus::TrackNotProcessed);
  }

  TruthBHLossEventData eventDataTruth;
  const bool needEventDataTruth =
      m_recordTruthMaterialIntervals.value() ||
      m_truthBHLossOverride.value();
  bool eventDataTruthEventValid = true;
  std::string eventDataTruthEventError;
  if (needEventDataTruth) {
    bool eventDataPrepared = false;
    try {
      std::vector<const edm4hep::MCRecoTrackerAssociationCollection*>
          associations;
      auto appendAvailableAssociation = [&associations](auto& handle) {
        try {
          if (const auto* collection = handle.get())
            associations.push_back(collection);
        } catch (...) {
          // An unrequested detector collection is harmless unless the
          // selected track contains one of its hits; matchTrack then rejects
          // that track through the strict all-or-nothing association check.
        }
      };
      appendAvailableAssociation(m_vxdTruthAssociations);
      appendAvailableAssociation(m_itkBarrelTruthAssociations);
      appendAvailableAssociation(m_itkEndcapTruthAssociations);
      appendAvailableAssociation(m_tpcTruthAssociations);
      appendAvailableAssociation(m_otkBarrelTruthAssociations);
      appendAvailableAssociation(m_otkEndcapTruthAssociations);
      if (associations.empty()) {
        eventDataTruthEventError =
            "no requested tracker truth-association collection is available";
      } else {
        eventDataPrepared = eventDataTruth.prepare(
            m_gsfTruthSteps.get(), m_gsfTruthLinks.get(), associations,
            eventDataTruthEventError);
      }
    } catch (const std::exception& exception) {
      eventDataTruthEventError =
          std::string("cannot retrieve embedded truth event data: ") +
          exception.what();
    } catch (...) {
      eventDataTruthEventError =
          "cannot retrieve embedded truth event data: unknown exception";
    }
    if (!eventDataPrepared) {
      eventDataTruthEventValid = false;
      warning() << "Embedded truth material event=" << eventIndex
                << " is invalid: " << eventDataTruthEventError
                << "; passive records are unavailable and the configured "
                   "GSF workflow remains unchanged"
                << endmsg;
      const int selectedTrack = m_truthBHLossInputTrackIndex.value();
      if (m_recordTruthMaterialIntervals.value() && selectedTrack >= 0 &&
          selectedTrack < static_cast<int>(truthMaterialStatus->size())) {
        (*truthMaterialStatus)[static_cast<std::size_t>(selectedTrack)] =
            truthBHLossStatusValue(TruthBHLossScopeStatus::InvalidTruthEvent);
      }
      if (m_truthBHLossOverride.value()) {
        ++m_truthBHLossInvalidTruthEvents;
        if (selectedTrack >= 0 &&
            selectedTrack < static_cast<int>(truthBHLossStatus->size())) {
          (*truthBHLossStatus)[static_cast<std::size_t>(selectedTrack)] =
              truthBHLossStatusValue(
                  TruthBHLossScopeStatus::InvalidTruthEvent);
        }
      }
    }
  }
  std::map<TruthBHLossKey, double> dynamicTruthRetainedFractions;
  int nFit = 0;
  int inputTrackIndex = -1;
  double bz = m_field;
  std::string materialPathMode = m_materialPathMode.value();
  std::transform(materialPathMode.begin(), materialPathMode.end(),
                 materialPathMode.begin(), ::tolower);
  const bool useDD4hepBetweenSurfaces =
      materialPathMode == "dd4hepbetweensurfaces";
  std::string configuredReverseSelection = m_reverseSelectionMode.value();
  std::transform(configuredReverseSelection.begin(),
                 configuredReverseSelection.end(),
                 configuredReverseSelection.begin(), ::tolower);
  const bool selectSurfaceConsistency =
      configuredReverseSelection == "surfaceconsistency";
  const bool trackSurfaceLineageMass =
      m_surfaceLineageMassDump.value() || selectSurfaceConsistency;

  for (const auto& trk : *in) {
    ++inputTrackIndex;
    const bool truthSourceSelectsTrack =
        m_truthBHLossOverride.value() &&
        inputTrackIndex == m_truthBHLossInputTrackIndex.value();
    TruthBHLossScopeStatus truthBHLossScopeStatus =
        m_truthBHLossOverride.value()
            ? TruthBHLossScopeStatus::NotSelected
            : TruthBHLossScopeStatus::Disabled;
    std::string truthBHLossScopeReason;
    if (truthSourceSelectsTrack) {
      if (!eventDataTruthEventValid) {
        truthBHLossScopeStatus =
            TruthBHLossScopeStatus::InvalidTruthEvent;
        truthBHLossScopeReason = eventDataTruthEventError;
      } else {
        // Replaced by Valid or a precise negative reason once the complete
        // accepted-hit interval map has been checked.
        truthBHLossScopeStatus =
            TruthBHLossScopeStatus::TrackNotProcessed;
      }
    }
    (*truthBHLossStatus)[static_cast<std::size_t>(inputTrackIndex)] =
        truthBHLossStatusValue(truthBHLossScopeStatus);
    const bool materialSourceSelectsTrack =
        m_recordTruthMaterialIntervals.value() &&
        inputTrackIndex == m_truthBHLossInputTrackIndex.value();
    TruthBHLossScopeStatus truthMaterialScopeStatus =
        m_recordTruthMaterialIntervals.value()
            ? TruthBHLossScopeStatus::NotSelected
            : TruthBHLossScopeStatus::Disabled;
    if (materialSourceSelectsTrack) {
      truthMaterialScopeStatus = eventDataTruthEventValid
          ? TruthBHLossScopeStatus::TrackNotProcessed
          : TruthBHLossScopeStatus::InvalidTruthEvent;
    }
    (*truthMaterialStatus)[static_cast<std::size_t>(inputTrackIndex)] =
        truthBHLossStatusValue(truthMaterialScopeStatus);

    auto assocHits = trk.getTrackerHits();
    if (assocHits.size() < 5) continue;

    // ---- Step 1: seed from LCIO ----
    LcioSeed seed = extractSeed(trk);
    if (seed.omega == 0) continue;

    // ---- Step 2: match hits to measurement layers ----
    std::vector<MatchedHit> hits;
    std::size_t inputHitOrder = 0;
    for (const auto& th : assocHits) {
      const std::size_t thisInputOrder = inputHitOrder++;
      if (!th.isAvailable()) continue;

      TVector3 pos(th.getPosition().x, th.getPosition().y,
                    th.getPosition().z);

      auto* layer = findLayer(th, m_cellIDToLayer, m_cradle);
      if (!layer) continue;

      DDVTrackHit* khit = layer->ConvertLCIOTrkHit(
          const_cast<edm4hep::TrackerHit&>(th));
      if (!khit) continue;

      const auto* surface = dynamic_cast<const TVSurface*>(layer);
      const double surfaceOrder = surface ? surface->GetSortingPolicy() : 0.0;
      hits.push_back({th, layer, khit,
                      std::hypot(pos.X(), pos.Y()), thisInputOrder,
                      layer->GetIndex(), surfaceOrder});
    }

    std::sort(hits.begin(), hits.end(),
              [](auto& a, auto& b) { return a.radius < b.radius; });
    if (hits.empty()) continue;

    auto truthBHLossKey = [&](int hitFromIndex, int hitToIndex) {
      const auto& fromHit = hits[static_cast<std::size_t>(hitFromIndex)];
      const auto& toHit = hits[static_cast<std::size_t>(hitToIndex)];
      return TruthBHLossKey{
          eventIndex, inputTrackIndex, hitFromIndex, hitToIndex,
          static_cast<std::uint64_t>(fromHit.lcioHit.getCellID()),
          static_cast<std::uint64_t>(toHit.lcioHit.getCellID())};
    };
    bool dynamicTruthTrackValid = false;
    bool truthMaterialTrackMatched = false;
    TruthBHLossEventDataMatch eventDataMatch;
    const bool eventDataRequestedForTrack =
        materialSourceSelectsTrack || truthSourceSelectsTrack;

    if (eventDataRequestedForTrack && eventDataTruthEventValid) {
      std::vector<edm4hep::TrackerHit> orderedHits;
      orderedHits.reserve(hits.size());
      for (const auto& hit : hits) orderedHits.push_back(hit.lcioHit);

      std::string matchError;
      if (!eventDataTruth.matchTrack(
              orderedHits, m_truthBHLossMaxEndpointDistance.value(),
              materialSourceSelectsTrack, eventDataMatch, matchError)) {
        const auto invalidStatus =
            eventDataMatch.maxEndpointDistance >
                    m_truthBHLossMaxEndpointDistance.value()
                ? TruthBHLossScopeStatus::EndpointDistanceExceeded
                : TruthBHLossScopeStatus::InvalidIntervalMapping;
        if (materialSourceSelectsTrack) {
          truthMaterialScopeStatus = invalidStatus;
          (*truthMaterialStatus)[static_cast<std::size_t>(inputTrackIndex)] =
              truthBHLossStatusValue(truthMaterialScopeStatus);
        }
        if (truthSourceSelectsTrack) {
          truthBHLossScopeStatus = invalidStatus;
          truthBHLossScopeReason = matchError;
          if (invalidStatus ==
              TruthBHLossScopeStatus::EndpointDistanceExceeded) {
            ++m_truthBHLossInvalidEndpointTracks;
          } else {
            ++m_truthBHLossInvalidIntervalTracks;
          }
        }
        warning() << "Embedded truth material event=" << eventIndex
                  << " inputTrack=" << inputTrackIndex << " is invalid: "
                  << matchError << "; passive records are unavailable and "
                     "the configured GSF workflow remains unchanged"
                  << endmsg;
      } else {
        const bool exactIntervalCounts =
            eventDataMatch.retainedFractions.size() == hits.size() - 1 &&
            (!materialSourceSelectsTrack ||
             eventDataMatch.materialIntervals.size() == hits.size() - 1);
        if (!exactIntervalCounts) {
          const std::string countError =
              "EventData interval count does not match accepted hits";
          if (materialSourceSelectsTrack) {
            truthMaterialScopeStatus =
                TruthBHLossScopeStatus::InvalidIntervalMapping;
            (*truthMaterialStatus)[static_cast<std::size_t>(inputTrackIndex)] =
                truthBHLossStatusValue(truthMaterialScopeStatus);
          }
          if (truthSourceSelectsTrack) {
            truthBHLossScopeStatus =
                TruthBHLossScopeStatus::InvalidIntervalMapping;
            truthBHLossScopeReason = countError;
            ++m_truthBHLossInvalidIntervalTracks;
          }
          warning() << "Embedded truth material event=" << eventIndex
                    << " inputTrack=" << inputTrackIndex << " is invalid: "
                    << countError << endmsg;
        } else {
          truthMaterialTrackMatched = materialSourceSelectsTrack;
          if (truthSourceSelectsTrack) {
            bool inserted = true;
            for (std::size_t hitFrom = 0;
                 hitFrom < eventDataMatch.retainedFractions.size();
                 ++hitFrom) {
              inserted = dynamicTruthRetainedFractions.emplace(
                  truthBHLossKey(static_cast<int>(hitFrom),
                                 static_cast<int>(hitFrom + 1)),
                  eventDataMatch.retainedFractions[hitFrom]).second &&
                  inserted;
            }
            if (!inserted) {
              truthBHLossScopeStatus =
                  TruthBHLossScopeStatus::InvalidIntervalMapping;
              truthBHLossScopeReason =
                  "duplicate EventData accepted-hit interval key";
              ++m_truthBHLossInvalidIntervalTracks;
            } else {
              dynamicTruthTrackValid = true;
              truthBHLossScopeStatus = TruthBHLossScopeStatus::Valid;
              truthBHLossScopeReason.clear();
              ++m_truthBHLossDynamicTracks;
              m_truthBHLossMaxObservedEndpointDistance = std::max(
                  m_truthBHLossMaxObservedEndpointDistance,
                  eventDataMatch.maxEndpointDistance);
            }
          }
          if (m_verboseDump && materialSourceSelectsTrack) {
            info() << "Passive truth-material recorder matched event="
                   << eventIndex << " inputTrack=" << inputTrackIndex
                   << " G4Track=" << eventDataMatch.g4TrackID << " with "
                   << eventDataMatch.materialIntervals.size()
                   << " accepted-hit intervals; max endpoint distance="
                   << eventDataMatch.maxEndpointDistance << " mm" << endmsg;
          }
        }
      }
    }

    const auto& truthRetainedFractions = dynamicTruthRetainedFractions;
    auto truthRetainedFraction = [&](int hitFromIndex, int hitToIndex) {
      return truthRetainedFractions.at(
          truthBHLossKey(hitFromIndex, hitToIndex));
    };
    bool applyTruthBHLossOverride =
        truthSourceSelectsTrack && dynamicTruthTrackValid;

    if (applyTruthBHLossOverride) {
      for (std::size_t hitFrom = 0; hitFrom + 1 < hits.size(); ++hitFrom) {
        const auto key = truthBHLossKey(
            static_cast<int>(hitFrom), static_cast<int>(hitFrom + 1));
        if (truthRetainedFractions.find(key) ==
            truthRetainedFractions.end()) {
          std::ostringstream reason;
          reason << "no exact interval for hit " << hitFrom << "->"
                 << hitFrom + 1 << " cell " << std::get<4>(key) << "->"
                 << std::get<5>(key);
          truthBHLossScopeStatus =
              TruthBHLossScopeStatus::InvalidIntervalMapping;
          truthBHLossScopeReason = reason.str();
          applyTruthBHLossOverride = false;
          ++m_truthBHLossInvalidIntervalTracks;
          warning() << "Embedded truth material event=" << eventIndex
                    << " inputTrack=" << inputTrackIndex << " has "
                    << truthBHLossScopeReason
                    << "; using the configured BH model for the whole track"
                    << endmsg;
          break;
        }
      }
    }
    (*truthBHLossStatus)[static_cast<std::size_t>(inputTrackIndex)] =
        truthBHLossStatusValue(truthBHLossScopeStatus);

    if (m_truthBHLossOverride.value() && !applyTruthBHLossOverride) {
      ++m_truthBHLossPassthroughTracks;
      if (truthBHLossScopeStatus == TruthBHLossScopeStatus::NotSelected) {
        info() << "Truth BH-loss oracle has no scope for event=" << eventIndex
               << " inputTrack=" << inputTrackIndex
               << "; this whole unselected track uses the configured BH "
                  "model"
               << endmsg;
      }
    }

    // These summaries mirror material values already computed by the live
    // filter. They are filled only for the passive truth-material scope and
    // are never read by propagation, splitting, weighting, or selection.
    std::vector<RuntimeMaterialSummary> forwardMaterialSummaries(
        hits.size() > 1 ? hits.size() - 1 : 0);
    std::vector<RuntimeMaterialSummary> reverseMaterialSummaries(
        hits.size() > 1 ? hits.size() - 1 : 0);

    if (m_verboseDump && m_componentDebugDump) {
      bool inputMonotonic = true;
      bool surfaceMonotonic = true;
      bool repeatedSurface = false;
      int directionReversals = 0;
      double minimumStepCosine = 1.0;
      std::set<int> visitedSurfaceIndices;
      TVector3 previousStep;
      bool havePreviousStep = false;
      info() << boost::format("  NAV audit: hits=%d radius-sorted surface records")
                % (int)hits.size() << endmsg;
      for (std::size_t i = 0; i < hits.size(); ++i) {
        const auto& navHit = hits[i];
        const auto& position = navHit.lcioHit.getPosition();
        if (i > 0) {
          inputMonotonic &= navHit.inputOrder > hits[i - 1].inputOrder;
          surfaceMonotonic &= navHit.surfaceIndex >= hits[i - 1].surfaceIndex;
          const auto& previousPosition = hits[i - 1].lcioHit.getPosition();
          TVector3 step(position.x - previousPosition.x,
                        position.y - previousPosition.y,
                        position.z - previousPosition.z);
          if (havePreviousStep && step.Mag2() > 0.0 && previousStep.Mag2() > 0.0) {
            const double cosine = step.Dot(previousStep) /
                std::sqrt(step.Mag2() * previousStep.Mag2());
            minimumStepCosine = std::min(minimumStepCosine, cosine);
            if (cosine < 0.0) ++directionReversals;
          }
          previousStep = step;
          havePreviousStep = step.Mag2() > 0.0;
        }
        repeatedSurface |= !visitedSurfaceIndices.insert(navHit.surfaceIndex).second;
        info() << boost::format("    NAV hit=%3d input=%3d surface=%4d order=%9.3f cell=%lld xyz=(%.3f,%.3f,%.3f) r=%.3f")
                  % (int)i % (int)navHit.inputOrder % navHit.surfaceIndex
                  % navHit.surfaceOrder % (long long)navHit.lcioHit.getCellID()
                  % position.x % position.y % position.z % navHit.radius << endmsg;
      }
      info() << boost::format("  NAV summary: inputMonotonic=%d surfaceMonotonic=%d repeatedSurface=%d directionReversals=%d minStepCos=%.6f")
                % (inputMonotonic ? 1 : 0) % (surfaceMonotonic ? 1 : 0)
                % (repeatedSurface ? 1 : 0) % directionReversals
                % minimumStepCosine << endmsg;
    }

    // The complete component-lineage DAG is an automatic passive output for
    // every multi-component workflow. Forward-only runs keep the collections
    // present but empty.
    LineageGraphRecorder lineageGraph(
        m_reverseFiltering.value() || m_gaussianSumSmoothing.value(),
        bz);

    // ---- Step 3: standard-KF-style fresh prefit and first-hit update ----
    std::vector<edm4hep::TrackerHit> orderedHits;
    orderedHits.reserve(hits.size());
    for (const auto& hit : hits) orderedHits.push_back(hit.lcioHit);
    GsfTrackInitializer initializer(m_gsfMarlinTrkSystem);
    auto initialization = initializer.initialize(
        orderedHits, *hits.front().layer, *hits.front().kalHit, bz,
        GsfTrackInitializationDirection::Outward,
        m_kappaSeedCov.value());
    if (!initialization.valid()) {
      warning() << "GSF standard-KF-style initialization failed for event="
                << eventIndex << " inputTrack=" << inputTrackIndex << ": "
                << initialization.error << endmsg;
      for (auto& hit : hits) delete hit.kalHit;
      continue;
    }
    const int charge = initialization.seedFilteredState.omega > 0.0 ? 1 : -1;
    if (m_verboseDump) {
      info() << boost::format(
          "  INIT standard-kf-prefit twoDHits=%d firstHitDChi2=%.9g "
          "firstHitNdf=%d firstHitDim=%d prefitOmega=%.9g filteredOmega=%.9g "
          "prefitVarOmega=%.9g prefitVarKappa=%.9g")
            % initialization.twoDimensionalHitCount
            % initialization.seedHitDeltaChi2
            % initialization.seedHitNdf
            % initialization.seedHitMeasurementDimension
            % initialization.prefitState.omega
            % initialization.seedFilteredState.omega
            % initialization.prefitOmegaVariance
            % initialization.prefitKappaVariance << endmsg;
    }

    // ---- Step 4: forward GSF filter ----
    const bool fitBackwards = !MarlinTrk::IMarlinTrack::backward;
    const size_t gsfStartHit = 1;
    TKalTrackSite* site = initialization.site;
    int nextComponentDebugId = 0;
    auto* initComp = new GsfComponent();
    initComp->pendingProcessJacobian.UnitMatrix();
    initComp->weight = 1.0;
    initComp->charge = charge;
    initComp->debugId = nextComponentDebugId++;
    initComp->debugHistory = "seed";
    initComp->fitChi2 = initialization.seedHitDeltaChi2;
    initComp->kaltrack = new TKalTrack();
    initComp->kaltrack->SetOwner();
    initComp->kaltrack->Add(site);
    initComp->lineageNodeId = lineageGraph.seed(
        *initComp, LineageNodeSource::ForwardFiltering, 0,
        hits[0].surfaceIndex);

    std::vector<GsfComponent*> comps = {initComp};
    std::vector<GsfSmootherNode> smootherGraph;
    SharedForwardFilterResult sharedForwardResult(
        hits.size(), bz, m_reverseFiltering.value());
    int nProc = 0, nSplits = 0, nReductions = 0, maxCompsEver = 1;
    double totalTX0 = 0.0, maxTX0Layer = 0.0;
    bool justSplit = false;
    int totalAccept = 0, totalRecover = 0, totalReject = 0;
    int lastAccept = 0, lastRecover = 0, lastReject = 0;

    auto truncateHistory = [&](const std::string& h) {
      const int maxLen = std::max(0, m_componentDebugMaxHistory.value());
      if ((int)h.size() <= maxLen) return h;
      if (maxLen <= 3) return h.substr(0, maxLen);
      return h.substr(0, maxLen - 3) + "...";
    };

    auto formatProcessModeFractions = [](
        const std::map<std::pair<int, int>, double>& fractions) {
      std::ostringstream text;
      text.setf(std::ios::fixed, std::ios::floatfield);
      text.precision(6);
      int currentHit = -1;
      double radiativeMass = 0.0;
      bool firstHit = true;
      auto finishHit = [&]() {
        if (currentHit >= 0) text << ",rad=" << radiativeMass << "]";
      };
      for (const auto& item : fractions) {
        const int hit = item.first.first;
        const int mode = item.first.second;
        if (hit != currentHit) {
          finishHit();
          if (!firstHit) text << ";";
          text << hit << ":[";
          currentHit = hit;
          radiativeMass = 0.0;
          firstHit = false;
        } else {
          text << ",";
        }
        text << "g" << mode << "=" << item.second;
        if (mode > 0) radiativeMass += item.second;
      }
      finishHit();
      return text.str();
    };

    auto dumpComponents = [&](const char* label, int ih,
                              const std::vector<GsfComponent*>& vc) {
      if (!m_verboseDump || !m_verboseSplitDump) return;
      double sumW = 0.0, minPt = 1e300, maxPt = 0.0;
      std::vector<size_t> order(vc.size());
      for (size_t i = 0; i < vc.size(); ++i) {
        order[i] = i;
        const auto* c = vc[i];
        sumW += c->weight;
        const double k = c->helixAtLastSite(bz).GetKappa();
        const double pt = (k != 0.0) ? 1.0 / std::abs(k) : 0.0;
        minPt = std::min(minPt, pt);
        maxPt = std::max(maxPt, pt);
      }
      std::sort(order.begin(), order.end(), [&](size_t a, size_t b) {
        return vc[a]->weight > vc[b]->weight;
      });
      info() << boost::format("  MIX %-18s hit=%3d n=%2d sumW=%.4g pT=[%.3f, %.3f]")
                % label % ih % (int)vc.size() % sumW
                % (vc.empty() ? 0.0 : minPt) % (vc.empty() ? 0.0 : maxPt) << endmsg;
      const size_t nShow = m_componentDebugDump ? vc.size() : std::min<size_t>(3, vc.size());
      for (size_t rank = 0; rank < nShow; rank++) {
        const size_t ci = order[rank];
        const auto* c = vc[ci];
        const double k = c->helixAtLastSite(bz).GetKappa();
        const double pt = (k != 0.0) ? 1.0 / std::abs(k) : 0.0;
        const int entries = c->kaltrack ? c->kaltrack->GetEntriesFast() : 0;
        const double chi2 = componentFitChi2(*c);
        const int ndf = c->kaltrack
            ? c->kaltrack->GetNDF() +
                  initialization.seedHitMeasurementDimension
            : 0;
        info() << boost::format("      top%-2d comp[%02d] id=%d parent=%d gen=%d noRad=%d procH=%d procG=%d procF=%.6g w=%.6g domFrac=%.6g domW=%.6g pT=%.6g kappa=%.6e chi2=%.3f ndf=%d sites=%d")
                  % (int)rank % (int)ci % c->debugId % c->debugParentId % c->generation
                  % (int)c->noRadiationLineage
                  % c->lastReverseProcessHit
                  % c->lastReverseProcessComponent
                  % c->lastReverseProcessFraction
                  % c->weight % c->dominantLineageFraction
                  % (c->weight * c->dominantLineageFraction)
                  % pt % k % chi2 % ndf % entries << endmsg;
        if (m_componentDebugDump) {
          info() << boost::format("          history=%s")
                    % truncateHistory(c->debugHistory) << endmsg;
          if (std::string(label).find("reverse-") == 0) {
            info() << boost::format("          signatures forward=%s reverse=%s")
                      % c->forwardProcessSignature
                      % c->reverseProcessSignature << endmsg;
          }
          if (m_surfaceLineageMassDump) {
            info() << boost::format(
                "          surface-mode-mass forward=%s reverse=%s")
                      % formatProcessModeFractions(
                            c->forwardProcessModeFractions)
                      % formatProcessModeFractions(
                            c->reverseProcessModeFractions) << endmsg;
          }
        }
      }
    };

    sharedForwardResult.captureFiltered(0, comps);

    // The seed already contains the filtered hit-0 state. In full-interval
    // mode, convolve it through the hit-0 -> hit-1 material before the first
    // measurement update; the legacy current-surface mode intentionally keeps
    // its historical behavior for controlled comparison.
    if (useDD4hepBetweenSurfaces && hits.size() > 1) {
      const auto& seedPosition = hits[1].lcioHit.getPosition();
      const TVector3 seedDestination(
          seedPosition.x, seedPosition.y, seedPosition.z);
      const auto seedMaterial = componentGeometryTransitionMaterialPath(
          m_materialManager, hits[1].layer, seedDestination, *initComp, bz);
      if (truthMaterialTrackMatched) {
        forwardMaterialSummaries[0].add(
            seedMaterial, initComp->weight, initComp->debugId,
            m_bhSplitThresh.value());
      }
      if (seedMaterial.valid) {
        maxTX0Layer = std::max(maxTX0Layer, seedMaterial.pathTX0);
        totalTX0 += seedMaterial.pathTX0;
      }
      if (m_forwardBHSplitting.value() && seedMaterial.valid &&
          seedMaterial.pathTX0 > m_bhSplitThresh.value() && m_isElectron) {
        BetheHeitlerSplitter splitter(m_bhModel.value());
        const int parentDebugId = initComp->debugId;
        const int parentLineageNodeId = initComp->lineageNodeId;
        std::vector<BetheHeitlerMixtureComponent> appliedMixture;
        auto children = applyTruthBHLossOverride
            ? splitter.splitWithRetainedFraction(
                  initComp, truthRetainedFraction(0, 1), bz, false,
                  &appliedMixture)
            : splitter.split(initComp, seedMaterial.pathTX0, bz, false,
                             &appliedMixture);
        if (applyTruthBHLossOverride) ++m_truthBHLossOverrideCalls;
        comps.clear();
        for (size_t childIndex = 0; childIndex < children.size(); ++childIndex) {
          auto* child = children[childIndex];
          child->debugId = nextComponentDebugId++;
          child->debugParentId = parentDebugId;
          child->generation = 1;
          if (childIndex < appliedMixture.size()) {
            child->lineageNodeId = lineageGraph.split(
                *child, parentLineageNodeId,
                LineageNodeSource::ForwardFiltering, 0,
                hits[0].surfaceIndex, static_cast<int>(childIndex),
                appliedMixture[childIndex], seedMaterial.pathTX0);
          }
          if (trackSurfaceLineageMass)
            child->forwardProcessModeFractions[{0, (int)childIndex}] = 1.0;
          comps.push_back(child);
        }
        ++nSplits;
        justSplit = true;
        maxCompsEver = std::max(maxCompsEver, (int)comps.size());
        GsfMixture::normalizeWeights(comps);
        dumpComponents("seed-material", 0, comps);
      }
    }

    for (size_t ih = gsfStartHit; ih < hits.size(); ih++) {
      auto& hi = hits[ih];
      const int compsAtHitBegin = (int)comps.size();
      int compsAfterSplit = compsAtHitBegin;
      int compsAfterUpdate = 0;
      int compsAfterReduce = 0;
      bool didReduceHit = false;
      // measurement position (same for all components)
      TVector3 measPos = hi.layer->HitToXv(*hi.kalHit);
      if (m_verboseDump && m_verboseSplitDump) {
        info() << boost::format("  FLOW hit=%3d begin: inputComps=%2d measurementR=%.1f mm")
                  % (int)ih % compsAtHitBegin % hi.radius << endmsg;
      }

      const double stepTX0 = thicknessInX0(hi.layer);
      if (m_verboseDump && m_componentDebugDump) {
        info() << boost::format("  MAT hit=%d r=%.1f stepTX0=%.6g comps=%d")
                  % (int)ih % hi.radius % stepTX0 % (int)comps.size() << endmsg;
      }

      justSplit = false;
      const int reductionTarget = (m_reductionTargetComponents.value() > 0)
          ? std::min(m_reductionTargetComponents.value(), m_maxComponents.value())
          : m_maxComponents.value();
      GsfMixture::normalizeWeights(comps);
      if (justSplit || (int)comps.size() > 1)
        dumpComponents("before-hit", (int)ih, comps);

      std::vector<GsfComponent*> accepted;
      std::vector<double> dchi2s;
      std::vector<double> acceptedLogWeights;
      if (m_verboseDump && m_verboseSplitDump && (justSplit || (int)comps.size() > 1)) {
        dchi2s.reserve(comps.size());
      }
      int nAccept = 0, nRecover = 0, nReject = 0;
      double minDChi2 = 1e300, maxDChi2 = -1e300;
      for (auto* comp : comps) {
        const int parentLineageNodeId = comp->lineageNodeId;
        if (m_gsfMarlinTrkSystem) {
          bool baselineAccepted = false;
          double dchi = 0.0;
          const double beforeKappa = comp->helixAtLastSite(bz).GetKappa();
          const double beforePt = (beforeKappa != 0.0) ? 1.0 / std::abs(beforeKappa) : 0.0;
          const double beforeWeight = comp->weight;
          const std::string predictedSurface =
              (m_verboseDump && m_verboseSplitDump && m_componentDebugDump)
                  ? surfacePredictionResidual(*comp, hi, bz)
                  : std::string();
          edm4hep::TrackState componentState;
          edm4hep::TrackState updatedState;
          MarlinTrk::MeasurementUpdate measurementUpdate;
          double posteriorLogWeight = -std::numeric_limits<double>::infinity();
          double updateChi2 = 0.0;
          int updateNdf = -999;
          try {
            edm4hep::TrackerHit trkHit = hi.lcioHit;
            edm4hep::TrackerHit referenceHit = hits[ih - 1].lcioHit;
            componentState = trackStateFromComponent(*comp, bz, DH::AtOther);
            std::unique_ptr<MarlinTrk::IMarlinTrack> baselineTrack(m_gsfMarlinTrkSystem->createTrack());
            if (baselineTrack &&
                baselineTrack->addHit(referenceHit) == MarlinTrk::IMarlinTrack::success &&
                baselineTrack->initialise(componentState, bz, fitBackwards) == MarlinTrk::IMarlinTrack::success &&
                baselineTrack->addAndFit(trkHit, dchi, measurementUpdate, DBL_MAX) == MarlinTrk::IMarlinTrack::success &&
                measurementUpdate.valid) {
              if (baselineTrack->getTrackState(trkHit, updatedState, updateChi2, updateNdf) == MarlinTrk::IMarlinTrack::success &&
                  appendBaselineStateToComponent(*comp, updatedState, hi, bz,
                                                 componentState,
                                                 measurementUpdate)) {
                baselineAccepted = true;
                posteriorLogWeight = std::log(beforeWeight) -
                    0.5 * (dchi + measurementUpdate.logDetInnovation);
              }
            }
          } catch (const std::exception& e) {
            if (m_verboseDump && m_componentDebugDump) {
              warning() << "GSF baseline component update threw exception: " << e.what() << endmsg;
            }
          } catch (...) {
            if (m_verboseDump && m_componentDebugDump) {
              warning() << "GSF baseline component update threw unknown exception" << endmsg;
            }
          }

          if (baselineAccepted && m_gaussianSumSmoothing.value() &&
              appendMeasurementSmootherNode(
                  *comp, static_cast<int>(ih), smootherGraph) < 0) {
            baselineAccepted = false;
          }

          const double unavailable =
              std::numeric_limits<double>::quiet_NaN();
          comp->lineageNodeId = lineageGraph.measurement(
              *comp, parentLineageNodeId,
              LineageNodeSource::ForwardFiltering, static_cast<int>(ih),
              hi.surfaceIndex, baselineAccepted ? 1 : 0, beforeWeight,
              measurementUpdate.valid ? dchi : unavailable,
              measurementUpdate.valid
                  ? measurementUpdate.logDetInnovation : unavailable,
              baselineAccepted ? posteriorLogWeight : unavailable,
              &measurementUpdate);

          if (baselineAccepted) {
            comp->fitChi2 += dchi;
            const double afterKappa = comp->helixAtLastSite(bz).GetKappa();
            const double afterPt = (afterKappa != 0.0) ? 1.0 / std::abs(afterKappa) : 0.0;
            nAccept++;
            minDChi2 = std::min(minDChi2, dchi);
            maxDChi2 = std::max(maxDChi2, dchi);
            if (m_verboseDump && m_verboseSplitDump && m_componentDebugDump) {
              info() << boost::format("      UPDATE accept comp[%02d] id=%d pT %.6g -> %.6g dchi2=%.6g chi2=%.6g logDetS=%.6g logWeight %.6g -> %.6g")
                        % (int)accepted.size() % comp->debugId % beforePt % afterPt % dchi % comp->fitChi2
                        % measurementUpdate.logDetInnovation % std::log(beforeWeight) % posteriorLogWeight << endmsg;
              info() << boost::format("          exact-predicted-state: %s")
                        % compactMatrix(measurementUpdate.predictedState) << endmsg;
              info() << boost::format("          exact-Ppred: %s")
                        % compactMatrix(measurementUpdate.predictedCovariance) << endmsg;
              info() << boost::format("          exact-measurement: predicted=%s residual=%s H=%s R=%s S=%s")
                        % compactMatrix(measurementUpdate.predictedMeasurement)
                        % compactMatrix(measurementUpdate.residual)
                        % compactMatrix(measurementUpdate.projector)
                        % compactMatrix(measurementUpdate.measurementCovariance)
                        % compactMatrix(measurementUpdate.innovationCovariance) << endmsg;
              info() << boost::format("          predict: %s")
                        % compactTrackState(componentState, bz) << endmsg;
              info() << boost::format("          predicted-surface: %s")
                        % predictedSurface << endmsg;
              info() << boost::format("          measure: pos=(%.4f,%.4f,%.4f) r=%.4f")
                        % measPos.X() % measPos.Y() % measPos.Z() % hi.radius << endmsg;
              info() << boost::format("          updated: %s fitChi2=%.6g ndf=%d")
                        % compactTrackState(updatedState, bz) % updateChi2 % updateNdf << endmsg;
            }
            accepted.push_back(comp);
            acceptedLogWeights.push_back(posteriorLogWeight);
            if (m_verboseDump && m_verboseSplitDump && (justSplit || (int)comps.size() > 1)) {
              dchi2s.push_back(dchi);
            }
          } else {
            nReject++;
            if (m_verboseDump && m_verboseSplitDump && m_componentDebugDump) {
              info() << boost::format("      UPDATE reject comp id=%d pT=%.6g weight=%.6g at hit=%d")
                        % comp->debugId % beforePt % beforeWeight % (int)ih << endmsg;
              info() << boost::format("          predict: %s")
                        % compactTrackState(componentState, bz) << endmsg;
              info() << boost::format("          predicted-surface: %s")
                        % predictedSurface << endmsg;
              info() << boost::format("          measure: pos=(%.4f,%.4f,%.4f) r=%.4f")
                        % measPos.X() % measPos.Y() % measPos.Z() % hi.radius << endmsg;
              info() << boost::format("          update-diagnostics: valid=%d Ppred=%s H=%s S=%s")
                        % (measurementUpdate.valid ? 1 : 0)
                        % compactMatrix(measurementUpdate.predictedCovariance)
                        % compactMatrix(measurementUpdate.projector)
                        % compactMatrix(measurementUpdate.innovationCovariance) << endmsg;
            }
            delete comp;
          }
          continue;
        }

        DDVTrackHit* khClone = nullptr;
        if (auto* ch = dynamic_cast<DDCylinderHit*>(hi.kalHit))
          khClone = new DDCylinderHit(*ch);
        else if (auto* ph = dynamic_cast<DDPlanarHit*>(hi.kalHit))
          khClone = new DDPlanarHit(*ph);
        else {
          comp->lineageNodeId = lineageGraph.measurement(
              *comp, parentLineageNodeId,
              LineageNodeSource::ForwardFiltering, static_cast<int>(ih),
              hi.surfaceIndex, 0, comp->weight,
              std::numeric_limits<double>::quiet_NaN(),
              std::numeric_limits<double>::quiet_NaN(),
              std::numeric_limits<double>::quiet_NaN(), nullptr);
          delete comp;
          continue;
        }

        auto* st = new TKalTrackSite(*khClone, kSdim);
        st->SetHitOwner();

        if (comp->kaltrack->AddAndFilter(*st)) {
          double dchi = st->GetDeltaChi2();
          const double oldWeight = comp->weight;
          const double posteriorLogWeight = std::log(oldWeight) - 0.5 * std::min(dchi, 100.0);
          comp->lineageNodeId = lineageGraph.measurement(
              *comp, parentLineageNodeId,
              LineageNodeSource::ForwardFiltering, static_cast<int>(ih),
              hi.surfaceIndex, 1, oldWeight, dchi,
              std::numeric_limits<double>::quiet_NaN(), posteriorLogWeight,
              nullptr);
          nAccept++;
          minDChi2 = std::min(minDChi2, dchi);
          maxDChi2 = std::max(maxDChi2, dchi);
          if (m_verboseDump && m_verboseSplitDump && m_componentDebugDump &&
              (justSplit || (int)comps.size() > 1)) {
            info() << boost::format("      hit-update accept comp[%02d] dchi2=%.6g logWeight %.6g -> %.6g")
                      % (int)accepted.size() % dchi % std::log(oldWeight) % posteriorLogWeight << endmsg;
          }
          accepted.push_back(comp);
          acceptedLogWeights.push_back(posteriorLogWeight);
          if (m_verboseDump && m_verboseSplitDump && (justSplit || (int)comps.size() > 1)) {
            dchi2s.push_back(dchi);
          }
        } else {
          bool recovered = false;
          // KalTest can fail re-crossing after Transport has already pivoted exactly to the hit.
          // In that narrow case keep the predicted state instead of dropping the component.
          if (st->GetEntriesFast() > TVKalSite::kPredicted) {
            auto& preState = dynamic_cast<const TKalTrackState&>(st->GetState(TVKalSite::kPredicted));
            auto preHel = preState.GetHelix();
            const auto& pv = preHel.GetPivot();
            const double pivotResidual = std::sqrt((pv.X() - measPos.X()) * (pv.X() - measPos.X()) +
                                                   (pv.Y() - measPos.Y()) * (pv.Y() - measPos.Y()) +
                                                   (pv.Z() - measPos.Z()) * (pv.Z() - measPos.Z()));
            if (pivotResidual < 1e-3) {
              st->Add(new TKalTrackState(preState, preState.GetCovMat(), *st, TVKalSite::kFiltered));
              st->SetOwner();
              comp->kaltrack->Add(st);
              accepted.push_back(comp);
              acceptedLogWeights.push_back(std::log(comp->weight));
              recovered = true;
            }
          }
          if (!recovered) {
            nReject++;
            comp->lineageNodeId = lineageGraph.measurement(
                *comp, parentLineageNodeId,
                LineageNodeSource::ForwardFiltering, static_cast<int>(ih),
                hi.surfaceIndex, 0, comp->weight,
                std::numeric_limits<double>::quiet_NaN(),
                std::numeric_limits<double>::quiet_NaN(),
                std::numeric_limits<double>::quiet_NaN(), nullptr);
            if (m_verboseDump && m_verboseSplitDump && m_componentDebugDump &&
                (justSplit || (int)comps.size() > 1)) {
              info() << boost::format("      hit-update reject comp at hit=%d") % (int)ih << endmsg;
            }
            delete st;
            delete comp;
          } else {
            nRecover++;
            comp->lineageNodeId = lineageGraph.measurement(
                *comp, parentLineageNodeId,
                LineageNodeSource::ForwardFiltering, static_cast<int>(ih),
                hi.surfaceIndex, 2, comp->weight,
                std::numeric_limits<double>::quiet_NaN(),
                std::numeric_limits<double>::quiet_NaN(),
                std::log(comp->weight), nullptr);
            if (m_verboseDump && m_verboseSplitDump && (justSplit || (int)comps.size() > 1)) {
              dchi2s.push_back(-1.0);
            }
            if (m_verboseDump && m_verboseSplitDump && m_componentDebugDump &&
                (justSplit || (int)comps.size() > 1)) {
              info() << boost::format("      hit-update recover comp at hit=%d w=%.6g")
                        % (int)ih % comp->weight << endmsg;
            }
          }
        }
      }

      totalAccept += nAccept;
      totalRecover += nRecover;
      totalReject += nReject;
      lastAccept = nAccept;
      lastRecover = nRecover;
      lastReject = nReject;

      if (m_verboseDump && m_verboseSplitDump &&
          (justSplit || (int)comps.size() > 1 || nRecover > 0 || nReject > 0)) {
        info() << boost::format("  HIT hit=%3d r=%7.1f accepted=%2d recovered=%2d rejected=%2d dchi2=[%.3g, %.3g]")
                  % (int)ih % hi.radius % nAccept % nRecover % nReject
                  % (nAccept ? minDChi2 : -1.0) % (nAccept ? maxDChi2 : -1.0) << endmsg;
      }

      if (accepted.empty()) {
        warning() << boost::format("GSF event index %d track %d: all components rejected at hit %d (r=%.1f mm); no GSF output track")
                     % (m_nEvt - 1) % (nFit + 1) % ih % hi.radius << endmsg;
        comps.clear();
        break;
      }

      // Convert posterior log weights to a safely scaled linear
      // representation.  The common shift cancels in normalization.
      const double maxLogWeight = *std::max_element(
          acceptedLogWeights.begin(), acceptedLogWeights.end());
      for (size_t i = 0; i < accepted.size(); ++i)
        accepted[i]->weight = std::exp(acceptedLogWeights[i] - maxLogWeight);

      // ── verbose: prediction vs measurement after pre-hit split ──
      if (m_verboseDump && m_verboseSplitDump && justSplit && !accepted.empty()) {
        info() << boost::format("  POST-SPLIT hit=%d r=%.1f survivors=%d measurement=(%.3f, %.3f, %.3f)")
                  % ih % hi.radius % (int)accepted.size()
                  % measPos.X() % measPos.Y() % measPos.Z() << endmsg;
        if (m_componentDebugDump) {
          for (size_t ci = 0; ci < accepted.size(); ci++) {
            auto* c = accepted[ci];
            double k = c->helixAtLastSite(bz).GetKappa();
            double dchi = (ci < dchi2s.size()) ? dchi2s[ci] : -1;
            info() << boost::format("      comp[%d] kappa=%.4e pT=%.3f weight=%.4f dchi2=%.1f")
                      % ci % k % (1.0/std::abs(k)) % c->weight % dchi << endmsg;
          }
        }
      }

      comps = std::move(accepted);
      compsAfterUpdate = (int)comps.size();
      if (justSplit || (int)comps.size() > 1)
        dumpComponents("after-hit/raw", (int)ih, comps);
      GsfMixture::normalizeWeights(comps);
      for (const auto* component : comps)
        lineageGraph.setNormalizedPosterior(
            component->lineageNodeId, component->weight);
      if (justSplit || (int)comps.size() > 1)
        dumpComponents("after-hit/norm", (int)ih, comps);

      // Reduce only after every process child has reached and incorporated
      // the target measurement.  The posterior weights now include the exact
      // innovation likelihood, so neither the low-weight cutoff nor KL moment
      // merging can discard/aggregate a child before the hit evaluates it.
      const int beforeCutoff = (int)comps.size();
      auto cutoffObserver = [&](const GsfComponent& component) {
        lineageGraph.mark(component.lineageNodeId,
                          LineageNodeFate::WeightCutoff);
      };
      GsfMixture::removeLowWeight(
          comps, m_componentWeightCutoff.value(),
          m_protectIdentityLineage.value(), cutoffObserver);
      if (m_verboseDump && m_verboseSplitDump &&
          (int)comps.size() != beforeCutoff) {
        info() << boost::format(
            "  FLOW hit=%3d posterior-cutoff: n=%d -> %d threshold=%.3g")
                  % (int)ih % beforeCutoff % (int)comps.size()
                  % m_componentWeightCutoff.value() << endmsg;
        dumpComponents("posterior-cutoff", (int)ih, comps);
      }

      const bool shouldReduce =
          ((int)comps.size() > m_maxComponents.value()) ||
          (reductionTarget < m_maxComponents.value() &&
           (int)comps.size() >= m_maxComponents.value());
      if (shouldReduce && (int)comps.size() > reductionTarget) {
        if (m_verboseDump && m_verboseSplitDump) {
          info() << boost::format(
              "  FLOW hit=%3d posterior-reduce: n=%d max=%d target=%d cost=%s")
                    % (int)ih % (int)comps.size()
                    % m_maxComponents.value() % reductionTarget
                    % m_reductionMergeCost.value() << endmsg;
          dumpComponents("posterior-pre-reduce", (int)ih, comps);
        }
        auto reductionLogger = [&](const std::string& line) {
          if (m_verboseDump && m_verboseSplitDump) info() << line << endmsg;
        };
        auto reductionObserver = [&](GsfComponent& merged,
                                     int keepSourceNodeId,
                                     int dropSourceNodeId,
                                     double mergeCost) {
          merged.lineageNodeId = lineageGraph.merge(
              merged, keepSourceNodeId, dropSourceNodeId,
              LineageNodeSource::ForwardFiltering,
              static_cast<int>(ih), hi.surfaceIndex, mergeCost);
        };
        GsfMixture::reduce(
            comps, reductionTarget, bz,
            m_protectIdentityLineage.value(), reductionLogger,
            m_reductionMergeCost.value(), reductionObserver);
        ++nReductions;
        didReduceHit = true;
        dumpComponents("posterior-post-reduce", (int)ih, comps);
        GsfMixture::normalizeWeights(comps);
        for (const auto* component : comps)
          lineageGraph.setWeight(component->lineageNodeId,
                                 component->weight);
        dumpComponents("posterior-post-reduce/norm", (int)ih, comps);
      }
      if (m_gaussianSumSmoothing.value() &&
          !appendReductionSmootherNodes(
              comps, static_cast<int>(ih), bz, smootherGraph)) {
        warning() << boost::format("GSF event index %d track %d: failed to record KL reduction graph at hit %d")
                     % (m_nEvt - 1) % (nFit + 1) % (int)ih << endmsg;
        for (auto* component : comps) delete component;
        comps.clear();
        break;
      }
      compsAfterReduce = (int)comps.size();
      sharedForwardResult.captureFiltered(ih, comps);

      // ACTS-like surface ordering: preserve the filtered measurement state,
      // then convolve components through the material associated with this
      // surface.  The splitter writes only the continuation snapshot, leaving
      // the Kalman measurement history unchanged.
      const bool hasNextForwardSurface = ih + 1 < hits.size();
      std::vector<ComponentMaterialPath> materialPaths;
      materialPaths.reserve(comps.size());
      bool anyComponentMaterial = false;
      TVector3 nextMaterialDestination;
      if (hasNextForwardSurface) {
        const auto& position = hits[ih + 1].lcioHit.getPosition();
        nextMaterialDestination.SetXYZ(position.x, position.y, position.z);
      }
      for (const auto* comp : comps) {
        materialPaths.push_back(
            hasNextForwardSurface && useDD4hepBetweenSurfaces
                ? componentGeometryTransitionMaterialPath(
                      m_materialManager, hits[ih + 1].layer,
                      nextMaterialDestination, *comp, bz)
                : componentMaterialPath(hi.layer, *comp, bz));
        anyComponentMaterial |= materialPaths.back().valid &&
            materialPaths.back().pathTX0 > m_bhSplitThresh.value();
      }
      if (truthMaterialTrackMatched && hasNextForwardSurface) {
        auto& summary = forwardMaterialSummaries[ih];
        for (std::size_t componentIndex = 0;
             componentIndex < comps.size(); ++componentIndex) {
          summary.add(materialPaths[componentIndex],
                      comps[componentIndex]->weight,
                      comps[componentIndex]->debugId,
                      m_bhSplitThresh.value());
        }
      }
      if (hasNextForwardSurface) {
        double weightedPathTX0 = 0.0;
        double validWeight = 0.0;
        for (size_t ci = 0; ci < comps.size(); ++ci) {
          if (!materialPaths[ci].valid) continue;
          weightedPathTX0 += comps[ci]->weight * materialPaths[ci].pathTX0;
          validWeight += comps[ci]->weight;
          maxTX0Layer = std::max(maxTX0Layer, materialPaths[ci].pathTX0);
        }
        if (validWeight > 0.0) totalTX0 += weightedPathTX0 / validWeight;

      }
      if (m_verboseDump && m_componentDebugDump && hasNextForwardSurface &&
          (ih < 3 || anyComponentMaterial)) {
        for (size_t ci = 0; ci < comps.size(); ++ci) {
          const auto& path = materialPaths[ci];
          info() << boost::format("  MAT-COMP hit=%d comp=%d id=%d owner=outgoing-current normalTX0=%.9g absCos=%.9g pathTX0=%.9g valid=%d")
                    % (int)ih % (int)ci % comps[ci]->debugId
                    % path.normalTX0 % path.absCosIncidence % path.pathTX0
                    % (path.valid ? 1 : 0) << endmsg;
        }
      }
      // Represent the complete outgoing process mixture.  Its children remain
      // unreduced until they have incorporated the next measurement posterior.
      if (m_forwardBHSplitting.value() && hasNextForwardSurface &&
          anyComponentMaterial && m_isElectron) {
        if (m_verboseDump && m_verboseSplitDump) {
          info() << boost::format("  ── BH Split after hit %d (r=%.1f mm, component-local path tX0) — %d comps before split")
                    % ih % hi.radius % (int)comps.size() << endmsg;
        }
        BetheHeitlerSplitter bhs(m_bhModel.value());
        std::vector<GsfComponent*> newCps;
        for (size_t componentIndex = 0; componentIndex < comps.size(); ++componentIndex) {
          auto* comp = comps[componentIndex];
          const auto& materialPath = materialPaths[componentIndex];
          if (!materialPath.valid ||
              materialPath.pathTX0 <= m_bhSplitThresh.value()) {
            newCps.push_back(comp);
            continue;
          }
          const int parentDebugId = comp->debugId;
          const int parentLineageNodeId = comp->lineageNodeId;
          const int childGeneration = comp->generation + 1;
          const double parentKappa = comp->helixAtLastSite(bz).GetKappa();
          std::vector<BetheHeitlerMixtureComponent> appliedMixture;
          auto children = applyTruthBHLossOverride
              ? bhs.splitWithRetainedFraction(
                    comp, truthRetainedFraction(
                              static_cast<int>(ih),
                              static_cast<int>(ih + 1)),
                    bz, false, &appliedMixture)
              : bhs.split(comp, materialPath.pathTX0, bz, false,
                          &appliedMixture);
          if (applyTruthBHLossOverride) ++m_truthBHLossOverrideCalls;
          if (m_verboseDump && m_verboseSplitDump && m_componentDebugDump) {
            const double parentPT = (bz != 0 && parentKappa != 0)
                ? 1.0 / std::abs(parentKappa) : 0.0;
            info() << boost::format("    filtered parent kappa=%.4e (pT=%.3f) weight=%.4f normalTX0=%.6g absCos=%.6g pathTX0=%.6g -> %d post-material children")
                      % parentKappa % parentPT % comp->weight
                      % materialPath.normalTX0 % materialPath.absCosIncidence
                      % materialPath.pathTX0 % (int)children.size() << endmsg;
            for (size_t ci = 0; ci < children.size(); ++ci) {
              const double childKappa = children[ci]->helixAtLastSite(bz).GetKappa();
              const double childPT = (bz != 0 && childKappa != 0)
                  ? 1.0 / std::abs(childKappa) : 0.0;
              const double filteredKappa =
                  children[ci]->helixAtMeasurementSite(bz).GetKappa();
              info() << boost::format("      post-material child[%d] filteredKappa=%.4e continuationKappa=%.4e pT=%.3f weight=%.4f")
                        % ci % filteredKappa % childKappa % childPT
                        % children[ci]->weight << endmsg;
            }
          }
          for (size_t childIndex = 0; childIndex < children.size();
               ++childIndex) {
            auto* child = children[childIndex];
            child->debugId = nextComponentDebugId++;
            child->debugParentId = parentDebugId;
            child->generation = childGeneration;
            if (childIndex < appliedMixture.size()) {
              child->lineageNodeId = lineageGraph.split(
                  *child, parentLineageNodeId,
                  LineageNodeSource::ForwardFiltering,
                  static_cast<int>(ih), hi.surfaceIndex,
                  static_cast<int>(childIndex),
                  appliedMixture[childIndex], materialPath.pathTX0);
            }
            if (!child->forwardProcessSignature.empty())
              child->forwardProcessSignature += ";";
            child->forwardProcessSignature += std::to_string(ih) + ":g" +
                std::to_string(childIndex);
            if (trackSurfaceLineageMass)
              child->forwardProcessModeFractions[
                  {(int)ih, (int)childIndex}] = 1.0;
            newCps.push_back(child);
          }
        }
        comps = std::move(newCps);
        compsAfterSplit = (int)comps.size();
        ++nSplits;
        justSplit = true;
        dumpComponents("after-split/raw", (int)ih, comps);
        GsfMixture::normalizeWeights(comps);
        dumpComponents("after-split/norm", (int)ih, comps);
      } else {
        compsAfterSplit = (int)comps.size();
        if (!hasNextForwardSurface && m_verboseDump && m_verboseSplitDump) {
          info() << boost::format("  FLOW hit=%3d final-surface: preserve filtered mixture; no outgoing convolution")
                    % (int)ih << endmsg;
        }
      }
      if ((int)comps.size() > maxCompsEver) maxCompsEver = (int)comps.size();
      if (m_verboseDump && m_verboseSplitDump) {
        info() << boost::format("  FLOW hit=%3d summary: begin=%2d update=%2d reduce=%2d%s split=%2d A/R/J=%d/%d/%d dchi2=[%.4g, %.4g]")
                  % (int)ih % compsAtHitBegin % compsAfterUpdate
                  % compsAfterReduce % (didReduceHit ? " yes" : " no ")
                  % compsAfterSplit
                  % nAccept % nRecover % nReject
                  % (nAccept > 0 ? minDChi2 : 0.0) % (nAccept > 0 ? maxDChi2 : 0.0) << endmsg;
      }
      nProc++;
    }

    sharedForwardResult.complete(comps);

    bool reverseIpAvailable = false;
    THelicalTrack reverseOutputIp(TMatrixD(5, 1), TVector3(0, 0, 0), bz);
    TMatrixD reverseOutputIpCov(5, 5);
    bool reverseWeightedIpAvailable = false;
    THelicalTrack reverseWeightedIp(
        TMatrixD(5, 1), TVector3(0, 0, 0), bz);
    TMatrixD reverseWeightedIpCov(5, 5);
    bool reverseFullMixtureModeIpAvailable = false;
    THelicalTrack reverseFullMixtureModeIp(
        TMatrixD(5, 1), TVector3(0, 0, 0), bz);
    TMatrixD reverseFullMixtureModeIpCov(5, 5);
    FullMixtureModeStatus reverseFullMixtureModeStatus =
        FullMixtureModeStatus::MethodEndpointUnavailable;
    FullMixtureModeDiagnostics reverseFullMixtureModeDiagnostics;
    std::vector<FinalMixtureComponentRecord>
        finalMixtureComponentRecords;
    double reverseOutputChi2 = 0.0;
    int reverseOutputNdf = 0;
    double reverseOutputWeight = 0.0;
    int reverseOutputComps = 0;
    std::string reverseOutputLabel = "ReverseMixture";
    bool ecalConstrainedIpAvailable = false;
    bool ecalConstraintActivated = false;
    THelicalTrack ecalConstrainedIp(
        TMatrixD(5, 1), TVector3(0, 0, 0), bz);
    TMatrixD ecalConstrainedIpCov(5, 5);
    double ecalConstrainedChi2 = 0.0;
    int ecalConstrainedNdf = 0;
    double ecalConstrainedEnergy = 0.0;
    int ecalConstrainedClusterCount = 0;

    // The reverse inward GSF always propagates the locally updated B_updated
    // states.  LocalMeasurement weights them with the target-hit likelihood;
    // SmoothedMarginal instead attaches the forward-marginalized interior
    // F_updated x B_predicted pair weights.  The complete smoothed mixtures
    // remain independently recorded and never replace the propagated states.
    auto runGsfInwardFilter = [&]() -> GsfInwardFilterResult {
      GsfInwardFilterResult inwardFilterResult(hits.size());
      if (!m_reverseFiltering.value() ||
          sharedForwardResult.finalComponents().empty() ||
          hits.size() <= 1) {
        return inwardFilterResult;
      }
      auto& reverseComps = inwardFilterResult.terminalBackward;
      int nextReverseId = 0;
      const auto& reverseSeedComponents =
          sharedForwardResult.finalComponents();
      const double inwardSeedCovarianceScale =
          m_inwardSeedCovarianceScale.value();
      inwardFilterResult.seedCovarianceScale =
          inwardSeedCovarianceScale;
      inwardFilterResult.seedMeasurementDimension =
          initialization.seedHitMeasurementDimension;
      if (inwardSeedCovarianceScale > 0.0) {
        for (const auto* forwardComp : reverseSeedComponents) {
          edm4hep::TrackState finalState =
              trackStateFromComponent(*forwardComp, bz, DH::AtOther);
          for (auto& covariance : finalState.covMatrix)
            covariance *= inwardSeedCovarianceScale;
          auto* reverseComp = new GsfComponent();
          reverseComp->weight =
              (m_reverseInitialWeightMode.value() == "Uniform" ||
               m_reverseInitialWeightMode.value() == "uniform")
                  ? 1.0
                  : forwardComp->weight;
          reverseComp->charge = forwardComp->charge;
          reverseComp->debugId = nextReverseId++;
          reverseComp->noRadiationLineage =
              forwardComp->noRadiationLineage;
          reverseComp->forwardProcessSignature =
              forwardComp->forwardProcessSignature;
          if (trackSurfaceLineageMass)
            reverseComp->forwardProcessModeFractions =
                forwardComp->forwardProcessModeFractions;
          reverseComp->debugHistory =
              "reverse(" + forwardComp->debugHistory + ")";
          reverseComp->kaltrack = new TKalTrack();
          reverseComp->kaltrack->SetOwner();
          auto* reverseSite = makeInitialSiteFromTrackState(
              finalState, hits.back(), bz);
          if (!reverseSite) {
            delete reverseComp;
            continue;
          }
          reverseComp->kaltrack->Add(reverseSite);
          reverseComp->continuationState = finalState;
          reverseComp->continuationValid = true;
          reverseComp->lineageNodeId = lineageGraph.reverseSeed(
              *reverseComp, forwardComp->lineageNodeId,
              static_cast<int>(hits.size()) - 1,
              hits.back().surfaceIndex);
          reverseComps.push_back(reverseComp);
        }
      } else {
        inwardFilterResult.freshSeedInitialization = true;
        auto inwardInitialization = initializer.initialize(
            orderedHits, *hits.back().layer, *hits.back().kalHit, bz,
            GsfTrackInitializationDirection::Inward,
            m_kappaSeedCov.value());
        if (!inwardInitialization.valid()) {
          warning() << "GSF standard-KF-style inward initialization failed "
                    << "for event=" << eventIndex
                    << " inputTrack=" << inputTrackIndex << ": "
                    << inwardInitialization.error << endmsg;
          return inwardFilterResult;
        }
        inwardFilterResult.seedMeasurementDimension =
            inwardInitialization.seedHitMeasurementDimension;
        auto* reverseComp = new GsfComponent();
        reverseComp->weight = 1.0;
        reverseComp->charge =
            inwardInitialization.seedFilteredState.omega > 0.0 ? 1 : -1;
        reverseComp->debugId = nextReverseId++;
        reverseComp->noRadiationLineage = true;
        reverseComp->debugHistory = "inward-standard-kf-seed";
        reverseComp->fitChi2 = inwardInitialization.seedHitDeltaChi2;
        reverseComp->kaltrack = new TKalTrack();
        reverseComp->kaltrack->SetOwner();
        reverseComp->kaltrack->Add(inwardInitialization.site);
        reverseComp->continuationState =
            inwardInitialization.seedFilteredState;
        reverseComp->continuationValid = true;
        reverseComp->lineageNodeId = lineageGraph.seed(
            *reverseComp, LineageNodeSource::ReverseFiltering,
            static_cast<int>(hits.size()) - 1,
            hits.back().surfaceIndex);
        reverseComps.push_back(reverseComp);
        if (m_verboseDump) {
          info() << boost::format(
              "  INWARD INIT standard-kf-prefit twoDHits=%d "
              "outerHitDChi2=%.9g outerHitNdf=%d outerHitDim=%d "
              "prefitOmega=%.9g filteredOmega=%.9g "
              "prefitVarOmega=%.9g prefitVarKappa=%.9g")
                    % inwardInitialization.twoDimensionalHitCount
                    % inwardInitialization.seedHitDeltaChi2
                    % inwardInitialization.seedHitNdf
                    % inwardInitialization.seedHitMeasurementDimension
                    % inwardInitialization.prefitState.omega
                    % inwardInitialization.seedFilteredState.omega
                    % inwardInitialization.prefitOmegaVariance
                    % inwardInitialization.prefitKappaVariance << endmsg;
        }
      }
      GsfMixture::normalizeWeights(reverseComps);
      for (const auto* component : reverseComps)
        lineageGraph.setWeight(component->lineageNodeId,
                               component->weight);
      if (m_verboseDump && m_verboseSplitDump)
        dumpComponents("reverse-start", (int)hits.size() - 1, reverseComps);

      auto& reverseAcceptedTotal =
          inwardFilterResult.acceptedMeasurements;
      auto& reverseRejectedTotal =
          inwardFilterResult.rejectedMeasurements;
      auto& reverseSplits = inwardFilterResult.splits;
      auto& reverseReductions = inwardFilterResult.reductions;
      const int reverseReductionTarget =
          (m_reductionTargetComponents.value() > 0)
              ? std::min(m_reductionTargetComponents.value(),
                         m_maxComponents.value())
              : m_maxComponents.value();
      std::string inwardWeightMode = m_inwardWeightMode.value();
      std::transform(inwardWeightMode.begin(), inwardWeightMode.end(),
                     inwardWeightMode.begin(), ::tolower);
      const bool useSmoothedMarginalWeights =
          inwardWeightMode == "smoothedmarginal";
      for (int reverseHit = (int)hits.size() - 2;
           reverseHit >= 0 && !reverseComps.empty(); --reverseHit) {
        auto& target = hits[reverseHit];

        // Propagate the full mixture through the outer-to-inner interval
        // before updating its inner bounding measurement.  This mirrors the
        // ACTS backward actor and is the direction-reversed counterpart of the
        // forward update-then-outgoing-material ordering.
        std::vector<ComponentMaterialPath> reverseMaterialPaths;
        reverseMaterialPaths.reserve(reverseComps.size());
        bool anyReverseMaterial = false;
        const auto& reverseTargetPosition = target.lcioHit.getPosition();
        const TVector3 reverseMaterialDestination(
            reverseTargetPosition.x, reverseTargetPosition.y,
            reverseTargetPosition.z);
        TVector3 reverseMaterialOuterEndpoint;
        const auto& reverseOuterPosition =
            hits[static_cast<size_t>(reverseHit + 1)].lcioHit.getPosition();
        reverseMaterialOuterEndpoint.SetXYZ(
            reverseOuterPosition.x, reverseOuterPosition.y,
            reverseOuterPosition.z);
        for (const auto* component : reverseComps) {
          reverseMaterialPaths.push_back(
              useDD4hepBetweenSurfaces
                  ? componentGeometryTransitionMaterialPath(
                        m_materialManager, target.layer,
                        reverseMaterialDestination, *component, bz, -1,
                        &reverseMaterialOuterEndpoint)
                  : componentMaterialPathAtCrossing(
                        target.layer, *component, bz, -1));
          anyReverseMaterial |= reverseMaterialPaths.back().valid &&
              reverseMaterialPaths.back().pathTX0 > m_bhSplitThresh.value();
        }
        if (truthMaterialTrackMatched) {
          auto& summary = reverseMaterialSummaries[
              static_cast<std::size_t>(reverseHit)];
          for (std::size_t componentIndex = 0;
               componentIndex < reverseComps.size(); ++componentIndex) {
            summary.add(reverseMaterialPaths[componentIndex],
                        reverseComps[componentIndex]->weight,
                        reverseComps[componentIndex]->debugId,
                        m_bhSplitThresh.value());
          }
        }
        if (m_verboseDump && m_verboseSplitDump && m_componentDebugDump) {
          for (size_t componentIndex = 0;
               componentIndex < reverseComps.size(); ++componentIndex) {
            const auto& path = reverseMaterialPaths[componentIndex];
            info() << boost::format(
                "  MAT-REVERSE-COMP hit=%d comp=%d id=%d mode=%s "
                "owner=outgoing-target normalTX0=%.9g absCos=%.9g "
                "pathTX0=%.9g valid=%d")
                      % reverseHit % componentIndex
                      % reverseComps[componentIndex]->debugId
                      % m_materialPathMode.value() % path.normalTX0
                      % path.absCosIncidence % path.pathTX0
                      % (path.valid ? 1 : 0) << endmsg;
          }
        }
        if (m_inwardBHSplitting.value() && anyReverseMaterial && m_isElectron) {
          BetheHeitlerSplitter splitter(m_bhModel.value());
          std::vector<GsfComponent*> reverseChildren;
          for (size_t componentIndex = 0;
               componentIndex < reverseComps.size(); ++componentIndex) {
            auto* parent = reverseComps[componentIndex];
            const auto& materialPath = reverseMaterialPaths[componentIndex];
            if (!materialPath.valid ||
                materialPath.pathTX0 <= m_bhSplitThresh.value()) {
              reverseChildren.push_back(parent);
              continue;
            }
            const int parentId = parent->debugId;
            const int parentLineageNodeId = parent->lineageNodeId;
            const int childGeneration = parent->generation + 1;
            const double alpha = bz * 2.99792458e-4;
            const double parentPt = parent->continuationState.omega != 0.0
                ? std::abs(alpha / parent->continuationState.omega) : 0.0;
            std::vector<BetheHeitlerMixtureComponent> appliedMixture;
            auto children = applyTruthBHLossOverride
                ? splitter.splitWithRetainedFraction(
                      parent, truthRetainedFraction(
                                  reverseHit, reverseHit + 1),
                      bz, true, &appliedMixture)
                : splitter.split(parent, materialPath.pathTX0, bz, true,
                                 &appliedMixture);
            if (applyTruthBHLossOverride) ++m_truthBHLossOverrideCalls;
            for (size_t childIndex = 0; childIndex < children.size(); ++childIndex) {
              auto* child = children[childIndex];
              child->debugParentId = parentId;
              child->debugId = nextReverseId++;
              child->generation = childGeneration;
              if (childIndex < appliedMixture.size()) {
                child->lineageNodeId = lineageGraph.split(
                    *child, parentLineageNodeId,
                    LineageNodeSource::ReverseFiltering, reverseHit,
                    target.surfaceIndex, static_cast<int>(childIndex),
                    appliedMixture[childIndex], materialPath.pathTX0);
              }
              child->lastReverseProcessHit = reverseHit;
              child->lastReverseProcessComponent = static_cast<int>(childIndex);
              const double childPt = child->continuationState.omega != 0.0
                  ? std::abs(alpha / child->continuationState.omega) : 0.0;
              child->lastReverseProcessFraction = childPt > 0.0
                  ? parentPt / childPt : 1.0;
              if (!child->reverseProcessSignature.empty())
                child->reverseProcessSignature += ";";
              child->reverseProcessSignature += std::to_string(reverseHit) +
                  ":g" + std::to_string(childIndex) + ":f" +
                  std::to_string(child->lastReverseProcessFraction);
              if (trackSurfaceLineageMass)
                child->reverseProcessModeFractions[
                    {reverseHit, (int)childIndex}] = 1.0;
              child->debugHistory += "->reverse-material[h=" +
                  std::to_string(reverseHit) + "]";
              reverseChildren.push_back(child);
            }
          }
          reverseComps = std::move(reverseChildren);
          ++reverseSplits;
          GsfMixture::normalizeWeights(reverseComps);
        }

        struct ReverseMeasurementCandidate {
          GsfComponent* component = nullptr;
          int parentLineageNodeId = -1;
          double priorWeight = 0.0;
          double dchi = 0.0;
          double updateChi2 = 0.0;
          int updateNdf = -999;
          edm4hep::TrackState componentState;
          edm4hep::TrackState updatedState;
          MarlinTrk::MeasurementUpdate update;
          bool evaluated = false;
        };
        std::vector<ReverseMeasurementCandidate> reverseCandidates;
        reverseCandidates.reserve(reverseComps.size());
        std::vector<GaussianComponentSnapshot> backwardPredictedComponents;
        backwardPredictedComponents.reserve(reverseComps.size());

        // Evaluate each temporary MarlinTrk update without mutating the live
        // reverse component.  The exact predicted and updated states returned
        // by the same baseline operation are buffered so B_smoothed[i] and
        // B_updated[i] can branch independently from B_predicted[i].
        for (auto* component : reverseComps) {
          ReverseMeasurementCandidate candidate;
          candidate.component = component;
          candidate.parentLineageNodeId = component->lineageNodeId;
          candidate.priorWeight = component->weight;
          candidate.componentState =
              trackStateFromComponent(*component, bz, DH::AtOther);
          try {
            const int referenceIndex = std::min(
                reverseHit + 1, (int)hits.size() - 1);
            edm4hep::TrackerHit referenceHit = hits[referenceIndex].lcioHit;
            edm4hep::TrackerHit targetHit = target.lcioHit;
            std::unique_ptr<MarlinTrk::IMarlinTrack> reverseTrack(
                m_gsfMarlinTrkSystem->createTrack());
            if (reverseTrack &&
                reverseTrack->addHit(referenceHit) == MarlinTrk::IMarlinTrack::success &&
                reverseTrack->initialise(candidate.componentState, bz,
                    MarlinTrk::IMarlinTrack::backward) == MarlinTrk::IMarlinTrack::success &&
                reverseTrack->addAndFit(
                    targetHit, candidate.dchi, candidate.update, DBL_MAX) ==
                    MarlinTrk::IMarlinTrack::success &&
                candidate.update.valid &&
                reverseTrack->getTrackState(
                    targetHit, candidate.updatedState, candidate.updateChi2,
                    candidate.updateNdf) == MarlinTrk::IMarlinTrack::success) {
              candidate.evaluated = true;
            }
          } catch (...) {
            candidate.evaluated = false;
          }

          if (candidate.evaluated && reverseHit > 0) {
            GaussianComponentSnapshot snapshot;
            snapshot.weight = candidate.priorWeight;
            snapshot.componentId = component->debugId;
            // This is the pre-measurement backward message.  Its smoothing edge
            // must originate from the split/previous-update node, not from the
            // measurement node that will be created for B_updated[i] below.
            snapshot.lineageNodeId = candidate.parentLineageNodeId;
            snapshot.noRadiationLineage = component->noRadiationLineage;
            snapshot.state.mean = updateVector5(
                candidate.update.predictedState);
            snapshot.state.covariance =
                updateMatrix5(candidate.update.predictedCovariance);
            snapshot.state.valid =
                std::isfinite(snapshot.state.mean(2, 0)) &&
                std::isfinite(snapshot.state.covariance(2, 2));
            if (snapshot.state.valid)
              backwardPredictedComponents.push_back(std::move(snapshot));
          }

          reverseCandidates.push_back(std::move(candidate));
        }

        // The interior smoothed state is a non-propagated sibling of the live
        // measurement update:
        //   B_smoothed[i] = F_updated[i] x B_predicted[i]
        // It is materialized immediately from immutable prediction snapshots,
        // before any component is committed to B_updated[i].  The explicit
        // SmoothedMarginal mode uses only its direct pair weights, marginalized
        // by backward parent; the propagated state remains B_updated[i].
        if (reverseHit > 0) {
          auto& smoothedSurface = inwardFilterResult.smoothedSurfaces[
              static_cast<std::size_t>(reverseHit)];
          smoothedSurface = buildSmoothedSurfaceMixture(
              sharedForwardResult.filteredAt(
                  static_cast<std::size_t>(reverseHit)),
              backwardPredictedComponents, reverseHit, target.surfaceIndex,
              reverseMaterialDestination, bz, reverseReductionTarget,
              m_componentWeightCutoff.value(),
              m_protectIdentityLineage.value(),
              m_reductionMergeCost.value(), lineageGraph);
          if (m_verboseDump) {
            info() << boost::format(
                "  SMOOTHED MIXTURE hit=%d forward=%d backward=%d "
                "pairs=%d failures=%d retained=%d")
                      % reverseHit
                      % (int)sharedForwardResult.filteredAt(
                            static_cast<std::size_t>(reverseHit)).size()
                      % (int)backwardPredictedComponents.size()
                      % smoothedSurface.pairCandidates
                      % smoothedSurface.pairFailures
                      % (int)smoothedSurface.components.size() << endmsg;
          }
        }

        std::vector<GsfComponent*> acceptedReverse;
        std::vector<double> reverseLogWeights;
        acceptedReverse.reserve(reverseCandidates.size());
        reverseLogWeights.reserve(reverseCandidates.size());
        for (auto& candidate : reverseCandidates) {
          auto* component = candidate.component;
          bool accepted = false;
          if (candidate.evaluated) {
            try {
              accepted = appendBaselineStateToComponent(
                  *component, candidate.updatedState, target, bz,
                  candidate.componentState, candidate.update);
            } catch (...) {
              accepted = false;
            }
          }

          const double unavailable =
              std::numeric_limits<double>::quiet_NaN();
          const double localMeasurementLogPosterior = accepted
              ? std::log(candidate.priorWeight) -
                    0.5 * (candidate.dchi +
                           candidate.update.logDetInnovation)
              : unavailable;
          double smoothedMarginalWeight = unavailable;
          if (accepted && useSmoothedMarginalWeights && reverseHit > 0) {
            const auto& marginalWeights =
                inwardFilterResult.smoothedSurfaces[
                    static_cast<std::size_t>(reverseHit)]
                    .backwardMarginalWeights;
            const auto marginal = marginalWeights.find(component->debugId);
            if (marginal != marginalWeights.end() &&
                marginal->second > 0.0 &&
                std::isfinite(marginal->second)) {
              smoothedMarginalWeight = marginal->second;
            } else {
              accepted = false;
            }
          }
          const double selectedReverseLogWeight = accepted
              ? (useSmoothedMarginalWeights && reverseHit > 0
                    ? std::log(smoothedMarginalWeight)
                    : localMeasurementLogPosterior)
              : unavailable;
          component->lineageNodeId = lineageGraph.measurement(
              *component, candidate.parentLineageNodeId,
              LineageNodeSource::ReverseFiltering, reverseHit,
              target.surfaceIndex, accepted ? 1 : 0,
              candidate.priorWeight,
              candidate.update.valid ? candidate.dchi : unavailable,
              candidate.update.valid
                  ? candidate.update.logDetInnovation : unavailable,
              localMeasurementLogPosterior, &candidate.update);

          if (accepted) {
            component->fitChi2 += candidate.dchi;
            reverseLogWeights.push_back(selectedReverseLogWeight);
            acceptedReverse.push_back(component);
            ++reverseAcceptedTotal;
            if (m_verboseDump && m_verboseSplitDump && m_componentDebugDump) {
              info() << boost::format("      REVERSE UPDATE accept hit=%d id=%d pT=%.6g priorWeight=%.6g dchi2=%.6g logDetS=%.6g weightMode=%s smoothedMarginal=%.6g")
                        % reverseHit % component->debugId
                        % ptFromTrackState(candidate.updatedState, bz)
                        % candidate.priorWeight % candidate.dchi
                        % candidate.update.logDetInnovation
                        % (useSmoothedMarginalWeights && reverseHit > 0
                               ? "SmoothedMarginal" : "LocalMeasurement")
                        % smoothedMarginalWeight << endmsg;
              info() << boost::format("          reverse-exact-measurement: predicted=%s residual=%s H=%s R=%s S=%s")
                        % compactMatrix(candidate.update.predictedMeasurement)
                        % compactMatrix(candidate.update.residual)
                        % compactMatrix(candidate.update.projector)
                        % compactMatrix(candidate.update.measurementCovariance)
                        % compactMatrix(candidate.update.innovationCovariance)
                     << endmsg;
            }
          } else {
            delete component;
            ++reverseRejectedTotal;
          }
        }

        reverseComps.clear();
        if (acceptedReverse.empty()) break;
        const double maxReverseLog = *std::max_element(
            reverseLogWeights.begin(), reverseLogWeights.end());
        for (std::size_t i = 0; i < acceptedReverse.size(); ++i)
          acceptedReverse[i]->weight =
              std::exp(reverseLogWeights[i] - maxReverseLog);
        reverseComps = std::move(acceptedReverse);
        GsfMixture::normalizeWeights(reverseComps);
        for (const auto* component : reverseComps)
          lineageGraph.setNormalizedPosterior(
              component->lineageNodeId, component->weight);

        // Let every reverse-process child incorporate the inward target hit
        // before cutoff or mixture reduction.  The Gaussian states below are
        // always B_updated target-surface states.  Their normalized weights
        // are either local-measurement posteriors or the selected interior
        // smoothed marginals.
        if (m_verboseDump && m_verboseSplitDump)
          dumpComponents("reverse-posterior/norm", reverseHit, reverseComps);
        const int reverseBeforeCutoff = (int)reverseComps.size();
        auto reverseCutoffObserver = [&](const GsfComponent& component) {
          lineageGraph.mark(component.lineageNodeId,
                            LineageNodeFate::WeightCutoff);
        };
        GsfMixture::removeLowWeight(
            reverseComps, m_componentWeightCutoff.value(),
            m_protectIdentityLineage.value(), reverseCutoffObserver);
        if (m_verboseDump && m_verboseSplitDump &&
            (int)reverseComps.size() != reverseBeforeCutoff) {
          info() << boost::format(
              "  REVERSE FLOW hit=%3d posterior-cutoff: n=%d -> %d threshold=%.3g")
                    % reverseHit % reverseBeforeCutoff
                    % (int)reverseComps.size()
                    % m_componentWeightCutoff.value() << endmsg;
          dumpComponents("reverse-posterior-cutoff", reverseHit,
                         reverseComps);
        }
        if ((int)reverseComps.size() > reverseReductionTarget) {
          if (m_verboseDump && m_verboseSplitDump) {
            info() << boost::format(
                "  REVERSE FLOW hit=%3d posterior-reduce: n=%d target=%d cost=%s")
                % reverseHit % (int)reverseComps.size()
                % reverseReductionTarget % m_reductionMergeCost.value()
                << endmsg;
            dumpComponents("reverse-pre-reduce", reverseHit, reverseComps);
          }
          auto reverseReductionLogger = [&](const std::string& line) {
            if (m_verboseDump && m_verboseSplitDump) info() << line << endmsg;
          };
          auto reverseReductionObserver = [&](GsfComponent& merged,
                                              int keepSourceNodeId,
                                              int dropSourceNodeId,
                                              double mergeCost) {
            merged.lineageNodeId = lineageGraph.merge(
                merged, keepSourceNodeId, dropSourceNodeId,
                LineageNodeSource::ReverseFiltering, reverseHit,
                target.surfaceIndex, mergeCost);
          };
          GsfMixture::reduce(
              reverseComps, reverseReductionTarget, bz,
              m_protectIdentityLineage.value(), reverseReductionLogger,
              m_reductionMergeCost.value(), reverseReductionObserver);
          ++reverseReductions;
          if (m_verboseDump && m_verboseSplitDump)
            dumpComponents("reverse-post-reduce", reverseHit, reverseComps);
          GsfMixture::normalizeWeights(reverseComps);
          for (const auto* component : reverseComps)
            lineageGraph.setWeight(component->lineageNodeId,
                                   component->weight);
          if (m_verboseDump && m_verboseSplitDump)
            dumpComponents("reverse-post-reduce/norm", reverseHit,
                           reverseComps);
        }

        if (m_verboseDump && m_verboseSplitDump) {
          dumpComponents("reverse-after-hit", reverseHit, reverseComps);
        }
      }

      if (m_verboseDump) {
        info() << boost::format(
            "  REVERSE summary: finalComps=%d accepted=%d rejected=%d "
            "splits=%d reductions=%d seedMode=%s")
                  % (int)reverseComps.size() % reverseAcceptedTotal
                  % reverseRejectedTotal % reverseSplits % reverseReductions
                  % (inwardFilterResult.freshSeedInitialization
                         ? "fresh-standard-kf" : "forward-copy")
               << endmsg;
      }

      return inwardFilterResult;
    };

    auto inwardFilterResult = runGsfInwardFilter();
    auto& reverseComps = inwardFilterResult.terminalBackward;
    if (m_reverseFiltering.value() &&
        !sharedForwardResult.finalComponents().empty() &&
        hits.size() > 1) {
      GsfMixture::normalizeWeights(reverseComps);
      // Reverse publishes the terminal inward mixture, which is also the
      // inner boundary smoothed state:
      // B_smoothed[0] = B_updated[0] = measurement[0] x B_predicted[0].
      // Explicit F_updated[i] x B_predicted[i] products exist only at interior
      // surfaces; B_smoothed[N-1] is the final forward mixture.
      auto& endpointComponents = reverseComps;
      for (const auto& surface : inwardFilterResult.smoothedSurfaces)
        lineageGraph.markInwardInternalMessage(surface.components);
      GsfMixture::normalizeWeights(endpointComponents);
      if (!endpointComponents.empty()) {
        std::string reverseSelectionMode = m_reverseSelectionMode.value();
        std::transform(reverseSelectionMode.begin(), reverseSelectionMode.end(),
                       reverseSelectionMode.begin(), ::tolower);
        const bool selectDominantLineage =
            reverseSelectionMode == "dominantlineage";
        auto surfaceCoincidenceProbability = [](const GsfComponent* component) {
          std::map<int, double> forwardRadiativeMass;
          std::map<int, double> reverseRadiativeMass;
          for (const auto& item : component->forwardProcessModeFractions) {
            if (item.first.second > 0)
              forwardRadiativeMass[item.first.first] += item.second;
          }
          for (const auto& item : component->reverseProcessModeFractions) {
            if (item.first.second > 0)
              reverseRadiativeMass[item.first.first] += item.second;
          }
          double noCoincidence = 1.0;
          for (const auto& item : forwardRadiativeMass) {
            const auto reverse = reverseRadiativeMass.find(item.first);
            if (reverse == reverseRadiativeMass.end()) continue;
            const double forwardMass = std::clamp(item.second, 0.0, 1.0);
            const double reverseMass = std::clamp(reverse->second, 0.0, 1.0);
            noCoincidence *= 1.0 - forwardMass * reverseMass;
          }
          return std::clamp(1.0 - noCoincidence, 0.0, 1.0);
        };
        auto surfaceConsistencyLikelihood = [&](const GsfComponent* component) {
          const double floor =
              m_surfaceConsistencyUninformativeFloor.value();
          return floor + (1.0 - floor) *
              surfaceCoincidenceProbability(component);
        };
        auto reverseSelectionScore = [&](const GsfComponent* component) {
          if (selectDominantLineage)
            return component->weight * component->dominantLineageFraction;
          if (selectSurfaceConsistency)
            return component->weight *
                surfaceConsistencyLikelihood(component);
          return component->weight;
        };
        auto* reverseBest = *std::max_element(
            endpointComponents.begin(), endpointComponents.end(),
            [&](const GsfComponent* a, const GsfComponent* b) {
              return reverseSelectionScore(a) < reverseSelectionScore(b);
            });
        THelicalTrack reverseIp(TMatrixD(5, 1), TVector3(0, 0, 0), bz);
        TMatrixD reverseIpCov(5, 5);
        bool reverseOutputOk = false;
        reverseOutputOk = extrapolateContinuationToIP(
            *reverseBest, bz, reverseIp, reverseIpCov);
        reverseOutputLabel = "InwardBestBranch";
        reverseWeightedIpAvailable = weightedReverseMixtureAtIP(
            endpointComponents, bz, reverseWeightedIp,
            reverseWeightedIpCov);
        if (!reverseWeightedIpAvailable) {
          warning() << "Reverse WeightedMean publication failed; the paired "
                       "collection will preserve the BestBranch state"
                    << endmsg;
        }
        reverseFullMixtureModeStatus = findFullMixtureModeAtIP(
            endpointComponents, bz,
            [bz](GsfComponent& component, THelicalTrack& helix,
                 TMatrixD& covariance) {
              return extrapolateContinuationToIP(
                  component, bz, helix, covariance);
            },
            reverseFullMixtureModeIp, reverseFullMixtureModeIpCov,
            reverseFullMixtureModeDiagnostics);
        reverseFullMixtureModeIpAvailable =
            reverseFullMixtureModeStatus ==
            FullMixtureModeStatus::Success;
        if (!reverseFullMixtureModeIpAvailable) {
          warning() << "Reverse FullMixtureMode publication failed with status "
                    << fullMixtureModeStatusValue(
                           reverseFullMixtureModeStatus)
                    << "; the paired collection will preserve the "
                       "BestBranch state"
                    << endmsg;
        }
        if (reverseOutputOk) {
          lineageGraph.markFinal(endpointComponents, reverseBest);
          if (m_reverseFiltering.value()) {
            finalMixtureComponentRecords =
                captureFinalMixtureComponentsAtIP(
                    endpointComponents, bz,
                    FinalMixtureComponentSource::ReverseFiltering,
                    [bz](GsfComponent& component, THelicalTrack& helix,
                         TMatrixD& covariance) {
                      return extrapolateContinuationToIP(
                          component, bz, helix, covariance);
                    });
          }
          const double reversePt = reverseIp.GetKappa() != 0.0
              ? 1.0 / std::abs(reverseIp.GetKappa()) : 0.0;
          if (m_verboseDump) {
            info() << boost::format("  REVERSE IP output: mode=%s selection=%s bestId=%d bestWeight=%.6g dominantFraction=%.6g selectionScore=%.6g pT=%.6g d0=%.6g z0=%.6g phi=%.6g tanL=%.6g")
                      % reverseOutputLabel % m_reverseSelectionMode.value()
                      % reverseBest->debugId
                      % reverseBest->weight
                      % reverseBest->dominantLineageFraction
                      % reverseSelectionScore(reverseBest) % reversePt
                      % (-reverseIp.GetDrho()) % reverseIp.GetDz()
                      % normalizePhi(reverseIp.GetPhi0() + M_PI / 2.0)
                      % reverseIp.GetTanLambda() << endmsg;
            if (selectSurfaceConsistency) {
              info() << boost::format(
                  "  REVERSE surface-consistency: coincidence=%.9g "
                  "likelihood=%.9g floor=%.9g maxBayesFactor=%.9g")
                    % surfaceCoincidenceProbability(reverseBest)
                    % surfaceConsistencyLikelihood(reverseBest)
                    % m_surfaceConsistencyUninformativeFloor.value()
                    % (1.0 / m_surfaceConsistencyUninformativeFloor.value())
                    << endmsg;
            }
            if (m_componentDebugDump) {
              info() << boost::format("  REVERSE SELECTED process-signature=%s")
                        % reverseBest->reverseProcessSignature << endmsg;
              info() << boost::format(
                  "  REVERSE SELECTED forward-seed-process-signature=%s")
                        % reverseBest->forwardProcessSignature << endmsg;
              info() << boost::format("  REVERSE SELECTED branch id=%d full-history=%s")
                        % reverseBest->debugId
                        % truncateHistory(reverseBest->debugHistory) << endmsg;
            }
          }
          reverseOutputIp = reverseIp;
          reverseOutputIpCov = reverseIpCov;
          reverseOutputChi2 = componentFitChi2(*reverseBest);
          reverseOutputNdf = reverseBest->kaltrack
              ? reverseBest->kaltrack->GetNDF() +
                    inwardFilterResult.seedMeasurementDimension
              : 0;
          reverseOutputWeight = reverseBest->weight;
          reverseOutputComps = (int)endpointComponents.size();
          reverseIpAvailable = true;

          if (m_verboseDump && reverseWeightedIpAvailable) {
            const double weightedPt = reverseWeightedIp.GetKappa() != 0.0
                ? 1.0 / std::abs(reverseWeightedIp.GetKappa()) : 0.0;
            info() << boost::format(
                "  REVERSE IP paired output: mode=%s "
                "components=%d pT=%.6g d0=%.6g z0=%.6g phi=%.6g "
                "tanL=%.6g")
                      % "ReverseWeightedMean"
                      % (int)endpointComponents.size() % weightedPt
                      % (-reverseWeightedIp.GetDrho())
                      % reverseWeightedIp.GetDz()
                      % normalizePhi(reverseWeightedIp.GetPhi0() + M_PI / 2.0)
                      % reverseWeightedIp.GetTanLambda() << endmsg;
          }
          if (m_verboseDump && reverseFullMixtureModeIpAvailable) {
            const double modePt = reverseFullMixtureModeIp.GetKappa() != 0.0
                ? 1.0 / std::abs(reverseFullMixtureModeIp.GetKappa()) : 0.0;
            info() << boost::format(
                "  REVERSE IP paired output: mode=FullMixtureMode "
                "components=%d starts=%d maxima=%d iterations=%d "
                "logDensity=%.9g scaledGradient=%.3g pT=%.9g")
                      % reverseFullMixtureModeDiagnostics.usableComponents
                      % reverseFullMixtureModeDiagnostics.starts
                      % reverseFullMixtureModeDiagnostics.maxima
                      % reverseFullMixtureModeDiagnostics.iterations
                      % reverseFullMixtureModeDiagnostics.logDensity
                      % reverseFullMixtureModeDiagnostics.scaledGradient
                      % modePt
                   << endmsg;
          }

          // Preserve the tracker-only result above.  The default-off ECAL
          // experiment produces a separate paired output and changes only
          // the final component posterior score, never a component state or
          // covariance.  The ECAL observation is built without LCIO/PFO
          // momentum: sum clusters around the extrapolated outer forward-GSF
          // direction, then use a symmetric log(p/E) likelihood.
          if (m_ecalComponentConstraint.value()) {
            ecalConstrainedIp = reverseOutputIp;
            ecalConstrainedIpCov = reverseOutputIpCov;
            ecalConstrainedChi2 = reverseOutputChi2;
            ecalConstrainedNdf = reverseOutputNdf;
            ecalConstrainedIpAvailable = true;

            const GsfComponent* outerReference = nullptr;
            for (const auto* component : comps) {
              if (!component || !component->kaltrack ||
                  component->kaltrack->GetEntriesFast() <= 1)
                continue;
              if (!outerReference || component->weight > outerReference->weight)
                outerReference = component;
            }

            bool ecalObservationValid = false;
            double outerReferencePhi = 0.0;
            if (outerReference && m_ecalClusters.isValid()) {
              const auto outerState = trackStateFromComponent(
                  *outerReference, bz, DH::AtOther);
              const double x0 = outerState.referencePoint.x;
              const double y0 = outerState.referencePoint.y;
              const double r0sq = x0 * x0 + y0 * y0;
              const double ux = std::cos(outerState.phi);
              const double uy = std::sin(outerState.phi);
              const double rdotu = x0 * ux + y0 * uy;
              const auto* clusters = m_ecalClusters.get();
              if (clusters && r0sq > 0.0) {
                for (const auto& cluster : *clusters) {
                  const auto& position = cluster.getPosition();
                  const double clusterRadius =
                      std::hypot(position.x, position.y);
                  const double discriminant = rdotu * rdotu +
                      clusterRadius * clusterRadius - r0sq;
                  if (!(clusterRadius > std::sqrt(r0sq)) ||
                      discriminant < 0.0)
                    continue;
                  const double flight = -rdotu + std::sqrt(discriminant);
                  if (!(flight >= 0.0)) continue;
                  const double predictedX = x0 + flight * ux;
                  const double predictedY = y0 + flight * uy;
                  const double predictedZ =
                      outerState.referencePoint.z +
                      flight * outerState.tanLambda;
                  const double predictedPhi =
                      std::atan2(predictedY, predictedX);
                  const double predictedTheta = std::atan2(
                      std::hypot(predictedX, predictedY), predictedZ);
                  const double clusterPhi =
                      std::atan2(position.y, position.x);
                  const double clusterTheta =
                      std::atan2(clusterRadius, position.z);
                  const double deltaPhi = std::abs(
                      normalizePhi(clusterPhi - predictedPhi));
                  const double deltaTheta =
                      std::abs(clusterTheta - predictedTheta);
                  if (deltaPhi > m_ecalConstraintPhiWindow.value() ||
                      deltaTheta > m_ecalConstraintThetaWindow.value())
                    continue;
                  const double energy = cluster.getEnergy();
                  if (!std::isfinite(energy) || !(energy > 0.0)) continue;
                  ecalConstrainedEnergy += energy;
                  ++ecalConstrainedClusterCount;
                  outerReferencePhi = predictedPhi;
                }
                ecalObservationValid = ecalConstrainedClusterCount > 0 &&
                    std::isfinite(ecalConstrainedEnergy) &&
                    ecalConstrainedEnergy > 0.0;
              }
            }

            const double baselinePt = reverseIp.GetKappa() != 0.0
                ? 1.0 / std::abs(reverseIp.GetKappa()) : 0.0;
            const double baselineP = baselinePt *
                std::sqrt(1.0 + reverseIp.GetTanLambda() *
                                    reverseIp.GetTanLambda());
            const double baselineRatio = ecalObservationValid && baselineP > 0.0
                ? std::max(baselineP / ecalConstrainedEnergy,
                           ecalConstrainedEnergy / baselineP)
                : 0.0;
            ecalConstraintActivated = ecalObservationValid &&
                baselineRatio > m_ecalConstraintRatioThreshold.value();

            if (ecalConstraintActivated) {
              GsfComponent* ecalBest = reverseBest;
              double ecalBestScore = -1.0;
              double ecalBestLikelihood = 1.0;
              THelicalTrack ecalBestIp(
                  TMatrixD(5, 1), TVector3(0, 0, 0), bz);
              TMatrixD ecalBestCov(5, 5);
              bool ecalBestIpValid = false;
              for (auto* component : endpointComponents) {
                THelicalTrack componentIp(
                    TMatrixD(5, 1), TVector3(0, 0, 0), bz);
                TMatrixD componentCov(5, 5);
                if (!extrapolateContinuationToIP(
                        *component, bz, componentIp, componentCov))
                  continue;
                const double componentPt = componentIp.GetKappa() != 0.0
                    ? 1.0 / std::abs(componentIp.GetKappa()) : 0.0;
                const double componentP = componentPt *
                    std::sqrt(1.0 + componentIp.GetTanLambda() *
                                        componentIp.GetTanLambda());
                if (!(componentP > 0.0) || !std::isfinite(componentP))
                  continue;
                const double logResidual =
                    std::log(componentP / ecalConstrainedEnergy);
                const double pull =
                    logResidual / m_ecalConstraintLogPSigma.value();
                const double likelihood =
                    m_ecalConstraintLikelihoodFloor.value() +
                    (1.0 - m_ecalConstraintLikelihoodFloor.value()) *
                        std::exp(-0.5 * pull * pull);
                const double score =
                    reverseSelectionScore(component) * likelihood;
                if (m_verboseDump && m_componentDebugDump) {
                  info() << boost::format(
                      "  ECAL COMPONENT id=%d p=%.9g trackerScore=%.9g "
                      "logPoverE=%.9g likelihood=%.9g constrainedScore=%.9g")
                            % component->debugId % componentP
                            % reverseSelectionScore(component) % logResidual
                            % likelihood % score
                         << endmsg;
                }
                if (score > ecalBestScore) {
                  ecalBest = component;
                  ecalBestScore = score;
                  ecalBestLikelihood = likelihood;
                  ecalBestIp = componentIp;
                  ecalBestCov = componentCov;
                  ecalBestIpValid = true;
                }
              }
              if (ecalBestIpValid) {
                ecalConstrainedIp = ecalBestIp;
                ecalConstrainedIpCov = ecalBestCov;
                ecalConstrainedChi2 = componentFitChi2(*ecalBest);
                ecalConstrainedNdf = ecalBest->kaltrack
                    ? ecalBest->kaltrack->GetNDF() +
                          inwardFilterResult.seedMeasurementDimension
                    : 0;
                if (m_verboseDump) {
                  const double constrainedPt =
                      ecalConstrainedIp.GetKappa() != 0.0
                          ? 1.0 / std::abs(ecalConstrainedIp.GetKappa()) : 0.0;
                  info() << boost::format(
                      "  ECAL CONSTRAINT selected baselineId=%d selectedId=%d "
                      "energy=%.9g clusters=%d pOverE=%.9g threshold=%.9g "
                      "likelihood=%.9g constrainedScore=%.9g pT=%.9g")
                            % reverseBest->debugId % ecalBest->debugId
                            % ecalConstrainedEnergy
                            % ecalConstrainedClusterCount
                            % (baselineP / ecalConstrainedEnergy)
                            % m_ecalConstraintRatioThreshold.value()
                            % ecalBestLikelihood % ecalBestScore
                            % constrainedPt
                         << endmsg;
                }
              }
            } else if (m_verboseDump) {
              info() << boost::format(
                  "  ECAL CONSTRAINT inactive observationValid=%d energy=%.9g "
                  "clusters=%d outerPhi=%.9g maxRatio=%.9g threshold=%.9g")
                        % (ecalObservationValid ? 1 : 0)
                        % ecalConstrainedEnergy
                        % ecalConstrainedClusterCount % outerReferencePhi
                        % baselineRatio
                        % m_ecalConstraintRatioThreshold.value()
                     << endmsg;
            }
          }
        }
      }
      if (m_reverseFiltering.value() && !reverseIpAvailable) {
        lineageGraph.markAbandoned(reverseComps);
      }
      for (auto* reverseComp : reverseComps) delete reverseComp;
      for (auto& surface : inwardFilterResult.smoothedSurfaces)
        for (auto* smoothedComponent : surface.components)
          delete smoothedComponent;
    }

    // ---- Step 5: smooth, extrapolate to IP, write output ----
    if (!comps.empty() && nProc > 0) {
      GsfMixture::normalizeWeights(comps);
      if (m_gaussianSumSmoothing.value()) {
        int activeSmootherNodes = 0;
        int reductionSmootherNodes = 0;
        if (!smoothKlReductionGraph(comps, smootherGraph,
                                    activeSmootherNodes,
                                    reductionSmootherNodes)) {
          warning() << boost::format("GSF event index %d track %d: KL reduction-aware Gaussian-sum smoothing failed; no GSF output track")
                       % (m_nEvt - 1) % (nFit + 1) << endmsg;
          lineageGraph.markAbandoned(comps);
          persistLineageGraph(lineageGraph, inputTrackIndex, -1);
          for (auto* c : comps) delete c;
          for (auto& h : hits) delete h.kalHit;
          continue;
        }
        if (m_verboseDump) {
          const double innerPt = comps.front()->smoothedInnerMean(2, 0) != 0.0
              ? 1.0 / std::abs(comps.front()->smoothedInnerMean(2, 0)) : 0.0;
          info() << boost::format("  GSF-SMOOTHER summary: graphNodes=%d activeNodes=%d reductionNodes=%d finalComponents=%d innerPt=%.9g")
                    % (int)smootherGraph.size() % activeSmootherNodes
                    % reductionSmootherNodes % (int)comps.size()
                    % innerPt << endmsg;
        }
      }

      // Pick best component among components with at least one real filtered hit.
      int bestIdx = -1;
      for (size_t i = 0; i < comps.size(); i++) {
        if (!comps[i]->kaltrack || comps[i]->kaltrack->GetEntriesFast() <= 1) continue;
        if (bestIdx < 0 || comps[i]->weight > comps[bestIdx]->weight)
          bestIdx = (int)i;
      }
      if (bestIdx < 0) {
        warning() << "GSF event index " << (m_nEvt - 1)
                  << ": no component has a filtered hit; no GSF output track" << endmsg;
        lineageGraph.markAbandoned(comps);
        persistLineageGraph(lineageGraph, inputTrackIndex, -1);
        for (auto* c : comps) delete c;
        for (auto& h : hits) delete h.kalHit;
        continue;
      }

      auto* best = comps[bestIdx];
      if (m_gaussianSumSmoothing.value() && !m_reverseFiltering.value())
        lineageGraph.markFinal(comps, best);
      if (m_reverseFiltering.value() && !reverseIpAvailable)
        lineageGraph.markFinal(comps, best);
      dumpComponents("final-smoothed", -1, comps);
      if (m_verboseDump && m_verboseSplitDump) {
        const double bestKappa = best->helixAtLastSite(bz).GetKappa();
        const double bestPt = (bestKappa != 0.0) ? 1.0 / std::abs(bestKappa) : 0.0;
        info() << boost::format("  MIX selected bestIdx=%d id=%d bestWeight=%.6g pT=%.6g kappa=%.6e outputMode=%s")
                  % bestIdx % best->debugId % best->weight % bestPt % bestKappa % m_outputMode.value() << endmsg;
        if (m_componentDebugDump) {
          info() << boost::format("  SELECTED branch id=%d full-history=%s")
                    % best->debugId % best->debugHistory << endmsg;
        }
      }

      if (m_verboseDump && m_verboseSplitDump) {
        auto dumpSite = [&](const char* label, int idx) {
          if (!best->kaltrack || idx < 0 || idx >= best->kaltrack->GetEntriesFast()) return;
          auto* site = dynamic_cast<const TKalTrackSite*>(best->kaltrack->At(idx));
          if (!site) return;
          auto& state = dynamic_cast<TKalTrackState&>(site->GetCurState());
          auto h = state.GetHelix();
          auto pv = h.GetPivot();
          info() << boost::format("  DIAG %s site=%d drho=%.6g phi0=%.6g kappa=%.6g dz=%.6g tanl=%.6g pivot=(%.3f, %.3f, %.3f)")
                    % label % idx % h.GetDrho() % h.GetPhi0() % h.GetKappa() % h.GetDz() % h.GetTanLambda()
                    % pv.X() % pv.Y() % pv.Z() << endmsg;
        };
        dumpSite("initial", 0);
        dumpSite("inner", 1);
        dumpSite("last", best->kaltrack->GetEntriesFast() - 1);
      }

      // Extrapolate to IP (method selected by MaterialIPExtrapolation).
      // Smoother and reverse workflows publish three endpoint views:
      // BestBranch is written to GSFTracksBestBranch, while the paired
      // moment-matched state is written to GSFTracksWeightedMean and the joint
      // density maximum to GSFTracksFullMixtureMode. The legacy selector
      // remains effective only for the forward-only workflow.
      THelicalTrack bestIpHelix(TMatrixD(5,1), TVector3(0, 0, 0), bz);
      TMatrixD bestIpCov(5, 5);
      extrapolateToIP_component(best, m_materialIPExtrap, m_cradle, m_ipLayer,
                                bz, bestIpHelix, bestIpCov);

      THelicalTrack ipHelix = bestIpHelix;
      TMatrixD ipCov = bestIpCov;
      THelicalTrack weightedIpHelix = bestIpHelix;
      TMatrixD weightedIpCov = bestIpCov;
      THelicalTrack fullMixtureModeIpHelix = bestIpHelix;
      TMatrixD fullMixtureModeIpCov = bestIpCov;
      FullMixtureModeStatus fullMixtureModeStatus =
          FullMixtureModeStatus::MethodEndpointUnavailable;
      bool usedReverseOutput = false;
      bool pairedWeightedOutputAvailable = false;
      if (m_reverseFiltering.value() && reverseIpAvailable) {
        ipHelix = reverseOutputIp;
        ipCov = reverseOutputIpCov;
        usedReverseOutput = true;
        weightedIpHelix = reverseWeightedIpAvailable
            ? reverseWeightedIp : reverseOutputIp;
        weightedIpCov = reverseWeightedIpAvailable
            ? reverseWeightedIpCov : reverseOutputIpCov;
        fullMixtureModeIpHelix = reverseFullMixtureModeIpAvailable
            ? reverseFullMixtureModeIp : reverseOutputIp;
        fullMixtureModeIpCov = reverseFullMixtureModeIpAvailable
            ? reverseFullMixtureModeIpCov : reverseOutputIpCov;
        fullMixtureModeStatus = reverseFullMixtureModeStatus;
        pairedWeightedOutputAvailable = true;
      } else if (m_gaussianSumSmoothing.value()) {
        if (!weightedMixtureAtIP(comps, m_materialIPExtrap, m_cradle,
                                 m_ipLayer, bz, weightedIpHelix,
                                 weightedIpCov)) {
          warning() << "Smoother WeightedMean publication failed; the paired "
                       "collection will preserve the BestBranch state"
                    << endmsg;
        }
        finalMixtureComponentRecords = captureFinalMixtureComponentsAtIP(
            comps, bz, FinalMixtureComponentSource::GaussianSumSmoother,
            [&](GsfComponent& component, THelicalTrack& helix,
                TMatrixD& covariance) {
              return extrapolateToIP_component(
                  &component, m_materialIPExtrap, m_cradle, m_ipLayer,
                  bz, helix, covariance);
            });
        FullMixtureModeDiagnostics smootherModeDiagnostics;
        fullMixtureModeStatus = findFullMixtureModeAtIP(
            comps, bz,
            [&](GsfComponent& component, THelicalTrack& helix,
                TMatrixD& covariance) {
              return extrapolateToIP_component(
                  &component, m_materialIPExtrap, m_cradle, m_ipLayer,
                  bz, helix, covariance);
            },
            fullMixtureModeIpHelix, fullMixtureModeIpCov,
            smootherModeDiagnostics);
        if (fullMixtureModeStatus != FullMixtureModeStatus::Success) {
          fullMixtureModeIpHelix = bestIpHelix;
          fullMixtureModeIpCov = bestIpCov;
          warning() << "Smoother FullMixtureMode publication failed with "
                       "status "
                    << fullMixtureModeStatusValue(fullMixtureModeStatus)
                    << "; the paired collection will preserve the "
                       "BestBranch state"
                    << endmsg;
        } else if (m_verboseDump) {
          const double modePt = fullMixtureModeIpHelix.GetKappa() != 0.0
              ? 1.0 / std::abs(fullMixtureModeIpHelix.GetKappa()) : 0.0;
          info() << boost::format(
              "  SMOOTHER IP paired output: mode=FullMixtureMode "
              "components=%d starts=%d maxima=%d iterations=%d "
              "logDensity=%.9g scaledGradient=%.3g pT=%.9g")
                    % smootherModeDiagnostics.usableComponents
                    % smootherModeDiagnostics.starts
                    % smootherModeDiagnostics.maxima
                    % smootherModeDiagnostics.iterations
                    % smootherModeDiagnostics.logDensity
                    % smootherModeDiagnostics.scaledGradient
                    % modePt
                 << endmsg;
        }
        pairedWeightedOutputAvailable = true;
      } else if (m_reverseFiltering.value()) {
        warning() << "Reverse endpoint unavailable; all three published "
                     "collections "
                     "will preserve the forward BestBranch state"
                  << endmsg;
        pairedWeightedOutputAvailable = true;
      }
      bool usedWeightedOutput = false;
      const std::string outputMode = m_outputMode.value();
      if (!pairedWeightedOutputAvailable && !usedReverseOutput &&
          outputMode == "WeightedMean") {
        THelicalTrack mixIpHelix(TMatrixD(5,1), TVector3(0, 0, 0), bz);
        TMatrixD mixIpCov(5, 5);
        if (weightedMixtureAtIP(comps, m_materialIPExtrap, m_cradle, m_ipLayer,
                                bz, mixIpHelix, mixIpCov)) {
          ipHelix = mixIpHelix;
          ipCov = mixIpCov;
          usedWeightedOutput = true;
        } else {
          warning() << "GSFOutputMode=WeightedMean failed; falling back to BestBranch"
                    << endmsg;
        }
      } else if (!pairedWeightedOutputAvailable && !usedReverseOutput &&
                 outputMode != "BestBranch") {
        warning() << "Unknown GSFOutputMode '" << outputMode
                  << "'; falling back to BestBranch" << endmsg;
      }
      if (m_verboseDump && m_verboseSplitDump) {
        auto pv = ipHelix.GetPivot();
        info() << boost::format("  DIAG ip    drho=%.6g phi0=%.6g kappa=%.6g dz=%.6g tanl=%.6g pivot=(%.3f, %.3f, %.3f)")
                  % ipHelix.GetDrho() % ipHelix.GetPhi0() % ipHelix.GetKappa() % ipHelix.GetDz() % ipHelix.GetTanLambda()
                  % pv.X() % pv.Y() % pv.Z() << endmsg;
        if (usedWeightedOutput) {
          auto bestPv = bestIpHelix.GetPivot();
          info() << boost::format("  DIAG best-ip drho=%.6g phi0=%.6g kappa=%.6g dz=%.6g tanl=%.6g pivot=(%.3f, %.3f, %.3f)")
                    % bestIpHelix.GetDrho() % bestIpHelix.GetPhi0() % bestIpHelix.GetKappa()
                    % bestIpHelix.GetDz() % bestIpHelix.GetTanLambda()
                    % bestPv.X() % bestPv.Y() % bestPv.Z() << endmsg;
        }
      }

      // Write output track
      auto ot = out->create();
      ot.setType(2);
      const double outputChi2 = usedReverseOutput
          ? reverseOutputChi2 : componentFitChi2(*best);
      const int outputNdf = usedReverseOutput
          ? reverseOutputNdf
          : best->kaltrack->GetNDF() +
                initialization.seedHitMeasurementDimension;
      ot.setChi2(outputChi2);
      ot.setNdf(outputNdf);

      edm4hep::TrackState ts;
      ts.location = DH::AtIP;
      fillTrackState(ts, ipHelix, ipCov, bz);
      ot.addToTrackStates(ts);

      for (const auto& h : assocHits) ot.addToTrackerHits(h);

      if (weightedMeanOut && pairedWeightedOutputAvailable) {
        auto weightedTrack = weightedMeanOut->create();
        weightedTrack.setType(2);
        // A moment-matched mixture has no unique branch chi2/NDF. Preserve
        // the same selected-branch fit-quality metadata used historically
        // when WeightedMean was the single published endpoint.
        weightedTrack.setChi2(outputChi2);
        weightedTrack.setNdf(outputNdf);
        edm4hep::TrackState weightedState;
        weightedState.location = DH::AtIP;
        fillTrackState(weightedState, weightedIpHelix, weightedIpCov, bz);
        weightedTrack.addToTrackStates(weightedState);
        for (const auto& h : assocHits) weightedTrack.addToTrackerHits(h);
      }

      if (fullMixtureModeOut && fullMixtureModeStatusOut &&
          pairedWeightedOutputAvailable) {
        auto modeTrack = fullMixtureModeOut->create();
        modeTrack.setType(2);
        // A density mode has no unique component fit quality. Keep the same
        // selected-component metadata convention used by WeightedMean.
        modeTrack.setChi2(outputChi2);
        modeTrack.setNdf(outputNdf);
        edm4hep::TrackState modeState;
        modeState.location = DH::AtIP;
        fillTrackState(modeState, fullMixtureModeIpHelix,
                       fullMixtureModeIpCov, bz);
        modeTrack.addToTrackStates(modeState);
        for (const auto& h : assocHits) modeTrack.addToTrackerHits(h);
        fullMixtureModeStatusOut->push_back(
            fullMixtureModeStatusValue(fullMixtureModeStatus));
      }

      persistFinalMixtureComponents(
          finalMixtureComponentRecords, inputTrackIndex, nFit);
      persistLineageGraph(lineageGraph, inputTrackIndex, nFit);

      if (truthMaterialTrackMatched) {
        const std::int16_t runtimeMaterialMode =
            useDD4hepBetweenSurfaces ? 2 : 1;
        for (std::size_t intervalIndex = 0;
             intervalIndex < eventDataMatch.materialIntervals.size();
             ++intervalIndex) {
          const auto& truth =
              eventDataMatch.materialIntervals[intervalIndex];
          const auto& fromHit = hits[intervalIndex];
          const auto& toHit = hits[intervalIndex + 1];
          const TVector3 truthFrom(
              truth.startPosition.x, truth.startPosition.y,
              truth.startPosition.z);
          const TVector3 truthTo(
              truth.endPosition.x, truth.endPosition.y,
              truth.endPosition.z);
          const auto truthHookDD4hep = geometryTransitionMaterialPath(
              m_materialManager, truthFrom, truthTo);
          const auto& forward = forwardMaterialSummaries[intervalIndex];
          const auto& reverse = reverseMaterialSummaries[intervalIndex];

          auto record = truthMaterialIntervals->create();
          record.setInputTrackIndex(inputTrackIndex);
          record.setOutputTrackIndex(nFit);
          record.setHitFromIndex(static_cast<int>(intervalIndex));
          record.setHitToIndex(static_cast<int>(intervalIndex + 1));
          record.setSurfaceFromIndex(fromHit.surfaceIndex);
          record.setSurfaceToIndex(toHit.surfaceIndex);
          record.setCellFrom(static_cast<std::uint64_t>(
              fromHit.lcioHit.getCellID()));
          record.setCellTo(static_cast<std::uint64_t>(
              toHit.lcioHit.getCellID()));
          record.setTruthTrackID(eventDataMatch.g4TrackID);
          record.setTruthFirstStepNumber(truth.firstStepNumber);
          record.setTruthLastStepNumber(truth.lastStepNumber);
          record.setTruthStartHookFraction(truth.startHookFraction);
          record.setTruthEndHookFraction(truth.endHookFraction);
          record.setTruthStartPosition(truth.startPosition);
          record.setTruthEndPosition(truth.endPosition);
          record.setTruthStepCount(truth.stepCount);
          record.setTruthG4TX0(truth.truthTX0);
          record.setTruthMomentumBefore(truth.momentumBefore);
          record.setTruthEbremLoss(truth.ebremLoss);
          record.setTruthRetainedMomentumFraction(truth.retainedFraction);
          record.setDd4hepTruthHookValid(
              truthHookDD4hep.valid ? 1 : 0);
          record.setDd4hepTruthHookLayerCount(truthHookDD4hep.layerCount);
          record.setDd4hepTruthHookTX0(truthHookDD4hep.pathTX0);
          record.setRuntimeMaterialMode(runtimeMaterialMode);
          record.setSplitThreshold(m_bhSplitThresh.value());

          record.setForwardCandidateCount(forward.candidateCount);
          record.setForwardValidCount(forward.validCount);
          record.setForwardAboveThresholdCount(
              forward.aboveThresholdCount);
          record.setForwardWeightedTX0(forward.weightedTX0());
          record.setForwardMinTX0(forward.minimumTX0());
          record.setForwardMaxTX0(forward.maximumTX0());
          record.setForwardLeadingComponentID(forward.leadingComponentID);
          record.setForwardLeadingComponentWeight(forward.leadingWeight());
          record.setForwardLeadingTX0(forward.leadingTX0);

          record.setReverseCandidateCount(reverse.candidateCount);
          record.setReverseValidCount(reverse.validCount);
          record.setReverseAboveThresholdCount(
              reverse.aboveThresholdCount);
          record.setReverseWeightedTX0(reverse.weightedTX0());
          record.setReverseMinTX0(reverse.minimumTX0());
          record.setReverseMaxTX0(reverse.maximumTX0());
          record.setReverseLeadingComponentID(reverse.leadingComponentID);
          record.setReverseLeadingComponentWeight(reverse.leadingWeight());
          record.setReverseLeadingTX0(reverse.leadingTX0);
        }
        truthMaterialScopeStatus = TruthBHLossScopeStatus::Valid;
        (*truthMaterialStatus)[static_cast<std::size_t>(inputTrackIndex)] =
            truthBHLossStatusValue(truthMaterialScopeStatus);
      }

      // Paired experimental output.  When the ECAL gate is inactive or the
      // observation is unavailable this is an exact parameter/covariance copy
      // of the unconstrained result, making preservation directly testable.
      if (ecalOut) {
        auto ecalTrack = ecalOut->create();
        ecalTrack.setType(2);
        ecalTrack.setChi2(ecalConstrainedIpAvailable
                              ? ecalConstrainedChi2 : outputChi2);
        ecalTrack.setNdf(ecalConstrainedIpAvailable
                             ? ecalConstrainedNdf : outputNdf);
        edm4hep::TrackState ecalState;
        ecalState.location = DH::AtIP;
        if (ecalConstrainedIpAvailable) {
          fillTrackState(ecalState, ecalConstrainedIp,
                         ecalConstrainedIpCov, bz);
        } else {
          ecalState = ts;
        }
        ecalTrack.addToTrackStates(ecalState);
        for (const auto& h : assocHits) ecalTrack.addToTrackerHits(h);
      }

      // ── MC truth ──
      double t_pT = 0, t_eta = 0, t_phi = 0, t_p = 0;
      if (m_mcParticles.isValid()) {
        auto* mcCol = m_mcParticles.get();
        if (mcCol && mcCol->size() > 0) {
          auto mcp = (*mcCol)[0];  // first particle = primary electron
          auto& mom = mcp.getMomentum();
          t_pT  = std::hypot(mom.x, mom.y);
          t_p   = std::hypot(t_pT, mom.z);
          t_eta = (t_p > 0 && std::abs(t_pT / t_p) < 1.0)
                  ? std::atanh(mom.z / t_p) : 0.0;
          t_phi = std::atan2(mom.y, mom.x);
        }
      }

      // ── fill TrackSummary ──
      auto toPtEta = [bz](double omega, double tanl) {
        double a = bz * 2.99792458e-4;
        double pT = (omega != 0) ? std::abs(a / omega) : 0.0;
        double eta = std::asinh(tanl);
        return std::make_pair(pT, eta);
      };
      double lcio_pT, lcio_eta, gsf_pT, gsf_eta;
      std::tie(lcio_pT, lcio_eta) = toPtEta(seed.omega, seed.tanl);
      std::tie( gsf_pT,  gsf_eta) = toPtEta(ts.omega, ts.tanLambda);
      double lcio_p = lcio_pT * std::cosh(lcio_eta);
      double  gsf_p =  gsf_pT * std::cosh( gsf_eta);

      TrackSummary sum;
      sum.iev      = m_nEvt;
      sum.charge   = charge;
      sum.nHits    = nProc + 1;
      sum.nComps   = usedReverseOutput ? reverseOutputComps : (int)comps.size();
      sum.truth_pT = t_pT; sum.truth_eta = t_eta; sum.truth_phi = t_phi; sum.truth_p = t_p;
      sum.lcio_pT  = lcio_pT; sum.lcio_eta  = lcio_eta;  sum.lcio_phi  = seed.phi;
      sum.lcio_d0  = seed.d0; sum.lcio_z0   = seed.z0;   sum.lcio_p    = lcio_p;
      sum.lcio_chi2 = trk.getChi2(); sum.lcio_ndf = trk.getNdf();
      sum.gsf_pT   = gsf_pT;   sum.gsf_eta   = gsf_eta;   sum.gsf_phi   = ts.phi;
      sum.gsf_d0   = ts.D0;    sum.gsf_z0    = ts.Z0;     sum.gsf_p     = gsf_p;
      sum.gsf_chi2 = outputChi2; sum.gsf_ndf = outputNdf;
      // GSF diagnostics
      sum.nSplits     = nSplits;
      sum.nReductions = nReductions;
      sum.maxCompsEver= maxCompsEver;
      sum.finalComps  = sum.nComps;
      sum.bestWeight  = usedReverseOutput ? reverseOutputWeight : best->weight;
      sum.meanWeight  = 1.0 / (int)comps.size();
      sum.maxTX0Layer = maxTX0Layer;
      sum.totalTX0    = totalTX0;
      m_summaries.push_back(sum);

      // ── pretty-print (guarded by VerboseDump) ──
      if (m_verboseDump) {
        double dphi = ts.phi - seed.phi;
        while (dphi >  M_PI) dphi -= 2 * M_PI;
        while (dphi < -M_PI) dphi += 2 * M_PI;
        std::string const sep(60, '-');

        // ── parameter comparison table ──
        info() << sep << endmsg;
        info() << boost::format("%s %02d  |  comps %2d  hits %d/%d  q=%+d  p %.2f GeV  χ²/ndf %.1f/%d")
                  % "GSF Track" % (nFit + 1) % sum.nComps % sum.nHits % (int)hits.size()
                  % charge % gsf_p % outputChi2 % outputNdf << endmsg;
        info() << sep << endmsg;
        info() << boost::format("  %-6s  %10s  %10s  %10s  %16s  %s") % ""    % "Truth"  % "LCIO"   % "GSF"    % "AddFilter" % "" << endmsg;
        info() << boost::format("  %-6s  %10.4f  %10.4f  %10.4f  %16s  %s")   % "pT"  % t_pT      % lcio_pT   % gsf_pT   % "" % "GeV" << endmsg;
        info() << boost::format("  %-6s  %10.4f  %10.4f  %10.4f  %16s")       % "eta" % t_eta     % lcio_eta  % gsf_eta  % "" << endmsg;
        info() << boost::format("  %-6s  %10.4f  %10.4f  %10.4f  %16s")       % "phi" % t_phi     % seed.phi   % ts.phi   % "" << endmsg;
        info() << boost::format("  %-6s  %10s  %10.4f  %10.4f  %16s  %s")     % "d0"  % "-"       % seed.d0    % ts.D0    % "" % "mm"  << endmsg;
        info() << boost::format("  %-6s  %10s  %10.4f  %10.4f  %16s  %s")     % "z0"  % "-"       % seed.z0    % ts.Z0    % "" % "mm"  << endmsg;
        info() << boost::format("  %-6s  %10.3f  %10.3f  %10.3f  %16s  %s")   % "p"   % t_p       % lcio_p     % gsf_p    % "" % "GeV" << endmsg;
        info() << boost::format("  %-6s  %10s  %7.1f/%-2d  %7.1f/%-2d  last %2d/%2d/%2d")
                  % "chi2" % "-" % trk.getChi2() % trk.getNdf()
                  % outputChi2 % outputNdf
                  % lastAccept % lastRecover % lastReject << endmsg;
        info() << boost::format("  %-6s  %10s  %10s  %10s  total %2d/%2d/%2d")
                  % "A/R/J" % "-" % "-" % "-"
                  % totalAccept % totalRecover % totalReject << endmsg;

        // ── GSF diagnostics table ──
        info() << sep << endmsg;
        info() << boost::format("  GSF diagnostics | splits %d  reductions %d  peak-comps %d  final-comps %d")
                  % nSplits % nReductions % maxCompsEver % sum.finalComps << endmsg;
        info() << boost::format("  weights        | best %.4f  mean %.4f  ratio %.2f")
                  % sum.bestWeight % (1.0 / sum.finalComps)
                  % (sum.bestWeight * sum.finalComps) << endmsg;
        info() << boost::format("  output         | mode %s")
                  % (pairedWeightedOutputAvailable
                         ? "BestBranch + WeightedMean + FullMixtureMode"
                         : (usedReverseOutput ? reverseOutputLabel :
                            (usedWeightedOutput ? "WeightedMean" :
                                                  "BestBranch")))
               << endmsg;
        info() << boost::format("  material       | max-tX0 %.2e  total-tX0 %.2e")
                  % maxTX0Layer % totalTX0 << endmsg;
        info() << sep << endmsg;
      }

      nFit++;
    } else {
      // Keep the complete evaluated graph even when this input track cannot
      // publish a GSF endpoint.  The -1 mapping distinguishes it from every
      // row-aligned output track without discarding its diagnostic history.
      lineageGraph.markAbandoned(comps);
      persistLineageGraph(lineageGraph, inputTrackIndex, -1);
    }

    for (auto* c : comps) delete c;
    for (auto& h : hits) delete h.kalHit;
  }

  info() << "Fitted: " << nFit << " / " << in->size() << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode RecGsfTracking::finalize() {
  info() << "Processed " << m_nEvt << " events" << endmsg;
  if (m_truthBHLossOverride.value()) {
    info() << "Truth BH-loss oracle replaced "
           << m_truthBHLossOverrideCalls << " executed BH responses; "
           << m_truthBHLossPassthroughTracks
           << " input tracks had no oracle scope and used the configured BH "
              "model"
           << endmsg;
    info() << "Truth BH-loss EventData matched "
           << m_truthBHLossDynamicTracks
           << " selected input tracks; maximum observed runtime-hit/truth-"
              "anchor distance was "
           << m_truthBHLossMaxObservedEndpointDistance << " mm" << endmsg;
    info() << "Truth BH-loss EventData fallback tags: invalid truth "
           << "events=" << m_truthBHLossInvalidTruthEvents
           << ", endpoint-distance tracks="
           << m_truthBHLossInvalidEndpointTracks
           << ", interval-map tracks="
           << m_truthBHLossInvalidIntervalTracks << endmsg;
  }
  delete m_materialManager;
  m_materialManager = nullptr;
  m_detectors.clear();
  if (m_cradle) { m_cradle->SetOwner(true); delete m_cradle; m_cradle = nullptr; }
  return Algorithm::finalize();
}
