#include "BeamSpotMeasurement.h"
#include "ScalarMeasurementSite.h"
#include "kaltest/THelicalTrack.h"
#include "TVector3.h"
#include <cmath>
#include <stdexcept>

namespace breakpoint {
TrackState repivotBeamState(const TrackState& state, const edm4hep::Vector3d& pivot, double bz) {
  auto result = state;
  THelicalTrack helix(state.mean(0,0), state.mean(1,0), state.mean(2,0),
      state.mean(3,0), state.mean(4,0), state.pivot.x, state.pivot.y, state.pivot.z, bz);
  double angle = 0;
  helix.MoveTo(TVector3(pivot.x, pivot.y, pivot.z), angle, nullptr, &result.covariance);
  helix.PutInto(result.mean);
  result.pivot = pivot;
  validateCovariance(result.covariance);
  return result;
}

GaussianHitModel beamSpotObservation(const TrackState& state, const BeamSpotSettings& beam) {
  if (!std::isfinite(beam.x) || !std::isfinite(beam.y) ||
      !std::isfinite(beam.sigmaX) || !std::isfinite(beam.sigmaY) ||
      beam.sigmaX <= 0 || beam.sigmaY <= 0)
    throw std::invalid_argument("Invalid beam-spot position/widths");
  if (std::abs(state.pivot.x-beam.x) > 1.e-10 ||
      std::abs(state.pivot.y-beam.y) > 1.e-10 || std::abs(state.pivot.z) > 1.e-10)
    throw std::invalid_argument("Beam measurement requires a beam-pivot state");
  GaussianHitModel observation{TMatrixD(1,5), TMatrixD(1,1), TMatrixD(1,1)};
  observation.derivative.Zero();
  observation.derivative(0,0) = 1.;
  const double c = std::cos(state.mean(1,0)), s = std::sin(state.mean(1,0));
  observation.noise(0,0) = c*c*beam.sigmaX*beam.sigmaX + s*s*beam.sigmaY*beam.sigmaY;
  observation.residual(0,0) = -state.mean(0,0); // measured drho is zero at the beam
  return observation;
}

BeamSpotMeasurement updateBeamSpot(const TrackState& state, const BeamSpotSettings& beam) {
  BeamSpotMeasurement result;
  result.predicted = state;
  result.observation = beamSpotObservation(state, beam);
  result.predictedDrho = -result.observation.residual(0,0);
  result.beamVariance = result.observation.noise(0,0);
  const auto& h = result.observation.derivative;
  result.innovationVariance = (h * state.covariance * transpose(h))(0,0) + result.beamVariance;
  if (!std::isfinite(result.innovationVariance) || result.innovationVariance <= 0)
    throw std::runtime_error("Invalid beam innovation variance");
  result.filtered = updateScalarState(state, result.observation);
  result.filteredDrho = result.filtered.mean(0,0);
  result.chi2 = result.predictedDrho * result.predictedDrho / result.innovationVariance;
  return result;
}

void smoothBeamSpot(BeamSpotMeasurement& measurement, const TrackState& firstHitSmoothed) {
  const auto& prediction = measurement.firstHitPredicted;
  const TMatrixD delta = stateDifference(firstHitSmoothed.mean, prediction.mean);
  const TMatrixD inverse = inverseCovariance(prediction.covariance);
  const TMatrixD gain = measurement.filtered.covariance * transpose(measurement.transport) * inverse;
  measurement.smoothed = measurement.filtered;
  measurement.smoothed.mean += gain * delta;
  TMatrixD remainder(5,5);
  remainder.UnitMatrix();
  remainder -= gain * measurement.transport;
  measurement.smoothed.covariance = remainder * measurement.filtered.covariance * transpose(remainder)
      + gain * (measurement.noise + firstHitSmoothed.covariance) * transpose(gain);
  validateCovariance(measurement.smoothed.covariance);
  measurement.smoothedDrho = measurement.smoothed.mean(0,0);
  measurement.smoothedAffineDrho = measurement.smoothedDrho;
  measurement.smoothedChi2 = measurement.smoothedAffineDrho * measurement.smoothedAffineDrho /
      measurement.beamVariance;
  const TMatrixD u = inverse * delta;
  measurement.processChi2 = (transpose(u) * measurement.noise * u)(0,0);
  if (!std::isfinite(measurement.processChi2) || measurement.processChi2 < -1.e-8)
    throw std::runtime_error("Invalid beam-to-hit process penalty");
  measurement.processChi2 = std::max(0., measurement.processChi2);
}
} // namespace breakpoint
