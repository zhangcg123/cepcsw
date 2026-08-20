# Category-aligned runtime-interval BH fit

Date: 2026-08-21

## Question and decision boundary

The user requested a one-dimensional Bethe-Heitler (BH) candidate fitted
directly from complete surface-to-surface intervals, with knots aligned to the
physical VXD, TPC, tracker-service, and bridge interval bands. The runtime
model must consume only radiation length. It must not split an interval into
individual Geant4 eBrem steps and must not use detector category, energy,
angle, topology, or truth as runtime inputs.

This work is analysis-only. It does not authorize a source edit, selectable
model value, default change, split-threshold change, or track-performance
claim.

## Model definition

One fit row is one valid forward, non-seed runtime interval with exact
`DD4hepBetweenSurfaces pathTX0` and spatially matched truth bounds. The truth
response is the aggregate Geant4 eBrem loss between those bounds:

```text
z = 1 - clamp(sum of matched Geant4 eBrem step losses / interval entrance p,
              0, 1)
```

An interval with no eBrem step contributes to an exact atom at `z=1`. The
four radiative components retain the existing truth strata: 0--1%, 1--5%,
5--20%, and greater-than-20% aggregate loss. Individual Geant4 steps are only
used to construct the aggregate target; they are not independent training
samples.

The category-aligned knot positions are:

| role | knot `t/X0` | log-midpoint fit cell |
|---|---:|---:|
| adjacent TPC rows | 0.0000469346 | 0 -- 0.000184550 |
| ordinary thin VXD | 0.000725659 | 0.000184550 -- 0.00172899 |
| outer-VXD service | 0.00411955 | 0.00172899 -- 0.00582419 |
| VXD/ITK and ordinary ITK | 0.00823418 | 0.00582419 -- 0.00957352 |
| thick tracker bridge / ITK-to-TPC | 0.0111307 | 0.00957352 -- 0.0135149 |
| final TPC row to OTK | 0.0164098 | 0.0135149 -- 0.0180367 |
| skipped/high interval control | 0.0198249 | 0.0180367 -- 0.0243875 |
| thick-interval guard | 0.0300000 | 0.0243875 -- 0.0500000 |

The physical names motivate the positions only. Every fit-cell assignment and
all interpolation use `pathTX0` alone. The ordinary-TPC knot remains below the
production `BHSplitThreshold=1e-4`, so it has no production effect without a
separate threshold experiment.

## Population and held-out split

The input is the compacted exact-runtime audit from 411 completed files and
40,040 audited events. Legacy filenames do not describe the actual broad
10--50 GeV, theta 40--140 degree campaign.

- parameter training: seed modulo 5 nonzero, 329 files;
- held out: seed modulo 5 zero, 82 files;
- accepted training intervals: 7,399,340;
- topology-clear intervals in the primary fitted range: 6,610,482;
- accepted held-out intervals: 1,756,441;
- topology-clear held-out intervals above the production threshold: 59,639.

Rows require forward direction, `is_seed=0`, valid component paths, valid
truth matching, endpoint offsets no greater than 5 mm, positive finite runtime
thickness, and positive finite interval entrance momentum. The primary
candidate excludes secondary tracker activity in accordance with the active
single-track optimization law. An inclusive fit and the secondary population
are kept as controls. The topology label is not runtime logic.

The physical knot locations were motivated by the prior complete category
reference from the same campaign. The parameter estimates are seed-held-out,
but this is not an independent-campaign validation of the knot design.

## Controlled held-out result

For the 59,639 topology-clear, production-eligible held-out intervals:

| quantity | observed | current default | same-runtime generic-grid fit | category-aligned fit |
|---|---:|---:|---:|---:|
| eBrem probability | 0.071732 | 0.084599 | 0.073025 | 0.070464 |
| mean fractional loss | 0.002993 | 0.007165 | 0.003096 | 0.002955 |
| `P(loss > 5%)` | 0.013951 | 0.021497 | 0.013135 | 0.012630 |
| `P(loss > 20%)` | 0.003957 | 0.010515 | 0.003695 | 0.003536 |
| radiative Brier score | -- | 0.063600 | 0.063332 | 0.063284 |
| zero-inflated NLL | -- | 0.039461 | 0.034956 | 0.034147 |

The same-runtime generic-grid control uses the identical event split,
accepted runtime population, topology selection, loss strata, and
interpolation. It isolates the knot-grid effect from the much larger effect of
changing the training population.

Relative to the same-runtime generic grid, the category grid lowers NLL by
about 2.3%, lowers the absolute mean-loss bias from `1.03e-4` to `3.77e-5`,
and slightly improves radiative probability and Brier score. It does not win
every closure quantity: the generic grid has smaller absolute errors for the
5% and 20% tail probabilities. Both new runtime fits are much closer than the
current default on this topology-clear population.

The topology-clear candidate cannot be presented as a general physical BH
model. In the production-eligible secondary-activity control, it predicts
mean loss `0.00301` versus `0.01818` observed and eBrem probability `0.0717`
versus `0.1407` observed. The inclusive candidate reduces but does not close
that opposite bias. This control must remain reported separately and must not
be converted into truth-dependent runtime selection.

## Mechanical and statistical checks

- Across a dense `t/X0` grid from `1e-8` to 1, mixture weights remain finite
  and sum to one within `4.44e-16`; all weights and variances are positive and
  all means remain within `[0,1]`.
- Every category cell is statistically populated. The only component with
  fewer than 25 primary training entries is the greater-than-20% component at
  the 0.03 guard knot: 12 entries.
- The 0.03 guard cell has 993 clear training intervals. Only three accepted
  held-out intervals exceed the fitted upper edge 0.05, so high-thickness
  extrapolation is not validated.
- The first TPC cell is 99.99% canonical `TPC -> TPC`, and the thin-VXD and
  service cells are dominated by their intended VXD populations. At larger
  thickness the cells contain multiple physical categories, as required by a
  genuinely one-dimensional fit.

## Interface and integration conclusion

No material/BH call-interface change is needed. The existing splitter already
receives:

```cpp
split(parent, pathTX0, bz, reverse, returnedMixture)
```

The candidate response itself consumes only `pathTX0`; `parent`, `bz`, and
`reverse` are existing machinery used to apply the returned retained-momentum
mixture to the track state. A reviewed integration can therefore preserve all
call sites.

Preserving the current default while testing this candidate requires a new
parallel compiled model table and selector value inside `RecGsfTracking`.
That changes the allowed `BHModel` option surface. Under the active project
law, such integration requires a dedicated sub-agent audit and synchronized
updates to `Reconstruction/RecGsfTracking/README.md` and
`DumpGsfTrks/gsf.py.bk`. None of those changes was made here.

Before promotion, the candidate still requires reviewed default-off
integration, exact verbose mixture reproduction, hard events 11/16/17,
branch-local truth-lineage checks, same-code track-level A/B, clean-track
safety, and an independent held-out campaign. The invalid-path coverage
population and threshold remain separate mechanisms and were excluded rather
than tuned in this fit.

## Artifacts

Generated, uncommitted study artifacts live under
`TrackingPerformanceStudies/bh_runtime_category_aligned_model_2026-08-21/`.
Its `README.md` enumerates the candidate/control JSON files, tables, audits,
and plots. The temporary analysis implementations are
`/tmp/fit_category_aligned_runtime_bh.py` and
`/tmp/compare_category_vs_generic_runtime_bh.py`.

No `RecGsfTracking` source, installed model data, property, run card, or
default changed.
