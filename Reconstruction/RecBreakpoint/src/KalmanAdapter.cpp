#include "KalmanAdapter.h"
#include "BreakpointTrackSystem.h"
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
    const TrackState& input, edm4hep::TrackerHit sourceHit, edm4hep::TrackerHit targetHit) {
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
  if (!track.AddAndFilter(*target)) throw std::runtime_error("KalTest rejected propagation/hit update");
  auto* targetPointer = target.release(); // track now owns the accepted site
  MeasurementStep result;
  result.filtered = readNative(*targetPointer, TVKalSite::kFiltered);
  result.predicted = readNative(*targetPointer, TVKalSite::kPredicted);
  result.transport = sourcePointer->GetState(TVKalSite::kFiltered).GetPropMat() * pivotJacobian;
  result.noise = sourcePointer->GetState(TVKalSite::kFiltered).GetProcNoiseMat();
  result.chi2 = targetPointer->GetDeltaChi2();
  result.dimension = targetPointer->GetDimension();
  return result;
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

edm4hep::TrackState KalmanAdapter::prefit(const std::vector<edm4hep::TrackerHit>& hits,
                                       double scale) const {
  std::vector<edm4hep::TrackerHit> prefitHits;
  for (auto hit : hits) {
    if (!UTIL::BitSet32(hit.getType())[UTIL::ILDTrkHitTypeBit::ONE_DIMENSIONAL])
      prefitHits.push_back(hit);
    if (prefitHits.size() == 3) break;
  }
  if (prefitHits.size() != 3) throw std::runtime_error("Need three two-dimensional seed hits");
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
    const std::vector<edm4hep::TrackerHit>& hits, double scale) const {
  auto track = initialized(prefit(hits, scale), hits.front(), false);
  for (auto hit : hits) {
    double chi2 = 0;
    requireSuccess(track->addAndFit(hit, chi2, m_maxChi2), "Reference KF update");
  }
  requireSuccess(track->smooth(), "Reference KF smooth");
  edm4hep::TrackState ip;
  auto first = hits.front();
  double chi2 = 0;
  int ndf = 0;
  requireSuccess(track->extrapolate(edm4hep::Vector3d{0, 0, 0}, first, ip, chi2, ndf),
                 "Reference KF IP extrapolation");
  return ip;
}

MeasurementStep KalmanAdapter::advance(const TrackState& source,
    edm4hep::TrackerHit sourceHit, edm4hep::TrackerHit targetHit) const {
  return nativeStep(*m_system, m_bz, m_maxChi2, source, sourceHit, targetHit);
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
