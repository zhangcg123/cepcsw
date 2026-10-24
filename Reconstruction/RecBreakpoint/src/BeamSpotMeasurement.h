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
  TrackState firstHitPredicted;
  TMatrixD transport{5,5}, noise{5,5}; // native beam -> first tracker hit
  GaussianHitModel observation{TMatrixD(1,5), TMatrixD(1,1), TMatrixD(1,1)};
  double predictedDrho = 0, filteredDrho = 0, smoothedDrho = 0, smoothedAffineDrho = 0;
  double beamVariance = 0, innovationVariance = 0, chi2 = 0, smoothedChi2 = 0;
  double processChi2 = 0, covarianceClosure = 0;
};

// Coordinate change only: used for the loose seed and beam-to-origin output.
// Physical beam -> tracker transport is performed separately by KalTest.
TrackState repivotBeamState(const TrackState& state, const edm4hep::Vector3d& pivot, double bz);
GaussianHitModel beamSpotObservation(const TrackState& state, const BeamSpotSettings& beam);
BeamSpotMeasurement updateBeamSpot(const TrackState& state, const BeamSpotSettings& beam);
void smoothBeamSpot(BeamSpotMeasurement& measurement, const TrackState& firstHitSmoothed);
} // namespace breakpoint
#endif
