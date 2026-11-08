# One sequential SIM-to-breakpoint event loop

## Authorization and protected scope

The user requests merging the separated next-batch steps to remove intermediate
ROOT serialization/deserialization and deletion. They explicitly restrict work
to the local DumpGsfTrks `.py.bk` copies, preserving the original cards under
Detector/DetCRD/scripts/. A fresh idle-job question was answered **No jobs are
active** before implementation. No builds/installs, C++ changes, shared physics
changes, batch submission, migration or branch/remote operation are involved.
Original scripts have no working-tree changes. Unrelated dirty cards/helper/tests
and study outputs are preserved. The previous AGENTS is preserved byte-for-byte
in [the snapshot](2026-11-09-agents-before-in-memory-chain.md); global project
instructions are unchanged.

## Implementation

- `DumpGsfTrks/full_chain.py.bk` composes the frozen SIM/TRK/calodigi/REC and
  optional breakpoint sources. It collects their real algorithms/services,
  excludes PodioInput/intermediate PodioOutput, and schedules one final REC
  writer followed by RecBreakpoint. There is one sequential event loop, not Hive.
- Use SIM's full detector geometry, DD4hep mode, and TRK's SimplePIDSvc parameter
  file. Later standalone cards otherwise overwrite these singleton properties.
  Keep all other algorithm/fit settings from the existing cards unchanged.
- TRK, generated pass-through blocks and breakpoint card skip file metadata
  probes only when `_in_memory_chain=True`. RecBreakpoint still checks missing
  pre-reclustering collections during execute; standalone preflight remains.
- New preparations selecting all four detector stages automatically create a
  self-contained `full_chain.py` with embedded frozen sources and a checksum.
  Per-stage cards are retained for inspection. The same generated chain card
  can run locally via gaudirun.py. Partial selections and old manifests remain
  separate-stage. A changed combined card is refused before starting/submitting.
- Worker launches gaudirun once, verifies REC/flat, and records only actual final
  outputs plus `not_written_in_memory` for SIM/TRK/calodigi. It never invokes
  intermediate cleanup for a combined run. Failure leaves no completed marker.
- Final REC retains the prior twelve explicit raw calorimeter drops. Fit output
  collections are created after the REC writer and are not serialized in REC.
- Default submission memory request is8000 MB (formerly5000); explicit overrides
  still apply. The combined smoke process exceeded 5 GiB during initialization. A later
host `/proc` sample recorded VmHWM=6,034,956 KiB (about 5.76 GiB), VmPeak=
6,568,692 KiB, four threads; see `native-memory.txt`. This is an observed
high-water sample, not a 200-event production memory bound.

## Focused verification

Artifacts: `/tmp/breakpoint-in-memory-20261109/` (not committed).

All 65 existing batch-planning/legacy-worker tests, 5 new combined-card/worker tests,
2 compact-truth-card tests and 2 REC-output-policy tests pass (74 total). Python
compilation and git diff --check pass. The maintained truth-card test now
dedents its extracted metadata fragment after the new standalone guard.

The completed two-event eeHX ISR/FSR-off run uses the first two events of
`GeneratorStudies/eeHX_ISRFSROFF_50k_20261106/e1e1h_X.e0.p0.ISRFSROFF.001.stdhep`,
seed 1, Truth selection, default 110 scans, production-electron selection and
verbose dumps. It was prepared locally with DRY_RUN=1 and run with
`bash dump_breakpoint.sh .../native/runcards/higgs-1/job.json`.
The complete worker exited successfully and wrote `completed.json`. Native
REC has two events and is 2,064,573 bytes; the flat tuple has 32 track rows and
two event rows, 3,351,268 bytes. Output verification reports three ordinary
fit successes among the four selected production tracks; the existing event0
electron invalid-provenance warning remains. No SIM/TRK/calodigi ROOT file
exists, and no BreakpointTracks endpoint collection is serialized in REC.
All three intermediate statuses are `not_written_in_memory`. Reports:
`native-output-audit.json`, `native.log`, `native/runcards/higgs-1/completed.json`.
The same frozen breakpoint source, changing only BP_OUTPUT, was rerun
on the saved REC (`reread.py`, `reread.log`, `reread-flat.root`) and exited
successfully. All **953 breakpoint branches across 32 rows** and all **114
event branches across two rows** compare EXACTLY with the in-memory result.
No rounding tolerance, dropped field or status normalization was needed.
This includes measurement/cluster data, loss fits, NLL/location scans, means,
covariances, truth records and the existing invalid-track status. Report:
`in-memory-comparison.json`. This establishes focused in-memory/file-fit
equivalence, not identity with a separately simulated detector sample. Existing baseline Hcal tile-map
fallback/error messages are unchanged, not newly introduced or repaired.

The standalone events 0,11,16,17 regression on
`/tmp/truth-recorder-20261109/compact-rec.root` completed successfully. All 953
breakpoint branches across 89 rows and all 114 event branches across four rows
compare EXACTLY with `/tmp/rec-calo-pruning-20261109/focused-control-flat.root`
(including NaN/status locations and all covariances). No t/X0 tolerance is
needed. Report: `focused-comparison.json`. This verifies that the new metadata
guards preserve the old file-based card path. The five new full-chain tests
and both output-policy tests also pass against a selectively staged HEAD-based
helper, independent of unrelated dirty Higgs/cleanup changes.

## Limits

No measured campaign speedup or population physics validation yet. One process
shares random streams and initialization; identical seeds need not yield the
same detector sample as separate executables. A fixed-REC/in-memory comparison
isolates serialization/fit equivalence from that question. Peak memory and
startup differ; do not submit full-chain jobs with the previous memory request
without checking it. No raw calorimeter deposit file survives the default full
chain; raw-shower/re-digitization studies need a separate preserved simulation.
No existing production files or prepared jobs are rewritten.

Core implementation checkpoint: **dc7b097**. Only this task's hunks in the
batch helper, test fixture and submission wrapper were staged; unrelated
Higgs/expected-event/SIM-cleanup campaign edits remain in the worktree.
