# Current shared-sigma free-loss population comparison

## Fixed plan, before results

User requested more events to establish whether free-loss fitting is better.
No source, maintained card, installed library, shared KF/GSF code, batch
submission or remote change is part of this study. Starting checkpoint e2b4f3d.
The current installed shared-SigmaLogLoss implementation is used, not the
retired fixed-b optimizer or a diffuse/no-prior augmented KF.

All 100 events in each existing tracker file for seeds21--40 are included:
2,000 input events. Selection is by seed range, not previous residual or fit
quality. Two fixed blocks21--30 and31--40 will be reported separately.
These seeds are separate from the latest selected-event smoke test, but are
not claimed never to have appeared in historical development studies.

Input directory: gsf_doublebhoff_freshseed_diagnostic.
Card: Reconstruction/RecBreakpoint/options/run_breakpoint.py, unchanged.
Defaults: LocalMarginal, Truth largest-absolute-loss interval selection,
MeanLogLoss=0, SigmaLogLoss=.001, Forward SeedScale=1, BackwardSeedScale=100,
FirstMiddleLast, MS on, deterministic Eloss off, free-loss on, truth-centred on.
All three pairs are generated in the same current-code execution.
Only I/O/event count and diagnostic verbosity are set by the test driver.

Library SHA256:
8689b40e378ce8886e7e0784de983a17c0d943be2b3d014939106d0a73482b0d.
The driver records/checks card/library checksums and input sizes/timestamps.
Jobs run locally, three concurrent processes with numerical-library threads=1.
Runtime evidence, raw output and scripts:
/tmp/recbreakpoint-population-20260930.TbPv8E/.
Analysis tables/plots:
TrackingPerformanceStudies/breakpoint_shared_sigma_population_20260930/.
ROOT/log/plot/table/script artifacts remain uncommitted.

## Definitions and safeguards

Residual = 100*(pT_reco/pT_truth - 1), in percent.
Compare free-loss RTS and backward against both the baseline CompleteTracks KF
and their respective zero-centred Gaussian breakpoint counterparts.
Truth-centred endpoints are a reference, not used to tune the optimizer.
All ordinary/free comparisons are same-code paired results.

Use the existing topology-clear catalogue; verify event identity against the
same sample's passive GSF truth and baseline KF values. Truth categories use
cumulative G4 loss over matched tracker intervals, not the largest selected
interval alone: no loss, (0,1%), >=1%; additionally split1--10% and>=10%.
The fitter still treats only its single selected largest absolute-loss interval.

Primary paired sample requires one input track, topology clear, valid primary
association and finite ordinary/free endpoints. Optimizer KF fallbacks remain
included. Do not drop them by requiring optimization success. Do not require
the truth-centred pair to succeed when forming ordinary/free comparisons.
Primary failures/missing outputs and secondary activity are counted separately;
report candidate-normalized within-window rates as an efficiency cross-check.
Secondary events never enter the main physics comparison.

Predeclared metrics: median, width68=(q84-q16)/2, RMS about zero, within +/-1%,
tails outside +/-5% and +/-10%, paired improvement/worsening and new tails.
Intervals use 2,000 paired seed-cluster bootstrap repetitions. Fixed seed-block
agreement provides an additional stability check. Plotted-window overflow is
not discarded when calculating metrics or normalizing histograms.

## Completed result

All 20 local jobs exited successfully. Current default free-loss is not an
overall safe replacement: it improves the inclusive central 68% interval and
many >=1% loss tracks, but significantly damages light-loss tracks and creates
extreme positive outliers. This conclusion concerns LocalMarginal with Minuit
optimizing the Gaussian prior centre at fixed SigmaLogLoss=.001, not a diffuse
or no-prior augmented KF. No source, card, installed runtime or remote changed.
The complete outgoing focus is preserved in
2026-09-30-agents-before-free-loss-population.md; section1/laws remain unchanged.

### Accounting and reproducibility

- 2,000 input events, 2,162 track rows; every event produced rows.
- 237 secondary-activity events are excluded from the main comparison and
  reported separately. Of 1,763 topology-clear events, 52 have multiple tracks.
- 1,711 baseline-valid single-track candidates; 34 fail interval truth
  association before fitting, leaving 1,677 paired tracks.
- Free-loss: 945 optimized, 731 no-interval copies, one optimizer failure
  (seed27:event6, no converged valid Minuit minimum) using the explicit KF
  fallback. That fallback remains included. 371 lower-bound and 11 upper-bound
  solutions are retained; no residual or boundary cut is applied.
- All paired truth-centred endpoints succeed. Physical free IP covariances
  and fitted loss variances pass validity checks.
- 728 no matched-interval eBrem, 533 with cumulative loss (0,1%), 413 with
  loss >=1%, three unknown-category tracks retained inclusively (21:28,
  30:1, 32:61). All 728 no-loss free endpoints exactly copy the corresponding
  zero-centred breakpoint endpoints, not baseline KF.
- Generator pT checked for 2,162 rows, input KF identity for 2,000 first-track
  rows, cumulative G4 interval loss for 1,772 rows. Truth pT in the paired
  sample spans 6.5912--49.7378 GeV; filename momentum labels are not the
  actual generated momenta. Old tuples supply metadata, not refit results.
- 212 valid first-track secondary-control rows are reported separately.

Code implementation checkpoint: e2b4f3d; fixed-plan checkpoint: df3de6e.
Card SHA256: e77fa8171839cdfec8f840fdf6b8657342921f1fd05ef1c803278267850f6fe6.
Final analysis script SHA256:
dfdf6070b93a5dbf95fd1af4ca057cb5ac7e6158136670ad0dd5a2b79d359b58.
Topology catalogue SHA256:
c1c458699c0aa16080d68cd801cd5ac11946dd1dc0399befb42b9d122173fa56.
Category catalogue SHA256:
ffd8dcd5f88f264cda8bc9c33f2919bf02b7fa6cb3b1a2cde14ea5755f1c51c0.
Exact per-event results, failures, configuration, job audit, seed-block metrics
and bootstrap intervals are in the output directory named above, particularly
all_rows.csv, analysis_summary.json, resolution_summary.csv,
paired_seed_bootstrap.csv and failure_inclusive_window.csv.

### Inclusive paired resolution

Residual, median, width68 and RMS are in percent. RMS includes all outliers;
width68 is (q84-q16)/2, not a Gaussian fitted sigma.

| Endpoint | Median | width68 | RMS | Within +/-1% (%) | Outside +/-5% (%) | Outside +/-10% (%) |
|---|---:|---:|---:|---:|---:|---:|
| Baseline KF | -0.1128 | 0.8608 | 6.7592 | 80.3220 | 9.1831 | 6.0227 |
| Zero-centred RTS | -0.1308 | 0.8677 | 6.7463 | 80.3220 | 9.1831 | 5.9630 |
| Zero-centred backward | -0.1313 | 0.8101 | 6.7328 | 80.7990 | 9.1234 | 5.9034 |
| Free-loss RTS | -0.0210 | 0.3413 | 12.5630 | 82.0513 | 7.6923 | 5.7245 |
| Free-loss backward | -0.0221 | 0.3802 | 12.5812 | 82.1705 | 7.7519 | 5.7841 |
| Truth-centred RTS | -0.0347 | 0.2038 | 2.8589 | 95.4085 | 1.4908 | 0.6559 |
| Truth-centred backward | -0.0376 | 0.2335 | 2.8619 | 95.3488 | 1.4311 | 0.6559 |

### Loss categories

| Category | N | KF width68 | Free RTS width68 | KF RMS | Free RTS RMS | KF within +/-1% (%) | Free RTS within +/-1% (%) |
|---|---:|---:|---:|---:|---:|---:|---:|
| No matched-interval eBrem | 728 | 0.1693 | 0.1721 | 2.3339 | 2.3347 | 98.6264 | 98.4890 |
| Loss (0,1%) | 533 | 0.2589 | 0.5811 | 3.9219 | 10.0223 | 96.9981 | 82.9268 |
| Loss >=1% | 413 | 6.1370 | 2.6507 | 12.4922 | 22.3969 | 26.6344 | 52.0581 |

Light-loss zero-centred RTS width68/RMS/within1 are .2550/3.9245/97.1857%,
so the deterioration is also relative to the same breakpoint fitter without
free prior-centre optimization. For >=1% they are 6.1602/12.4625/26.6344%.
Free backward follows the same pattern: light-loss .5959/9.9968/82.9268%,
>=1% 2.7030/22.4525/52.3002%.
No matched-interval eBrem does not assert no loss anywhere before the first hit.

### Stability, tails and failure-inclusive result

2,000 seed-cluster bootstrap repetitions, free RTS minus baseline KF:

| Difference | Estimate | 95% interval |
|---|---:|---:|
| Inclusive width68 (percentage points) | -0.51952 | [-0.76892,-0.31539] |
| Inclusive RMS (percentage points) | +5.80378 | [+3.38175,+8.16648] |
| Inclusive within +/-1% rate (percentage points) | +1.72928 | [-0.00146,+3.54211] |
| Light-loss within +/-1% rate (percentage points) | -14.07129 | [-17.49540,-10.75644] |
| >=1% loss within +/-1% rate (percentage points) | +25.42373 | [+21.14418,+30.12082] |

Seeds21--30 (834 tracks): KF/free RTS width68 .9342/.3718, RMS7.2536/12.2633.
Seeds31--40 (843 tracks): width68 .7777/.3270, RMS6.2316/12.8526.
Thus both fixed blocks reproduce narrower width68 and worse RMS.

Eight free RTS and backward residuals exceed +100%; baseline KF and
zero-centred pairs have none. Free RTS examples (seed:event, residual percent):
21:4 +125.099248; 21:16 +152.722280; 22:5 +132.221209;
31:60 +165.578148; 31:95 +105.829327; 34:40 +122.274967;
36:16 +143.773759; 36:32 +102.467410. One is light-loss, seven >=1%.
Do not describe every tail rate as worse: the >5% tail frequency decreases,
while its far-positive extremes become much larger and dominate RMS.
Inclusive free RTS improves absolute residual for 805 tracks, worsens 871,
leaves one unchanged versus KF; versus zero-centred RTS the counts are
327/259/1091. It creates 49 new >5% cases and rescues 74.

Using all 1,711 baseline-valid candidates, counting absent refits as unavailable
rather than silently substituting KF: within +/-1% is KF1371/1711=80.1286%,
free RTS1376/1711=80.4208%, free backward1378/1711=80.5377%.
The net usable gain is only five/seven tracks, respectively.

Secondary controls, not optimization evidence (212 valid primary rows):
KF width68/RMS/within1 =27.7071/37.6514/25.0%; free RTS
26.2283/41.2390/33.9623%, with six >100% outliers versus zero for KF.

### Interpretation and next decision

Larger-loss central recovery is real in this sample, but current free-loss
steering is not justified as a general improvement. The truth-centred
reference remains considerably better; the free optimization has not recovered
that performance. These results do not establish the cause of the extreme
solutions, validate the objective, or resolve no-prior augmented-KF proposals.
Next useful diagnostic is same-event inspection of light-loss degradation and
the eight extreme minima, including fitted loss and objective profile. No new
prior, objective, default or algorithm change is authorized by this study.

## Location split, 2026-10-01

User requested light/hard categories by eBrem layer. Reused the exact 946
paired loss tracks, without rerunning or changing the fitter. Light remains
0<total matched loss<1%; hard remains >=1%. Location is the selected interval
with the largest summed absolute G4 eBrem loss, matching current Truth
interval selection. It is NOT the exact physical emission layer or a
first-half/second-half classification, and multiple losses still receive one
dominant-interval label. Labels identify both ordered runtime hit anchors.

Decoded hit_cell_id: system is low five bits, layer begins at bit7 with nine
bits for silicon and thirteen for TPC. Checked against geometry readout
encodings and existing location analysis; row identity, selected interval and
truth-override interval/loss agree for all 946 rows. Detector layer labels are
zero-based encoded layers, not a global hit index. Same-layer and skipped-layer
intervals are retained explicitly. Internal TPC rows are pooled for plotting
only; exact TPC row-to-row metrics are retained in the tables.

| Dominant interval region | Light N | Hard N | Light KF within1 (%) | Light free RTS within1 (%) | Hard KF within1 (%) | Hard free RTS within1 (%) |
|---|---:|---:|---:|---:|---:|---:|
| Within VXD | 51 | 46 | 96.1 | 82.4 | 0.0 | 4.3 |
| VXD to ITK | 77 | 55 | 97.4 | 67.5 | 0.0 | 1.8 |
| Within ITK | 122 | 103 | 95.1 | 63.9 | 2.9 | 39.8 |
| ITK to TPC | 78 | 60 | 93.6 | 84.6 | 6.7 | 66.7 |
| Within TPC | 105 | 62 | 100.0 | 100.0 | 38.7 | 83.9 |
| TPC to OTK | 100 | 87 | 99.0 | 99.0 | 90.8 | 90.8 |

All eight >100% free RTS outliers are VXD-internal (three, including the one
light-loss event) or VXD-to-ITK (five). Hard-loss improvement is concentrated
in ITK/internal, ITK-to-TPC and internal-TPC intervals; earlier losses remain
poorly recovered. These are empirical location associations, not a causal
diagnosis or an equal-loss/equal-momentum comparison across detector regions.

Outputs below the original study directory:
- analyze_locations.py: reproducible read-only analysis, not committed.
- by_location/event_locations.csv: all 946 paired event IDs, exact anchors,
  radii/z, selected loss and all existing endpoint residuals.
- by_location/location_counts.csv and location_resolution.csv: region,
  plotting-interval and exact-interval counts and full-range metrics.
- by_location/location_paired_changes.csv: improvements, worsening and tails.
- by_location/{light,hard}_{rts,backward}_regions_{core,tails}.{png,pdf}:
  six-region comparison panels. Core is +/-1%, tails +/-100% with log y.
- by_location/intervals/: individual plots for all 24 plotting interval
  categories, showing light/hard and core/tails, separately for RTS/backward.
  N<20 is marked low-statistics. Each canvas contains only KF, ordinary and
  free-loss; truth-centred statistics are also available in the CSV.
- by_location/summary.json: definitions and provenance hashes.

Histogram normalization uses the full category population, not the displayed
window; overflow counts are shown. No residual selection or fit is applied.
No source, card, installed runtime or remote operation changed.

### Strict single-eBrem restriction, 2026-10-01

User requested the same table for real single-emission events. Counted actual
Geant4 processSubtype=3 steps for the matched primary electron, in the full
embedded tracker record of the same input trk files. Require exactly one
occurrence with positive momentumLoss; no extra analysis loss threshold.
This is stricter than one nonzero-loss interval. The scope is the recorded
tracker region, not the calorimeter/entire detector. Light/hard and location
definitions remain unchanged from the preceding table.

Of 946 paired loss tracks, 536 are strict single-eBrem: 348 light and 188 hard.
The remaining 410 have multiple steps: 270 have two, 108 three, 26 four,
four five, one six and one 29. No nonpositive-loss eBrem process occurrences
were found. All 946 generator pT values agree with input truth. For all 536
single-emission tracks, the unique step belongs to the selected breakpoint's
exact (start,end] truth hooks and its momentumLoss reproduces that interval's
truth loss. No optimizer fallback remains in this subset.

| Interval region | Light N | Hard N | Light KF within1 (%) | Light free RTS within1 (%) | Hard KF within1 (%) | Hard free RTS within1 (%) |
|---|---:|---:|---:|---:|---:|---:|
| Within VXD | 32 | 19 | 100.0 | 81.25 | 0.0 | 0.0 |
| VXD to ITK | 50 | 22 | 98.0 | 66.0 | 0.0 | 0.0 |
| Within ITK | 74 | 49 | 100.0 | 64.865 | 2.041 | 44.898 |
| ITK to TPC | 46 | 22 | 100.0 | 91.304 | 9.091 | 59.091 |
| Within TPC | 77 | 28 | 100.0 | 100.0 | 53.571 | 100.0 |
| TPC to OTK | 69 | 48 | 100.0 | 100.0 | 97.917 | 97.917 |

Overall light within1: KF347/348, free RTS295/348; hard: KF65/188,
free RTS110/188. Two >100% free RTS overshoots remain, 22:5 and 36:16,
both VXD L5 -> ITK L0. Thus ignored additional eBrem emissions do not alone
explain the early-loss failures or the light-loss degradation. This is not a
causal isolation of other differences, and per-region statistics remain small.

Reproducible analysis: analyze_single_ebrem.py in the study directory.
Outputs: by_location/single_ebrem/{step_count_audit.csv,event_rows.csv,
location_table.csv,resolution_summary.csv,summary.json}. These preserve all
endpoint metrics and exact layer-pair labels as well as the region table.
Native ROOT readout was used after the uproot array read stalled; neither
reader attempt reran or modified tracking. No source/card/runtime change.

### Which single-emission events prefer free-loss in complete RTS chi2?

2026-10-01 read-only comparison, same 536 strict single-emission tracks.
Gain = truth_override_smoothed_chi2 - free_loss_smoothed_chi2. Use 1e-6
tolerance for ties, not a significance threshold. There are 497 free-lower,
38 truth-lower and one numerical tie; strictly positive gain also counts 497.
All ordinary/free/truth complete scores were checked finite with status1;
free_loss_quadratic agrees with free_loss_smoothed_chi2. Per-hit complete
contributions reproduce both totals. Baseline KF chi2 was read separately
from CompleteTracks and is not mislabeled as complete RTS chi2.

| Loss category | Free-lower / total | Median chi2 gain among free-lower | Free pT worse than truth among free-lower | Free outside +/-1% | Truth outside +/-1% |
|---|---:|---:|---:|---:|---:|
| Light | 322/348 | 0.09983 | 242/322 | 52/322 | 0/322 |
| Hard | 175/188 | 0.40903 | 128/175 | 65/175 | 7/175 |
| All | 497/536 | 0.19324 | 370/497 | 117/497 | 7/497 |

Of the 497 lower-chi2 tracks, 373 gain less than one chi2 unit, 124 gain more
than one, 33 more than five and 24 more than ten. There is no universal
overestimated-loss class: 242 overestimate the fitted loss, 255 underestimate;
190 use a zero prior centre. However all 52 light-loss failures outside +/-1%
overestimate loss and have positive pT residuals. Across both loss categories,
98/117 outside-window free tracks overshoot pT, 19 undershoot.

Location among free-lower: VXD28 (15 outside +/-1%), VXD-to-ITK56 (35),
ITK123 (53), ITK-to-TPC68 (13), TPC105 (0), TPC-to-OTK117 (1).
Corresponding median numbers of hits up to the upstream breakpoint anchor:
4.5,6,8,9,130,232. Thus 116/117 outside-window free tracks have a dominant
interval before or entering TPC. This association is consistent with weak
inner-track/loss separation but does not independently prove the cause.

Representative actual paired values (loss in percent, residual in percent):

| Seed:event | Interval region | Truth loss | Free posterior loss | Chi2 gain | Free pT residual | Truth-centred pT residual |
|---|---|---:|---:|---:|---:|---:|
| 24:10 | ITK-to-TPC | 0.02582 | 6.75854 | 234.75346 | +6.90937 | +0.50259 |
| 26:60 | VXD-to-ITK | 0.05511 | 39.02955 | 16.78635 | +63.71931 | -0.07776 |
| 22:5 | VXD-to-ITK | 14.75915 | 63.21219 | 18.40059 | +132.22121 | +0.40219 |
| 36:16 | VXD-to-ITK | 10.44886 | 63.21232 | 67.76841 | +143.77376 | +0.30573 |

The last two hit the optimized-prior upper bound b=1. Example24:10 has
206.422 of the 234.753 gain on contributions through the upstream anchor,
and28.331 from the target outward. Those local contributions include process
and seed terms; they are NOT pure measurement-only chi2. Do not infer that
all reductions are numerical noise or merely tiny statistical improvements.
Minuit minimizes normalized likelihood, not chi2 alone. Independent real-event
objective closure and decomposition of pathological minima remain unperformed.

Outputs: by_location/single_ebrem/chi2_preference/{paired_rows.csv,
category_summary.csv,summary.json}; script analyze_chi2_preference.py.
The preceding four-column performance table with medians is preserved in
by_location/single_ebrem/location_performance_with_chi2.csv, with paired
scores in event_rows_with_chi2.csv and checks in chi2_summary.json.
No source, card, fitting settings or remote state changed.

### Complete-score decomposition: seven same-code paired examples

2026-10-01 user authorized separating measurement/process/seed contributions.
The free/truth flat outputs contain local complete scores but not separated
terms. Recovered these through the unchanged shared fitter by placing each
saved optimized prior centre or truth centre in MeanLogLoss, keeping
SigmaLogLoss=.001 and all other physics settings unchanged, disabling only
the extra optimization/truth passes. This exposes the ordinary slot's detailed
score arrays for the identical Gaussian-centre fit. No source/card/install
change, temporary implementation, build-tree override or remote operation.

Four known failures:24:10,26:60,22:5,36:16. Three explicitly selected
within-window controls:21:5 (hard/internal TPC),21:27 (light/internal TPC),
23:13 (hard/internal ITK). These are diagnostic examples, not an unbiased
population measurement. Fourteen isolated local jobs all exited successfully.
Driver, options steering, hashes, verbose full state/covariance dumps and raw
tuples: /tmp/recbreakpoint-score-decomposition-20261001.83KFP8/.
Installed library/card SHA256 remain the same as the population run.

Reproduction: compared 255,446 numerical entries. All14 RTS pT values AND IP
parameter vectors reproduce exactly. Twelve complete/local RTS chi2 records
are exact; largest total difference2.1007e-9, largest local difference7.36e-9.
The original strict all-field gate did NOT pass:22:5 free has these tiny local
differences, and several backward records differ (largest backward chi2
difference3.3832e-5, largest backward IP parameter difference3.8417e-8).
Do not claim blanket bit-identical reproduction or hide these discrepancies.
The quantities used for this decomposition pass exact endpoint and absolute
1e-7 RTS-score gates, with strict-gate details retained in the evidence.
All measurement+process+seed sums reproduce their saved local/total scores.

Gain below means truth-centred minus free-centred; positive favors free.

| Seed:event | Measurement gain | Process gain | Seed gain | Complete gain |
|---|---:|---:|---:|---:|
| 24:10 bad | 21.225979 | 213.527481 | -2.744e-7 | 234.753460 |
| 26:60 bad | 14.521710 | 2.264654 | -1.819e-5 | 16.786346 |
| 22:5 bad | 17.261096 | 1.139498 | -2.285e-6 | 18.400591 |
| 36:16 bad | 31.195296 | 36.573140 | -2.995e-5 | 67.768406 |
| 21:5 control | 0.034804 | -0.025365 | -9.262e-12 | 0.009439 |
| 21:27 control | 2.311247 | 0.067232 | 2.472e-11 | 2.378479 |
| 23:13 control | -0.010839 | 0.095529 | 2.231e-9 | 0.084690 |

All four bad examples genuinely improve the measurement-noise-weighted hit
score as well as the complete score. This persists using direct native hit
projection rather than affine measurement approximation: respective native
measurement gains are21.131271,15.504575,17.540755,36.629408. Thus the sign is
not an artifact of using the affine hit score. It is not universal: control
23:13 slightly worsens its measurement contribution while improving process.
Seed penalties are numerically negligible here, not the driver of the effect.

24:10 is process dominated (91.0% of its gain). Its process contribution
drops239.5433 ->26.01585, while measurement536.5572 ->515.3312. Incoming
edges to ITK L1 and L2 account for process gains118.1457 and56.5187; both
precede the selected ITK L2 -> TPC L0 loss interval. The large gain is not
simply a penalty at the breakpoint itself. Other useful local evidence:
26:60 obtains9.8661 measurement gain at VXD L0;36:16 gains both in silicon
hit agreement and in incoming process terms around VXD L5/ITK L0/ITK L2.

This establishes the requested distinction for the selected examples: wrong
IP momentum can accompany genuinely better hit agreement, sometimes with a
large additional process-score benefit. It does not independently validate
the transport/noise model or identify a coding bug. Next narrow check is the
actual smoothed process residuals versus propagated Q on the dominant ITK
edges of24:10, with an independent same-linearized-model score closure.

Durable generated tables: TrackingPerformanceStudies/
breakpoint_shared_sigma_population_20260930/score_decomposition/
score_components.csv,paired_gains.csv,per_hit_components.csv,per_hit_gains.csv,
reproduction_gate.csv,summary.json. No generated artifacts are committed.

### Additional twelve paired score-decomposition examples

2026-10-01, user requested more events. Reused the same installed library,
card and Gaussian-centre replay procedure above; no Minuit rerun, source,
maintained card, build, installation or remote changes. SigmaLogLoss=.001.
Twenty-four isolated fits exited successfully. Raw evidence and selection:
/tmp/recbreakpoint-score-more-20261001.w0gDTw/.

Selection was fixed before reruns, excluding the preceding seven: first
seed/event in each qualifying light/hard x VXD/ITK/ITK-to-TPC failure cell
(free chi2 lower, free outside +/-1%, truth inside); four within-window
controls across tracker locations; two opposite, truth-lower-score controls.
All belong to the strict single-emission sample. This is selected diagnostic
evidence, not a population failure-rate estimate or held-out validation.

All24 RTS IP pT, IP parameter vectors, complete chi2 and local chi2 arrays
reproduce saved results exactly. All12 available free smoothed state and
covariance arrays reproduce exactly. Across437,328 numerical comparisons,
some other fields are not bit-identical, but all pass the original allclose
gate (rtol1e-10, atol1e-9). Do not generalize exactness to every field.

Gain = truth-centred chi2 minus free-centred chi2; positive favors free.
pT residuals are100*(pT_reco/pT_truth-1), in percent. Seed gain is omitted
below because its largest absolute value is4.983e-6; it remains in totals.

| Seed:event | Role / interval | Measurement gain | Process gain | Complete gain | Free pT residual % | Truth pT residual % |
|---|---|---:|---:|---:|---:|---:|
| 21:67 | light bad / VXD L4 -> L5 | 1.615675 | 0.062383 | 1.678057 | +30.062293 | -0.200370 |
| 21:60 | light bad / ITK L1 -> L2 | 0.724666 | 3.042238 | 3.766904 | +2.428449 | +0.066929 |
| 32:3 | light bad / ITK L2 -> TPC L0 | 1.696595 | 1.659163 | 3.355758 | +1.410393 | +0.111644 |
| 21:96 | hard bad / VXD L4 -> L5 | 2.451045 | -0.347766 | 2.103274 | +40.724165 | +0.077128 |
| 21:83 | hard bad / ITK L1 -> L2 | 4.298582 | 9.872895 | 14.171478 | +4.948464 | +0.726743 |
| 23:21 | hard bad / ITK L1 -> TPC L0 | 3.611921 | 9.869567 | 13.481488 | +3.763482 | +0.571853 |
| 21:39 | light control / TPC L166 -> L167 | -0.288860 | 0.837148 | 0.548288 | +0.139470 | +0.074952 |
| 21:44 | hard control / TPC L105 -> L106 | 0.435485 | -0.046132 | 0.389353 | -0.239160 | -0.082478 |
| 21:25 | light control / TPC L222 -> OTK L0 | 0.100448 | 0.028583 | 0.129031 | -0.068321 | -0.066873 |
| 23:50 | hard control / ITK L0 -> L1 | 0.237254 | 0.364385 | 0.601639 | -0.973003 | +0.122105 |
| 21:6 | truth-lower light / VXD L5 -> ITK L0 | -0.000138 | 0.000022 | -0.000116 | +0.212543 | +0.218500 |
| 21:72 | truth-lower hard / VXD L1 -> L3 | -0.132589 | 0.001308 | -0.131278 | -26.561480 | +0.003708 |

All six new bad examples improve native (non-affine) hit-projection chi2 too:
gains1.594644,.722784,1.657941,2.449476,4.312180,3.680552 in table order.
Together with the previous four, all ten selected bad examples show genuinely
better measurement-noise-weighted hit agreement despite worse IP momentum.
Three new failures are process-gain dominated; the VXD cases are measurement
dominated. This is not explained solely by seed penalties or affine hit
projection. Control21:39, however, worsens measurement chi2 and improves the
complete score through process terms; never infer measurement improvement
from complete-score improvement alone. Truth-lower controls are not by
themselves optimizer failures: Minuit minimizes normalized likelihood,
including its log determinant, rather than chi2 alone. Transport/noise-model
closure and the physical cause remain unresolved; no model fix is claimed.

Durable generated evidence: TrackingPerformanceStudies/
breakpoint_shared_sigma_population_20260930/score_decomposition_more/
paired_gains.csv,score_components.csv,per_hit_components.csv,per_hit_gains.csv,
reproduction_gate.csv,summary.json. Generated artifacts remain uncommitted.
