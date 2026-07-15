# Light-eBrem decisive-prior audit and rejected split test

## Decisive-hit evidence

The representative state survey was extended from pT trajectories to exact
pre-update weights and innovation likelihoods.

- Missed recovery 37/7 reaches hit 0 with identity prior weight 0.634 and a
  truth-closest radiative branch at only `2.17e-5`. The latter has only about a
  1.63 measurement-likelihood advantage, far too small to overcome its prior.
- Partial recovery 420/3 at hit 6 gives the selected under-correction branch
  prior weight 0.137 versus 0.00472 for the truth-closest branch. Their
  innovation likelihoods are comparable; the selected branch wins mainly
  before the measurement.
- Good recovery 369/1 changes at hit 5. The selected branch has prior 0.0208
  and receives about 67 times the identity likelihood, giving posterior 0.405.
- Moderate overshoot 65/6 at hit 5 has selected prior 0.0106 versus 0.000144
  for the truth-closest branch, followed by an additional likelihood advantage.
- Boundary overshoot 469/6 is different: at hit 0 the eventual winner has
  prior odds about 0.505 relative to identity and measurement likelihood ratio
  about 2.61, producing posterior weights 0.343 versus 0.260. It is a genuine
  near-boundary measurement flip.

Thus missed and partial recovery are often prior/component-history limited,
while some overshoots combine prior misallocation with inner-hit likelihood
selection. No single identity-weight multiplier can fix both directions.

## Retained-component diagnostic

The reverse component cap was made environment-configurable in the focused
option card with unchanged default 12. Running 469/6 with 24 retained
components leaves the result essentially unchanged: pT 2.12346 GeV versus
2.12295 GeV and best weight 0.337 versus 0.343. Missing capacity is therefore
not the immediate cause.

## Rejected 5--20% stratum split

The eBrem-only training data contains one broad 5--20% loss component centered
near 9--11% loss. A controlled six-component candidate split it into 5--10%
and 10--20% strata. The extraction and interpolation constraints passed, and
the package built and installed. Focused comparisons were mixed:

| Event | Baseline residual | Split residual |
|---|---:|---:|
| 37/7 missed | -7.722% | -7.722% |
| 420/3 partial | -2.127% | -2.124% |
| 369/1 good | -0.066% | -0.060% |
| 65/6 overshoot | +2.513% | +2.300% |
| 469/6 boundary | +6.128% | -6.394% |
| 413/3 low-loss | +1.761% | +1.733% |
| 62/9 clean-like | +3.237% | +3.282% |
| 1/3 hard | -0.460% | -0.788% |

All runs completed with finite output; focused reverse summaries still showed
the pre-existing small rejection counts where applicable. The candidate did
not improve missed or partial recovery, merely flipped one moderate overshoot
to a similarly sized under-correction, and degraded the hard representative.
It was rejected before category-scale production. The established
five-component arrays were restored, rebuilt, installed, and 469/6 reproduced
its baseline +6.128% residual.

The extractor retains an opt-in `--split-light-large` diagnostic and the option
card retains `GSF_MAX_COMPONENTS`; neither changes default execution.
