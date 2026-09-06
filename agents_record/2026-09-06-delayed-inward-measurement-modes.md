# Inward look-ahead measurement modes

Date: 2026-09-06

## Superseded first contract: live hit skipping

This experiment tests whether the measurement immediately adjacent to an
inward Bethe-Heitler split is responsible for the light-loss negative peak.
For a reverse state last updated at hit `i+1`, the existing
`LocalMeasurement` flow splits for the adjacent interval `i+1 -> i`,
propagates to hit `i`, then updates and reweights there.

The first implementation added two default-off `InwardWeightMode` values:

- `NextMeasurement`: after an actual inward BH split for `i+1 -> i`, omit hit
  `i` completely and first update/reweight at hit `i-1`.
- `NextNextMeasurement`: after the split, omit hits `i` and `i-1` completely
  and first update/reweight at hit `i-2`.

Omitted hits contribute neither a Kalman update nor a likelihood. The selected
target is clamped to hit 0, so the terminal reverse state is still updated at
the innermost hit. An interval without an actual BH split continues to use its
adjacent inward hit normally. To keep this bounded and avoid mixing two
pending split cohorts, the jump to the selected measurement makes no
additional BH split on an intervening skipped interval. Cutoff and KL
reduction occur only after the selected measurement update.

This live-hit-skipping contract was superseded later on the same date. Its
complete evidence is retained below because it explains why simply discarding
the adjacent measurements was rejected. The compiled and maintained-card
default remained `LocalMeasurement`; `SmoothedMarginal` was unchanged.

## Superseded hit-skipping same-code setup

The focused tests inherited
`gsf_barrel_newbh_20260906/runcards/rungsf-e--2.0-85-1-truth-bh-off.py`:

- reverse refit;
- `CEPCRuntimeCategoryAligned9Clear`;
- `DD4hepBetweenSurfaces`;
- forward/inward splitting `false/true`;
- fresh inward seed (`InwardSeedCovarianceScale=-1`, `BackwardSeed=1`);
- `MaxComponents=10`, `ComponentWeightCutoff=1e-4`;
- `SymmetricKL` and identity protection disabled, following the immediately
  preceding negative-peak identity-merge experiment;
- truth loss override disabled;
- `MSOn=true`, `ElossOn=false`.

Only `InwardWeightMode`, selected events, verbosity, and temporary output names
were overridden. A comprehensive verbose run on zero-based event 7 confirmed
that `NextMeasurement` jumped from local hit 231 to target 230 and that
`NextNextMeasurement` jumped from local 231 to target 229. The latter also
showed the explicit near-IP `local=2, target=0, hit0-clamped` gate. Both runs
completed with ten final components and successful FullMixtureMode status.

## Superseded hit-skipping negative-peak evidence

The selected topology-clear events have 0.2--2% Geant4 eBrem loss and a
negative stored LocalMeasurement FullMixtureMode residual. Residuals below are
`100 * (pT_reco/pT_truth - 1)` in percent. `iev` is the one-based flat-tuple
event number.

| iev | truth loss [%] | LCIO | Local FullMix | Next FullMix | NextNext FullMix |
|---:|---:|---:|---:|---:|---:|
| 3 | 1.092 | -0.860 | -0.883 | -0.865 | -0.879 |
| 5 | 0.814 | -1.086 | -1.094 | -1.108 | -1.062 |
| 8 | 0.275 | -0.263 | -0.225 | -0.261 | -0.283 |
| 31 | 0.501 | -0.608 | -0.629 | -0.642 | -0.627 |
| 41 | 1.728 | -1.833 | -1.807 | -1.826 | -1.818 |
| 55 | 1.319 | -1.229 | -1.210 | -1.247 | -1.276 |
| 62 | 1.258 | -0.866 | -0.851 | -0.780 | -0.823 |
| 73 | 0.462 | -0.389 | -0.405 | -0.400 | -0.408 |
| 98 | 0.232 | -0.428 | -0.417 | -0.452 | -0.427 |
| 103 | 1.114 | -0.791 | -0.880 | -0.922 | +1.860 |

Endpoint summaries over these deliberately selected ten events are:

| endpoint | mode | mean [%] | mean absolute [%] | RMS [%] |
|---|---|---:|---:|---:|
| BestBranch | Local | -0.403 | 0.713 | 0.863 |
| BestBranch | Next | -0.674 | 0.842 | 0.950 |
| BestBranch | NextNext | -0.575 | 0.947 | 1.087 |
| WeightedMean | Local | -0.197 | 0.474 | 0.643 |
| WeightedMean | Next | -0.361 | 0.573 | 0.710 |
| WeightedMean | NextNext | -0.347 | 0.675 | 0.905 |
| FullMixtureMode | Local | -0.840 | 0.840 | 0.948 |
| FullMixtureMode | Next | -0.850 | 0.850 | 0.957 |
| FullMixtureMode | NextNext | -0.574 | 0.946 | 1.086 |

The less-negative NextNext mean is cancellation from the `iev=103` positive
outlier, not improved resolution.

## Superseded hit-skipping hard-loss gate

The standard zero-based events 11, 16, and 17 were rerun with all three modes.

| selected index | LCIO [%] | Local FullMix [%] | Next FullMix [%] | NextNext FullMix [%] |
|---:|---:|---:|---:|---:|
| 11 | +0.402 | +0.414 | +0.389 | +0.419 |
| 16 | -51.726 | -51.742 | -51.742 | -51.739 |
| 17 | -21.228 | -0.265 | +5.618 | -2.938 |

All modes executed successfully, retained finite endpoint mixtures, and kept
the LocalMeasurement regression bit-for-bit consistent on the ten selected
negative events. The delayed modes are mechanically available but are not
promoted: they did not improve the negative-peak sample and substantially
degraded the recovered hard-loss control event 17. The result is also specific
to the bounded no-intervening-split contract and is not a general validation
of every possible delayed-likelihood construction.

## Current contract: temporary look-ahead reweighting

The requested replacement keeps every hit in the live reconstruction. For an
inward BH split on interval `i+1 -> i`:

1. live children are created at the split surface with normalized
   `parent weight * BH prior` weights;
2. temporary copies propagate directly from `i+1` to the probe hit, without
   intermediate measurement updates or additional explicit BH splitting;
3. `NextMeasurement` probes `max(0,i-1)` and `NextNextMeasurement` probes
   `max(0,i-2)`;
4. the exact probe likelihood is multiplied by each child prior and the probe
   posterior is normalized over successful children;
5. only those normalized weights are transferred back to the live children
   at the split surface; temporary states are discarded;
6. the live mixture resumes the ordinary adjacent update at hit `i`, followed
   by the normal cutoff, KL reduction, material evaluation, and remaining
   inward hits.

The probe target is deliberately evaluated again when the live recursion
later reaches it. This first diagnostic therefore double-counts that
measurement evidence. No cutoff or KL reduction is applied to the temporary
probe population. A failed probe removes that child because no replacement
posterior exists. Hit-0 clamping preserves the terminal update.

Accepted temporary measurements are persisted as passive source-2,
operation-3 lineage nodes with `measurement_status=3`. They are side children
of the split node and receive their normalized probe posterior. The live
lineage remains anchored at the split node, receives the same transferred
weight, and then advances through the ordinary adjacent measurement node.

## Current same-code gates

The current implementation used the same setup and selected events listed
above, including `ProtectIdentityLineage=false`, with a fresh inward seed,
inward-only BH splitting, the category-aligned nine-radiative-component BH
model, ten-component reduction target, `1e-4` cutoff, and no truth override.
The code was rebuilt and installed before all reruns.

The focused zero-based event 7 verbose gates completed for both modes. Each
published endpoint retained all 233 measurement sites, proving that the live
recursion no longer skips hits. Both modes recorded 910 accepted look-ahead
nodes, with finite normalized posteriors. `NextNextMeasurement` explicitly
showed `outer=232, local=231, probe=229`; the analogous
`NextMeasurement` probe is hit 230.

For the ten deliberately selected negative-peak events, current
FullMixtureMode residuals are:

| iev | truth loss [%] | LCIO | Local FullMix | Next FullMix | NextNext FullMix |
|---:|---:|---:|---:|---:|---:|
| 3 | 1.092 | -0.860 | -0.883 | -0.882 | -0.881 |
| 5 | 0.814 | -1.086 | -1.094 | +0.066 | +0.058 |
| 8 | 0.275 | -0.263 | -0.225 | -0.193 | -0.182 |
| 31 | 0.501 | -0.608 | -0.629 | -0.629 | -0.630 |
| 41 | 1.728 | -1.833 | -1.807 | -1.810 | -1.807 |
| 55 | 1.319 | -1.229 | -1.210 | -1.206 | -1.203 |
| 62 | 1.258 | -0.866 | -0.851 | +0.262 | +0.206 |
| 73 | 0.462 | -0.389 | -0.407 | -0.408 | -0.408 |
| 98 | 0.232 | -0.428 | -0.417 | -0.401 | -0.451 |
| 103 | 1.114 | -0.791 | -0.880 | +0.817 | +0.886 |

Same-code endpoint summaries are:

| endpoint | mode | mean [%] | mean absolute [%] | RMS [%] |
|---|---|---:|---:|---:|
| BestBranch | Local | -0.403 | 0.713 | 0.863 |
| BestBranch | Next | -0.439 | 0.668 | 0.838 |
| BestBranch | NextNext | -0.372 | 0.651 | 0.836 |
| WeightedMean | Local | -0.197 | 0.474 | 0.643 |
| WeightedMean | Next | -0.015 | 0.526 | 0.647 |
| WeightedMean | NextNext | +0.025 | 0.503 | 0.628 |
| FullMixtureMode | Local | -0.840 | 0.840 | 0.948 |
| FullMixtureMode | Next | -0.438 | 0.667 | 0.837 |
| FullMixtureMode | NextNext | -0.441 | 0.671 | 0.844 |

The selected negative sample improves in FullMixture mean absolute residual,
but events 5, 62, and 103 cross to positive residuals. This selection was
constructed from negative LocalMeasurement events and is not a population
validation.

The mandatory zero-based hard-loss gates give:

| selected index | LCIO [%] | Local FullMix [%] | Next FullMix [%] | NextNext FullMix [%] |
|---:|---:|---:|---:|---:|
| 11 | +0.402 | +0.414 | +0.406 | +0.407 |
| 16 | -51.726 | -51.742 | -51.742 | -51.742 |
| 17 | -21.228 | -0.265 | -0.932 | -0.532 |

Both modes run to completion and retain event-17 hard-loss recovery, although
they degrade it relative to `LocalMeasurement`. Event 16 remains unrecovered.
The FullMixture optimizer reports its pre-existing non-positive-definite
fallback for event 16 in every current look-ahead run. These modes remain
default-off diagnostics pending an unbiased population check and removal or
explicit calibration of the deliberate probe-evidence double counting.

## Replacement: numeric averaged look-ahead with a preserved prior channel

Later on 2026-09-06, the two fixed string modes above were retired. The live
API now keeps `InwardWeightMode=LocalMeasurement` and adds the nonnegative
integer `InwardLookaheadDepth`, compiled default zero. A positive depth is
invalid with `SmoothedMarginal`.

After a real inward BH split on `i+1 -> i`, depth `N` independently probes
every available farther-inward hit `i-1` through `i-N`. Each temporary probe
starts from the split state, performs no intervening measurement update or
explicit BH split, and is discarded after its likelihood is evaluated. Its
posterior is normalized separately over the successfully evaluated children.
The normalized probe posteriors are then averaged arithmetically. A failed
temporary evaluation contributes zero for that component at that probe and
never deletes the live child.

A wholly invalid probe hit is omitted from the average. If every requested
probe is invalid, the feedback vector falls back to the original prior, so the
two equal channels reduce to the baseline local weighting after normalization.
At local hit zero no farther-inward target exists and only the ordinary
terminal measurement update runs.

The original normalized BH-prior vector and the averaged-feedback vector stay
as two weight channels. The ordinary adjacent hit `i` is evaluated once, so
there is only one updated state and covariance per live child. Both channels
receive that same local likelihood, their unnormalized contributions are
added with no configurable feedback fraction, and the union is globally
normalized before the existing cutoff, KL reduction, and inward recursion.
The probed hits are intentionally used again when the live recursion reaches
them; this remains an uncalibrated evidence-reuse experiment.

Two row-aligned flat-tuple diagnostics decompose the accepted local node:
`lineage_node_prior_local_posterior` and
`lineage_node_lookahead_local_posterior`. The existing
`lineage_node_normalized_posterior` is the combined live posterior. Status-3
nodes retain each separately normalized temporary-probe posterior.

The implementation was built and installed successfully. A verbose depth-1
run on selected index 11 showed finite ten-component probe posteriors and the
original, feedback, and combined weights at each split. A depth-2 run showed
`requested=2, valid=2` where two farther-inward hits existed and one unique
probe near hit 0. On every active local surface, each separately persisted
channel and the combined posterior summed to one within floating-point
precision.

With the current maintained physics controls (including identity protection),
same-code depth-0 reruns reproduced the stored local results exactly for
selected indices 11, 16, and 17. The first depth-1 mechanical comparison was:

| selected index | truth pT [GeV] | depth-0 FullMix pT [GeV] | depth-1 FullMix pT [GeV] | depth-0 residual [%] | depth-1 residual [%] |
|---:|---:|---:|---:|---:|---:|
| 11 | 40.731567 | 40.892867 | 40.892684 | +0.396 | +0.396 |
| 16 | 37.894016 | 18.287020 | 18.287020 | -51.742 | -51.742 |
| 17 | 18.796978 | 18.738430 | 18.659459 | -0.311 | -0.732 |

This is a mechanical gate, not performance validation. Depth 1 preserved the
event-17 hard-loss recovery but degraded its residual, and event 16 remained
unrecovered. The next required gate is an unbiased topology-clear population
comparison across depth 0, 1, and larger depths, with no-ebrem safety and
extreme tails reported separately.
