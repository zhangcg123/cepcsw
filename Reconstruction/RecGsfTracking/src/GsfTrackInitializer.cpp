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
#include <utility>

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
    const DDVMeasLayer& seedLayer,
    const DDVTrackHit& seedKalHit,
    double bz,
    GsfTrackInitializationDirection direction,
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

  std::vector<std::pair<int, edm4hep::TrackerHit>> twoDimensionalHits;
  twoDimensionalHits.reserve(orderedHits.size());
  for (std::size_t hitIndex = 0; hitIndex < orderedHits.size(); ++hitIndex) {
    const auto& hit = orderedHits[hitIndex];
    if (!UTIL::BitSet32(hit.getType())[
            UTIL::ILDTrkHitTypeBit::ONE_DIMENSIONAL]) {
      twoDimensionalHits.emplace_back(
          static_cast<int>(hitIndex), hit);
    }
  }
  result.twoDimensionalHitCount =
      static_cast<int>(twoDimensionalHits.size());
  if (result.twoDimensionalHitCount < 3) {
    result.error = "fewer than three two-dimensional hits";
    return result;
  }

  const std::size_t prefitBegin =
      direction == GsfTrackInitializationDirection::Inward
          ? twoDimensionalHits.size() - 3
          : 0;
  std::vector<edm4hep::TrackerHit> directionalPrefitHits;
  directionalPrefitHits.reserve(3);
  for (std::size_t prefitIndex = 0; prefitIndex < 3; ++prefitIndex) {
    const auto& selected =
        twoDimensionalHits[prefitBegin + prefitIndex];
    result.prefitHitIndices[prefitIndex] = selected.first;
    directionalPrefitHits.push_back(selected.second);
  }

  const bool fitDirection =
      direction == GsfTrackInitializationDirection::Inward
          ? MarlinTrk::IMarlinTrack::backward
          : MarlinTrk::IMarlinTrack::forward;
  if (MarlinTrk::createPrefit(
          directionalPrefitHits, &result.prefitState, bz, fitDirection) !=
      MarlinTrk::IMarlinTrack::success) {
    result.error = "directional three-hit helix prefit failed";
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

  edm4hep::TrackerHit seedHit =
      direction == GsfTrackInitializationDirection::Inward
          ? orderedHits.back() : orderedHits.front();
  if (track->addHit(seedHit) != MarlinTrk::IMarlinTrack::success ||
      track->initialise(
          result.prefitState, bz, fitDirection) !=
          MarlinTrk::IMarlinTrack::success) {
    result.error = "baseline seed-hit track initialization failed";
    return result;
  }

  MarlinTrk::MeasurementUpdate update;
  if (track->addAndFit(
          seedHit, result.seedHitDeltaChi2, update, DBL_MAX) !=
          MarlinTrk::IMarlinTrack::success ||
      !update.valid) {
    result.error = "baseline seed-hit measurement update failed";
    return result;
  }
  result.seedHitMeasurementDimension = update.residual.rows;
  if (result.seedHitMeasurementDimension <= 0)
    result.seedHitMeasurementDimension = seedKalHit.GetDimension();

  double chi2 = 0.0;
  if (track->getTrackState(
          seedHit, result.seedFilteredState, chi2,
          result.seedHitNdf) != MarlinTrk::IMarlinTrack::success) {
    result.error = "failed to retrieve the seed filtered state";
    return result;
  }

  result.site = makeSiteFromTrackState(
      result.seedFilteredState, seedLayer, seedKalHit, bz);
  if (!result.site)
    result.error = "failed to construct the GSF seed-hit site";
  return result;
}
