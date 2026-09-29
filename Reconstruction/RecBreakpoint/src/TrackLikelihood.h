#ifndef RECBREAKPOINT_TRACKLIKELIHOOD_H
#define RECBREAKPOINT_TRACKLIKELIHOOD_H

#include "GaussianTrackModel.h"

namespace breakpoint {

struct TrackLikelihoodResult {
  double nll2 = 0; // normalized -2 log L; a continuous-density value can be negative
  double quadratic = 0; // complete RTS-smoothed chi2 in the free-loss objective
  double logDeterminant = 0; // log det of the joint measurement covariance S_all
  int measurementDimensions = 0;
  int latentDimensions = 0;
};

/// Marginalize the Gaussian seed/process variables for the frozen native
/// model. QR computes the normalized full-track likelihood.
TrackLikelihoodResult evaluateTrackLikelihood(const GaussianTrackModel& model);

/// Use the existing RTS pass's complete measurement + process + seed chi2
/// directly. The supplied model must belong to that same loss-prior trial,
/// including the configured loss variance in its birth process covariance.
/// Add log det S_all and M log(2*pi), NOT a smoothed-state determinant.
TrackLikelihoodResult evaluateSmoothedTrackLikelihood(
    const GaussianTrackModel& model, double completeSmoothedChi2);

} // namespace breakpoint
#endif
