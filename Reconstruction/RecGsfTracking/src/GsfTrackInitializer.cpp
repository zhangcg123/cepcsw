#include "GsfTrackInitializer.h"

#include "DDKalTest/DDCylinderHit.h"
#include "DDKalTest/DDPlanarHit.h"
#include "DDKalTest/DDVMeasLayer.h"
#include "DDKalTest/DDVTrackHit.h"
#include "TrackSystemSvc/IMarlinTrack.h"
#include "TrackSystemSvc/IMarlinTrkSystem.h"
#include "TrackSystemSvc/MarlinTrkUtils.h"
#include "kaltest/KalTrackDim.h"
#include "kaltest/THelicalTrack.h"
#include "kaltest/TKalTrackSite.h"
#include "kaltest/TKalTrackState.h"

#include "UTIL/BitSet32.h"
#include "UTIL/ILDConf.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <memory>

namespace {

int covarianceIndex(int row, int column) {
  if (row < column) std::swap(row, column);
  return row * (row + 1) / 2 + column;
}

double assignStandardPrefitCovariance(
    edm4hep::TrackState& state, double bz,
    double kappaCovarianceOverride) {
  const double alpha = bz * 2.99792458e-4;
  const double omegaVariance = kappaCovarianceOverride > 0.0
      ? kappaCovarianceOverride * alpha * alpha
      : 1.0e-4;
  for (auto& value : state.covMatrix) value = 0.0;
  state.covMatrix[0] = 1.0e6;   // Var(d0) [mm^2]
  state.covMatrix[2] = 1.0e2;   // Var(phi) [rad^2]
  state.covMatrix[5] = omegaVariance;  // Var(omega) [mm^-2]
  state.covMatrix[9] = 1.0e6;   // Var(z0) [mm^2]
  state.covMatrix[14] = 1.0e2;  // Var(tanLambda)
  return omegaVariance;
}

TKalTrackSite* makeSiteFromTrackState(
    const edm4hep::TrackState& state,
    const DDVMeasLayer& layer,
    const DDVTrackHit& kalHit,
    double bz) {
  const double alpha = bz * 2.99792458e-4;
  if (alpha == 0.0) return nullptr;

  const TVector3 statePivot(
      state.referencePoint.x, state.referencePoint.y,
      state.referencePoint.z);
  const TVector3 hitPivot = layer.HitToXv(kalHit);
  THelicalTrack helix(
      -state.D0, state.phi - M_PI / 2.0, state.omega / alpha,
      state.Z0, state.tanLambda, statePivot.X(), statePivot.Y(),
      statePivot.Z(), bz);

  TMatrixD covariance(5, 5);
  covariance.Zero();
  const double scale[5] = {-1.0, 1.0, 1.0 / alpha, 1.0, 1.0};
  for (int row = 0; row < 5; ++row) {
    for (int column = 0; column < 5; ++column) {
      covariance(row, column) =
          scale[row] * scale[column] *
          state.covMatrix[covarianceIndex(row, column)];
    }
  }

  double dphi = 0.0;
  TMatrixD jacobian(5, 5);
  jacobian.UnitMatrix();
  helix.MoveTo(hitPivot, dphi, &jacobian, &covariance);

  TKalMatrix parameters(kSdim, 1);
  parameters(0, 0) = helix.GetDrho();
  parameters(1, 0) = helix.GetPhi0();
  parameters(2, 0) = helix.GetKappa();
  parameters(3, 0) = helix.GetDz();
  parameters(4, 0) = helix.GetTanLambda();
  parameters(5, 0) = 0.0;

  TKalMatrix kalCovariance(kSdim, kSdim);
  kalCovariance.Zero();
  for (int row = 0; row < 5; ++row)
    for (int column = 0; column < 5; ++column)
      kalCovariance(row, column) = covariance(row, column);
  kalCovariance(5, 5) = 1.0e6;

  TVTrackHit* siteHit = nullptr;
  if (const auto* cylinder = dynamic_cast<const DDCylinderHit*>(&kalHit))
    siteHit = new DDCylinderHit(*cylinder);
  else if (const auto* plane = dynamic_cast<const DDPlanarHit*>(&kalHit))
    siteHit = new DDPlanarHit(*plane);
  if (!siteHit) return nullptr;

  auto* site = new TKalTrackSite(*siteHit, kSdim);
  site->SetHitOwner();
  site->SetOwner();
  site->SetPivot(hitPivot);
  site->Add(new TKalTrackState(
      parameters, kalCovariance, *site, TVKalSite::kPredicted));
  site->Add(new TKalTrackState(
      parameters, kalCovariance, *site, TVKalSite::kFiltered));
  return site;
}

}  // namespace

GsfTrackInitializationResult GsfTrackInitializer::initialize(
    const std::vector<edm4hep::TrackerHit>& orderedHits,
    const DDVMeasLayer& firstLayer,
    const DDVTrackHit& firstKalHit,
    double bz,
    double kappaCovarianceOverride) const {
  GsfTrackInitializationResult result;
  if (!m_trackSystem) {
    result.error = "baseline MarlinTrk system is unavailable";
    return result;
  }
  if (orderedHits.empty()) {
    result.error = "no ordered hits";
    return result;
  }
  if (bz == 0.0 || !std::isfinite(bz)) {
    result.error = "invalid magnetic field";
    return result;
  }
  if (!std::isfinite(kappaCovarianceOverride)) {
    result.error = "non-finite kappa covariance override";
    return result;
  }

  for (const auto& hit : orderedHits) {
    if (!UTIL::BitSet32(hit.getType())[
            UTIL::ILDTrkHitTypeBit::ONE_DIMENSIONAL])
      ++result.twoDimensionalHitCount;
  }
  if (result.twoDimensionalHitCount < 3) {
    result.error = "fewer than three two-dimensional hits";
    return result;
  }

  std::vector<edm4hep::TrackerHit> mutableHits = orderedHits;
  if (MarlinTrk::createPrefit(
          mutableHits, &result.prefitState, bz,
          MarlinTrk::IMarlinTrack::forward) !=
      MarlinTrk::IMarlinTrack::success) {
    result.error = "standard three-hit helix prefit failed";
    return result;
  }
  result.prefitOmegaVariance = assignStandardPrefitCovariance(
      result.prefitState, bz, kappaCovarianceOverride);
  const double alpha = bz * 2.99792458e-4;
  result.prefitKappaVariance =
      result.prefitOmegaVariance / (alpha * alpha);

  std::unique_ptr<MarlinTrk::IMarlinTrack> track(
      m_trackSystem->createTrack());
  if (!track) {
    result.error = "failed to create baseline MarlinTrk track";
    return result;
  }

  edm4hep::TrackerHit firstHit = orderedHits.front();
  if (track->addHit(firstHit) != MarlinTrk::IMarlinTrack::success ||
      track->initialise(
          result.prefitState, bz, MarlinTrk::IMarlinTrack::forward) !=
          MarlinTrk::IMarlinTrack::success) {
    result.error = "baseline first-hit track initialization failed";
    return result;
  }

  MarlinTrk::MeasurementUpdate update;
  if (track->addAndFit(
          firstHit, result.firstHitDeltaChi2, update, DBL_MAX) !=
          MarlinTrk::IMarlinTrack::success ||
      !update.valid) {
    result.error = "baseline first-hit measurement update failed";
    return result;
  }
  result.firstHitMeasurementDimension = update.residual.rows;
  if (result.firstHitMeasurementDimension <= 0)
    result.firstHitMeasurementDimension = firstKalHit.GetDimension();

  double chi2 = 0.0;
  if (track->getTrackState(
          firstHit, result.firstFilteredState, chi2,
          result.firstHitNdf) != MarlinTrk::IMarlinTrack::success) {
    result.error = "failed to retrieve the first filtered state";
    return result;
  }

  result.site = makeSiteFromTrackState(
      result.firstFilteredState, firstLayer, firstKalHit, bz);
  if (!result.site)
    result.error = "failed to construct the GSF first-hit site";
  return result;
}
