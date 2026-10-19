#include "NeutralEnergyMeasurement.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace breakpoint {
namespace {
double energyError(const NeutralEnergyProjection& projection, const LossTrackState& state) {
  const double variance = (projection.derivative * state.covariance *
                           transpose(projection.derivative))(0, 0);
  if (!std::isfinite(variance) || variance < -1.e-12)
    throw std::runtime_error("Invalid neutral-energy covariance");
  return std::sqrt(std::max(0., variance));
}
}

NeutralEnergyProjection neutralEnergyProjection(const LossTrackState& state) {
  const double kappa = state.mean(2, 0), tanl = state.mean(4, 0), b = state.mean(5, 0);
  if (!std::isfinite(kappa) || kappa == 0 || !std::isfinite(tanl) || !std::isfinite(b))
    throw std::runtime_error("Invalid neutral-energy state");
  const double momentum = std::hypot(1., tanl) / std::abs(kappa);
  NeutralEnergyProjection result;
  result.energy = momentum * std::expm1(b);
  result.derivative.Zero();
  result.derivative(0, 2) = -result.energy / kappa;
  result.derivative(0, 4) = result.energy * tanl / (1. + tanl * tanl);
  result.derivative(0, 5) = momentum * std::exp(b);
  if (!std::isfinite(result.energy) || !std::isfinite(result.derivative(0, 5)))
    throw std::runtime_error("Nonfinite neutral-energy prediction");
  return result;
}

NeutralEnergyMeasurement updateNeutralEnergy(const LossTrackState& predicted,
                                            double energy, double sigma) {
  if (!std::isfinite(energy) || energy <= 0 || !std::isfinite(sigma) || sigma <= 0)
    throw std::invalid_argument("Invalid neutral ECAL energy measurement");
  NeutralEnergyMeasurement result;
  result.predicted = predicted;
  result.filtered = predicted;
  result.energy = energy;
  result.sigma = sigma;
  const auto projection = neutralEnergyProjection(predicted);
  const auto& h = projection.derivative;
  result.predictedEnergy = projection.energy;
  result.predictedError = energyError(projection, predicted);
  result.residual = energy - projection.energy;
  const double variance = sigma * sigma;
  result.innovationVariance = result.predictedError * result.predictedError + variance;
  if (!std::isfinite(result.innovationVariance) || result.innovationVariance <= 0)
    throw std::runtime_error("Invalid neutral-energy innovation variance");
  const TMatrixD gain = predicted.covariance * transpose(h) * (1. / result.innovationVariance);
  result.filtered.mean += gain * result.residual;
  // The energy Jacobian contains 1/|kappa|; crossing its pole invalidates this trial.
  if (result.filtered.mean(2, 0) * predicted.mean(2, 0) <= 0)
    throw std::runtime_error("Neutral-energy update crosses zero curvature");
  TMatrixD remaining(6, 6);
  remaining.UnitMatrix();
  remaining -= gain * h;
  result.filtered.covariance = remaining * predicted.covariance * transpose(remaining)
      + variance * gain * transpose(gain);
  validateCovariance(result.filtered.covariance);
  result.chi2 = result.residual * result.residual / result.innovationVariance;
  const auto filtered = neutralEnergyProjection(result.filtered);
  result.filteredEnergy = filtered.energy;
  result.filteredError = energyError(filtered, result.filtered);
  result.observation.derivative.ResizeTo(h);
  result.observation.derivative = h;
  result.observation.noise.ResizeTo(1, 1);
  result.observation.noise(0, 0) = variance;
  result.observation.residual.ResizeTo(1, 1);
  result.observation.residual(0, 0) = result.residual;
  return result;
}

void assignSmoothedNeutralEnergy(NeutralEnergyMeasurement& measurement,
                                const LossTrackState& smoothed) {
  if (smoothed.mean(2, 0) * measurement.predicted.mean(2, 0) <= 0)
    throw std::runtime_error("Neutral-energy smoothing crosses zero curvature");
  measurement.smoothed = smoothed;
  const auto projection = neutralEnergyProjection(smoothed);
  measurement.smoothedEnergy = projection.energy;
  measurement.smoothedError = energyError(projection, smoothed);
  measurement.smoothedAffineEnergy = measurement.predictedEnergy +
      (measurement.observation.derivative *
       stateDifference(smoothed.mean, measurement.predicted.mean))(0, 0);
  const double residual = measurement.energy - measurement.smoothedAffineEnergy;
  measurement.smoothedChi2 = residual * residual / (measurement.sigma * measurement.sigma);
}
} // namespace breakpoint
