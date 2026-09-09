# Remove intermediate tracker outputs despite bad fit rows

The user explicitly requested that bad fits no longer retain intermediate
tracker tuples once breakpoint jobs finish. This supersedes the all-rows-valid
cleanup rule in the 2026-09-09 tracker-cleanup record.

The worker still requires successful subprocess exit and a readable,
non-recovered, nonempty flat ROOT tree with the required branches. Per-track
ordinary/oracle failures are reported but no longer block cleanup, even when
there are zero successful ordinary fits. Failure statuses remain in the flat
tuple. Crashed jobs and invalid output files still retain tracker inputs.

Only a tracker output produced by this same job is removed. External tracker
inputs and trk-only outputs remain protected. Exact path, regular-file,
non-symlink and unchanged production-identity checks remain. No shared KF/GSF
source or run-card settings changed; no remote operations or new submissions.

All 19 batch helper tests pass, including all-failed, partially failed and
invalid-oracle rows accepted for file cleanup; corrupt/recovered/empty or
missing-branch outputs still rejected; and failed subprocesses retaining
intermediate outputs. Real existing flat tuples for seeds 1,2,3,4,5,6,7,10 in
breakpoint_barrel also passed the updated ROOT verification.
