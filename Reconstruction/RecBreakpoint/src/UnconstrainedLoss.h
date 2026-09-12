#ifndef RECBREAKPOINT_UNCONSTRAINEDLOSS_H
#define RECBREAKPOINT_UNCONSTRAINEDLOSS_H

#include "GaussianTrackModel.h"

namespace breakpoint {

/// Scalar diffuse regression accompanying the EXISTING conditional native KF.
/// For a fixed affine model, innovation(b) = innovation(0) - response * delta_b.
/// Starting with precision=0 imposes NO Gaussian prior on delta_b. This is not
/// a large-variance approximation and does not perform a track measurement update.
class UnconstrainedLoss {
public:
  /// Return the increment of the minimized prefix innovation quadratic.
  double add(const GaussianHitModel& hit, const TMatrixD& predictedCovariance,
             const TMatrixD& predictedResponse);
  bool identified() const;
  double shift() const;
  double variance() const;
  double information() const { return m_information; }
  double quadratic() const;
private:
  double m_information = 0, m_score = 0, m_referenceQuadratic = 0;
};

} // namespace breakpoint
#endif
