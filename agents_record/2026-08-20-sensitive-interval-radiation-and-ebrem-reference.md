# Sensitive-interval radiation-length and eBrem reference

Date: 2026-08-20

## Purpose

This record freezes the truth-recorder interval table that must be compared
with the actual intervals used by GSF at runtime before fitting or promoting a
new Bethe-Heitler model. It is a diagnostic reference, not a model validation
or production-default decision.

The source sample contains 411 completed files, 41,100 events, and 9,547,585
accepted valid primary-electron intervals. An interval runs from the
track-length midpoint of one sensitive-volume traversal to the midpoint of the
next observed sensitive traversal. It therefore contains the downstream half
of the first sensitive layer, intervening material, and the upstream half of
the second sensitive layer. Adjacent TPC lower/upper sensitive half-volumes
are combined into one pad-row anchor.

`median total DD4hep t/X0` is the median complete midpoint-to-midpoint
thickness, not a sum across the population. An eBrem-bearing interval has
`ebrem_step_count > 0`; multiple eBrem steps in one interval count once. The
sample contains 39,282 eBrem-bearing intervals in total.

## Frozen reference table

| sensitive-layer interval | median total DD4hep t/X0 | statistics: total / eBrem intervals | P(eBrem given interval) | share of all eBrem-bearing intervals |
|---|---:|---:|---:|---:|
| VXD0 -> VXD1 | 0.00075935 | 37,580 / 394 | 1.048% | 1.003% |
| VXD1 -> VXD2 | 0.00065179 | 37,748 / 320 | 0.848% | 0.815% |
| VXD2 -> VXD3 | 0.00064277 | 37,888 / 323 | 0.853% | 0.822% |
| VXD3 -> VXD4-S04 | 0.00072566 | 36,125 / 355 | 0.983% | 0.904% |
| VXD4-S04 -> VXD4-S05 | 0.00411955 | 38,618 / 2,125 | 5.503% | 5.410% |
| VXD4-S05 -> neighboring VXD4-S04 | 0.00051117 | 6,992 / 38 | 0.543% | 0.097% |
| VXD4-S05 -> ITKB0 | 0.00823418 | 33,088 / 3,628 | 10.965% | 9.236% |
| VXD4-S04 -> ITKB0 | 0.01126697 | 5,860 / 858 | 14.642% | 2.184% |
| ITKB0 -> ITKB1 | 0.00817601 | 37,077 / 3,995 | 10.775% | 10.170% |
| ITKB1 -> ITKB2 | 0.00846299 | 37,069 / 4,148 | 11.190% | 10.560% |
| ITKB2 -> first TPC row | 0.01113070 | 39,038 / 5,156 | 13.208% | 13.126% |
| adjacent TPC rows | 0.00004693 | 9,136,913 / 7,802 | 0.0854% | 19.862% |
| last TPC row -> OTKB | 0.01640980 | 40,770 / 7,566 | 18.558% | 19.261% |
| ITKB1 -> TPC (noncanonical missing ITKB2 anchor) | 0.01982494 | 1,995 / 447 | 22.406% | 1.138% |

The 13 repeated categories contain 93.447% of all eBrem-bearing intervals.
The displayed noncanonical `ITKB1 -> TPC` category raises the table coverage
to 94.585%; the remaining sparse categories stay available in the complete
90-category artifact.

## Exact interpretation of the noncanonical ITKB1-to-TPC row

The truth track entered an ITKB1 sensitive sensor and subsequently the first
TPC sensitive pad row, but no ITKB2 sensitive-volume traversal was present to
form an intermediate anchor. The interval therefore collapses the normal
`ITKB1 -> ITKB2` and `ITKB2 -> TPC` bounds into one runtime-relevant path.

The observed median `0.01982494 t/X0` is 1.18% above the sum of the two normal
medians, `0.00846299 + 0.01113070 = 0.01959370 t/X0`. Its observed 22.406%
eBrem probability is close to the independent combined expectation,
`1 - (1 - 0.111899)(1 - 0.132076) = 22.920%`. This is a missing-sensitive-
anchor topology, plausibly a passage through an inactive module/stave gap,
not evidence for a new material layer. It is a required validation row, not a
standalone proposed BH knot.

`VXD4-S05 -> VXD4-S04` also needs exact semantics: it is normally an overlap
crossing from one outer-VXD ladder to a neighboring ladder at typical radii
about 43.23 to 43.63 mm. It is not the reverse of the same-ladder
`VXD4-S04 -> VXD4-S05` interval.

## Radiation-length distribution overlap

Normalized log10(t/X0) histograms were compared with the overlap coefficient
`sum(min(p_i, q_i))`, where zero is disjoint and one is identical. Within a
fixed physical row, DD4hep forward/reverse overlap is at least 0.9985 and
DD4hep/bounded-Geant4 overlap is at least 0.9915. Thus the three material
estimators overlay within each table row at this histogram resolution.

Physical rows do not all separate by thickness. The strongest cross-row
overlaps are 0.961 for `VXD1 -> VXD2` versus `VXD2 -> VXD3`, 0.893 for
`VXD4-S04 -> ITKB0` versus `ITKB2 -> TPC`, 0.880 for `ITKB0 -> ITKB1`
versus `ITKB1 -> ITKB2`, and 0.835 for `VXD0 -> VXD1` versus
`VXD3 -> VXD4-S04`. The noncanonical `ITKB1 -> TPC` has 0.524 overlap with
`TPC -> OTKB`. TPC rows are essentially disjoint from all other categories,
and the same-ladder VXD4 S04-to-S05 service interval is also a separate band.

Therefore t/X0 alone does not identify the physical interval. It gives broad
TPC-row, ordinary-VXD, VXD-service, ITK/bridge, and outer/high-thickness bands.
Rows within an overlapping band must retain their labels during runtime
closure and may be pooled for a one-dimensional BH fit only if their held-out
retained-energy distributions also agree.

## Required GSF runtime comparison

The next material/BH check must establish whether the actual GSF candidate
paths and executed BH calls reproduce this interval population. For the same
unbiased events and exact steering:

1. Normalize every runtime source/target measurement pair to the same
   sensitive-layer labels used above. Keep the seed path separate.
2. Record candidate paths before `BHSplitThreshold` and executed BH calls after
   the threshold; do not infer the former from the latter. The TPC row category
   is normally below the production `1e-4` split threshold.
3. Compare forward and reverse separately. Match directions only for
   equivalent component states and canonicalize the physical interval label.
4. For every table row compare path count, valid/invalid count, median,
   16--84% and 5--95% `pathTX0`, material composition, fraction above the split
   threshold, and fraction above the last BH knot.
5. Explicitly search for `ITKB1 -> TPC`; verify that it appears only when the
   runtime reconstructed track lacks an ITKB2 measurement anchor and that its
   thickness matches the combined interval rather than a new material layer.
6. Preserve the expected endpoint distinction: the reference uses Geant4
   sensitive-traversal midpoints, whereas GSF uses reconstructed hit points.
   Compare population closure and paired same-event geometry without requiring
   unrelated endpoints to be numerically identical.
7. Only after interval closure, compare the returned BH mixture and Geant4
   retained-energy response at each exact runtime input.

Failure classification remains: surface-pair/count mismatch is an anchor or
tracking-topology issue; matching pairs but mismatched `pathTX0` is a material
path/endpoint issue; matching paths but mismatched retained-energy response is
a BH issue; closure through the first wrong branch moves the diagnosis to
measurement or selection.

## Artifacts

The generated study lives under
`TrackingPerformanceStudies/bh_interval_categories_2026-08-20/`:

- `README.md` documents definitions and interpretation;
- `major_interval_ebrem_fractions.csv` is the numerical reference table;
- `major_interval_ebrem_fractions.png` plots both eBrem denominators;
- `table_interval_tx0_overlap.png` shows the row-pair overlap matrix and
  same-row material-estimator agreement;
- `table_interval_tx0_pairwise_overlap.csv` and
  `table_interval_tx0_source_overlap.csv` contain the exact coefficients;
- `coarse_interval_summary.csv` retains all 90 normalized categories;
- `candidate_interval_bh_knots.csv` records candidate roles and exact physical
  sensitive-layer bounds.

No tracking source, installed BH model, or run-card default was changed by this
study.
