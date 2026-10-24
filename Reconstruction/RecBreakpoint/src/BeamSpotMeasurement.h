#ifndef RECBREAKPOINT_BEAMSPOTMEASUREMENT_H
#define RECBREAKPOINT_BEAMSPOTMEASUREMENT_H

#include "TrackState.h"
#include "GaussianTrackModel.h"

namespace breakpoint {
struct BeamSpotSettings {
  double x = 0, y = 0; // transverse beam position [mm]; no longitudinal constraint
  double sigmaX = 0.0145, sigmaY = 3.6e-5; // transverse widths [mm]
};

struct BeamSpotMeasurement {
  TrackState predicted, filtered, smoothed;
  GaussianHitModel observation{TMatrixD(1,5), TMatrixD(1,1), TMatrixD(1,1)};
  double predictedDrho = 0, filteredDrho = 0, smoothedDrho = 0, smoothedAffineDrho = 0;
  double beamVariance = 0, innovationVariance = 0, chi2 = 0, smoothedChi2 = 0;
};

// The physical beam observation is expressed at the first-hit pivot using the
// native helix Jacobian. This boundary projection is geometric/material-free.
GaussianHitModel beamSpotObservation(const TrackState& state, double bz,
                                    const BeamSpotSettings& beam);
BeamSpotMeasurement updateBeamSpot(const TrackState& state, double bz,
                                   const BeamSpotSettings& beam);
void smoothBeamSpot(BeamSpotMeasurement& measurement, const TrackState& state,
                    double bz, const BeamSpotSettings& beam);
} // namespace breakpoint
#endif
