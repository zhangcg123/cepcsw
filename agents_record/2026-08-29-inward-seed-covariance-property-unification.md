# Common inward seed-covariance property

## Decision

The shared reverse/CMS inward filter now has one covariance-scale control:

```text
InwardSeedCovarianceScale
```

It replaces and removes both historical method-specific properties:

```text
ReverseKappaSeedCov
CmsErrorRescaling
```

No compatibility aliases remain. A stale card must translate the active
method's old value directly to `InwardSeedCovarianceScale`. This is lossless
because `ReverseFiltering` and `CmsGsfSmoothing` are mutually exclusive and
both copy the same final-forward component population into the same inward
filter.

The compiled and active reverse-template default remains 100. The maintained
`DumpGsfTrks/gsf.py.bk` comparison campaign explicitly uses 1.0 for either
reverse or CMS-like. This is campaign steering, not a production-default
change.

## Option-surface audit

The required dedicated sub-agent audited the complete option surface. The two
retired scale properties were the only duplicate method-specific controls in
the shared inward mechanics. The following controls remain intentionally
distinct:

- `ReverseFiltering` and `CmsGsfSmoothing` select different endpoint
  publication: terminal backward versus hit-1 product with terminal fallback.
- `GaussianSumSmoothing` selects a separate retained-graph algorithm.
- `KappaSeedCov` controls the fresh forward initializer, not the inward copied
  covariance.
- `ReverseInitialWeightMode` is already one common seed-weight control; its
  historical name is imperfect but it has no duplicate counterpart.
- reverse selection, ECAL, forward publication, and global-loss properties act
  in different downstream algorithms and are not common-inward duplicates.

After the removal/addition, `RecGsfTracking` exposes 41 properties. The
maintained comparison card explicitly steers 40 and deliberately inherits only
`RecordTruthMaterialIntervals=true`.

The active template environment migration is:

```text
GSF_REVERSE_KAPPA_SEED_COV  -- removed
GSF_CMS_ERROR_RESCALING     -- removed
GSF_INWARD_SEED_COVARIANCE_SCALE -- current
```

The focused no-eBrem helper's command-line option is now
`--inward-seed-covariance-scale`.

Historical `agents_record/` references to the old properties remain unchanged
because they identify the exact steering used by earlier experiments.

## Implementation effect

`GsfAlgorithm` now validates one finite positive property and applies it once
to every covariance copied into `runGsfInwardFilter`. The former method ternary
and one duplicate validation block are gone. `GsfAlgorithm.cpp` decreased from
6,056 to 6,050 lines. No EDM or flat-tuple schema changed.

The generated build-tree and installed `RecGsfTrackingConf.py` expose only
`InwardSeedCovarianceScale=100`; neither retired property remains in generated
metadata.

## Mechanical gate

`RecGsfTracking` and `RecGsfFlatTuple` built and installed successfully. The
maintained reverse template then reran seed 1 entries 11, 16, and 17 with the
five-component `CEPC2GeV85StepConditioned` model, `MaxComponents=12`, cutoff
`5e-3`, and `InwardSeedCovarianceScale=100` for both reverse and CMS-like.

Each method reproduced its pre-migration endpoint values. The ordered
source-1/source-2 statistical-node comparisons found zero mismatches:

| event | compared nodes, reverse | compared nodes, CMS-like |
|---:|---:|---:|
| 11 | 5,773 | 5,773 |
| 16 | 2,976 | 2,976 |
| 17 | 3,536 | 3,536 |

The reproduced FullMixtureMode endpoints are:

| event | truth pT | LCIO pT | reverse | CMS-like |
|---:|---:|---:|---:|---:|
| 11 | 40.731567 | 40.895454 | 40.909320 | 40.909266 |
| 16 | 37.894016 | 18.292832 | 18.318878 | 18.318878 |
| 17 | 18.796978 | 14.806669 | 18.620237 | 18.620109 |

This proves configuration migration and mechanical equivalence only. It does
not add population-level physics validation.
