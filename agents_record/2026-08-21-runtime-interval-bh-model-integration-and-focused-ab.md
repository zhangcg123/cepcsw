# Runtime-interval BH model integration and focused A/B

Date: 2026-08-21

## Decision boundary

Two topology-clear, exact-runtime-interval fits were integrated as parallel,
default-off Bethe-Heitler controls inside `RecGsfTracking`:

- `CEPCRuntimeGenericGrid5Clear`: the same-runtime generic logarithmic knot
  grid control;
- `CEPCRuntimeCategoryAligned5Clear`: the interval-band-aligned candidate.

The production default remains `CEPC2GeV85StepConditioned`. Both new models
use the existing scalar `pathTX0` splitter input; no GSF caller interface,
material mode, split threshold, component capacity, reduction, reverse
selection, or publication behavior changed. Neither candidate is approved as
a replacement without held-out population validation and clean-track safety.

## Durable implementation

The authoritative fitted artifacts and their exact dependency-free compiled
tables are paired under:

- `Reconstruction/RecGsfTracking/data/CEPCRuntimeGenericGrid5Clear/`;
- `Reconstruction/RecGsfTracking/data/CEPCRuntimeCategoryAligned5Clear/`.

`BetheHeitlerSplitter` accepts the two canonical selector strings above. The
existing CEPC interpolation was generalized over knot and component counts;
the default and six-component models call that same generalized function with
their original tables. New selector aliases were deliberately not added.

The complete option surface was independently audited. The authoritative
property reference documents all five canonical `BHModel` values, and the
maintained `DumpGsfTrks/gsf.py.bk` explicitly keeps the production default
while naming all four default-off controls. There is no maintained-card versus
baseline difference requiring a `DumpGsfTrks/README.md` entry.

## Mechanical validation

The EL9/LCG 105 `RecGsfTracking` target built and installed successfully. All
focused runs used `rec-e--2.0-85-1.root` with otherwise identical production
reverse steering: `DD4hepBetweenSurfaces`, split and component cutoffs
`1e-4`, 12 components, `SymmetricKL`, identity protection, forward-posterior
reverse weights, covariance scale 100, `AggregateWeight`, `BestBranch`, and
ECAL off.

The verbose 11/16/17 regression gate and additional focused events completed
for all three models. Runtime `MaterialBHAuditCSV` values were independently
recomputed from each packaged JSON table. Over 1,222 generic and 1,223 category
BH calls, maximum absolute differences were:

| Model | Weight | Retained mean | Retained variance |
|---|---:|---:|---:|
| Generic grid | `1.44e-15` | `1.69e-14` | `1.35e-15` |
| Category aligned | `1.44e-15` | `7.05e-15` | `2.22e-15` |

The refactored current default reproduced the stored same-input pT exactly on
all six verbose selected events; the maximum absolute difference was zero.

## Focused same-code A/B

`SelectedEventIndices` is zero based, while the flat tuple displays `iev` as
one based. The table records both to avoid the earlier naming ambiguity.
Residuals are `(pT_reco / pT_truth) - 1`. Event index 16 belongs to the
secondary-tracker-activity control and is reported separately rather than used
as single-track optimization evidence; every other row is topology clear.

| Selector index | Flat `iev` | Truth pT | LCIO residual | Default residual | Generic residual | Category residual |
|---:|---:|---:|---:|---:|---:|---:|
| 11 | 12 | 40.7316 | +0.402% | +0.500% | +0.506% | +0.502% |
| 16 (secondary control) | 17 | 37.8940 | -51.726% | -51.658% | -51.658% | -51.658% |
| 17 | 18 | 18.7970 | -21.228% | -0.916% | -0.859% | -0.859% |
| 19 | 20 | 19.0525 | -8.180% | +5.424% | +5.200% | +5.231% |
| 27 | 28 | 29.8464 | -12.339% | +11.958% | +10.268% | +10.322% |
| 41 | 42 | 12.4253 | -0.262% | +79.418% | -0.101% | -0.105% |
| 33 | 34 | 41.6471 | -30.662% | -21.618% | -21.582% | -21.620% |
| 35 | 36 | 34.4746 | -92.973% | -92.974% | -92.974% | -92.974% |
| 57 | 58 | 16.4785 | -66.906% | -36.463% | -37.198% | -36.820% |

Event 41 has zero matched interval eBrem in the runtime truth join. Both new
priors eliminate its extreme false-radiative output and return a momentum near
truth and LCIO. This is the strongest focused positive result. The candidates
also retain the hard-loss recovery in event 17, but their changes in events
19, 27, and 33 are modest, event 35 is unchanged, and event 57 is slightly
worse. The topology-unclear event 16 is also unchanged.

These selected events demonstrate that the new response can change a harmful
branch decision without universally suppressing useful recovery. They do not
establish a population improvement, clean-core safety, or superiority of the
category-aligned knot grid over the generic-grid control. The next valid gate
is a same-code held-out population A/B after the unresolved invalid-coverage
paths and branch-local crossover diagnostics are handled or explicitly
controlled.

Generated ROOT, EDM, CSV, and verbose log outputs were kept under `/tmp` and
are not source or status artifacts.
