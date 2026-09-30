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
