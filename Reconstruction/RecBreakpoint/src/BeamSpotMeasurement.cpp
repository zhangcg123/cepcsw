#include "BeamSpotMeasurement.h"
#include "ScalarMeasurementSite.h"
#include "kaltest/THelicalTrack.h"
#include "TVector3.h"
#include <cmath>
#include <stdexcept>

namespace breakpoint {
GaussianHitModel beamSpotObservation(const TrackState& state, double bz,
                                    const BeamSpotSettings& beam) {
  if (!std::isfinite(beam.x) || !std::isfinite(beam.y) ||
      !std::isfinite(beam.sigmaX) || !std::isfinite(beam.sigmaY) ||
      beam.sigmaX <= 0 || beam.sigmaY <= 0)
    throw std::invalid_argument("Invalid beam-spot position/widths");
  THelicalTrack helix(state.mean(0,0), state.mean(1,0), state.mean(2,0),
      state.mean(3,0), state.mean(4,0), state.pivot.x, state.pivot.y, state.pivot.z, bz);
  TMatrixD jacobian(5,5);
  jacobian.UnitMatrix();
  double angle = 0;
  helix.MoveTo(TVector3(beam.x, beam.y, 0), angle, &jacobian);
  GaussianHitModel observation{TMatrixD(1,5), TMatrixD(1,1), TMatrixD(1,1)};
  for (int j=0;j<5;++j) observation.derivative(0,j) = jacobian(0,j);
  const double c = std::cos(helix.GetPhi0()), s = std::sin(helix.GetPhi0());
  observation.noise(0,0) = c*c*beam.sigmaX*beam.sigmaX + s*s*beam.sigmaY*beam.sigmaY;
  observation.residual(0,0) = -helix.GetDrho(); // measured drho is zero
  return observation;
}

BeamSpotMeasurement updateBeamSpot(const TrackState& state, double bz,
                                   const BeamSpotSettings& beam) {
  BeamSpotMeasurement result;
  result.predicted = state;
  result.observation = beamSpotObservation(state, bz, beam);
  result.predictedDrho = -result.observation.residual(0,0);
  result.beamVariance = result.observation.noise(0,0);
  const auto& h = result.observation.derivative;
  result.innovationVariance = (h * state.covariance * transpose(h))(0,0) + result.beamVariance;
  if (!std::isfinite(result.innovationVariance) || result.innovationVariance <= 0)
    throw std::runtime_error("Invalid beam innovation variance");
  result.filtered = updateScalarState(state, result.observation);
  result.filteredDrho = -beamSpotObservation(result.filtered, bz, beam).residual(0,0);
  result.chi2 = result.predictedDrho * result.predictedDrho / result.innovationVariance;
  return result;
}

void smoothBeamSpot(BeamSpotMeasurement& measurement, const TrackState& state,
                    double bz, const BeamSpotSettings& beam) {
  measurement.smoothed = state;
  measurement.smoothedDrho = -beamSpotObservation(state, bz, beam).residual(0,0);
  measurement.smoothedAffineDrho = measurement.predictedDrho +
      (measurement.observation.derivative * stateDifference(state.mean, measurement.predicted.mean))(0,0);
  measurement.smoothedChi2 = measurement.smoothedAffineDrho * measurement.smoothedAffineDrho /
      measurement.beamVariance;
}
} // namespace breakpoint
