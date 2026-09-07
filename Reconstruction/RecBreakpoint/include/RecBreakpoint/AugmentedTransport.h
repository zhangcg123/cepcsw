#ifndef RECBREAKPOINT_AUGMENTEDTRANSPORT_H
#define RECBREAKPOINT_AUGMENTEDTRANSPORT_H

#include <array>

namespace breakpoint {

using Vector5 = std::array<double, 5>;
using Vector6 = std::array<double, 6>;
using Matrix5 = std::array<double, 25>;
using Matrix6 = std::array<double, 36>;

/// Local linearization for five helix coordinates and one persistent parameter.
/// All matrices are row-major. Coordinate units/order must match the adapter.
/// This helper makes no assumption that KalTest's native sixth coordinate is
/// the breakpoint parameter (KalTest uses that coordinate for t0).
class AugmentedTransport {
public:
  /// Construct J = [F, derivative; 0, 1]. The derivative is the derivative of
  /// the complete predicted helix with respect to the added parameter, at
  /// fixed input helix. F must include any applicable breakpoint mapping.
  static Matrix6 jacobian(const Matrix5& helixJacobian,
                          const Vector5& parameterDerivative);

  /// Embed ordinary five-dimensional process noise without inventing noise
  /// for the persistent fitted parameter. Its uncertainty resides in P.
  static Matrix6 processNoise(const Matrix5& helixNoise);

  /// Return J P J^T + Q, preserving all helix/parameter cross covariances.
  /// Inputs must be finite; P and Q must be symmetric. Positive semidefinite
  /// validation remains the responsibility of the fitter's numerical layer.
  static Matrix6 covariance(const Matrix6& prior, const Matrix6& jacobian,
                            const Matrix6& noise);
};

} // namespace breakpoint
#endif
