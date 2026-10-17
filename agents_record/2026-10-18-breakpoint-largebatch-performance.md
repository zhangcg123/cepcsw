# Layer-owned breakpoint campaign: performance snapshot

Read-only analysis of sim_large_barrel_20261018. No refits, source/card edits,
installation, input deletion, or scheduler submission. Results live in
TrackingPerformanceStudies/breakpoint_layer_owned_resolution_20261018/;
reproduce with its analyze.py (or --replot for cached rows).

## Inputs and selection

- Fixed snapshot of 371 completed jobs out of 500 prepared/submitted jobs.
- 74,200 REC events, 74,189 events with track rows, 79,298 track rows.
- 78,256 matched primary-electron rows; 1,042 nonprimary/unmatched/invalid-truth
  rows excluded from primary-electron comparisons.
- 61,974 topology-clear rows: exactly one track row per event and no secondary
  simulated tracker-hit associations in VXD/ITK/TPC/OTK. This selection was
  recomputed from current REC truth; no old fixed track-key list was reused.
- 16,282 primary control rows retained separately. There are 10,245 events
  with secondary tracker hits in the full REC sample.
- All completed jobs have identical frozen fitter card bodies and controls:
  Persistent6D, PostReference, SigmaLogLoss=0.001, beam objective on. Card
  checksums passed. Input sizes/timestamps stayed unchanged during extraction.

Loss categories retain the prior performance convention: no positive primary
G4 subtype-3 loss steps; cumulative fractional loss below 1%; at least 1%.
Fraction is 1-product(1-min(1,step_momentum_loss/pre_step_momentum)). This is
not the identification model's ANY-owner-group >1% target. Unknown owners
remain eBrem and are not relabelled no-loss. Location panels use the selected
eligible layer owner, not old hook-boundary intervals or exact photon positions.

## Coverage and paired comparison

Topology-clear counts: no-eBrem 23,261; below 1% 20,049; at least 1% 18,664.
Ordinary fit status is successful for 60,560 rows and failed for 1,414.
Free-loss: 22,601 applied, 17 failed, 37,942 no selected interval, 1,414 not
attempted. Beam-guided free-loss: 21,465 applied and 1,153 failed, with the
same no-interval/not-attempted counts. ECAL: 3,416 applied, 256 fit failures,
56,888 ordinary copies, 1,414 unavailable after ordinary failure.

All eleven tracking endpoints are positive/finite on 60,557 common tracks.
Their category counts are 22,790 / 19,588 / 18,179. Compare methods on these
common cohorts, while the unfiltered plots/status tables retain failure and
coverage information. Fallback copies are never removed merely for being copies.

Inclusive common-track results (percent residual = 100*(pT_reco/pT_truth-1)):

| Method | Width68 (%) | Within +/-1% (%) | Outside +/-5% (%) |
|---|---:|---:|---:|
| KF | 1.055 | 79.44 | 10.19 |
| Ordinary RTS | 1.058 | 79.29 | 10.19 |
| Ordinary backward | 1.023 | 79.70 | 9.97 |
| Free-loss RTS | 0.370 | 83.37 | 6.56 |
| Free-loss backward | 0.392 | 83.34 | 6.51 |
| Beam-guided free RTS | 0.382 | 83.41 | 6.56 |
| Beam-guided free backward | 0.403 | 83.34 | 6.51 |
| Diffuse RTS | 0.542 | 79.84 | 8.59 |
| ECAL-constrained RTS | 0.653 | 81.56 | 6.58 |
| Truth-override RTS | 0.257 | 89.95 | 4.76 |
| Truth-override backward | 0.282 | 89.99 | 4.68 |
| KF + collected hit-supported neutral ECAL | 0.482 | 82.77 | 4.73 |

Width68 = (q84-q16)/2 over the full finite residual range, not a Gaussian fit.
The charged-cluster-only curve is also saved; available for 59,746 of the
60,557 common tracks, width68=2.852%. Energy-to-pT conversions use the actual
KF AtIP direction and electron mass, not generator directions. Duplicate
charged cluster IDs are removed; synthetic neutral clusters are not added.

## Main observations, not production validation

- No-eBrem: KF width68=0.174%, ordinary/free/diffuse/truth RTS about 0.176%.
- Below 1%: KF 0.252%, free RTS 0.316%, diffuse 0.475%, truth RTS 0.205%.
  Free RTS creates 1.35% outside +/-5%; diffuse creates 5.14%.
- At least 1%: KF 6.270%, free RTS 2.955%, diffuse 3.821%, ECAL RTS 3.268%,
  truth RTS 2.493%. KF+neutral gives 1.846%, but creates tails in clean/light
  samples. There is no universal winner across categories.
- Beam guidance does not improve the inclusive width and has many more
  optimizer failures than the unconstrained free-loss variant.
- On 22,598 common tracks with applied free loss and valid stored scores,
  free-loss NLL is below ordinary in 18,537 and below truth in 22,561.
  Median 2*NLL differences are -0.503110 and -0.223398 respectively.
  Smaller stored objective still does not imply better IP truth resolution.

## Outputs and checks

171 PNGs cover 19 cohorts with RTS, backward and ECAL overlays, each in
[-5,5]%, [-10,10]%, and [-100,200]% views. Full-cohort normalization is kept
when zooming, including unavailable outputs in the denominator. Includes
single-eBrem, neutral-collected, ECAL-applied, hard selected-owner VXD/ITK/TPC,
hard no-eligible-owner, VXD layers3/4/5, and topology-control panels.
No selected OTK owner occurs in this sample; no empty OTK plot is fabricated.

Tables: all_primary_rows.csv, resolution_summary.csv, cohort_counts.csv,
status_summary.csv, paired_vs_kf.csv, likelihood_comparison.csv.
input_snapshot.json records input paths/sizes/timestamps. Checks covered
unique row keys, complete expected plot inventory, unchanged inputs/card
steering, zero-neutral reproduction of KF pT, residual/missing-value handling,
and visual review of inclusive RTS/backward panels.

No gallery server was changed. This is the completed-job snapshot, not all
500 submitted jobs, and not held-out validation or a diagnosis of fit failures.
