# Shared ECAL measurements for ordinary augmented KF and free-loss

## Request and implementation

User requested that ordinary augmented KF/RTS and free-loss use the same two
optional ECAL constraints, controlled by the same run-card options. No change
to the loss mapping, KalTest transport, energy derivatives, scalar update,
RTS algebra, optimizer or cluster selection was requested or made.

- Renamed the two Gaudi properties to `ChargedEcalMeasurement` and
  `NeutralEcalMeasurement`, both default false. Environment names are
  `BP_CHARGED_ECAL_MEASUREMENT` and `BP_NEUTRAL_ECAL_MEASUREMENT`.
  The old free-loss-prefixed environment names are explicitly rejected in
  the maintained card and batch helper. Old Gaudi property names are removed.
  Regenerate old prepared cards after installing this update.
- Select observations once per input track, then pass them to independent
  copies of ordinary and free-loss settings. Ordinary fitting now accepts
  both switches even with `FreeLossFit=false`. No separate output family.
- As with the prior free-loss workflow, a selected breakpoint is required.
  The no-breakpoint 5D path is unchanged, including charged-ECAL behavior.
  No available neutral skips neutral without disabling charged.
- Ordinary fits retry without neutral on failure, then without charged if
  that also fails. If the ordinary unconstrained fit fails, retain the existing
  failed-track handling; do not claim a successful constrained fit.
  Free-loss keeps its original independent retry/optimization/KF-fallback path.
- Add `ordinary_charged_ecal_measurement_status`,
  `ordinary_neutral_ecal_measurement_status`, `ordinary_ecal_measurement_error`.
  Status: 0 off; 1 unavailable/no breakpoint; 2 applied; -1 removed on retry.
  Legacy `free_loss_charged_ecal_*` observation fields retain their names and
  now also describe the observation supplied to ordinary fitting. Their
  selection status is NOT an ordinary-fit success flag.
- The independent beam-guided, diffuse and truth-centered fits still receive
  the original unconstrained settings. Existing disabled-family copies may
  naturally copy the constrained ordinary/free-loss result.
- Extract existing free-loss score construction into `evaluateFitLikelihood`,
  shared with ordinary likelihood publication. Neutral is already captured
  in the tracker Gaussian model; charged adds its conditional innovation
  likelihood once. No new objective or double measurement was introduced.
- Verbose charged/neutral state/covariance dumps cover both ordinary and
  free-loss, labeled with `family=`. Backward defaults remain false; no new
  inward energy updates were introduced.

## Validation and exact artifacts

Built via the normal LCG105 build tree, not installed. Seven package CTests
pass. Batch/card suite: 38/39 pass. The existing failure is
`test_submission_wrapper_full_chain_default_and_rec_override`: the user's
locally changed submission wrapper defaults to existing rec inputs, whereas
the test expects the full chain. The wrapper and unrelated working-tree
changes were not modified. The shared-control test passes with free-loss
disabled, freezes both controls, and rejects retired environment names.

Local runs used `sim_large_barrel_20261018/rec-barrel-{1,2}.root`, no Condor.
All normal tests use SigmaLogLoss=0.001, Persistent6D and backward OFF.

- OFF: file 1 events 0,5,11,12,16,17,25,31, with beam/diffuse/truth enabled.
  `/tmp/bp-shared-off-20261024.root` matches all 515 old branches of
  `/tmp/bp-native-off-all-20261023.root` exactly: 4,120 NaN-aware comparisons.
  Only three new ordinary status/error branches are added (518 total).
- Charged, neutral, both: file 2 events 1,148,166, beam/diffuse/truth disabled.
  `/tmp/bp-shared-{charged,neutral,both}-20261024.root` retains all 127
  `free_loss_*` fields exactly against the corresponding native validation
  files from October 23: 381 comparisons in each mode, 1,143 total.
  Ordinary successfully applies every requested available measurement.
- `/tmp/bp-shared-ordinary-20261024.root`: same three tracks, both ECAL ON,
  FreeLossFit=0. Ordinary pT, error, full persistent smoothed states/covariances,
  ECAL state/covariance and likelihood exactly equal the ordinary results
  with FreeLossFit=1. Disabled free-loss outputs copy ordinary as specified.
- 18 charged edges across charged/both/ordinary verbose logs pass the existing
  native transport checker, including positive-semidefinite Q and covariance
  closure. Maximum normalized closure residual: 2.02e-16.
- Ordinary complete RTS quadratic agrees with the separately factored full
  likelihood quadratic within 1e-8 relative. Largest absolute difference is
  3.60e-6 on a quadratic of about 926 (tight 1e-6 absolute alone was too strict).
- `/tmp/bp-shared-controls-20261024.root`: file 1 events 0,5,11,16,17, both ON.
  0/11/17 have no selected breakpoint and skip both. 16 has no collected
  neutral and applies charged only. 5 applies both. These numeric 11/16/17
  identities are not all hard-loss events in this input file.
- Deliberately extreme `MeanLogLoss=4`, file 2 event 1:
  `/tmp/bp-shared-retry-20261024.root` records both retry errors/status -1;
  the unconstrained fit also fails, leaving a failed row rather than a false
  success. Additional b0=1 checks on 1/148/166 finish without retry. Neither
  stress setting is a proposed physics default.

Regression helper: `tests/check_shared_ecal_outputs.py BEFORE AFTER` checks
all old fields; `--free-only` isolates free-loss fields for ON comparisons.
Logs share each ROOT file stem. Build/batch/CTest logs are
`/tmp/bp-shared-ecal-{build,batch-tests,ctest}-20261024.log`.

## Deployment and limits

Built plugin SHA256:
`95215a5a701e8139cbf239076364ce04086e190980a8d3edeb4a9daf30057d1f`.
Installed plugin is unchanged:
`72164a985e9fb7fc8df77601991bc4d9735d4450a27e84c68f01595d7e6e2df5`.
Do not submit the new card against the old installed configurable database.
The new module and generated configuration database must be installed together.

This is steering/regression validation, not physics validation. Ordinary b0=0
with sigma=0.001 remains a tight prior and does not reproduce optimized free-loss.
The known neutral affine/nonlinear energy mismatch remains unchanged, including
the bad free-loss file 2 event 166. No iteration, recalibration or revived
cluster-prior approach was implemented.

## Outgoing current focus, preserved verbatim

Active work on local `breakpoint` is consistency of the optional charged and
neutral ECAL energy measurements in the existing free-loss Persistent6D result.
Both remain default OFF and steer `BreakpointTracksFreeLossRTS` / `free_loss_rts_*`;
no separate neutral output family exists. Charged transport now obtains its
state/F/Q directly from native KalTest detector transport, not finite differences
or covariance subtraction. Both energy updates use native `TVKalSite::Filter()`
with full 6D correlations and reject curvature-sign crossings. Charged stays at
the ECAL face; neutral stays at the virtual post-loss site. No-neutral/no-loss
skips neutral; failed neutral retries without it, retaining charged if selected.
All backward defaults remain OFF; retired neutral-prior/penalty families stay retired.

The update is built/tested but NOT installed. Seven CTests pass; both-OFF matches
all 515 branches on eight prior rows exactly. Focused native ECAL covariance
closure passes, with no previous invalid-process-penalty failures in the tested
ON fits. This is not physics validation: the neutral affine prediction can still
agree with the measurement while the nonlinear final-state energy disagrees badly.
Next: discuss/fix that remaining linearization limit, then validate cluster
ownership, calibration/independence and held-out barrel/endcap performance before
default-on steering. The three review points, exact tests, deployment hashes and
complete outgoing focus are preserved in
`agents_record/2026-10-23-native-ecal-kaltest-machinery.md`.
