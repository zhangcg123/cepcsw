#ifndef RECBREAKPOINT_NEUTRALENERGYMEASUREMENT_H
#define RECBREAKPOINT_NEUTRALENERGYMEASUREMENT_H

#include "LossTrackState.h"
#include "GaussianTrackModel.h"
#include <limits>

namespace breakpoint {
struct NeutralEnergyProjection {
  double energy = 0;
  TMatrixD derivative{1, 6};
};

/// Photon energy at the post-loss side of the breakpoint; electron mass neglected.
NeutralEnergyProjection neutralEnergyProjection(const LossTrackState& state);

struct NeutralEnergyMeasurement {
  LossTrackState predicted, filtered, smoothed;
  GaussianHitModel observation{TMatrixD(1, 6), TMatrixD(1, 1), TMatrixD(1, 1)};
  double energy = std::numeric_limits<double>::quiet_NaN(), sigma = energy;
  double predictedEnergy = energy, predictedError = energy;
  double filteredEnergy = energy, filteredError = energy;
  double smoothedEnergy = energy, smoothedError = energy, smoothedAffineEnergy = energy;
  double residual = energy, innovationVariance = energy, chi2 = energy, smoothedChi2 = energy;
};

/// One scalar observation of the full joint state, retaining track/loss correlations.
NeutralEnergyMeasurement updateNeutralEnergy(const LossTrackState& predicted,
                                            double energy, double sigma);
void assignSmoothedNeutralEnergy(NeutralEnergyMeasurement& measurement,
                                const LossTrackState& smoothed);
} // namespace breakpoint
#endif
