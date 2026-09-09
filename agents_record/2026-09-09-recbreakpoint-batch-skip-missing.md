# Breakpoint batch preparation skips unavailable inputs

User request: skip missing simulation inputs instead of aborting the campaign.

The preparation helper used by subbreakpointjobs.sh now skips a sample when a
required external predecessor is missing, not a regular file, or zero bytes.
It prints the sample label and missing/empty paths, continues through the seed
range, and reports the total skipped count. No cards or jobs are created for
skipped samples. The same rule covers external tracker inputs for breakpoint
stages without a selected trk stage. Predecessors produced by selected stages
in the same job do not require an existing file.

If no eligible samples remain, preparation returns an error before creating
output directories/cards or submitting jobs. Other validation errors retain
the preflight fail-before-submission behavior. Existing prepared jobs and
worker-time input failures are not changed; this is preparation-time filtering,
not recovery for inputs deleted after submission.

All 17 standard-library tests pass. Added coverage checks missing and zero-byte
inputs interleaved with valid seeds and exactly two mock scheduler submissions;
all-missing campaigns; missing external tracker inputs; and sim generated in
the same job without an existing sim file. No actual Condor jobs were submitted.

Only the breakpoint Python batch helper, its tests, and documentation changed.
User-owned shell edits, physics cards, GSF/shared sources and remote refs remain
untouched. No build is needed for this Python workflow change.
