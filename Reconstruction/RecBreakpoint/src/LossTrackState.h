#ifndef RECBREAKPOINT_LOSSTRACKSTATE_H
#define RECBREAKPOINT_LOSSTRACKSTATE_H

#include "TrackState.h"
#include <cmath>
#include <stdexcept>

namespace breakpoint {
/// Live six-dimensional state. Coordinate 5 always names the SAME breakpoint,
/// not the material at the current pivot. The configured loss is applied only
/// when this state first crosses its owning interval.
struct LossTrackState {
  TMatrixD mean{6, 1};
  TMatrixD covariance{6, 6};
  edm4hep::Vector3d pivot{};

  TrackState track() const {
    TrackState result;
    result.pivot = pivot;
    for (int i = 0; i < 5; ++i) {
      result.mean(i, 0) = mean(i, 0);
      for (int j = 0; j < 5; ++j) result.covariance(i, j) = covariance(i, j);
    }
    return result;
  }
  static LossTrackState introduce(const TrackState& track, double lossValue, double variance) {
    LossTrackState result;
    result.pivot = track.pivot;
    for (int i = 0; i < 5; ++i) {
      result.mean(i, 0) = track.mean(i, 0);
      for (int j = 0; j < 5; ++j) result.covariance(i, j) = track.covariance(i, j);
    }
    result.mean(5, 0) = lossValue;
    result.covariance(5, 5) = variance;
    return result;
  }
};

struct LossMeasurementStep {
  LossTrackState predicted, filtered;
  TMatrixD transport{6, 6};
  TMatrixD noise{6, 6};
  double chi2 = 0;
  int dimension = 0;
};

/// Outward loss at the owning surface, before geometric propagation.
inline LossTrackState applyBreakpointLoss(const LossTrackState& source, TMatrixD& jacobian) {
  const double kappa = source.mean(2, 0);
  if (!std::isfinite(kappa) || kappa == 0)
    throw std::runtime_error("Invalid curvature at persistent breakpoint");
  LossTrackState mapped = source;
  jacobian.ResizeTo(6, 6);
  jacobian.UnitMatrix();
  const double scale = std::exp(source.mean(5, 0));
  mapped.mean(2, 0) *= scale;
  jacobian(2, 2) = scale;
  jacobian(2, 5) = mapped.mean(2, 0);
  mapped.covariance = jacobian * source.covariance * transpose(jacobian);
  return mapped;
}
} // namespace breakpoint
#endif
