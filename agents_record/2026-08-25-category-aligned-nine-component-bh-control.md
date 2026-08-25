# Category-aligned nine-component BH control

## Decision boundary

`CEPCRuntimeCategoryAligned9Clear` is integrated as a parallel, default-off
Bethe-Heitler selector. It is not a new production default and is not a
validated replacement for `CEPC2GeV85StepConditioned` or
`CEPCRuntimeCategoryAligned5Clear`. The sole runtime input remains the
DD4hep-between-surfaces `pathTX0`; Geant4 truth is used only to extract and
audit the packaged response.

The new model keeps the eight category-aligned t/X0 knots and the identity
probability of `CEPCRuntimeCategoryAligned5Clear`. It redistributes each
knot's radiative probability over eight fixed aggregate-loss proposals:

| component | mean loss | loss sigma | fitted weight source |
|---:|---:|---:|---|
| 0 | 0% | effectively zero | no Geant4 eBrem step |
| 1 | 1% | 0.5% | 0--2% truth-loss cell |
| 2 | 3% | 0.5% | 2--4% truth-loss cell |
| 3 | 5% | 0.5% | 4--6% truth-loss cell |
| 4 | 7% | 0.5% | 6--8% truth-loss cell |
| 5 | 9% | 0.5% | 8--10% truth-loss cell |
| 6 | 15% | 2.5% | 10--20% truth-loss cell |
| 7 | 30% | 5% | 20--40% truth-loss cell |
| 8 | 70% | 15% | 40--100% truth-loss cell |

Here the mixture is stored in retained fraction `z`, so a mean loss of 15%
is represented by `mean_z=0.85`. The identity component has
`mean_z=1` and `variance_z=1e-12`.

## Extraction population and sparsity

The extraction used the 411 preserved runtime-interval Parquet files from the
40,040-event audit. The seed-modulo-five split was frozen before fitting:

- training: 329 files, 6,610,524 accepted rows, 6,610,482 fit rows;
- held out: 82 files, 1,567,288 accepted rows, 1,567,286 fit rows;
- population used for weights: topology-clear runtime intervals only.

Every one of the 64 radiative grid cells has nonzero training support, but
nine cells have fewer than 25 entries. Two occur in the thin-VXD grid
(`6--8%`: 14, `8--10%`: 18); seven occur at the 0.03-t/X0 thick guard
(`2--4%` through `40--100%`: 18, 13, 6, 6, 14, 7, and 5). These raw sparse
weights are deliberately retained in this research control. Any pooling or
smoothing rule requires a separate review.

The authoritative table and its dependency-free compiled representation are:

- `Reconstruction/RecGsfTracking/data/CEPCRuntimeCategoryAligned9Clear/cepc_runtime_category_aligned9_clear.json`;
- `Reconstruction/RecGsfTracking/data/CEPCRuntimeCategoryAligned9Clear/compiled_table.inc`.

Interpolation is the existing generalized CEPC interpolation: physical
zero-to-first-knot interpolation, log-t/X0 between knots, radiative log-weight
ratios relative to identity, logit means, log variances, and constant response
above the last knot.

## Old-versus-new response plots

Generated analysis outputs are under
`TrackingPerformanceStudies/bh_runtime_category_aligned9_clear_2026-08-25/`
and remain uncommitted as required by repository policy. The principal plots
are:

- `category_aligned5_vs_9_by_grid_zoom_0_20pct.png`;
- `category_aligned5_vs_9_by_grid_full.png`;
- `runtime_category_aligned9_support_metadata.png`.

They overlay the held-out exact aggregate-loss distribution, the old
five-component response, the new nine-component response, and the individual
Gaussian proposals for every t/X0 grid. The old and new eBrem probabilities
are identical by construction; only the conditional loss-magnitude response
changes. In particular, the fixed 1% first proposal lies above the observed
sub-percent peak in several grids, so finer spacing is not automatically a
better fit.

## Mechanical gate

The normal EL9/LCG-105 build and installation of `RecGsfTracking` and
`RecGsfFlatTuple` completed successfully. A same-code reverse run selected
events 11, 16, and 17 from
`trk_large_20260823/trk-e--2.0-85-1.root` with
`MaterialPathMode=DD4hepBetweenSurfaces`, `MaxComponents=12`,
`ComponentWeightCutoff=1e-4`, and `ReverseKappaSeedCov=100`.

All 5,481 persisted BH-child lineage nodes matched the packaged JSON weight,
mean, and variance exactly. The maximum absolute difference for all three
quantities was zero. All events finalized and component indices 0--8 were
observed.

The focused BestBranch endpoint comparison was:

| event | truth pT (GeV) | LCIO pT (GeV) | Category5 pT (GeV) | Category9 pT (GeV) | Category5 residual | Category9 residual |
|---:|---:|---:|---:|---:|---:|---:|
| 11 | 40.73157 | 40.89545 | 40.93614 | 40.90481 | +0.5023% | +0.4253% |
| 16 | 37.89402 | 18.29283 | 18.31881 | 18.31881 | -51.6578% | -51.6578% |
| 17 | 18.79698 | 14.80667 | 18.63542 | 18.74174 | -0.8595% | -0.2939% |

This is an interface and focused-mechanics gate only. It does not establish
held-out population improvement, clean-track safety, lineage survival, or
tail control.
