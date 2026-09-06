# Beam boundary as an inward look-ahead target

## Decision and contract

`InwardLookaheadDepth` now counts inward boundaries rather than detector hits
only. After a real reverse BH split on `i+1 -> i`, the ordered passive probe
targets are:

```text
hit i-1, hit i-2, ..., hit 0, beam spot
```

The last target exists only when `BeamSpotConstraint=true`. Without beam mode,
the established hit-only behavior is unchanged and stops at hit 0. Therefore:

- a split on hit 1 to hit 0 reaches the beam at depth 1;
- a split on hit 2 to hit 1 probes hit 0 and the beam at depth 2;
- higher surfaces reach the beam only when the requested depth extends past
  all remaining real hits.

The beam look-ahead reuses the exact evaluator used by the live terminal beam
update: initialize a backward MarlinTrk state at the current outer reference,
propagate to `(BeamSpotX, BeamSpotY, 0)`, move the prediction to that exact
pivot, and apply the scalar `drho=0` Gaussian likelihood. It does not perform
an additional BH split on skipped intervals, mutate the live component, or
replace the ordinary recursion. Its separately normalized posterior enters the
same arithmetic probe average as real-hit look-aheads. The live algorithm
still updates hit 0 and subsequently performs the configured hit-0-to-beam BH
split, propagation, beam update, cutoff, and KL reduction.

Accepted passive beam probes use the existing lineage measurement status `3`
with hit and surface indices `-1`. The later live beam measurement remains
status `4`. Both persist scalar `dchi2`, `log(S)`, raw log posterior, predicted
state/covariance, and normalized posterior. No property, default, collection,
or flat branch was added.

## Mechanical validation

The EL9 focused build and install completed for `RecGsfTracking` and
`RecGsfFlatTuple`. The test used the maintained beam campaign controls with
`InwardLookaheadDepth=1`, both directional BH splits enabled, fresh inward
initialization, and selected barrel seed-12 index 11. At the hit-1 to hit-0
split, all 100 components produced a valid `probe=beam` result; the summary was
`requested=1, valid=1, components=100`. The ordinary hit-0 update and the later
live beam split/update also completed, and all three endpoints were published.

The required selected indices 11, 16, and 17 then ran successfully. Each event
persisted 100 status-3 beam-probe nodes;
their normalized posterior sums were respectively `0.9999999999999994`,
`0.9999999999999999`, and `0.9999999999999992`. Endpoint pT values in GeV
were:

| index | depth | BestBranch | WeightedMean | FullMixtureMode |
|---:|---:|---:|---:|---:|
| 11 | 0 | 9.0941872928 | 9.1974709981 | 9.0944669756 |
| 11 | 1 | 9.0941872928 | 9.2681549906 | 9.0942916700 |
| 16 | 0 | 38.2564372880 | 38.6806089941 | 38.2574496477 |
| 16 | 1 | 38.2564372880 | 38.8559696969 | 38.2578137551 |
| 17 | 0 | 18.2164186246 | 18.3502975287 | 18.2174039139 |
| 17 | 1 | 18.2164186246 | 18.3442123864 | 18.2173448472 |

The depth-0 values reproduce the pre-refactor beam-on focused results to the
stored precision, establishing that sharing the evaluator did not alter the
default look-ahead-off path. The depth-1 differences are mechanics evidence,
not a performance result. Population validation must compare beam depth 0 and
positive depths with no-eBrem safety and catastrophic tails reported
separately.
