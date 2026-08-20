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

## Sixty-event topology-clear stress/control A/B

After the initial focused check, all three models were rerun on the same fixed
60-event panel from `rec-e--2.0-85-1.root`. The selection was based only on the
unchanged default output and the pre-existing topology classification:

- overshoot: the 20 largest positive default residuals;
- underestimate: the 20 most negative default residuals;
- good controls: the 20 smallest absolute default residuals among the events
  remaining after removing both tail groups.

All 60 events are topology clear, the three groups are disjoint, and the
selector uses zero-based indices. The overshoot group spans `+0.217%` to
`+79.418%`, the underestimate group `-0.320%` to `-92.974%`, and the good
controls have absolute residuals from `0.00056%` to `0.0744%`. This is a
deliberately default-selected stress/control panel, not an unbiased or held-out
sample.

Every run fitted exactly the intended 60 entries, retained at least 440 NDF,
and exited successfully. The same-code default reproduced all 60 stored pT
values exactly. Summary metrics use absolute `(pT_GSF/pT_truth)-1` residuals:

| Group | Model | Mean abs. | Median abs. | 68% abs. quantile | Maximum abs. | `|r|>1%` | `|r|>3%` |
|---|---|---:|---:|---:|---:|---:|---:|
| Overshoot | Default | 5.365% | 0.379% | 0.567% | 79.418% | 5 | 4 |
| Overshoot | Generic | 1.306% | 0.335% | 0.500% | 10.268% | 4 | 3 |
| Overshoot | Category | 1.307% | 0.316% | 0.497% | 10.322% | 4 | 3 |
| Underestimate | Default | 8.538% | 0.926% | 1.661% | 92.974% | 9 | 4 |
| Underestimate | Generic | 8.549% | 0.898% | 1.659% | 92.974% | 9 | 4 |
| Underestimate | Category | 8.425% | 0.908% | 1.206% | 92.974% | 9 | 3 |
| Good control | Default | 0.0330% | 0.0316% | 0.0404% | 0.0744% | 0 | 0 |
| Good control | Generic | 0.0409% | 0.0331% | 0.0470% | 0.1766% | 0 | 0 |
| Good control | Category | 0.0411% | 0.0354% | 0.0516% | 0.1098% | 0 | 0 |

Across all 60 selected events, mean absolute residual changes from `4.646%`
for the default to `3.299%` generic and `3.258%` category. The 68% absolute
quantile changes from `0.580%` to `0.547%` and `0.552%`; counts above 3% change
from 8 to 7 and 6. Generic improves/worsens/is unchanged on 27/23/10 events;
category does so on 20/25/15. These aggregate means are dominated by flat
tuple event `iev=42`, where the default `+79.418%` no-eBrem false overshoot
becomes `-0.101%` generic and `-0.105%` category. Excluding that event, the
all-panel mean-absolute improvement is only `0.025` percentage points generic
and `0.067` category.

The next-largest category-aligned improvement is flat tuple `iev=14`, from
`-4.048%` to `-1.155%`; this single event drives most of its under-group
advantage. Conversely, `iev=58` changes from `-36.463%` to `-37.198%` generic
and `-36.820%` category. The good controls show small absolute broadening: 11
of 20 worsen for each candidate, but none crosses 1% or 3%. The strongest
generic control change is `iev=53`, `+0.074% -> +0.177%`; the strongest
category control change is `iev=30`, `-0.033% -> -0.110%`.

Therefore the 60-event check strengthens the evidence that both fitted priors
can prevent one important false-radiative branch without erasing all hard-loss
recovery. It does not show general repair of underestimates, a material
solution, or clear superiority of the category-aligned grid. It also exposes a
small clean-control broadening. A default decision still requires an
independently selected, same-code held-out population study with topology and
eBrem categories reported separately.

The generated outputs are isolated under
`/tmp/gsf_bh_model_ab60_20260821.JFrfkc/` and remain uncommitted.
