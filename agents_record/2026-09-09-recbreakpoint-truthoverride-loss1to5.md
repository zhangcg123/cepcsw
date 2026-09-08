# RecBreakpoint: eight new single-eBrem events with 1--5% loss

## Request and unchanged scope

The user postponed the proposed truth-centered finite-width prior experiment
and requested more events with 1--5% loss. This repeats the preceding paired
ordinary/TruthOverride comparison; no truth-centered prior or new algorithm
was tested. All fits use one pass, not iterative relinearization.

Local branch test_breakpoint, starting checkpoint b3a6185. No package source,
build, installed library, maintained card, workflow, default or remote changes.
The pre-existing tracked modifications in DumpGsfTrks/gsf.py.bk, sim.py.bk,
trk.py.bk, Reconstruction/CMakeLists.txt and subtrkjobs.sh remain untouched.

## Selection and provenance

Preselection uses the barrel rows in
TrackingPerformanceStudies/newbh_barrel_endcap_pt_resolution_2026-09-06/event_residuals.csv:
seeds 2--6, entries below 100, zero secondary tracker hits, valid truth scope,
one loss-bearing interval, and 1--5% cumulative interval loss. Previous 15
single-eBrem oracle events are excluded. No pT residual enters selection.

All 23 candidates are independently audited through reconstructed-hit
associations, SimTrackerHit provenance and exact Geant4 step/fraction hooks
in the existing tracker ROOTs. Interval losses/hooks/cell IDs are also compared
with existing passive GSF truth records. Exactly 16 have one positive eBrem
step in the complete primary recorded step sequence, inside one matched hit
interval. Seven have two emissions: 2:40, 3:18, 3:82, 4:64, 5:23, 6:48, 6:67.
These exclusions are made before fitting, not based on fit results.

Select two eligible events in each [1,2), [2,3), [3,4), [4,5) percent bin,
in increasing seed/entry order. This is a balanced diagnostic selection,
not a random or population-representative sample.

| Seed:entry | Ordered-hit interval | Truth loss % | Truth emission radius mm |
|---|---:|---:|---:|
| 2:68 | 7 -> 8 | 3.158228 | 600.219 |
| 3:12 | 5 -> 6 | 1.933355 | 234.245 |
| 3:75 | 8 -> 9 | 2.908665 | 554.496 |
| 3:83 | 233 -> 234 | 1.558422 | 1775.195 |
| 3:85 | 9 -> 10 | 2.066692 | 600.212 |
| 3:94 | 231 -> 232 | 3.876587 | 1807.642 |
| 4:65 | 230 -> 231 | 4.328695 | 1806.323 |
| 5:69 | 5 -> 6 | 4.550249 | 344.935 |

Entries and hit indices are zero-based. Truth loss is summed eBrem delta-p
divided by momentum at the upstream provenance hook, not total ionization
plus eBrem or loss relative to generator momentum. Intervals near index 230
are genuine final ordered-hit intervals, not fixed detector layer IDs.

## Same-code setup

Inputs: gsf_doublebhoff_freshseed_diagnostic/trk-e--2.0-85-SEED.root.
Dedicated unchanged card: Reconstruction/RecBreakpoint/options/run_breakpoint.py.
For each event run Persistent6D/RTS and LocalMarginal/BackwardFilter, each
paired with TruthOverride in the same BackwardMode. All MaxFitIterations=1,
FirstMiddleLast prefit, SeedScale=1, MS=true, Eloss=false, MaxChi2PerHit=1e100,
VerboseDump=true, VerifyKFReference=true. Ordinary b prior is mean 0,
sigma 0.05. TruthOverride uses fixed audited b and zero added loss variance,
and still applies the collapsed loss at the upstream surface. The explicit
interval is truth-selected by the diagnostic driver, not by the fitter.

Card SHA256 a2f0c03ff3db359f56f34505f482b5692ada632a73b9d68c380f09f4198100fb.
Installed libRecBreakpoint.so SHA256
5c65aa67cb6fd05e09baff26d8a6b4fe103266ce79f05d725ecfb948c37b0bed.

Artifacts under TrackingPerformanceStudies/recbreakpoint_truthoverride_2026-09-09/:
audit_range1to5.py, range1to5_audit_seed*.json, selected_range1to5.json,
rejected_range1to5.json, jobs_range1to5.json, range1to5_*.root/log.
run_more.py, check_results.py and summarize_more.py accept TEST_STAGE=range1to5;
the earlier more-stage defaults are preserved. Analysis scripts and generated
outputs remain uncommitted; only this durable evidence record is committed.

## Results and verification

All 32 jobs terminated successfully, and all 32 fit rows have status=1.
Full ordered-hit cell-ID vectors, truth pT and stored KF pT are identical
across each event's four runs. Verbose 5D/6D covariance dumps are finite,
symmetric and positive definite. Oracle rows have truth_override_status=1,
one pass, exactly zero local/final loss variances, and equal prior/local/final
b matching the independently audited b to 2e-12. Exact truth step numbers,
fractions and interval endpoint cell IDs also match the independent audit.
Ordinary rows keep truth_override_status=0 and empty override vectors.
No failed fit was excluded and no truth fallback was used. Card and installed
library SHA256 values remained unchanged after the tests.

Residual %=100*(pT_reco/pT_truth-1); these are one-pass results, not iterations.

| Seed:entry | Truth loss % | Ordinary RTS % | Truth RTS % | Ordinary backward % | Truth backward % |
|---|---:|---:|---:|---:|---:|
| 2:68 | 3.1582 | +0.6446 | -0.0145 | +0.6896 | +0.0167 |
| 3:12 | 1.9334 | -0.5418 | +0.3298 | -0.7152 | +0.3462 |
| 3:75 | 2.9087 | +1.9182 | +0.2642 | +1.7712 | +0.2834 |
| 3:83 | 1.5584 | +0.1125 | +0.1167 | +0.1108 | +0.1129 |
| 3:85 | 2.0667 | +0.4989 | +0.2762 | +0.4851 | +0.2695 |
| 3:94 | 3.8766 | -0.0864 | -0.0716 | -0.0857 | -0.0730 |
| 4:65 | 4.3287 | +0.2004 | +0.1994 | +0.2175 | +0.2185 |
| 5:69 | 4.5502 | +1.2149 | +0.4490 | +1.4361 | +0.4648 |

| Metric | Ordinary RTS | Truth RTS | Ordinary backward | Truth backward |
|---|---:|---:|---:|---:|
| Mean absolute residual % | 0.652204 | 0.215176 | 0.688886 | 0.223121 |
| Residual RMS % | 0.878483 | 0.253765 | 0.900593 | 0.263361 |
| Truth-mode count within +/-0.2% | -- | 4/8 | -- | 3/8 |
| Truth-mode count within +/-0.5% | -- | 8/8 | -- | 8/8 |

Absolute residual improves in 7/8 RTS and 6/8 backward cases. The small
changes in 3:83 worsen both; 4:65 worsens backward by approximately 0.001
percentage point while improving RTS by a similarly small amount. Final-edge
cases 3:83, 3:94 and 4:65 barely change relative to the other selected cases.
Several oracle results remain around +0.26--0.46%; 5:69 has the largest
remaining absolute residual, +0.4490% RTS and +0.4648% backward.

TruthOverride improves this selected sample overall but does not guarantee
truth pT. This study does not isolate whether remaining errors originate in
within-interval placement, noise/material assumptions or other fit limitations.
It changes both the loss magnitude and its variance, not just the prior center.
No new finite-width truth-centered-prior or iteration study was performed.
No GSF reruns were made. These eight balanced single-emission examples and
their RMS are not a validated population resolution or Gaussian fit sigma;
no new no-eBrem or secondary-activity control sample was run in this extension.

Final artifacts also include results_range1to5.json, comparison_range1to5.csv,
metrics_range1to5.json. The CSV retains truth pT, stored KF residual, and all
four newly fitted pT values in addition to residuals. Verification log:
/tmp/recbreakpoint_range1to5_check.log; summary:
/tmp/recbreakpoint_range1to5_summary.log. The durable numerical evidence is
the table above; temporary files are not required to understand it.
