# Delayed inward measurement modes

Date: 2026-09-06

## Question and implemented contract

This experiment tests whether the measurement immediately adjacent to an
inward Bethe-Heitler split is responsible for the light-loss negative peak.
For a reverse state last updated at hit `i+1`, the existing
`LocalMeasurement` flow splits for the adjacent interval `i+1 -> i`,
propagates to hit `i`, then updates and reweights there.

Two default-off `InwardWeightMode` values were added:

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

The compiled and maintained-card default remains `LocalMeasurement`.
`SmoothedMarginal` is unchanged.

## Same-code test setup

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

## Negative-peak evidence

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

## Required hard-loss regression gate

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
