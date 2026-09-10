#ifndef RECBREAKPOINT_TRACKLIKELIHOOD_H
#define RECBREAKPOINT_TRACKLIKELIHOOD_H

#include "GaussianTrackModel.h"
#include <limits>

namespace breakpoint {

struct TrackLikelihoodResult {
  double nll2 = 0; // normalized -2 log L; a continuous-density value can be negative
  double quadratic = 0;
  double logDeterminant = 0;
  int measurementDimensions = 0;
  int latentDimensions = 0;
  // Optional independent numerical checks of the SAME marginalized model.
  double reverseOrderNll2 = std::numeric_limits<double>::quiet_NaN();
  double jointSmoothedNll2 = std::numeric_limits<double>::quiet_NaN();
};

/// Marginalize the Gaussian seed/process variables for the frozen native
/// model. QR computes the normal score; optional reverse-order QR and joint
/// SVD check its consistency. These are not three independent objectives.
TrackLikelihoodResult evaluateTrackLikelihood(const GaussianTrackModel& model,
                                             bool checkEquivalentForms = false);

} // namespace breakpoint
#endif
