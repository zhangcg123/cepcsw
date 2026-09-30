# Default-on complete RTS chi2 breakdown for every breakpoint fit pair

User request: save the measurement/process/seed breakdown from the paired
diagnostic tables by default, without separate replay fits.

## Implementation

Ordinary RTS already saved the three contributions and a separate native
measurement check. RecBreakpoint now writes the same four branches for the
free-loss and truth-centred RTS results, always, without a new property:

- `{prefix}smoothed_measurement_chi2`: per-hit affine measurement contribution.
- `{prefix}smoothed_process_chi2`: incoming process contribution, zero at hit 0.
- `{prefix}smoothed_seed_chi2`: scalar seed contribution, included once at hit 0.
- `{prefix}smoothed_native_measurement_chi2`: per-hit nonlinear/native hit
  projection check, NOT an extra contribution to the complete score.

Prefixes: none for ordinary, `free_loss_`, `truth_override_`. All vectors use
the existing outward hit order and metadata. On valid status,
measurement[i]+process[i]+(i==0 ? seed : 0) equals smoothed_local_chi2[i].
Summing it equals smoothed_chi2. The fitter, likelihood, minimizer, covariance,
and endpoint publication are unchanged. No new backward or CompleteTracks
complete-score definition is invented; their existing scores remain intact.

Off/empty copies inherit the ordinary breakdown exactly. Unavailable extra
fits and KF fallback have empty vectors and NaN seed. Existing status/error
fields remain authoritative. Failed score evaluation may retain partial
diagnostics: require smoothed_chi2_status==1 for complete-score use.

Changed only RecBreakpoint serialization and documentation, plus a reusable
ROOT integration checker tests/check_score_tuple.py. GSF/shared KF/cards and
user-owned changes were not edited. No installed runtime or remote changes.

## Validation and recovery from interrupted test launch

The interrupted launch incorrectly preloaded the whole plugin through
LD_PRELOAD. Its logs show ROOT GUI initialization segmentation-violation
loops, so those attempts are NOT validation evidence. Tests were restarted
using normal Gaudi plugin loading through a one-library LD_LIBRARY_PATH
overlay. Every fit has an external 240-second timeout and 5-second kill grace.
Installed and rebuilt libraries were selected separately; installation was
not required to test the rebuilt module.

Successful direct A/B gate: 11 event executions per version, 22 total, seven
unique seed:event pairs. Both versions use the same maintained card, same
positive SigmaLogLoss=.001 and verbose dumps. Normal configured fits:
12:11,12:16,12:17,24:10,21:39. Events21:5,21:6 repeated with off, empty
Manual intervals, and Persistent6D/unsupported-free-loss fallback settings.
12:16 has no selected interval and copies both extra pairs. 12:17 remains a
secondary-activity mechanical control, not single-track physics evidence.

All 12 bounded Gaudi jobs exited0. Across11 paired rows, all1,793 pre-existing
field comparisons are exactly equal (NaN-aware), including all six endpoints,
optimizer trials, existing score lists and saved states/covariances. All29,195
verbose state/covariance records match exactly. New fields pass31 valid-score
sum/length gates, ten exact ordinary-copy checks, and two KF-unavailable
empty-vector/NaN checks. These counts include reused events/modes and are not
independent physics populations. Two numerical CTests and28 card/batch tests
also pass. This change is serialization-only, not a performance improvement.

Evidence (uncommitted generated outputs):
TrackingPerformanceStudies/breakpoint_score_persistence_20261001/
with bounded/{before,after}_*.root and .log, per-launch jobs JSON, validation.json,
validation.log, build.log, batch_tests.log and numerical_tests.log.
Old failed-launch logs are retained separately outside bounded/.

Installed baseline SHA256:
8689b40e378ce8886e7e0784de983a17c0d943be2b3d014939106d0a73482b0d

Validated rebuilt module SHA256:
c8920c0b5ba7339b5e8e5494f2b097f3d91681811841caa46e49c05ed04dfe53

Deployment pending: installed baseline remains unchanged. New branches become
available to normal jobs after installing this verified RecBreakpoint module;
no run-card change is needed. Existing ROOT files are not modified/backfilled.
