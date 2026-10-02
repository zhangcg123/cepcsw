#ifndef RECBREAKPOINT_BEAMSPOTLIKELIHOOD_H
#define RECBREAKPOINT_BEAMSPOTLIKELIHOOD_H

#include "edm4hep/TrackState.h"

namespace breakpoint {

struct BeamSpotSettings {
  double x = 0;       // nominal transverse beam position [mm]
  double y = 0;
  double sigmaX = 0.0145; // nominal transverse beam widths [mm]
  double sigmaY = 3.6e-5;
};

struct BeamSpotLikelihood {
  double residual = 0; // local drho at the beam pivot [mm]
  double trackVariance = 0;
  double beamVariance = 0;
  double innovationVariance = 0;
  double quadratic = 0;
  double logDeterminant = 0;
  double nll2 = 0; // one normalized scalar Gaussian: -2 log p(beam | hits, mu)
};

/// Score a virtual beam-origin measurement WITHOUT a Kalman update.
/// The supplied RTS IP state is conditioned on detector hits only. Moving its
/// pivot to the beam mean is geometric/material-free, not another fit pass.
BeamSpotLikelihood evaluateBeamSpotLikelihood(const edm4hep::TrackState& ip,
                                               double bz, const BeamSpotSettings& beam);

} // namespace breakpoint
#endif
