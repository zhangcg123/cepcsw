# RecBreakpoint: parallel ordinary and truth-override endpoints

## Request and scope

Add truth-override track results alongside the existing RTS/backward results.
When disabled, save the corresponding ordinary copies. Changes stay in the
RecBreakpoint algorithm/schema, dedicated local card, README and project
records on test_breakpoint. No fitter mathematics, shared KF, GSF, maintained
GSF/batch cards or remote state changed.

## Control and publication contract

- LossStateMode selects the ordinary Persistent6D or LocalMarginal fit.
- New TruthOverride bool defaults false. Dedicated-card BP_TRUTH_OVERRIDE=1
  enables it; its final value controls loading embedded truth collections.
- Public LossStateMode=TruthOverride is rejected with migration guidance.
  The internal fixed-loss fitter implementation remains unchanged.
- Ordinary RTS/backward always run first. True plus nonempty intervals runs
  an additional one-pass fixed-loss pair with the same hit selection, seed
  scales, native material/MS and Eloss setting. Ordinary iteration settings
  still apply only to the ordinary pair. Multiple ordinary intervals require
  LocalMarginal. No automatic truth-interval selection is added.
- False or empty intervals copies the completed ordinary pair into the extra
  outputs without rerunning the fit or accessing material-loss truth.
- Four independently named track collections: BreakpointTracksRTS,
  BreakpointTracksBackwardFilter, BreakpointTracksTruthOverrideRTS,
  BreakpointTracksTruthOverrideBackwardFilter. All names must be distinct.
- Each successful output has IP/first/last states, full EDM covariances,
  original ordered hits, and the corresponding RTS-complete or inward-sum
  chi2. NDF remains bookkeeping, not a calibrated fit probability.
- Extra input-row maps: BreakpointTruthOverrideRTSIndex and
  BreakpointTruthOverrideBackwardIndex; absent outputs map to -1.
- BreakpointTruthOverrideStatus / flat truth_override_result_status:
  0 absent/unattempted; 1 copied; 2 successful oracle;
  -1 event truth failure; -2 association failure; -3 truth interval failure;
  -4 oracle fitting/publication failure. Truth failure does NOT discard
  ordinary tracks or masquerade as a successful copied oracle.
- Existing truth_override_status retains truth-input validity meaning:
  0 off/empty, 1 valid, negative errors. It is NOT the new result-success flag.
- Extra flat endpoint pT, five EDM IP parameters and all 21 packed EDM covariance
  elements, forward/backward/smoothed chi2 lists and totals are automatic.
  Smoothed score validity/error is independent of endpoint validity.
  The first 15 covariance elements describe the five-parameter helix; the
  remaining EDM time row/column is not a fitted breakpoint coordinate.
- Ordinary fit failure leaves the oracle unattempted. Input loading failures
  may still abort before the algorithm; no claim of rescuing those is made.

The normal card remains flat-only, with optional PodioOutput lines retained.
Truth provenance and fixed-loss formula are unchanged from the original
truth-override implementation. The oracle remains a diagnostic, not a
production mode or a fit of the exact emission position within an interval.

## Build and focused tests

Built and package-installed libRecBreakpoint.so, SHA256:
`9d8620ae7310ec7c935ac00498091b12ea87642236e9b105d6630c972e3d5077`.
Generated configurable exposes the new bool and both output handles.

Artifacts (uncommitted):
`TrackingPerformanceStudies/recbreakpoint_truth_pair_2026-09-09/`.
run.py has smoke/gate stages, probe.py enables optional EDM serialization,
check.py checks flat/EDM results against each other and existing references.
All jobs use FirstMiddleLast, SeedScale=BackwardSeedScale=1, prior b=0,
sigma_b=.05, MS on, Eloss off, verbose state/covariance dumps and native KF
reference checks. Same ready tracker inputs as the preceding scale gate.

Focused seed2:68, interval7, Persistent6D, one pass:

| pT [GeV] | Ordinary RTS | Ordinary backward | Extra RTS | Extra backward |
|---|---:|---:|---:|---:|
| TruthOverride false | 44.97146297895819 | 44.99156384060726 | 44.97146297895819 | 44.99156384060726 |
| TruthOverride true | 44.97146297895819 | 44.99156384060726 | 44.67695906948779 | 44.69090711384354 |

Generator pT=44.68343734741211; stored KF pT=43.55779441768492 GeV.
The ordinary branches are exactly unchanged between on/off and the previous
scale1 run. Extra true pT and all three score lists exactly reproduce the
previous standalone truth-mode smoke. Disabled extra states/covariances and
scores exactly copy ordinary outputs, also after EDM serialization.

Expanded jobs completed successfully: seed12:11/16/17, interval5, each ordinary
mode with truth off/on; seed12:11 with 10 ordinary iterations plus oracle;
seed12:0 empty intervals plus enabled oracle; seed2:68 LocalMarginal intervals
5,7; and seed2:68 deliberately invalid association tolerance 1e-12 mm.
The invalid-truth job reports a 0.00814966 mm first-hook distance and preserves
its ordinary pair. Seed12:17 is secondary activity, not clean optimization.
Seed12 interval5 oracle losses are zero and are mechanical interval controls,
not claims of correcting all event losses.

These are mechanical regression checks, not population physics validation.

Final check.py gate passed all 18 attempted track/configuration cases (five
unique input events, including the secondary-activity control). Seventeen
extra pairs are successful copies/oracles; the deliberately invalid one is
absent with status -2, maps -1, NaN extra pT, and unchanged ordinary results.
Checks include full EDM endpoint parameters/covariances, positive-definite
five-dimensional covariance blocks, hit counts, row maps, exact disabled
copies, all three chi2 list sums, exact ordinary on/off equality, and exact
ordinary/standalone-oracle regression against preceding scale/parallel runs.
The test initially assumed a 15-element EDM covariance, then was corrected
against the installed TrackState header: EDM stores 21, and the implementation
already copied all 21. No fit or output code correction was needed for that.
Results and detailed tables are in results.json in the artifact directory.

## Memory maintenance

Complete outgoing AGENTS.md is preserved in
2026-09-09-agents-before-parallel-truth-pair.md. Only the current-focus section
was revised; global status, active laws and compile instructions were retained
byte-for-byte. No history directory was moved or deleted, so no migration
manifest was needed. Both substantive sections remain; the outgoing snapshot
covers every replaced focus statement.
