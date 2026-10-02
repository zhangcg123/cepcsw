#include "DiffuseLossState.h"

#include <cmath>
#include <stdexcept>

using breakpoint::DiffuseLossState;

namespace {
void close(double actual, double expected, double tolerance = 1.e-9) {
  if (!std::isfinite(actual) || std::abs(actual - expected) > tolerance)
    throw std::runtime_error("Diffuse scalar test failed");
}

DiffuseLossState initial() {
  DiffuseLossState state;
  state.mean.Zero();
  state.finiteCovariance.Zero();
  state.diffuseDirection.Zero();
  for (int i = 0; i < 6; ++i) state.finiteCovariance(i, i) = 1;
  state.finiteCovariance(0, 0) = 4;
  state.diffuseDirection(5, 0) = 1;
  return state;
}
} // namespace

int main() {
  // x~N(0,4), b diffuse, y=x+b+epsilon with Var(epsilon)=9.
  // The hit fixes x+b but cannot update x alone; b posterior is N(y,13).
  auto state = initial();
  TMatrixD h(1, 6), r(1, 1), v(1, 1);
  h.Zero(); h(0, 0) = h(0, 5) = 1;
  r(0, 0) = 9; v(0, 0) = 10;
  const auto score = breakpoint::updateDiffuseHit(state, h, r, v);
  if (score.diffuseDimensionsConsumed != 1 || state.unresolved())
    throw std::runtime_error("Diffuse direction was not consumed");
  close(state.mean(0, 0), 0);
  close(state.mean(5, 0), 10);
  close(state.finiteCovariance(0, 0), 4);
  close(state.finiteCovariance(0, 5), -4);
  close(state.finiteCovariance(5, 5), 13);

  // A measurement insensitive to b updates x normally and leaves b diffuse.
  state = initial();
  h(0, 5) = 0;
  const auto finite = breakpoint::updateDiffuseHit(state, h, r, v);
  if (finite.diffuseDimensionsConsumed || !state.unresolved())
    throw std::runtime_error("An insensitive hit consumed the diffuse direction");
  close(state.mean(0, 0), 40. / 13);
  close(state.finiteCovariance(0, 0), 36. / 13);
  h(0, 5) = 1;
  v(0, 0) = 0;
  breakpoint::updateDiffuseHit(state, h, r, v);
  if (state.unresolved()) throw std::runtime_error("Second hit failed to resolve b");
  close(state.finiteCovariance(5, 5), 36. / 13 + 9);

  // One native two-coordinate hit. The first coordinate consumes the diffuse
  // direction; the second then constrains the ordinary x coordinate.
  state = initial();
  TMatrixD h2(2, 6), r2(2, 2), v2(2, 1);
  h2.Zero(); r2.Zero();
  h2(0, 0) = h2(0, 5) = h2(1, 0) = 1;
  r2(0, 0) = 9; r2(1, 1) = 1;
  v2(0, 0) = 10; v2(1, 0) = 2;
  const auto two = breakpoint::updateDiffuseHit(state, h2, r2, v2);
  if (two.diffuseDimensionsConsumed != 1 || state.unresolved())
    throw std::runtime_error("Two-coordinate hit failed to identify diffuse b");
  close(state.mean(0, 0), 1.6);
  close(state.mean(5, 0), 8.4);
  close(state.finiteCovariance(0, 0), .8);
  close(state.finiteCovariance(0, 5), -.8);
  close(state.finiteCovariance(5, 5), 9.8);

  auto alternateReference = initial();
  alternateReference.finiteCovariance(5, 5) = 100;
  breakpoint::updateDiffuseHit(alternateReference, h2, r2, v2);
  close(alternateReference.mean(5, 0), state.mean(5, 0));
  close(alternateReference.finiteCovariance(5, 5), state.finiteCovariance(5, 5));

  // Birth RTS gain: x_after=x_before+b, b diffuse. A later exact x_after,b
  // posterior must send only their difference back to x_before.
  TMatrixD source(5, 5), sourceDirection(5, 1), transport(6, 5);
  TMatrixD target(6, 6), targetDirection(6, 1);
  source.UnitMatrix(); source(0, 0) = 4;
  sourceDirection.Zero(); transport.Zero();
  for (int i = 0; i < 5; ++i) transport(i, i) = 1;
  target.UnitMatrix(); target(0, 0) = 5; target(0, 5) = target(5, 0) = 1;
  targetDirection.Zero(); targetDirection(0, 0) = targetDirection(5, 0) = 1;
  const auto gain = breakpoint::diffuseRtsGain(
      source, sourceDirection, transport, target, targetDirection);
  close(gain(0, 0), 1);
  close(gain(0, 5), -1);

  // A loose/ill-conditioned birth edge must use the Joseph covariance, not
  // subtract two nearly equal covariance matrices. Let y=x+b+w, Var(w)=9,
  // and observe y with variance 1. A flat b prior means x remains N(0,4).
  target(0, 0) = 14;
  const auto noisyGain = breakpoint::diffuseRtsGain(
      source, sourceDirection, transport, target, targetDirection);
  close(noisyGain(0, 0), 4. / 13);
  close(noisyGain(0, 5), -4. / 13);
  TMatrixD process(6, 6), targetPosterior(6, 6);
  process.Zero(); targetPosterior.UnitMatrix();
  process(0, 0) = 10; process(0, 5) = process(5, 0) = process(5, 5) = 1;
  targetPosterior(0, 0) = 1;
  targetPosterior(0, 5) = targetPosterior(5, 0) = 1;
  targetPosterior(5, 5) = 14;
  const auto smoothed = breakpoint::diffuseRtsCovariance(
      source, noisyGain, transport, process, targetPosterior);
  close(smoothed(0, 0), 4);
  return 0;
}
