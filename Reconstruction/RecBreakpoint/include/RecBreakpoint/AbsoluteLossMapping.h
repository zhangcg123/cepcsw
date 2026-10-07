#ifndef RECBREAKPOINT_ABSOLUTELOSSMAPPING_H
#define RECBREAKPOINT_ABSOLUTELOSSMAPPING_H

#include "RecBreakpoint/AugmentedTransport.h"
#include <cmath>
#include <stdexcept>

namespace breakpoint {

struct AbsoluteLossMapping {
  Vector6 mean;
  Matrix6 jacobian{};
};

/// Loss at the upstream pivot: (drho,phi,kappa,dz,tanLambda,L), kappa=1/pT.
/// An optional track MEAN changes only the expansion point. The live mean is
/// transported as f(reference)+J(reference)*(source-reference), not replaced
/// by the reference. L's expansion point remains its independent ECAL prior.
/// The caller transports its original covariance with the returned Jacobian;
/// no covariance or likelihood from the reference fit enters here.
inline AbsoluteLossMapping absoluteLossMapping(const Vector6& source,
                                               const Vector5* trackReference = nullptr) {
  Vector6 reference = source;
  if (trackReference)
    for (int i = 0; i < 5; ++i) reference[i] = (*trackReference)[i];
  for (int i = 0; i < 6; ++i)
    if (!std::isfinite(source[i]) || !std::isfinite(reference[i]))
      throw std::runtime_error("Nonfinite absolute-loss mapping state");
  const double kappa = reference[2], tanLambda = reference[4], loss = reference[5];
  if (kappa == 0) throw std::runtime_error("Invalid curvature at absolute breakpoint");
  const double before = std::hypot(1.0, tanLambda) / std::abs(kappa);
  const double after = before - loss;
  if (!std::isfinite(before) || loss < 0 || !(after > 0) || !std::isfinite(after))
    throw std::runtime_error("Absolute breakpoint loss exceeds available momentum");
  const double scale = before / after;
  AbsoluteLossMapping result{source, {}};
  for (int i = 0; i < 6; ++i) result.jacobian[i * 6 + i] = 1;
  // Exact derivatives of kappa_after = kappa_before*p_before/(p_before-L).
  result.jacobian[2 * 6 + 2] = scale * scale;
  result.jacobian[2 * 6 + 4] = -kappa * loss * before * tanLambda /
      ((after * after) * (1.0 + tanLambda * tanLambda));
  result.jacobian[2 * 6 + 5] = kappa * before / (after * after);
  result.mean[2] = kappa * scale;
  if (trackReference) {
    result.mean[2] += result.jacobian[2 * 6 + 2] * (source[2] - reference[2])
        + result.jacobian[2 * 6 + 4] * (source[4] - reference[4]);
    // L_source == L_reference by construction. Other coordinates are identity
    // mapped, so they stay at source rather than inheriting diffuse means.
  }
  if (!std::isfinite(result.mean[2]) || result.mean[2] * source[2] <= 0)
    throw std::runtime_error("Absolute-loss affine prediction changed charge or became invalid");
  return result;
}
} // namespace breakpoint
#endif
