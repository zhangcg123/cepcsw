# Fifty additional direct fits with default-on chi2 breakdown

User requested50 more events after the score-persistence regression. Selected
the first50 seed/event-ordered entries in the existing strict single-eBrem,
topology-clear paired population, excluding all earlier decomposition examples
and the seven serialization-test events. No residual/chi2 selection. This is
an extension of the diagnostic examples, not an independent held-out sample or
unbiased measurement of the full production mixture. Seeds21(14) and22(36).
37 light (0<loss<1%),13 hard (loss>=1%); no no-eBrem category in this selection.
Regions: VXD6,VXD-to-ITK6,ITK9,ITK-to-TPC5,TPC11,TPC-to-OTK13.

Unchanged maintained card, rebuilt module from b26d0b3 via one-library
LD_LIBRARY_PATH overlay (no LD_PRELOAD). Installed runtime unchanged.
LocalMarginal; Truth interval selection; SigmaLogLoss=.001; SeedScale1;
BackwardSeedScale100; FirstMiddleLast; MS on; deterministic Eloss off;
FreeLossFit and TruthOverride on; VerboseDump off. Each actual Minuit fit and
ordinary/truth pair ran normally; these were not prior-centre replay fits.
11 local jobs, at most3 concurrent, maximum5 selected events per job, external
240-second timeout with5-second kill grace. All jobs exited0.

All50 ordinary/truth pairs valid; all50 free optimizations successful, no KF
fallback. All150 complete RTS score decompositions pass per-hit/total sum and
length gates. All500 comparisons with the earlier stored population are exact:
seven pT values (KF plus all six endpoints) and three complete RTS scores per
event. This stored-output comparison supplements, not replaces, the direct
old/new same-card11-row regression in the preceding persistence record.

## Results

Residuals below are100*(pT_reco/pT_truth-1), in percent. Breakpoint results are
RTS endpoints; backward endpoints are retained in the full event table.

| Category | Method | Within +/-1% | Median absolute residual % | RMS % |
|---|---|---:|---:|---:|
| All50 | KF | 43/50 | .1833 | 7.8618 |
| All50 | Ordinary | 43/50 | .2121 | 7.8812 |
| All50 | Free-loss | 38/50 | .2544 | 14.2689 |
| All50 | Truth-override | 49/50 | .1428 | 1.7723 |
| Light37 | KF | 37/37 | .1685 | .3089 |
| Light37 | Ordinary | 37/37 | .1567 | .3277 |
| Light37 | Free-loss | 31/37 | .2438 | 7.0957 |
| Light37 | Truth-override | 37/37 | .1430 | .2346 |
| Hard13 | KF | 6/13 | 1.6805 | 15.4094 |
| Hard13 | Ordinary | 6/13 | 1.7043 | 15.4464 |
| Hard13 | Free-loss | 7/13 | .2889 | 25.2939 |
| Hard13 | Truth-override | 12/13 | .1425 | 3.4532 |

Free total chi2 lower than truth in47/50; truth lower in3, no ties at1e-6.
Of47 free-lower events,32 have worse absolute pT residual and37 also have
lower measurement contribution. Therefore not every complete-score improvement
is a measurement-score improvement. Optimizer minimizes normalized likelihood,
not chi2 alone. These results do not establish a transport/noise-model cause.

Example22:72 (1.7611% loss,VXD L4->ITK L0): KF residual-1.6805%,
ordinary-1.7043%, free+76.8590%, truth+.05695%. Truth-minus-free chi2 gain:
measurement2.636080, process.393652, total3.029731 (seed difference tiny).
Light examples22:1 and22:8 overshoot+27.0886% and+25.8054% respectively,
versus truth-centred-.23654% and-.00473%.
Truth-centred is not universally safe:22:51 (40.7374% loss,VXD L3->L4)
has truth residual+12.4197%, while free remains-33.1569%.

Evidence: TrackingPerformanceStudies/breakpoint_score_more50_20261001/
plan.json has the exact roster, selection metadata, input provenance and
card/library hashes; jobs.json has exit codes/timings; all ROOT/log pairs,
events.csv (all50, all endpoints, all three score breakdowns), scores.csv,
resolution_summary.csv, stored_reproduction.csv and summary.json are retained.
Scripts and generated artifacts remain uncommitted. No source, card, installed
runtime or remote changes in this study.
