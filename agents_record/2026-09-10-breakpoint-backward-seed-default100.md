# Breakpoint backward-seed covariance default: 100

The user requested BackwardSeedScale=100 as the default after the independent
three-objective free-loss study. Its former default was 1.

Changed the Gaudi property, internal FitSettings default, flat bookkeeping
initializer and maintained run-card fallback together. The dedicated batch
helper already freezes an explicitly supplied BP_BACKWARD_SEED_SCALE and
otherwise uses the card fallback; no submission/worker change is needed.
An explicit value of 1 remains supported. Previously generated cards and the
isolated three-objective study explicitly selecting 1 are not rewritten.

The existing algorithm is unchanged: copy the final forward updated mean and
multiply its full 5x5 covariance by the scale once, then revisit hits N-2 to 0.
Default 100 means ten times the standard deviations, preserving correlations.
It is not a fresh independent seed. RTS and the loss-prior sigma are unchanged.
Both ordinary and truth-assisted backward pairs use the same setting.

Validation: package build and 22 dedicated batch tests pass. Focused verbose
checks are in TrackingPerformanceStudies/breakpoint_seed100_default_20260910.
Default versus explicit100 matches exactly across all 104 flat fields and
verbose state/covariance dumps for seed12 events11/16/17 (17 remains a secondary
control). Seed2 event68 also exactly reproduces the archived explicit100
iteration-removal gate. The generated configurable default is verified as100.
The audit initially accessed an unset Gaudi property directly; its corrected
getDefaultProperty check and repeat output are saved in verification.log.
The queue query fails because the site's Python cannot import htcondor, so
the shared installed library is deliberately not overwritten in this turn.
The rebuilt plugin is tested through the build-tree runner; the maintained
card explicitly assigns 100 and works with the already supported property.
No ROOT outputs, experimental scripts or unrelated GSF/card changes are staged.
