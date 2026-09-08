# Persistent6D: ten additional single-eBrem event comparisons

User requested a few more events after the persistent-6D implementation.
Ran unchanged code 9263ba8 on test_breakpoint. No source/card, branch or remote
changes. Generated jobs and checks are stage more_single under
TrackingPerformanceStudies/recbreakpoint_persistent6d_2026-09-08/.

## Selection and settings

Ten events from the earlier audited twenty-event single-eBrem negative-residual
sample, none used in the initial Persistent6D tests. Chosen for coverage of
inner (<50mm), ITK-radius and TPC-radius losses, not a random/held-out
population. Exactly one positive-loss primary Geant4 eBrem step in each event,
inside the fitted hit range, with zero secondary tracker hits. Original audit:
agents_record/2026-09-08-recbreakpoint-single-ebrem-twenty-events.md.

Fresh paired LocalMarginal and Persistent6D runs both use BackwardMode=RTS,
FirstMiddleLast, SeedScale1, mean b0, sigma b0.05, MS=true, Eloss=false,
MaxChi2PerHit=1e100. Truth selects only the breakpoint interval, not the loss
prior or fitted value. Full verbose diagnostics and native KF references on.
Inputs remain gsf_doublebhoff_freshseed_diagnostic/trk-e--2.0-85-{seed}.root.
Twenty successful fit rows across eighteen jobs; two events share seed3/h4.

## Results

Residual percent = 100*(pT_reco/pT_truth-1). Entries are zero-based. Radius is
the Geant4 eBrem step post-position. KF is stored CompleteTracks. The GSF column
is the already available fresh reverse FullMixtureMode reference from the
preceding twenty-event study; GSF was NOT rerun for this request. Both breakpoint
columns are new direct same-code runs, not copied previous endpoints.

| Seed:entry | Interval | r mm | Loss % | KF % | Previous GSF % | LocalMarginal RTS % | Persistent6D RTS % |
|---|---:|---:|---:|---:|---:|---:|---:|
| 2:16 | 6 | 347.152 | 1.1489 | -1.43393 | -1.42973 | -1.21268 | -1.21268 |
| 2:86 | 7 | 345.116 | 0.4905 | -0.54979 | -0.56375 | +0.20869 | +0.20869 |
| 3:10 | 4 | 40.608 | 0.7161 | -0.70811 | -0.71862 | -1.17142 | -1.17142 |
| 3:33 | 4 | 45.531 | 0.9355 | -1.11583 | -1.12874 | -1.10417 | -1.10417 |
| 3:12 | 5 | 234.245 | 1.9334 | -1.57601 | -0.30769 | -0.54183 | -0.54183 |
| 3:19 | 6 | 234.990 | 1.3663 | -1.24935 | -1.26541 | +0.37010 | +0.37010 |
| 5:49 | 7 | 554.944 | 0.2788 | -0.19071 | -0.17641 | +0.19293 | +0.19293 |
| 5:78 | 8 | 554.951 | 1.5640 | -1.45950 | -1.43828 | -0.56363 | -0.56363 |
| 6:17 | 13 | 666.722 | 0.9127 | -0.81052 | -0.81960 | -0.43437 | -0.43437 |
| 6:37 | 34 | 760.513 | 0.4890 | -0.27192 | -0.26463 | -0.10103 | -0.10103 |

The new and old path agree EXACTLY in stored IP pT for all ten. Internal
double-precision values are not claimed bitwise identical:

- Maximum fitted b difference: 1.64657e-11.
- Maximum fitted b variance difference: 3.08022e-16.
- Maximum outward filtered kappa difference: 3.25005e-12.
- Maximum persistent transition covariance closure: 3.27618e-16.

Mean absolute residuals: stored KF0.93657%, previous GSF0.81129%, both
breakpoint paths0.59009%. RMS: KF1.05105%, GSF0.93220%, both breakpoint
paths0.71344%. Both breakpoint methods improve versus KF in 8/10 cases,
but this selected subset must not override the less favorable full twenty-
event results. Fitted loss remains negative in 3:10 and 3:33.

## Checks and interpretation

All status, expected row-count, truth/KF identity and ordered-cell pairing
checks passed. Rechecked selected endpoint cells against the existing exact
truth audit. Every downstream full covariance is finite, symmetric and
positive definite. Every ordinary six-dimensional propagation preserves the
b mean/variance with no b process noise or repeated loss map; full J P J^T+Q
closure passes. Native measurement updates reduce b variance within numerical
tolerance. The final outward b equals the published fitted loss, and the RTS
smoothed b is the same static parameter at all downstream surfaces.

The live loss can move substantially while measurements accumulate. Example
seed5:49: b0.0289457 after h8, 0.0168221 after h120, 0.00401824 at h232;
sigma contracts from0.0419038 to0.0105023. These are direct live 6D filtered
states. Explicit persistence does not add information missing from the old
fixed-linearization local-joint/RTS calculation in these tests.

Artifacts: selected_more_single.json, jobs_more_single.json,
results_more_single.json, check_more_single.log, comparison_more_single.csv,
summary_more_single.log, and the more_seed*.root/.log files. No parameter
tuning or physics-performance claim beyond these selected comparisons.
