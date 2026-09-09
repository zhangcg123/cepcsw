#include "KalmanAdapter.h"
#include "BreakpointTrackSystem.h"
#include "RecBreakpoint/AugmentedTransport.h"
#include "TrackSystemSvc/MarlinTrkUtils.h"
#include "UTIL/BitSet32.h"
#include "UTIL/ILDConf.h"
#include "kaldet/ILDVMeasLayer.h"
#include "kaldet/ILDVTrackHit.h"
#include "kaltest/TKalTrack.h"
#include "kaltest/TKalTrackSite.h"
#include "kaltest/TKalTrackState.h"
#include "kaltest/TKalFilterCond.h"
#include "kaltest/THelicalTrack.h"

#include <stdexcept>

namespace breakpoint {
namespace {
void requireSuccess(int status, const char* operation) {
  if (status != MarlinTrk::IMarlinTrack::success)
    throw std::runtime_error(std::string(operation) + ": MarlinTrk status " + std::to_string(status));
}

class HitAcceptance : public TKalFilterCond {
public:
  explicit HitAcceptance(double maximum) : m_maximum(maximum) {}
  Bool_t IsAccepted(const TKalTrackSite& site) override {
    return std::isfinite(site.GetDeltaChi2()) && site.GetDeltaChi2() < m_maximum;
  }
private:
  double m_maximum;
};

/// Native KalTest Filter() operates on all SIX coordinates/covariances.
/// Only the measurement projection is specialized: b is NOT native t0.
class LossMeasurementSite : public TKalTrackSite {
public:
  explicit LossMeasurementSite(const TVTrackHit& hit, int dimension = 6)
      : TKalTrackSite(hit, dimension), m_dimension(dimension),
        m_reference(dimension, 1), m_expected(hit.GetDimension(), 1),
        m_derivative(hit.GetDimension(), dimension) {}
  void setReference(const TrackState& reference) {
    for (int i = 0; i < 5; ++i) m_reference(i, 0) = reference.mean(i, 0);
    const TKalTrackState helix(TKalMatrix(reference.mean), *this, TVKalSite::kPredicted, 5);
    TKalMatrix h(GetDimension(), 5);
    if (!TKalTrackSite::CalcExpectedMeasVec(helix, m_expected) ||
        !TKalTrackSite::CalcMeasVecDerivative(helix, h))
      throw std::runtime_error("Cannot linearize measurement at reference trajectory");
    for (int i = 0; i < GetDimension(); ++i)
      for (int j = 0; j < 5; ++j) m_derivative(i, j) = h(i, j);
    m_affine = true;
  }
  Int_t CalcExpectedMeasVec(const TVKalState& state, TKalMatrix& expected) override {
    if (m_affine) {
      expected = m_expected + m_derivative * stateDifference(state, m_reference);
      return 1;
    }
    const auto helix = trackOnly(state);
    return TKalTrackSite::CalcExpectedMeasVec(helix, expected);
  }
  Int_t CalcMeasVecDerivative(const TVKalState& state, TKalMatrix& derivative) override {
    if (m_affine) { derivative = m_derivative; return 1; }
    const auto helix = trackOnly(state);
    TKalMatrix h(GetDimension(), 5);
    if (!TKalTrackSite::CalcMeasVecDerivative(helix, h)) return 0;
    derivative.Zero();
    for (int i = 0; i < GetDimension(); ++i)
      for (int j = 0; j < 5; ++j) derivative(i, j) = h(i, j);
    return 1; // H_b=0, but the full Kalman gain has a nonzero b row via P_bx.
  }
private:
  int m_dimension;
  bool m_affine = false;
  TKalMatrix m_reference, m_expected, m_derivative;
  TKalTrackState trackOnly(const TVKalState& state) const {
    TKalMatrix mean(5, 1);
    for (int i = 0; i < 5; ++i) mean(i, 0) = state(i, 0);
    return TKalTrackState(mean, *this, TVKalSite::kPredicted, 5);
  }
  TVKalState& CreateState(const TKalMatrix& mean, Int_t type) override {
    return *new TKalTrackState(mean, *this, type, m_dimension);
  }
  TVKalState& CreateState(const TKalMatrix& mean, const TKalMatrix& covariance,
                          Int_t type) override {
    return *new TKalTrackState(mean, covariance, *this, type, m_dimension);
  }
};

std::unique_ptr<TKalTrackSite> makeSite(BreakpointTrackSystem& system,
                                      edm4hep::TrackerHit hit) {
  const auto* layer = system.layer(hit);
  if (!layer) throw std::runtime_error("No measurement layer for reconstructed hit");
  std::unique_ptr<ILDVTrackHit> nativeHit(layer->ConvertLCIOTrkHit(hit));
  if (!nativeHit) throw std::runtime_error("Cannot convert reconstructed hit");
  auto site = std::make_unique<TKalTrackSite>(*nativeHit, 5);
  site->SetHitOwner();
  nativeHit.release();
  site->SetOwner();
  return site;
}

TrackState readNative(TKalTrackSite& site, TVKalSite::EStType type) {
  const auto& native = site.GetState(type);
  TrackState state;
  state.pivot = {site.GetPivot().X(), site.GetPivot().Y(), site.GetPivot().Z()};
  for (int i = 0; i < 5; ++i) {
    state.mean(i, 0) = native(i, 0);
    for (int j = 0; j < 5; ++j) state.covariance(i, j) = native.GetCovMat()(i, j);
  }
  validateCovariance(state.covariance);
  return state;
}

MeasurementStep nativeStep(BreakpointTrackSystem& system, double bz, double maximum,
    const TrackState& input, edm4hep::TrackerHit sourceHit, edm4hep::TrackerHit targetHit,
    bool inverseLoss = false, double meanLoss = 0, double sigmaLoss = 0,
    bool predictionOnly = false) {
  TKalTrack track;
  track.SetOwner();
  auto source = makeSite(system, sourceHit);
  const auto& hit = source->GetHit();
  // Retain the exact surface pivot, not its float EDM serialization.
  const TVector3 pivot = hit.GetMeasLayer().HitToXv(hit);
  THelicalTrack helix(input.mean(0, 0), input.mean(1, 0), input.mean(2, 0),
      input.mean(3, 0), input.mean(4, 0), input.pivot.x, input.pivot.y, input.pivot.z, bz);
  TMatrixD covariance(input.covariance), pivotJacobian(5, 5);
  pivotJacobian.UnitMatrix();
  double angle = 0;
  helix.MoveTo(pivot, angle, &pivotJacobian, &covariance);
  source->SetPivot(pivot);
  TKalMatrix mean(5, 1), cov(covariance);
  mean(0, 0) = helix.GetDrho(); mean(1, 0) = helix.GetPhi0();
  mean(2, 0) = helix.GetKappa(); mean(3, 0) = helix.GetDz(); mean(4, 0) = helix.GetTanLambda();
  source->Add(new TKalTrackState(mean, cov, *source, TVKalSite::kPredicted, 5));
  source->Add(new TKalTrackState(mean, cov, *source, TVKalSite::kFiltered, 5));
  auto* sourcePointer = source.release();
  track.Add(sourcePointer);

  auto target = makeSite(system, targetHit);
  HitAcceptance acceptance(maximum);
  target->SetFilterCond(&acceptance);
  MeasurementStep result;
  TMatrixD lossMap(5, 5), lossNoise(5, 5);
  lossMap.UnitMatrix();
  lossNoise.Zero();
  if (inverseLoss) {
    // Same physical owner as outward: propagation reaches hit i with the
    // post-loss curvature, then exp(-b) restores the pre-loss state at i.
    // Native KalTest still performs the entire measurement update.
    // The local track constructor registers this sole live fit. Native
    // material routines use its mass; never silently inherit another fit.
    if (TVKalSystem::GetCurInstancePtr() != &track)
      throw std::runtime_error("Inverse breakpoint requires its own active native track");
    sourcePointer->GetState(TVKalSite::kFiltered).Propagate(*target);
    auto& prediction = target->GetState(TVKalSite::kPredicted);
    lossMap(2, 2) = std::exp(-meanLoss);
    TKalMatrix lossMean(prediction), lossCovariance(prediction.GetCovMat());
    lossMean(2, 0) *= lossMap(2, 2);
    const double derivative = -lossMean(2, 0);
    const double variance = sigmaLoss * sigmaLoss;
    lossNoise(2, 2) = derivative * derivative * variance;
    result.lossTargetCross(0, 2) = variance * derivative;
    TKalMatrix transformed(lossMap * lossCovariance * transpose(lossMap) + lossNoise);
    Matrix6 joint{};
    Matrix5 jacobian{};
    Vector5 d{};
    d[2] = derivative;
    for (int i = 0; i < 5; ++i)
      for (int j = 0; j < 5; ++j) {
        joint[i * 6 + j] = lossCovariance(i, j);
        jacobian[i * 5 + j] = lossMap(i, j);
      }
    joint[35] = variance;
    const auto mapped = AugmentedTransport::covariance(
        joint, AugmentedTransport::jacobian(jacobian, d), {});
    for (int i = 0; i < 5; ++i)
      for (int j = 0; j < 5; ++j)
        result.covarianceClosure = std::max(result.covarianceClosure,
            std::abs(mapped[i * 6 + j] - transformed(i, j)) /
            std::sqrt(transformed(i, i) * transformed(j, j)));
    prediction.SetStateVec(lossMean);
    prediction.SetCovMat(transformed);
    if (!target->Filter()) throw std::runtime_error("KalTest rejected inverse-breakpoint hit update");
    track.Add(target.get());
  } else if (predictionOnly) {
    sourcePointer->GetState(TVKalSite::kFiltered).Propagate(*target);
    track.Add(target.get());
  } else if (!track.AddAndFilter(*target)) {
    throw std::runtime_error("KalTest rejected propagation/hit update");
  }
  auto* targetPointer = target.release(); // track now owns the accepted site
  result.predicted = readNative(*targetPointer, TVKalSite::kPredicted);
  result.filtered = predictionOnly ? result.predicted : readNative(*targetPointer, TVKalSite::kFiltered);
  result.transport = lossMap * sourcePointer->GetState(TVKalSite::kFiltered).GetPropMat() * pivotJacobian;
  result.noise = lossMap * sourcePointer->GetState(TVKalSite::kFiltered).GetProcNoiseMat()
      * transpose(lossMap) + lossNoise;
  result.chi2 = targetPointer->GetDeltaChi2();
  result.dimension = targetPointer->GetDimension();
  return result;
}

// Change coordinates only: no extra material or measurement. Reference and live
// states must use the same pivot for the affine displacement to be meaningful.
TrackState rebase(const TrackState& input, const edm4hep::Vector3d& pivot,
                  double bz, TMatrixD& jacobian) {
  TrackState result = input;
  THelicalTrack helix(input.mean(0,0), input.mean(1,0), input.mean(2,0),
      input.mean(3,0), input.mean(4,0), input.pivot.x,input.pivot.y,input.pivot.z,bz);
  jacobian.UnitMatrix();
  double angle = 0;
  helix.MoveTo(TVector3(pivot.x,pivot.y,pivot.z),angle,&jacobian,&result.covariance);
  result.pivot = pivot;
  result.mean(0,0)=helix.GetDrho(); result.mean(1,0)=helix.GetPhi0();
  result.mean(2,0)=helix.GetKappa(); result.mean(3,0)=helix.GetDz();
  result.mean(4,0)=helix.GetTanLambda();
  return result;
}

// The only measurement-update algebra is native KalTest Filter(). This helper
// supplies a fixed affine measurement model h(ref)+H(ref)*(x-ref).
void affineUpdate(BreakpointTrackSystem& system, edm4hep::TrackerHit hit,
    const TrackState& reference, TMatrixD& mean, TMatrixD& covariance,
    double maximum, double& chi2, int& dimension) {
  const auto* layer=system.layer(hit);
  if (!layer) throw std::runtime_error("No relinearized measurement layer");
  std::unique_ptr<ILDVTrackHit> native(layer->ConvertLCIOTrkHit(hit));
  if (!native) throw std::runtime_error("Cannot convert relinearized hit");
  LossMeasurementSite site(*native,mean.GetNrows());
  site.SetOwner();
  site.SetPivot(TVector3(reference.pivot.x,reference.pivot.y,reference.pivot.z));
  site.setReference(reference);
  HitAcceptance acceptance(maximum);
  site.SetFilterCond(&acceptance);
  site.Add(new TKalTrackState(TKalMatrix(mean),TKalMatrix(covariance),site,
                             TVKalSite::kPredicted,mean.GetNrows()));
  if (!site.Filter()) throw std::runtime_error("KalTest rejected relinearized hit");
  mean=site.GetState(TVKalSite::kFiltered);
  covariance=site.GetState(TVKalSite::kFiltered).GetCovMat();
  validateCovariance(covariance);
  chi2=site.GetDeltaChi2(); dimension=site.GetDimension();
}
} // namespace

std::unique_ptr<MarlinTrk::IMarlinTrack> KalmanAdapter::initialized(
    const edm4hep::TrackState& state, edm4hep::TrackerHit hit, bool inward) const {
  std::unique_ptr<MarlinTrk::IMarlinTrack> track(m_system->createTrack());
  if (!track) throw std::runtime_error("Cannot create MarlinTrk track");
  requireSuccess(track->addHit(hit), "Register source hit");
  requireSuccess(track->initialise(state, m_bz, inward), "Initialize KF state");
  return track;
}

std::array<int, 3> KalmanAdapter::seedHitIndices(
    const std::vector<edm4hep::TrackerHit>& hits) const {
  std::vector<int> usable;
  for (std::size_t i = 0; i < hits.size(); ++i) {
    if (!UTIL::BitSet32(hits[i].getType())[UTIL::ILDTrkHitTypeBit::ONE_DIMENSIONAL])
      usable.push_back(static_cast<int>(i));
  }
  return selectSeedHitIndices(usable, m_seedSelection);
}

edm4hep::TrackState KalmanAdapter::prefit(const std::vector<edm4hep::TrackerHit>& hits,
                                       double scale) const {
  const auto indices = seedHitIndices(hits);
  std::vector<edm4hep::TrackerHit> prefitHits{
      hits[indices[0]], hits[indices[1]], hits[indices[2]]};
  edm4hep::TrackState prefit{};
  requireSuccess(MarlinTrk::createPrefit(prefitHits, &prefit, m_bz, false), "Three-hit prefit");
  prefit.covMatrix.fill(0);
  const int diagonal[] = {0, 2, 5, 9, 14};
  const double variances[] = {1.e6, 1.e2, 1.e-4, 1.e6, 1.e2};
  for (int i = 0; i < 5; ++i) prefit.covMatrix[diagonal[i]] = scale * variances[i];
  return prefit;
}

MeasurementStep KalmanAdapter::seed(const std::vector<edm4hep::TrackerHit>& hits,
                                    double scale) const {
  return nativeStep(*m_system, m_bz, m_maxChi2, fromEDM(prefit(hits, scale), m_bz),
                    hits.front(), hits.front());
}

edm4hep::TrackState KalmanAdapter::referenceKF(
    const std::vector<edm4hep::TrackerHit>& hits, double scale, bool backwardFilter) const {
  auto track = initialized(prefit(hits, scale), hits.front(), false);
  for (auto hit : hits) {
    double chi2 = 0;
    requireSuccess(track->addAndFit(hit, chi2, m_maxChi2), "Reference KF update");
  }
  edm4hep::TrackState ip;
  auto first = hits.front();
  double chi2 = 0;
  int ndf = 0;
  if (backwardFilter) {
    auto last = hits.back();
    requireSuccess(track->smooth(last), "Reference KF last-hit smooth");
    edm4hep::TrackState seed;
    requireSuccess(track->getTrackState(last, seed, chi2, ndf), "Reference KF last state");
    // Match the standard finalizer: register the last hit as dummy only.
    auto inward = initialized(seed, last, false);
    for (int i = static_cast<int>(hits.size()) - 2; i >= 0; --i) {
      auto hit = hits[i];
      requireSuccess(inward->addAndFit(hit, chi2, m_maxChi2), "Reference inward KF update");
    }
    requireSuccess(inward->propagate(edm4hep::Vector3d{0, 0, 0}, first, ip, chi2, ndf),
                   "Reference inward KF IP propagation");
    return ip;
  }
  requireSuccess(track->smooth(), "Reference KF smooth");
  requireSuccess(track->extrapolate(edm4hep::Vector3d{0, 0, 0}, first, ip, chi2, ndf),
                 "Reference KF IP extrapolation");
  return ip;
}

MeasurementStep KalmanAdapter::advance(const TrackState& source,
    edm4hep::TrackerHit sourceHit, edm4hep::TrackerHit targetHit) const {
  return nativeStep(*m_system, m_bz, m_maxChi2, source, sourceHit, targetHit);
}

MeasurementScore KalmanAdapter::measurementScore(edm4hep::TrackerHit hit,
    const TrackState& state, const TrackState& reference) const {
  auto site = makeSite(*m_system, hit);
  // All means use this native site's pivot, just as in the live filter.
  const auto pivot = site->GetPivot();
  const auto matches = [&](const TrackState& value) {
    return std::abs(value.pivot.x-pivot.X()) < 1.e-8 &&
           std::abs(value.pivot.y-pivot.Y()) < 1.e-8 &&
           std::abs(value.pivot.z-pivot.Z()) < 1.e-8;
  };
  if (!matches(state) || !matches(reference))
    throw std::runtime_error("Measurement-score pivot mismatch");
  TKalTrackState finalState(TKalMatrix(state.mean), *site, TVKalSite::kSmoothed, 5);
  TKalTrackState referenceState(TKalMatrix(reference.mean), *site, TVKalSite::kPredicted, 5);
  TKalMatrix finalExpected(site->GetDimension(),1), referenceExpected(site->GetDimension(),1);
  TKalMatrix h(site->GetDimension(),5);
  if (!site->CalcExpectedMeasVec(finalState,finalExpected) ||
      !site->CalcExpectedMeasVec(referenceState,referenceExpected) ||
      !site->CalcMeasVecDerivative(referenceState,h))
    throw std::runtime_error("Measurement-score native projection failed");
  const TMatrixD inverse = inverseCovariance(site->GetMeasNoiseMat());
  const auto score = [&](const TMatrixD& expected) {
    const TMatrixD residual = site->GetMeasVec()-expected;
    const double value = (transpose(residual)*inverse*residual)(0,0);
    if (!std::isfinite(value) || value < 0) throw std::runtime_error("Invalid measurement score");
    return value;
  };
  return {score(referenceExpected+h*stateDifference(state.mean,reference.mean)),score(finalExpected)};
}

MeasurementStep KalmanAdapter::advanceBackward(const TrackState& source,
    edm4hep::TrackerHit sourceHit, edm4hep::TrackerHit targetHit,
    bool breakpoint, double meanLoss, double sigmaLoss) const {
  return nativeStep(*m_system, m_bz, m_maxChi2, source, sourceHit, targetHit,
                    breakpoint, meanLoss, sigmaLoss);
}

LossMeasurementStep KalmanAdapter::advancePersistent(const LossTrackState& source,
    edm4hep::TrackerHit sourceHit, edm4hep::TrackerHit targetHit, bool applyLoss) const {
  LossTrackState mapped = source;
  TMatrixD lossMap(6, 6);
  lossMap.UnitMatrix();
  if (applyLoss) {
    const double scale = std::exp(source.mean(5, 0));
    mapped.mean(2, 0) *= scale;
    lossMap(2, 2) = scale;
    lossMap(2, 5) = mapped.mean(2, 0);
    mapped.covariance = lossMap * source.covariance * transpose(lossMap);
  }
  // Reuse native material/geometric propagation WITHOUT its 5D measurement
  // update. The actual live hit update below is six-dimensional.
  const auto geometry = nativeStep(*m_system, m_bz, m_maxChi2, mapped.track(),
      sourceHit, targetHit, false, 0, 0, true);
  LossMeasurementStep result;
  TMatrixD geometricTransport(6, 6);
  geometricTransport.UnitMatrix();
  for (int i = 0; i < 5; ++i)
    for (int j = 0; j < 5; ++j) {
      geometricTransport(i, j) = geometry.transport(i, j);
      result.noise(i, j) = geometry.noise(i, j);
    }
  result.transport = geometricTransport * lossMap;
  result.predicted = mapped;
  result.predicted.pivot = geometry.predicted.pivot;
  for (int i = 0; i < 5; ++i) result.predicted.mean(i, 0) = geometry.predicted.mean(i, 0);
  result.predicted.covariance = result.transport * source.covariance
      * transpose(result.transport) + result.noise;
  validateCovariance(result.predicted.covariance);

  const auto* layer = m_system->layer(targetHit);
  if (!layer) throw std::runtime_error("No persistent-6D measurement layer");
  std::unique_ptr<ILDVTrackHit> hit(layer->ConvertLCIOTrkHit(targetHit));
  if (!hit) throw std::runtime_error("Cannot convert persistent-6D hit");
  LossMeasurementSite site(*hit);
  site.SetOwner(); // site owns its states; hit is separately scoped above
  site.SetPivot(TVector3(result.predicted.pivot.x, result.predicted.pivot.y, result.predicted.pivot.z));
  HitAcceptance acceptance(m_maxChi2);
  site.SetFilterCond(&acceptance);
  site.Add(new TKalTrackState(TKalMatrix(result.predicted.mean),
      TKalMatrix(result.predicted.covariance), site, TVKalSite::kPredicted, 6));
  if (!site.Filter()) throw std::runtime_error("KalTest rejected persistent-6D hit update");
  const auto& updated = site.GetState(TVKalSite::kFiltered);
  result.filtered = result.predicted;
  result.filtered.mean = updated;
  result.filtered.covariance = updated.GetCovMat();
  validateCovariance(result.filtered.covariance);
  result.chi2 = site.GetDeltaChi2();
  result.dimension = site.GetDimension();
  return result;
}

MeasurementStep KalmanAdapter::seedRelinearized(const TrackState& originalPrior,
    edm4hep::TrackerHit hit, const TrackState& reference) const {
  MeasurementStep result;
  TMatrixD pivot(5,5);
  result.predicted=rebase(originalPrior,reference.pivot,m_bz,pivot);
  result.filtered=result.predicted;
  affineUpdate(*m_system,hit,reference,result.filtered.mean,result.filtered.covariance,
               m_maxChi2,result.chi2,result.dimension);
  return result;
}

MeasurementStep KalmanAdapter::advanceRelinearized(const TrackState& source,
    edm4hep::TrackerHit from, edm4hep::TrackerHit to,
    const TrackState& referenceSource, const TrackState& referenceTarget) const {
  const auto geometry=nativeStep(*m_system,m_bz,m_maxChi2,referenceSource,from,to,false,0,0,true);
  TMatrixD pivot(5,5);
  const auto referencePrediction=rebase(geometry.predicted,referenceTarget.pivot,m_bz,pivot);
  MeasurementStep result;
  result.transport=pivot*geometry.transport;
  result.noise=pivot*geometry.noise*transpose(pivot);
  result.predicted=referencePrediction;
  result.predicted.mean += result.transport*stateDifference(source.mean,referenceSource.mean);
  result.predicted.covariance=result.transport*source.covariance*transpose(result.transport)+result.noise;
  validateCovariance(result.predicted.covariance);
  result.filtered=result.predicted;
  affineUpdate(*m_system,to,referenceTarget,result.filtered.mean,result.filtered.covariance,
               m_maxChi2,result.chi2,result.dimension);
  return result;
}

LossMeasurementStep KalmanAdapter::advanceRelinearized(const LossTrackState& source,
    edm4hep::TrackerHit from, edm4hep::TrackerHit to, bool birth,
    const LossTrackState& referenceSource, const TrackState& referenceTarget) const {
  auto mapped=referenceSource;
  TMatrixD loss(6,6);
  loss.UnitMatrix();
  if (birth) {
    loss(2,2)=std::exp(referenceSource.mean(5,0));
    mapped.mean(2,0)*=loss(2,2);
    loss(2,5)=mapped.mean(2,0);
    mapped.covariance=loss*referenceSource.covariance*transpose(loss);
  }
  const auto geometry=nativeStep(*m_system,m_bz,m_maxChi2,mapped.track(),from,to,false,0,0,true);
  TMatrixD pivot(5,5);
  const auto target=rebase(geometry.predicted,referenceTarget.pivot,m_bz,pivot);
  const TMatrixD f=pivot*geometry.transport;
  const TMatrixD q=pivot*geometry.noise*transpose(pivot);
  TMatrixD transport(6,6);
  transport.UnitMatrix();
  LossMeasurementStep result;
  for(int i=0;i<5;++i)
    for(int j=0;j<5;++j) { transport(i,j)=f(i,j); result.noise(i,j)=q(i,j); }
  result.transport=transport*loss;
  result.predicted=referenceSource;
  result.predicted.pivot=target.pivot;
  for(int i=0;i<5;++i) result.predicted.mean(i,0)=target.mean(i,0);
  // Essential affine offset. A new expansion point is NOT a new prior mean.
  result.predicted.mean += result.transport*stateDifference(source.mean,referenceSource.mean);
  result.predicted.covariance=result.transport*source.covariance*transpose(result.transport)+result.noise;
  validateCovariance(result.predicted.covariance);
  result.filtered=result.predicted;
  affineUpdate(*m_system,to,referenceTarget,result.filtered.mean,result.filtered.covariance,
               m_maxChi2,result.chi2,result.dimension);
  return result;
}

MeasurementStep KalmanAdapter::advanceBackwardRelinearized(const TrackState& source,
    edm4hep::TrackerHit from, edm4hep::TrackerHit to, bool breakpoint,
    double priorLoss, double sigmaLoss, double referenceLoss,
    const TrackState& referenceSource, const TrackState& referenceTarget) const {
  if (!breakpoint)
    return advanceRelinearized(source, from, to, referenceSource, referenceTarget);

  // Match advanceBackward: geometry/material first, inverse loss at the
  // upstream target surface, then the measurement. No extra physical loss.
  const auto geometry = nativeStep(*m_system, m_bz, m_maxChi2,
      referenceSource, from, to, false, 0, 0, true);
  TMatrixD pivot(5, 5);
  auto prediction = rebase(geometry.predicted, referenceTarget.pivot, m_bz, pivot);
  TMatrixD inverseLoss(5, 5), derivative(5, 1);
  inverseLoss.UnitMatrix();
  inverseLoss(2, 2) = std::exp(-referenceLoss);
  prediction.mean(2, 0) *= inverseLoss(2, 2);
  derivative(2, 0) = -prediction.mean(2, 0);
  const double variance = sigmaLoss * sigmaLoss;

  MeasurementStep result;
  result.transport = inverseLoss * pivot * geometry.transport;
  result.noise = inverseLoss * pivot * geometry.noise
      * transpose(pivot) * transpose(inverseLoss)
      + variance * derivative * transpose(derivative);
  result.lossTargetCross = variance * transpose(derivative);
  result.predicted = prediction;
  result.predicted.mean += result.transport * stateDifference(source.mean, referenceSource.mean)
      + derivative * (priorLoss - referenceLoss);
  result.predicted.covariance = result.transport * source.covariance
      * transpose(result.transport) + result.noise;
  validateCovariance(result.predicted.covariance);
  // Independent joint-6D assembly audits the marginalized inverse transition.
  Matrix6 joint{};
  Matrix5 f{}, q{};
  Vector5 db{};
  const TMatrixD nativeNoise = inverseLoss * pivot * geometry.noise
      * transpose(pivot) * transpose(inverseLoss);
  for (int row = 0; row < 5; ++row) {
    db[row] = derivative(row, 0);
    for (int col = 0; col < 5; ++col) {
      joint[6 * row + col] = source.covariance(row, col);
      f[5 * row + col] = result.transport(row, col);
      q[5 * row + col] = nativeNoise(row, col);
    }
  }
  joint[35] = variance;
  const auto full = AugmentedTransport::covariance(joint,
      AugmentedTransport::jacobian(f, db), AugmentedTransport::processNoise(q));
  for (int row = 0; row < 5; ++row)
    for (int col = 0; col < 5; ++col)
      result.covarianceClosure = std::max(result.covarianceClosure,
          std::abs(full[6 * row + col] - result.predicted.covariance(row, col))
          / std::sqrt(result.predicted.covariance(row, row) * result.predicted.covariance(col, col)));
  result.filtered = result.predicted;
  affineUpdate(*m_system, to, referenceTarget, result.filtered.mean,
      result.filtered.covariance, m_maxChi2, result.chi2, result.dimension);
  return result;
}

edm4hep::TrackState KalmanAdapter::propagateToIP(const TrackState& state,
                                               edm4hep::TrackerHit hit) const {
  auto track = initialized(toEDM(state, m_bz, 2), hit, false);
  edm4hep::TrackState ip;
  double chi2 = 0;
  int ndf = 0;
  requireSuccess(track->propagate(edm4hep::Vector3d{0, 0, 0}, ip, chi2, ndf),
                 "Backward KF IP propagation");
  // Native propagation fills the parameters but does not label the EDM state.
  // Our published result is explicitly the interaction-point endpoint.
  ip.location = edm4hep::TrackState::AtIP;
  return ip;
}

edm4hep::TrackState KalmanAdapter::atIP(const TrackState& state,
                                      edm4hep::TrackerHit hit) const {
  (void)hit;
  THelicalTrack helix(state.mean(0, 0), state.mean(1, 0), state.mean(2, 0),
      state.mean(3, 0), state.mean(4, 0), state.pivot.x, state.pivot.y, state.pivot.z, m_bz);
  TrackState moved = state;
  double angle = 0;
  helix.MoveTo(TVector3(0, 0, 0), angle, nullptr, &moved.covariance);
  moved.mean(0, 0) = helix.GetDrho(); moved.mean(1, 0) = helix.GetPhi0();
  moved.mean(2, 0) = helix.GetKappa(); moved.mean(3, 0) = helix.GetDz();
  moved.mean(4, 0) = helix.GetTanLambda(); moved.pivot = {0, 0, 0};
  return toEDM(moved, m_bz, 1);
}
} // namespace breakpoint
