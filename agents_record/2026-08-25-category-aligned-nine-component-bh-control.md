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

## Ten-event Category5/max12 versus Category9/max30 combination test

A subsequent same-code combination A/B processed seed-1 entries 0--9 from
`trk_large_20260823/trk-e--2.0-85-1.root`. Both jobs used the reverse method,
`DD4hepBetweenSurfaces`, `ComponentWeightCutoff=1e-4`, SymmetricKL,
identity-lineage protection, reverse covariance scale 100, and truth override
off. The control selected `CEPCRuntimeCategoryAligned5Clear` with
`MaxComponents=12`; the candidate selected
`CEPCRuntimeCategoryAligned9Clear` with `MaxComponents=30`. All ten entries
are topology clear: eight are tracker-light-eBrem, one is hard-eBrem, and one
has no tracker eBrem.

BestBranch results were:

| configuration | mean absolute residual | median absolute residual | 68% absolute quantile | RMS | maximum absolute residual | within 1% |
|---|---:|---:|---:|---:|---:|---:|
| LCIO | 2.3413% | 0.1277% | 0.3345% | 6.5911% | 20.7940% | 8/10 |
| Category5/max12 | 0.2308% | 0.1991% | 0.2365% | 0.3081% | 0.7322% | 10/10 |
| Category9/max30 | 0.3667% | 0.1607% | 0.1794% | 0.6725% | 1.9574% | 9/10 |

The candidate improved three events, was numerically unchanged in three, and
worsened four by absolute BestBranch residual. The central median and 68%
quantile improved, but entry 5 changed from +0.2903% to +1.9574%; this single
tail makes the candidate mean absolute residual and RMS worse. Its LCIO value
was -20.7940%, so both GSF configurations still made a genuine recovery.

The jobs ran sequentially under `/usr/bin/time -v`, with a one-second RSS
sampler. The resource comparison was:

| configuration | wall time | CPU user time | peak RSS | late-window median RSS | flat-tuple size |
|---|---:|---:|---:|---:|---:|
| Category5/max12 | 169.39 s | 160.21 s | 1.647 GiB | 1.604 GiB | 5.67 MiB |
| Category9/max30 | 236.54 s | 224.29 s | 2.380 GiB | 2.229 GiB | 12.38 MiB |

The candidate therefore used 0.733 GiB more peak RSS (+44.5%) and 39.6% more
wall time. Its lineage contained 158,863 nodes versus 63,812, with 43,173
versus 10,790 BH split children. The largest split call site contained the
expected 270 children (`30*9`), versus 60 (`12*5`) in the control. After the
startup ramp the candidate RSS oscillated around a plateau rather than growing
without bound; a late-window linear slope was +8.6 MiB/min. Ten events are
insufficient to exclude a slow leak, but they show no severe event-by-event
runaway.

This comparison changes both the BH proposal bank and retained-component cap,
so it cannot attribute physics or resource differences to either factor
independently. A causal comparison requires the four combinations of
Category5/Category9 and max12/max30 on the same events. Generated outputs are
under
`TrackingPerformanceStudies/bh_category5_max12_vs_category9_max30_10event_2026-08-25/`
and remain uncommitted.
