#ifndef RECBREAKPOINT_DIFFUSELOSSSTATE_H
#define RECBREAKPOINT_DIFFUSELOSSSTATE_H

#include "TrackState.h"

namespace breakpoint {

// Exact rank-one diffuse representation P = P_finite + kappa*u*u^T,
// kappa -> infinity.  The finite reference variance of b is a numerical
// decomposition convention, not a Gaussian loss prior.
struct DiffuseLossState {
  TMatrixD mean{6, 1};
  TMatrixD finiteCovariance{6, 6};
  TMatrixD diffuseDirection{6, 1};

  bool unresolved() const;
};

struct DiffuseHitUpdate {
  int diffuseDimensionsConsumed = 0;
  double finiteChi2 = 0;
};

// The native site supplies H, R, and innovation at the predicted state.
// Only the unresolved diffuse phase uses these special scalar recursions;
// after u vanishes, subsequent complete hits use native KalTest filtering.
DiffuseHitUpdate updateDiffuseHit(DiffuseLossState& state,
                                  const TMatrixD& derivative,
                                  const TMatrixD& noise,
                                  const TMatrixD& innovation);

// Limiting RTS gain as kappa -> infinity. Source and target dimensions may
// differ at the 5D -> 6D birth transition.
TMatrixD diffuseRtsGain(const TMatrixD& sourceFiniteCovariance,
                        const TMatrixD& sourceDiffuseDirection,
                        const TMatrixD& transport,
                        const TMatrixD& targetPredictedFiniteCovariance,
                        const TMatrixD& targetPredictedDiffuseDirection);

// Numerically stable Joseph form for the finite part of the limiting RTS
// covariance. At the birth edge finiteProcessNoise includes the arbitrary
// reference b variance transported to the target; it is not a loss prior.
TMatrixD diffuseRtsCovariance(const TMatrixD& sourceFiniteCovariance,
                              const TMatrixD& gain, const TMatrixD& transport,
                              const TMatrixD& finiteProcessNoise,
                              const TMatrixD& targetSmoothedCovariance);

} // namespace breakpoint
#endif
