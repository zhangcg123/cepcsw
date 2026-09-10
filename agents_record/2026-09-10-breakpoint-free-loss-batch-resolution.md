# First completed largest-interval free-loss batch: pT resolution

The user requested resolution comparisons for the completed breakpoint batch.
This is a read-only stored-tuple study; no fitting code, run cards, library,
submission settings, inputs or existing results were changed.

## Inputs and selection

- Input: breakpoint_barrel/breakpoint_flat-e--2.0-85-{seed}.root.
- 48 files/jobs, all completed markers present, no invalid/recovered ROOT files.
  Seeds1--50 except8,9. 9,598 events with attempted input tracks,10,240 track rows.
- Frozen-card checksums verified. Batch controls uniformly sigma_b0.001.
  Runtime tuple steering agrees: LocalMarginal, Truth largest selected interval
  (at most one), FirstMiddleLast, backward scale100, free fitON,
  b_max1, calls180, tolerance0.001, truth treatmentPriorCenter.
  The retired audit field is absent.
- Topology-clear catalogue:
  TrackingPerformanceStudies/g4_material_topology_clear_all_2026-09-05/selected_topology_clear_events.csv.
  Truth pT and topology membership cross-checked against the existing barrel
  event-residual catalogue for9,379 track rows; repeated rows in multi-track
  events are counted separately here.
- Common paired selection: catalogue membership, exactly one input track/event,
  primary G4 association, successful ordinary and available free/truth endpoints,
  all seven pT values finite and positive. Final N=8,097.
- 1,064 events lie outside the clear catalogue;584 events have multiple input
  tracks (these exclusions overlap and must not be added). The8,276 clean
  single-track candidates include179 ordinary-fit failures excluded from the
  paired plots. Across all rows423 ordinary failures and16 truth-pair failures
  are retained in the audit/excluded-row tables. Do not interpret these plots
  as efficiency-inclusive performance or the control population's resolution.

## Free-loss outcomes within the paired population

3,414 no-selected-loss ordinary copies,4,676 optimized pairs,7 failed-optimizer
input-KF fallbacks. All7 fallbacks remain INCLUDED in the free-loss histograms,
and both free endpoint pT values equal the stored KF exactly.
There are1,792 lower-bound and96 upper-bound flags in the paired population.
Both free endpoints have62 residuals above+100%; the RTS maximum is173.2444%.
Convergence does not establish physically correct energy-loss recovery.

## Inclusive results

Residual=100*(pT_reco-pT_truth)/pT_truth. Width68=(q84-q16)/2 uses the full
distribution, not the plotted window and not a fitted Gaussian sigma.
RMS is sqrt(mean(residual^2)), about zero, also on the complete distribution.

| Endpoint | Median (%) | Width68 (%) | RMS (%) | Within ±1% (%) |
|---|---:|---:|---:|---:|
| KF | -0.1104 | 1.0467 | 7.9852 | 79.4615 |
| Ordinary RTS | -0.1274 | 1.0596 | 7.9742 | 79.4245 |
| Ordinary backward | -0.1335 | 1.0196 | 7.9542 | 79.6715 |
| Free-loss RTS | -0.0085 | 0.3699 | 15.5654 | 81.7463 |
| Free-loss backward | -0.0116 | 0.3992 | 15.6172 | 81.7216 |
| Truth-prior RTS | -0.0278 | 0.2117 | 4.8916 | 94.7017 |
| Truth-prior backward | -0.0305 | 0.2435 | 4.8924 | 94.6894 |

Free loss reduces the central68% width and changes median toward zero, but
introduces large positive tails and worsens the full RMS compared with KF.
The narrow ±1% peak alone does not show the full-distribution width68 change.
Truth-prior remains an adjustable Gaussian prior with shared sigma0.001, NOT
the historical fixed-loss/zero-variance oracle. Only the largest matched loss
interval is treated. No general physics-validation claim follows.

## Plots and reproducibility

Output directory:
TrackingPerformanceStudies/breakpoint_freeloss_batch_pt_resolution_20260910/.

Four comparisons, at most three curves per canvas:
RTS ordinary/free/KF; backward ordinary/free/KF;
RTS free/truth-prior/KF; backward free/truth-prior/KF.
Each has a linear-y ±1% core view (0.04 percentage-point bins) and log-y
±100% tail view (2 percentage-point bins), PNG and PDF. Tail panels explicitly
state the count outside±100%. Every histogram uses the same8,097-track
denominator; no renormalization within the displayed range. No Gaussian fit.

plot_resolution.py, analysis_summary.json, resolution_summary.csv,
all_rows.csv, excluded_rows.csv, job_audit.csv and per-plot histogram CSVs
preserve inputs, selection, status counts, normalization and exact numbers.
Analysis scripts/tables/plots/ROOT files remain uncommitted; this durable
summary is the tracked project record. Existing inputs are untouched.
