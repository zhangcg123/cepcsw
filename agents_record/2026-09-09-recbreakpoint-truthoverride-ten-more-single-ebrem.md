# TruthOverride: ten additional single-eBrem events

## Scope and provenance

User requested a few more events after the first truth-override comparison.
No source, card, build, defaults, branches or remote settings were changed.
Installed source checkpoint: `52744bc` on `test_breakpoint`.
Dedicated card SHA256:
`a2f0c03ff3db359f56f34505f482b5692ada632a73b9d68c380f09f4198100fb`.
Installed libRecBreakpoint.so SHA256:
`5c65aa67cb6fd05e09baff26d8a6b4fe103266ce79f05d725ecfb948c37b0bed`.

Ten entries were selected BEFORE these runs from the existing audited
twenty-event single-eBrem sample, excluding the five already tested by the
oracle. All have zero secondary tracker hits, exactly one positive primary
Geant4 eBrem step, and its post-point lies inside one matched hit interval.
No secondary/control event is included in the ten-event optimization count.
This is an extension of a selected diagnostic sample, not random or held-out
population validation. The configured interval is truth-informed, but no truth
loss magnitude is passed to the ordinary baseline fits.

The selected seed:entry pairs and interval indices are:
2:16[6], 3:19[6], 3:79[6], 4:4[8], 5:58[6], 5:62[4], 5:78[8],
6:4[7], 6:37[34], 6:50[7]. Entries are zero-based. Truth losses span
0.3133--1.5640%; emission radii span 40.873--760.513 mm.

Each event is run four ways using the SAME installed code: ordinary
Persistent6D/RTS, TruthOverride/RTS, ordinary LocalMarginal/BackwardFilter,
and TruthOverride/BackwardFilter. All use one pass, FirstMiddleLast, SeedScale1,
MS=true, Eloss=false, MaxChi2PerHit1e100, and the same selected interval.
Ordinary loss prior mean0/sigma0.05; TruthOverride uses the event's embedded
relation-driven G4 response with variance0. Native references and full verbose
state/covariance diagnostics are enabled. No GSF rerun is needed for this task.

The full campaign contains 32 grouped jobs and 40 fit rows. Input names are
gsf_doublebhoff_freshseed_diagnostic/trk-e--2.0-85-SEED.root; their names do not
imply constant truth momentum. Truth pT in this selection ranges 12.33--38.76 GeV.

Artifacts under TrackingPerformanceStudies/recbreakpoint_truthoverride_2026-09-09/:
run_more.py, selected_more.json, jobs_more.json, more_*.root/log,
check_results.py (TEST_STAGE=more), summarize_more.py, results_more.json,
comparison_more.csv, metrics_more.json. Scripts and generated outputs remain
uncommitted; this record is the durable evidence.

## Verification and results

All 32 jobs terminated successfully and all 40 fit rows passed. Event entries,
complete ordered-hit cell-ID vectors, truth pT and stored KF pT agree across
each four-way comparison. Every dumped 5D/6D covariance is finite, symmetric
and positive definite. Oracle rows have truth status1, exactly zero local/final
loss variance, and fitted/local/prior b equal to the matched truth b. Exact
step/fraction bounds, interval endpoint cell IDs and b agree with the independent
earlier Geant4 audit (b tolerance2e-12). Ordinary rows have disabled/empty truth
fields. There is no truth-failure fallback or excluded failed row in this table.

Residual %=100*(pT_reco/pT_truth-1). Both modes use one pass, not iterations.

| Seed:entry | Truth interval loss % | Ordinary RTS % | Truth RTS % | Ordinary backward % | Truth backward % |
|---|---:|---:|---:|---:|---:|
| 2:16 | 1.1489 | -1.2127 | -0.3419 | -1.1730 | -0.3550 |
| 3:19 | 1.3663 | +0.3701 | +0.0685 | +0.6576 | +0.0817 |
| 3:79 | 0.3133 | +0.4950 | +0.0555 | +0.6016 | +0.0224 |
| 4:4 | 1.0565 | -0.0825 | -0.2087 | -0.1471 | -0.2160 |
| 5:58 | 0.5637 | +1.2755 | +0.0437 | +1.2866 | +0.1010 |
| 5:62 | 0.5553 | +1.1607 | +0.4237 | +0.2825 | +0.4172 |
| 5:78 | 1.5640 | -0.5636 | -0.0648 | -0.6056 | -0.1010 |
| 6:4 | 0.3443 | +1.2102 | -0.0478 | +1.1701 | -0.0531 |
| 6:37 | 0.4890 | -0.1010 | +0.1204 | +0.0450 | +0.0937 |
| 6:50 | 1.1190 | +0.1671 | +0.0781 | +0.1265 | +0.0496 |

| Metric on these ten | RTS ordinary | RTS truth | Backward ordinary | Backward truth |
|---|---:|---:|---:|---:|
| Mean absolute residual % | 0.663841 | 0.145309 | 0.609560 | 0.149063 |
| Residual RMS % | 0.815719 | 0.194087 | 0.753723 | 0.197080 |
| Truth-mode maximum absolute residual % | -- | 0.423662 | -- | 0.417211 |
| Truth-mode count within 0.2% | -- | 7/10 | -- | 7/10 |
| Truth-mode count within 0.5% | -- | 10/10 | -- | 10/10 |

Absolute residual improves in8/10 RTS and7/10 backward fits. It worsens for
4:4 and6:37 in both methods, and for5:62 in backward mode. The remaining
5:62 positive residual (~0.42%) is associated with a truth emission at
r=40.873 mm, while2:16 remains around -0.35% at r=347.152 mm. These are
useful future diagnostics, not proof of a particular location/material error.

## Interpretation

This extension does NOT reproduce the earlier five-case statement that all
truth-mode residuals lie within0.2%. Improvement is common and selected-sample
mean absolute error is substantially reduced, but exact net interval loss does
not guarantee a better result for every event. TruthOverride simultaneously
fixes the loss magnitude and removes its uncertainty; this comparison does
not isolate those effects or remove the collapsed upstream loss-position
approximation, measurement noise, native material conventions, or backward
evidence reuse. These selected-sample RMS values are not Gaussian fit sigmas
or a validated population resolution. No new clean-track control population
was run; earlier zero-loss controls remain in the implementation record.

The active project focus and implementation/defaults are unchanged. This record
extends the existing oracle study; earlier evidence and plots were not removed.
