# Parallel truth-location reference and 10k Higgs silicon-scan batch

## Request and scope

User requested a roughly 10,000-event Higgs silicon-location scan, saving a
truth-location scan reference in parallel, then confirmed no active jobs and
authorized installation after validation. Branch: breakpoint-delayed-neutral.
No branch operation, remote push, PFA/shared-KF change, full sim/rec rerun or
Condor submission was performed. Production default selection remains Truth.

## What is saved

The existing blind HitScan candidate scope/scoring is unchanged: VXD/ITK hits
except the last track hit, with all tracker measurements retained. Score the
configured/available beam and charged ECAL observations (110 when available),
never the separate neutral update. Null is a separate 5D fit with the same
measurements. Gain = null NLL2 - best candidate NLL2. All candidate scores,
failures and the null remain in the main breakpoint tree.

After the blind decision is frozen, the same association/provenance reader and
largest-eligible-owner rule as Truth selection provide a passive location.
Truth LOSS AMOUNT is never injected into this free-loss reference. If the
location was scanned, reuse that exact candidate, including failures. If an
eligible truth owner lies outside the silicon candidate list (e.g. TPC), run
one extra fit with the same scorer and save it as reference ONLY. Never append
it to the candidate vectors or reselect the winner. No eligible owner uses
the null reference; this does not imply there was no physical eBrem. The
existing ownership vectors describe missing/shared/ineligible material.

HitScan now loads tracker-hit truth associations even with TruthOverride=False.
Missing/unusable truth is tagged independently and cannot fail or steer the
blind scan. No new configurable property was needed.

Eleven additional flat branches, all prefixed hit_scan_truth_:

| Suffix | Meaning |
|---|---|
| status | 0 inactive, 1 null reference, 2 fitted, -1 truth association failed, -2 fit failed |
| hit | Truth-owned ordered hit index, -1 for null/unavailable |
| nll2 | Normalized -2 log L for that location/reference |
| ip_omega, pt | IP curvature and pT from that same fit |
| prior_center | Optimized Gaussian loss-prior center |
| fitted_log_loss, fitted_variance | Posterior b and its variance |
| minuit_status | Minuit status; -99 means no optimizer was needed/accepted |
| reused_candidate | Whether an existing silicon candidate supplied the reference |
| error | Independent truth/reference failure explanation |

There are now 36 hit_scan_* branches. This family is distinct from
truth_override_*: that older optional diagnostic uses truth loss amounts at
the selected location, not a separate truth-location fit. The new campaign
disables TruthOverride.

## Batch prepared, not submitted

Launcher: subbreakpointhiggsscanjobs.sh. Existing batch_breakpoint.py and
dump_breakpoint.sh perform freezing, submission and execution; no new worker.

- Input: higgs_full_20261030/rec-higgs-{1..50}.root.
- Output: higgs_silicon_scan_20261103/breakpoint_flat-higgs-<seed>.root.
- Stages: breakpoint only. External rec inputs are not removed or overwritten.
- 50 jobs x 200 events = 10,000 requested events, not necessarily 10,000 tracks.
- Metadata audit: all 50 files readable, at least 200 events each, required
  pre-reclustering/truth provenance present; no inputs skipped.
- Initially prepared as 110. User subsequently requested final cumulative 111:
  all 50 unsubmitted cards/manifests were refreshed in place, with new checksums,
  neutral measurement ON and ExperimentalDelayedNeutral ON. Location ranking
  and passive truth-location scoring remain 110; algorithm/library unchanged.
  The 55 tests, all 50 card/manifest environment/hash comparisons and dry
  submission passed again. No numerical-fit rerun was needed for this
  steering-only change; the Higgs smoke results below used the initial 110.
- Mode HitScan; FreeLossFit=True; threshold 0; final beam/charged/neutral=111;
  ElectronPhoton with the maintained separate-neutral/no-breakpoint routing.
  Missing neutral retains the previous accepted 110 stage.
- SigmaLogLoss=0.001; prior-center bound [0,1]; 180 calls/start; tolerance0.001.
- TruthOverride=False; diffuse/backward filters OFF; verbose dumps OFF in batch.
- Requested group higgs, override SUBMIT_GROUP; 5000 MB/job from normal helper.
- Actual available measurement mask is persisted: missing charged input may
  leave mask1 (beam only), not a falsely labelled successful 110 observation.

Already prepared and checksum-audited 50 card/manifest pairs under
higgs_silicon_scan_20261103/runcards; outlog is inside the output directory.
Preparing again refuses existing cards. To submit these exact cards:

```bash
./subbreakpointhiggsscanjobs.sh submit-prepared
```

That explicit action submits by default (DRY_RUN defaults to 0 for this action).
For submission preview only:

```bash
DRY_RUN=1 ./subbreakpointhiggsscanjobs.sh submit-prepared
```

Plain ./subbreakpointhiggsscanjobs.sh prepares dry by default. Batch verification
requires scan and truth-reference fields when the frozen mode is HitScan, so
an older output schema cannot silently pass. No scheduler test or actual Condor
submission was performed; the worker's Gaudi and verifier were tested locally.

## Verification

- Module built and package-only staged in /tmp first; existing external-header
  and clock-skew warnings remain, no clock/shared-source workaround.
- 55 standard-library Python batch/card tests passed, including a 50-job
  launcher fixture, explicit dry submission, schema rejection and loading truth
  associations without enabling truth-loss override.
- Two freshly built standalone CTests passed, including passive-reference
  independence from ranking and resetting newly booked fields.
- Focused verbose HitScan events 1:11,16,17,38. The reference matches independent
  Truth-location free-loss110 runs in NLL (within1e-7) and IP pT exactly.
- Direct pre-change versus updated HitScan rows 1:11,38 reproduce all previous
  fields except intentionally populated truth ownership diagnostics. Winner,
  scan scores and published endpoints are unchanged.
- A second 1:38 scan deliberately omits tracker truth associations. Reference
  status=-1 and error are saved; all previous blind scan/endpoint fields match.
- Existing Truth-mode 1:11,16,17,38 reproduce all 951 prior branches across both
  trees exactly, NaN-aware. New reference fields remain inactive in Truth mode.
- Higgs smoke: 2 events, 5 track rows, all Gaudi jobs terminated successfully;
  batch output-schema verification passed. One Higgs track has no silicon
  candidates and only beam input; this is explicitly tagged. Reference statuses
  are [2,1,1,2,1], not five claims of truth no-eBrem.

| Seed1 event | Truth reference hit | Reference NLL2 | Reference IP pT [GeV] | Blind selected hit | Reused silicon candidate |
|---|---:|---:|---:|---:|---|
| 11 | -1/null | 38.66661619680595 | 26.685259653764938 | 7 | no |
| 16 | 8 | -32.745863268763536 | 11.317938247708826 | 6 | yes |
| 17 | 9/TPC | -43.832873319492435 | 35.46044335606067 | -1/null | no |
| 38 | 5 | 47.36762314718213 | 13.266871520955387 | 0 | yes |

This is mechanical/regression evidence, not calibrated breakpoint efficiency,
purity or pT-recovery validation. Threshold0 still allows weak false positives;
the earlier six-hard-event table in 2026-11-03-silicon-hit-scan110.md remains the
focused physics evidence (2/6 exact locations). Full 10k batch not run/analyzed.

## Deployment and artifacts

Installed only RecBreakpoint into the project-default
InstallArea/x86_64-el9-gcc11-opt after the user's quiet-window approval. The
installed library is byte-identical to the staged, numerically tested library:

SHA256 75c5edca3a7510e80b99721b3ebd98fd27efa6e05b81dce25324aba168b9b0ae.

Previous installed identity was
911962a6b7e9e91d533018fc699c522a7e3c3d68e50dd58dbbdea774872ec986.
The worker continues to source project setup.sh; no alternate runtime controller
or /tmp dependency is introduced into batch cards. A default-runtime frozen-card
EvtMax0 probe checks package/configuration/schema discovery; not a physics test.

Temporary evidence: /tmp/bp-hit-scan-truth-RFjhVe/ includes build/install logs,
runtime, scan.root/log, missing_truth.root/log, legacy_truth.root/log,
higgs_smoke.root/log, validation.json, audit_prepared.py/log, check.py/log,
installed_probe.py/root/log and prepare/submit-dry logs. Temporary files may
expire. Generated ROOT/cards/logs are not committed.

## Project-memory update

Only Current focus in AGENTS.md was replaced; Introduction/global status,
all project laws, scope and compile/run instructions remain byte-for-byte
unchanged. Complete 22,265-byte outgoing AGENTS preserved in
2026-11-03-agents-before-hit-scan-batch.md. Heading map: section1 retained
verbatim; old section2 preserved in snapshot, replaced by current scan/batch
focus. Effective-ownership guards and the open acceptance-audit prohibition
remain explicit in the new focus. Both substantive headings remain; no history
directory was moved and no manifest migration was necessary. No history file
was deleted. Existing unrelated worktree edits are preserved.
