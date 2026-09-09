# Breakpoint worker: remove verified job-owned tracker intermediates

User authorized removing tracker outputs once downstream jobs finish. The
dump_breakpoint.sh worker delegates execution to options/batch_breakpoint.py;
cleanup is implemented there without editing the user's modified shell scripts
or any existing GSF workflow/source/card.

After every selected stage has exited successfully and outputs have passed ROOT
verification, delete trk-SAMPLE.root only when BOTH trk and breakpoint ran in
the same job. All flat rows must have ordinary status1 and extra-pair result
status1 or2. Partial/failed refits or invalid/absent oracles retain the tracker.
Process/output verification failures do not reach cleanup. Tracker-only jobs
keep their output; breakpoint-only jobs never delete their external input.
Simulation files always remain untouched.

Deletion resolves the exact expected output-directory/sample filename, refuses
symlinks/nonregular or unexpected paths, and compares device/inode/size/mtime
against the identity captured immediately after tracker production/verification.
Changed files are retained with an explicit error. No broad globs or recursive
deletion. completed.json records tracker_cleanup status and the removed path.
Removal is permanent, not trash; rerun the retained simulation/frozen tracker
card to regenerate the intermediate. No retrospective campaign cleanup occurs.

Verification:

- Thirteen standard-library batch tests pass, including owned-success deletion,
  external/trk-only/incomplete retention, changed-file/symlink refusal, and a
  downstream process failure preserving tracker output without calling cleanup.
- Actual local one-event worker run, sim_large_barrel_20260823 seed12, stages
  trk,breakpoint, Truth interval selection, LocalMarginal, TruthOverride off.
  One valid ordinary flat row and copied extra pair. Worker exited0, verified
  the output, deleted its own trk-e--2.0-85-12.root and wrote completed.json.
- Surviving artifact:
  TrackingPerformanceStudies/breakpoint_cleanup_smoke_20260909/breakpoint_flat-e--2.0-85-12.root.
  Its runcards/job.json and completed.json record the exact inputs/cleanup.
  Original simulation input still exists. No other real tracker tuple deleted.
- No Condor job submission or C++ changes/rebuild were needed. Existing batch
  shell edits remain untouched and uncommitted in this checkpoint.

This supersedes the retain-all-intermediates rule in the initial independent
batch record, while preserving external-input and simulation protection.
