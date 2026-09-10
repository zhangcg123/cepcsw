#ifndef RECBREAKPOINT_TRACKLIKELIHOOD_H
#define RECBREAKPOINT_TRACKLIKELIHOOD_H

#include "GaussianTrackModel.h"

namespace breakpoint {

struct TrackLikelihoodResult {
  double nll2 = 0; // normalized -2 log L; a continuous-density value can be negative
  double quadratic = 0;
  double logDeterminant = 0;
  int measurementDimensions = 0;
  int latentDimensions = 0;
};

/// Marginalize the Gaussian seed/process variables for the frozen native
/// model. QR computes the normalized full-track likelihood.
TrackLikelihoodResult evaluateTrackLikelihood(const GaussianTrackModel& model);

} // namespace breakpoint
#endif
