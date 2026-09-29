#ifndef RECBREAKPOINT_GAUSSIANTRACKMODEL_H
#define RECBREAKPOINT_GAUSSIANTRACKMODEL_H

#include "TMatrixD.h"
#include <vector>

namespace breakpoint {

/// Native hit projection frozen at the forward prediction, not at the truth.
struct GaussianHitModel {
  TMatrixD derivative; // H: measured coordinates versus five track coordinates
  TMatrixD noise;      // V: native measurement covariance
  TMatrixD residual;   // measured hit - h(native predicted state)
};

/// Transition for deviations from the native forward prediction at each hit:
/// delta_i = F * (delta_(i-1) + sourceShift) + w, with w ~ N(0,Q).
struct GaussianTransitionModel {
  TMatrixD transport; // full F, linearized at the selected loss prior center
  TMatrixD noise;     // native Q + loss-prior noise at birth; may be singular
  TMatrixD sourceShift; // native predicted_(i-1) - native updated_(i-1)
};

/// Read-only affine model for one loss-prior trial. It neither updates
/// tracks nor owns any KalTest site. The initial deviation has mean zero.
struct GaussianTrackModel {
  TMatrixD seedCovariance;
  std::vector<GaussianHitModel> hits;
  std::vector<GaussianTransitionModel> transitions; // receiving hits 1..N-1
};

} // namespace breakpoint
#endif
