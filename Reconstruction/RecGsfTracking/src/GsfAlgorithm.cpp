#include "GsfAlgorithm.h"
#include "GsfComponent.h"
#include "GsfMixture.h"
#include "BetheHeitlerSplitter.h"

#include "DD4hep/Detector.h"
#include "DD4hep/DD4hepUnits.h"

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

#include <boost/format.hpp>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <exception>
#include <memory>
#include <limits>
#include <set>

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

static bool getTrackStateAt(const edm4hep::Track& trk, int location, DH& out) {
  for (const auto& ts : trk.getTrackStates()) {
    if (ts.location == location) {
      out = ts;
      return true;
    }
  }
  return false;
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

/// Build initial TKalTrack site, at the innermost hit.
static TKalTrackSite* makeInitialSite(
    const LcioSeed& seed, const MatchedHit& firstHit, double bz,
    double kappaSeed, double kappaCov) {

  TVector3 pivot = firstHit.layer->HitToXv(*firstHit.kalHit);
  THelicalTrack helix(-seed.d0, seed.phi - M_PI / 2., kappaSeed,
                       seed.z0, seed.tanl,
                       pivot.X(), pivot.Y(), pivot.Z(), bz);

  TKalMatrix sv(kSdim, 1);
  sv(0, 0) = helix.GetDrho();
  sv(1, 0) = helix.GetPhi0();
  sv(2, 0) = helix.GetKappa();
  sv(3, 0) = helix.GetDz();
  sv(4, 0) = helix.GetTanLambda();
  sv(5, 0) = 0.0;

  TKalMatrix cv(kSdim, kSdim);
  cv.Zero();
  cv(0, 0) = 100.0;
  cv(1, 1) = 0.01;
  cv(2, 2) = kappaCov;
  cv(3, 3) = 100.0;
  cv(4, 4) = 0.01;
  cv(5, 5) = 1e6;

  // Clone hit with huge errors → dummy initial site
  TVTrackHit* dummyHit = nullptr;
  if (auto* ch = dynamic_cast<DDCylinderHit*>(firstHit.kalHit))
    dummyHit = new DDCylinderHit(*ch);
  else if (auto* ph = dynamic_cast<DDPlanarHit*>(firstHit.kalHit))
    dummyHit = new DDPlanarHit(*ph);

  dummyHit->operator()(0, 1) = 1e16;
  if (dummyHit->GetDimension() > 1)
    dummyHit->operator()(1, 1) = 1e16;

  auto& site = *new TKalTrackSite(*dummyHit, kSdim);
  site.SetHitOwner();
  site.SetOwner();
  site.SetPivot(pivot);

  site.Add(new TKalTrackState(sv, cv, site, TVKalSite::kPredicted));
  site.Add(new TKalTrackState(sv, cv, site, TVKalSite::kFiltered));

  return &site;
}

static int covIndex5(int row, int col) {
  if (row < col) std::swap(row, col);
  return row * (row + 1) / 2 + col;
}

static TKalTrackSite* makeInitialSiteFromTrackState(
    const DH& ts, const MatchedHit& firstHit, double bz);

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

static bool appendBaselineStateToComponent(
    GsfComponent& comp, const edm4hep::TrackState& ts,
    const MatchedHit& hit, double bz) {
  TKalTrackSite* site = makeInitialSiteFromTrackState(ts, hit, bz);
  if (!site || !comp.kaltrack) {
    delete site;
    return false;
  }
  comp.kaltrack->Add(site);
  comp.continuationState = ts;
  comp.continuationValid = true;
  return true;
}

static TKalTrackSite* makeInitialSiteFromTrackState(
    const DH& ts, const MatchedHit& firstHit, double bz) {

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
      cov5(i, j) = scale[i] * scale[j] * ts.covMatrix[covIndex5(i, j)];
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
                                      double /*bz*/,
                                      THelicalTrack& outHelix, TMatrixD& outCov) {
  auto& innerSite = *dynamic_cast<const TKalTrackSite*>(comp->kaltrack->At(1));
  auto& innerState = dynamic_cast<TKalTrackState&>(innerSite.GetCurState());

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

// ============================================================================
// Algorithm
// ============================================================================

RecGsfTracking::RecGsfTracking(const std::string& name,
                                         ISvcLocator* svc)
  : Algorithm(name, svc) {}

StatusCode RecGsfTracking::initialize() {
  m_nEvt = 0;

  if (m_maxComponents.value() < 1) {
    error() << "MaxComponents must be at least 1" << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_reductionTargetComponents.value() < 0 ||
      m_reductionTargetComponents.value() > m_maxComponents.value()) {
    error() << "ReductionTargetComponents must be 0 or in [1, MaxComponents]" << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_reductionMinHitsAfterSplit.value() < 0) {
    error() << "ReductionMinHitsAfterSplit must be non-negative" << endmsg;
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
  std::string reductionMode = m_reductionMode.value();
  std::transform(reductionMode.begin(), reductionMode.end(), reductionMode.begin(), ::tolower);
  if (reductionMode != "kl" && reductionMode != "topn") {
    error() << "ReductionMode must be KL or TopN" << endmsg;
    return StatusCode::FAILURE;
  }
  std::string outputMode = m_outputMode.value();
  std::transform(outputMode.begin(), outputMode.end(), outputMode.begin(), ::tolower);
  if (outputMode != "bestbranch" && outputMode != "weightedmean") {
    error() << "GSFOutputMode must be BestBranch or WeightedMean" << endmsg;
    return StatusCode::FAILURE;
  }

  m_geosvc = service<IGeomSvc>("GeomSvc");
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
         << " reductionMode=" << m_reductionMode.value()
         << " reductionMinHitsAfterSplit=" << m_reductionMinHitsAfterSplit.value()
         << " outputMode=" << m_outputMode.value()
         << " verbose=" << m_verboseDump.value() << "/"
         << m_verboseSplitDump.value() << "/"
         << m_componentDebugDump.value() << endmsg;

  return StatusCode::SUCCESS;
}

// ---------------------------------------------------------------------------
StatusCode RecGsfTracking::execute() {
  m_nEvt++;
  const int eventIndex = m_nEvt - 1;
  const bool selectedOnly = !m_selectedEventIndices.value().empty();
  if (selectedOnly && std::find(m_selectedEventIndices.value().begin(),
                                m_selectedEventIndices.value().end(),
                                eventIndex) == m_selectedEventIndices.value().end()) {
    m_outputTracks.createAndPut();
    return StatusCode::SUCCESS;
  }

  info() << "GSF event index " << eventIndex
         << " (event count " << m_nEvt << ")" << endmsg;
  const auto* in = m_inputTracks.get();
  if (!in) return StatusCode::SUCCESS;

  auto* out = m_outputTracks.createAndPut();
  int nFit = 0;
  double bz = m_field;

  for (const auto& trk : *in) {
    auto assocHits = trk.getTrackerHits();
    if (assocHits.size() < 5) continue;

    // ---- Step 1: seed from LCIO ----
    LcioSeed seed = extractSeed(trk);
    if (seed.omega == 0) continue;
    int charge = (seed.omega > 0) ? 1 : -1;

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

    // ---- Step 3: compute kappa seed ----
    double alpha = bz * 2.99792458e-4;
    double kappaSeed = (bz != 0) ? (seed.omega / alpha) : 1e-5;

    // ---- Step 4: forward GSF filter ----
    const bool fitBackwards = !MarlinTrk::IMarlinTrack::backward;
    const size_t gsfStartHit = 1;
    TKalTrackSite* site = makeInitialSite(seed, hits[0], bz, kappaSeed, m_kappaSeedCov);
    int nextComponentDebugId = 0;
    auto* initComp = new GsfComponent();
    initComp->weight = 1.0;
    initComp->charge = charge;
    initComp->debugId = nextComponentDebugId++;
    initComp->debugHistory = "seed";
    initComp->kaltrack = new TKalTrack();
    initComp->kaltrack->SetOwner();
    initComp->kaltrack->Add(site);

    std::vector<GsfComponent*> comps = {initComp};
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
        const int ndf = c->kaltrack ? c->kaltrack->GetNDF() : 0;
        info() << boost::format("      top%-2d comp[%02d] id=%d parent=%d gen=%d age=%d w=%.6g pT=%.6g kappa=%.6e chi2=%.3f ndf=%d sites=%d")
                  % (int)rank % (int)ci % c->debugId % c->debugParentId % c->generation
                  % c->hitsSinceSplit % c->weight % pt % k % chi2 % ndf % entries << endmsg;
        if (m_componentDebugDump) {
          info() << boost::format("          history=%s")
                    % truncateHistory(c->debugHistory) << endmsg;
        }
      }
    };

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
      totalTX0 += stepTX0;
      if (stepTX0 > maxTX0Layer) maxTX0Layer = stepTX0;
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
                  appendBaselineStateToComponent(*comp, updatedState, hi, bz)) {
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
        else continue;

        auto* st = new TKalTrackSite(*khClone, kSdim);
        st->SetHitOwner();

        if (comp->kaltrack->AddAndFilter(*st)) {
          double dchi = st->GetDeltaChi2();
          const double oldWeight = comp->weight;
          const double posteriorLogWeight = std::log(oldWeight) - 0.5 * std::min(dchi, 100.0);
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
            if (m_verboseDump && m_verboseSplitDump && m_componentDebugDump &&
                (justSplit || (int)comps.size() > 1)) {
              info() << boost::format("      hit-update reject comp at hit=%d") % (int)ih << endmsg;
            }
            delete st;
            delete comp;
          } else {
            nRecover++;
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
      for (auto* comp : comps) ++comp->hitsSinceSplit;
      compsAfterUpdate = (int)comps.size();
      if (justSplit || (int)comps.size() > 1)
        dumpComponents("after-hit/raw", (int)ih, comps);
      GsfMixture::normalizeWeights(comps);
      if (justSplit || (int)comps.size() > 1)
        dumpComponents("after-hit/norm", (int)ih, comps);

      // ACTS-like surface ordering: preserve the filtered measurement state,
      // then convolve components through the material associated with this
      // surface.  The splitter writes only the continuation snapshot, leaving
      // the Kalman measurement history unchanged.
      const bool hasNextForwardSurface = ih + 1 < hits.size();
      if (hasNextForwardSurface && stepTX0 > m_bhSplitThresh && m_isElectron &&
          (int)comps.size() < m_maxComponents) {
        if (m_verboseDump && m_verboseSplitDump) {
          info() << boost::format("  ── BH Split after hit %d (r=%.1f mm, step tX0=%.2e) — %d comps before split")
                    % ih % hi.radius % stepTX0 % (int)comps.size() << endmsg;
        }
        BetheHeitlerSplitter bhs(m_bhModel.value());
        std::vector<GsfComponent*> newCps;
        for (auto* comp : comps) {
          const int parentDebugId = comp->debugId;
          const int childGeneration = comp->generation + 1;
          const double parentKappa = comp->helixAtLastSite(bz).GetKappa();
          auto children = bhs.split(comp, stepTX0, bz);
          if (m_verboseDump && m_verboseSplitDump && m_componentDebugDump) {
            const double parentPT = (bz != 0 && parentKappa != 0)
                ? 1.0 / std::abs(parentKappa) : 0.0;
            info() << boost::format("    filtered parent kappa=%.4e (pT=%.3f) weight=%.4f -> %d post-material children")
                      % parentKappa % parentPT % comp->weight % (int)children.size() << endmsg;
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
          for (auto* child : children) {
            child->debugId = nextComponentDebugId++;
            child->debugParentId = parentDebugId;
            child->generation = childGeneration;
            child->hitsSinceSplit = 0;
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

      const int beforeCutoff = (int)comps.size();
      GsfMixture::removeLowWeight(comps, m_componentWeightCutoff.value());
      if (m_verboseDump && m_verboseSplitDump &&
          (int)comps.size() != beforeCutoff) {
        info() << boost::format("  FLOW hit=%3d cutoff: n=%d -> %d threshold=%.3g")
                  % (int)ih % beforeCutoff % (int)comps.size()
                  % m_componentWeightCutoff.value() << endmsg;
        dumpComponents("after-cutoff", (int)ih, comps);
      }

      const bool shouldReduce = ((int)comps.size() > m_maxComponents.value()) ||
          (reductionTarget < m_maxComponents.value() &&
           (int)comps.size() >= m_maxComponents.value());
      const bool oldEnoughToReduce = std::all_of(
          comps.begin(), comps.end(), [&](const GsfComponent* comp) {
            return comp->hitsSinceSplit >= m_reductionMinHitsAfterSplit.value();
          });
      if (shouldReduce && !oldEnoughToReduce && m_verboseDump && m_verboseSplitDump) {
        info() << boost::format("  FLOW hit=%3d defer-reduce: n=%d target=%d minAge=%d requiredAge=%d")
                  % (int)ih % (int)comps.size() % reductionTarget
                  % (*std::min_element(comps.begin(), comps.end(),
                        [](const GsfComponent* a, const GsfComponent* b) {
                          return a->hitsSinceSplit < b->hitsSinceSplit;
                        }))->hitsSinceSplit
                  % m_reductionMinHitsAfterSplit.value() << endmsg;
      }
      if (shouldReduce && oldEnoughToReduce && (int)comps.size() > reductionTarget) {
        if (m_verboseDump && m_verboseSplitDump) {
          info() << boost::format("  FLOW hit=%3d reduce: n=%d max=%d target=%d mode=%s")
                    % (int)ih % (int)comps.size() % m_maxComponents.value()
                    % reductionTarget % m_reductionMode.value() << endmsg;
        }
        auto reductionLogger = [&](const std::string& line) {
          if (m_verboseDump && m_verboseSplitDump) info() << line << endmsg;
        };
        if (m_reductionMode.value() == "TopN" || m_reductionMode.value() == "topN" ||
            m_reductionMode.value() == "topn") {
          GsfMixture::reduceTopN(comps, reductionTarget, reductionLogger);
        } else {
          GsfMixture::reduce(comps, reductionTarget, bz, reductionLogger);
        }
        nReductions++;
        didReduceHit = true;
        dumpComponents("after-reduce", (int)ih, comps);
        GsfMixture::normalizeWeights(comps);
        dumpComponents("after-reduce/norm", (int)ih, comps);
      }
      compsAfterReduce = (int)comps.size();
      if (m_verboseDump && m_verboseSplitDump) {
        info() << boost::format("  FLOW hit=%3d summary: begin=%2d split=%2d update=%2d reduce=%2d%s A/R/J=%d/%d/%d dchi2=[%.4g, %.4g]")
                  % (int)ih % compsAtHitBegin % compsAfterSplit % compsAfterUpdate
                  % compsAfterReduce % (didReduceHit ? " yes" : " no ")
                  % nAccept % nRecover % nReject
                  % (nAccept > 0 ? minDChi2 : 0.0) % (nAccept > 0 ? maxDChi2 : 0.0) << endmsg;
      }
      nProc++;
    }

    bool reverseIpAvailable = false;
    THelicalTrack reverseOutputIp(TMatrixD(5, 1), TVector3(0, 0, 0), bz);
    TMatrixD reverseOutputIpCov(5, 5);
    double reverseOutputChi2 = 0.0;
    int reverseOutputNdf = 0;
    double reverseOutputWeight = 0.0;
    int reverseOutputComps = 0;

    // Experimental reverse GSF pass.  Initialize from the filtered mixture on
    // the final measurement surface, then revisit preceding measurements in
    // the audited reverse order.  Reverse process convolution increases the
    // momentum according to the same retained-fraction mixture.
    if (m_reverseFiltering.value() && !comps.empty() && hits.size() > 1) {
      std::vector<GsfComponent*> reverseComps;
      int nextReverseId = 0;
      for (const auto* forwardComp : comps) {
        edm4hep::TrackState finalState =
            trackStateFromComponent(*forwardComp, bz, DH::AtOther);
        auto* reverseComp = new GsfComponent();
        reverseComp->weight = forwardComp->weight;
        reverseComp->charge = forwardComp->charge;
        reverseComp->debugId = nextReverseId++;
        reverseComp->debugHistory = "reverse(" + forwardComp->debugHistory + ")";
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
        reverseComps.push_back(reverseComp);
      }
      GsfMixture::normalizeWeights(reverseComps);
      if (m_verboseDump && m_verboseSplitDump)
        dumpComponents("reverse-start", (int)hits.size() - 1, reverseComps);

      int reverseAcceptedTotal = 0;
      int reverseRejectedTotal = 0;
      int reverseSplits = 0;
      int reverseReductions = 0;
      const int reverseReductionTarget =
          (m_reductionTargetComponents.value() > 0)
              ? std::min(m_reductionTargetComponents.value(),
                         m_maxComponents.value())
              : m_maxComponents.value();
      for (int reverseHit = (int)hits.size() - 2;
           reverseHit >= 0 && !reverseComps.empty(); --reverseHit) {
        auto& target = hits[reverseHit];
        std::vector<GsfComponent*> acceptedReverse;
        std::vector<double> reverseLogWeights;
        for (auto* component : reverseComps) {
          double dchi = 0.0;
          double updateChi2 = 0.0;
          int updateNdf = -999;
          edm4hep::TrackState componentState =
              trackStateFromComponent(*component, bz, DH::AtOther);
          edm4hep::TrackState updatedState;
          MarlinTrk::MeasurementUpdate update;
          bool accepted = false;
          try {
            edm4hep::TrackerHit referenceHit = hits[reverseHit + 1].lcioHit;
            edm4hep::TrackerHit targetHit = target.lcioHit;
            std::unique_ptr<MarlinTrk::IMarlinTrack> reverseTrack(
                m_gsfMarlinTrkSystem->createTrack());
            if (reverseTrack &&
                reverseTrack->addHit(referenceHit) == MarlinTrk::IMarlinTrack::success &&
                reverseTrack->initialise(componentState, bz,
                    MarlinTrk::IMarlinTrack::backward) == MarlinTrk::IMarlinTrack::success &&
                reverseTrack->addAndFit(targetHit, dchi, update, DBL_MAX) ==
                    MarlinTrk::IMarlinTrack::success && update.valid &&
                reverseTrack->getTrackState(targetHit, updatedState,
                    updateChi2, updateNdf) == MarlinTrk::IMarlinTrack::success &&
                appendBaselineStateToComponent(*component, updatedState,
                    target, bz)) {
              accepted = true;
            }
          } catch (...) {
            accepted = false;
          }

          if (accepted) {
            component->fitChi2 += dchi;
            reverseLogWeights.push_back(std::log(component->weight) -
                0.5 * (dchi + update.logDetInnovation));
            acceptedReverse.push_back(component);
            ++reverseAcceptedTotal;
            if (m_verboseDump && m_verboseSplitDump && m_componentDebugDump) {
              info() << boost::format("      REVERSE UPDATE accept hit=%d id=%d pT=%.6g dchi2=%.6g logDetS=%.6g")
                        % reverseHit % component->debugId
                        % ptFromTrackState(updatedState, bz) % dchi
                        % update.logDetInnovation << endmsg;
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

        const double reverseTX0 = thicknessInX0(target.layer);
        if (reverseTX0 > m_bhSplitThresh && m_isElectron &&
            (int)reverseComps.size() < m_maxComponents.value()) {
          BetheHeitlerSplitter splitter(m_bhModel.value());
          std::vector<GsfComponent*> reverseChildren;
          for (auto* parent : reverseComps) {
            const int parentId = parent->debugId;
            auto children = splitter.split(parent, reverseTX0, bz, true);
            for (auto* child : children) {
              child->debugParentId = parentId;
              child->debugId = nextReverseId++;
              child->debugHistory += "->reverse-material";
              reverseChildren.push_back(child);
            }
          }
          reverseComps = std::move(reverseChildren);
          ++reverseSplits;
          GsfMixture::normalizeWeights(reverseComps);
        }
        GsfMixture::removeLowWeight(reverseComps,
                                    m_componentWeightCutoff.value());
        if ((int)reverseComps.size() > reverseReductionTarget) {
          if (m_reductionMode.value() == "TopN" ||
              m_reductionMode.value() == "topN" ||
              m_reductionMode.value() == "topn") {
            GsfMixture::reduceTopN(reverseComps, reverseReductionTarget);
          } else {
            GsfMixture::reduce(reverseComps, reverseReductionTarget, bz);
          }
          ++reverseReductions;
          GsfMixture::normalizeWeights(reverseComps);
        }
        if (m_verboseDump && m_verboseSplitDump &&
            (reverseHit < 3 || reverseHit == (int)hits.size() - 2)) {
          dumpComponents("reverse-after-hit", reverseHit, reverseComps);
        }
      }

      if (m_verboseDump) {
        info() << boost::format("  REVERSE summary: finalComps=%d accepted=%d rejected=%d splits=%d reductions=%d")
                  % (int)reverseComps.size() % reverseAcceptedTotal
                  % reverseRejectedTotal % reverseSplits % reverseReductions
               << endmsg;
      }
      GsfMixture::normalizeWeights(reverseComps);
      if (!reverseComps.empty()) {
        auto* reverseBest = *std::max_element(
            reverseComps.begin(), reverseComps.end(),
            [](const GsfComponent* a, const GsfComponent* b) {
              return a->weight < b->weight;
            });
        THelicalTrack reverseIp(TMatrixD(5, 1), TVector3(0, 0, 0), bz);
        TMatrixD reverseIpCov(5, 5);
        if (extrapolateContinuationToIP(*reverseBest, bz,
                                       reverseIp, reverseIpCov)) {
          const double reversePt = reverseIp.GetKappa() != 0.0
              ? 1.0 / std::abs(reverseIp.GetKappa()) : 0.0;
          if (m_verboseDump) {
            info() << boost::format("  REVERSE IP best: id=%d weight=%.6g pT=%.6g d0=%.6g z0=%.6g phi=%.6g tanL=%.6g")
                      % reverseBest->debugId % reverseBest->weight % reversePt
                      % (-reverseIp.GetDrho()) % reverseIp.GetDz()
                      % normalizePhi(reverseIp.GetPhi0() + M_PI / 2.0)
                      % reverseIp.GetTanLambda() << endmsg;
          }
          reverseOutputIp = reverseIp;
          reverseOutputIpCov = reverseIpCov;
          reverseOutputChi2 = componentFitChi2(*reverseBest);
          reverseOutputNdf = reverseBest->kaltrack
              ? reverseBest->kaltrack->GetNDF() : 0;
          reverseOutputWeight = reverseBest->weight;
          reverseOutputComps = (int)reverseComps.size();
          reverseIpAvailable = true;
        }
      }
      for (auto* reverseComp : reverseComps) delete reverseComp;
    }

    // ---- Step 5: smooth, extrapolate to IP, write output ----
    if (!comps.empty() && nProc > 0) {
      for (auto* c : comps)
        if (c->kaltrack->GetEntriesFast() > 1)
          c->kaltrack->SmoothAll();

      GsfMixture::normalizeWeights(comps);

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
        for (auto* c : comps) delete c;
        for (auto& h : hits) delete h.kalHit;
        continue;
      }

      auto* best = comps[bestIdx];
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
      // By default publish the highest-weight branch. Optionally publish the
      // moment-matched weighted mixture at IP.
      THelicalTrack bestIpHelix(TMatrixD(5,1), TVector3(0, 0, 0), bz);
      TMatrixD bestIpCov(5, 5);
      extrapolateToIP_component(best, m_materialIPExtrap, m_cradle, m_ipLayer,
                                bz, bestIpHelix, bestIpCov);

      THelicalTrack ipHelix = bestIpHelix;
      TMatrixD ipCov = bestIpCov;
      bool usedReverseOutput = false;
      if (m_reverseFiltering.value() && reverseIpAvailable) {
        ipHelix = reverseOutputIp;
        ipCov = reverseOutputIpCov;
        usedReverseOutput = true;
      }
      const std::string outputMode = m_outputMode.value();
      bool usedWeightedOutput = false;
      if (outputMode == "WeightedMean" && !usedReverseOutput) {
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
      } else if (outputMode != "BestBranch") {
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
          ? reverseOutputNdf : best->kaltrack->GetNDF();
      ot.setChi2(outputChi2);
      ot.setNdf(outputNdf);

      edm4hep::TrackState ts;
      ts.location = DH::AtIP;
      fillTrackState(ts, ipHelix, ipCov, bz);
      ot.addToTrackStates(ts);

      for (const auto& h : assocHits) ot.addToTrackerHits(h);

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
                  % (usedReverseOutput ? "ReverseBestBranch" :
                     (usedWeightedOutput ? "WeightedMean" : "BestBranch"))
               << endmsg;
        info() << boost::format("  material       | max-tX0 %.2e  total-tX0 %.2e")
                  % maxTX0Layer % totalTX0 << endmsg;
        info() << sep << endmsg;
      }

      nFit++;
    }

    for (auto* c : comps) delete c;
    for (auto& h : hits) delete h.kalHit;
  }

  info() << "Fitted: " << nFit << " / " << in->size() << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode RecGsfTracking::finalize() {
  info() << "Processed " << m_nEvt << " events" << endmsg;
  m_detectors.clear();
  if (m_cradle) { m_cradle->SetOwner(true); delete m_cradle; m_cradle = nullptr; }
  return Algorithm::finalize();
}
