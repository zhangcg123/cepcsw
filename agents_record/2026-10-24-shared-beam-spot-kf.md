# Shared beam-spot KF measurement — 2026-10-24

## Request and implementation

Move the beam observation into the ordinary augmented KF/RTS core, shared
with free-loss, and remove the separate beam-guided free-loss approach.

- New `BeamSpotConstraint=false`, maintained-card environment
  `BP_BEAM_SPOT_CONSTRAINT=0`. BeamSpotX/Y remain zero; widths remain
  0.0145 mm and 3.6e-5 mm. ChargedEcalMeasurement and NeutralEcalMeasurement
  remain independent, default OFF. All three remaining backward switches
  remain OFF. The beam works without a selected breakpoint and with free-loss OFF.
- Before the first real tracker update, express drho at the configured beam
  pivot as a scalar measurement of the native first-hit seed. Native
  THelicalTrack::MoveTo provides drho and its full five-coordinate Jacobian;
  native TVKalSite::Filter updates the full seed state/covariance. The first
  tracker hit is then used exactly once. Forward/breakpoint filtering and RTS
  consume this conditioned seed.
- This is a geometric, material-free transverse boundary observation, not
  detector-material transport from the beam or a z constraint. Its variance
  is cos(phi0_beam)^2 sigmaX^2 + sin(phi0_beam)^2 sigmaY^2, frozen at the seed.
  No new beam measurement is applied after RTS or in the optional inward refilter.
- The existing energy-only scalar KalTest site is reused/generalized to five
  or six dimensions as ScalarMeasurementSite. Charged and neutral updates
  retain their numerical behavior and curvature-sign rejection.
- Likelihood uses p(beam,hits,ECAL)=p(beam|original seed)*p(hits,ECAL|beam).
  The existing conditional model uses the beam-conditioned seed. Complete
  smoothed chi2 replaces its conditioned seed term by the original seed
  term plus the affine beam measurement residual. The latter is included
  in the first hit's measurement contribution. Normalization adds the initial
  beam innovation determinant/dimension once. No separate post-RTS penalty.
- Ordinary and every free-loss trial/final fit receive the same beam option.
  Independently executed diffuse/truth-centered fits do not; their existing
  disabled/no-interval copy behavior can copy an ordinary constrained result.
- Removed BeamSpotLikelihood and the separate beam-guided optimizer,
  BeamGuidedFreeLossBackwardFilter, its track/status/index collections,
  beam_guided_free_loss_* tuple family and old external beam penalty fields.
  Old environment controls fail explicitly. Regenerate old prepared cards.
- New beam configuration and ordinary_beam_spot_* / free_loss_beam_spot_*
  fields record application, predicted/filtered/smoothed drho, smoothed affine
  drho, measurement/innovation variance, innovation and smoothed chi2.
  Verbose dumps include the full predicted/filtered/smoothed seed matrices.
- Only the beam-controller lines of subbreakpointjobs.sh were migrated;
  the user's seed bounds, input/output paths and breakpoint-only campaign
  settings are preserved and excluded from this checkpoint.

## Validation (build tree only; no installation or Condor)

Eight CTests pass, including a new native beam Jacobian/update test and
joint-Gaussian versus seed-conditioning likelihood check. The initial test
caught ROOT's need to pre-size matrices before assignment; that was fixed
before the successful runs. Batch/card tests: 38/39 pass. The one pre-existing
failure expects full-chain submission, while the user's local wrapper
intentionally defaults to breakpoint-only. It is unchanged.

Inputs: sim_large_barrel_20261018/rec-barrel-{1,2}.root. Normal settings:
Persistent6D, SigmaLogLoss=0.001, all backward filters OFF.
Local artifacts are /tmp/bp-beam-kf-<mode>-20261024.root and matching logs.

- off: file 1, events 0,5,11,12,16,17,25,31. All 388 retained old fields match
  /tmp/bp-shared-off-20261024.root exactly (3,104 NaN-aware comparisons).
  Explicitly removed beam fields are absent.
- ecaloff: beam OFF, both ECAL ON; file 2 events 1,148,166. All 354 retained
  non-error free-loss field comparisons are exact versus
  /tmp/bp-shared-both-20261024.root. The only remaining differences are
  trial-error strings renamed from "Energy update crosses zero curvature"
  to "Scalar update crosses zero curvature"; rejection positions are identical.
- on: beam ON, ECAL OFF; file 1 events 0,5,11,16,17. Ordinary and free-loss
  beam flags true on all five tracks; ordinary/full likelihood scores valid.
  Empty-breakpoint tracks 0/11/17 correctly copy ordinary to free-loss.
  check_score_tuple.py passes: 15 valid scores, 8 copies, no unavailable scores.
- ordinary: same five tracks/beam ON, FreeLossFit=0. 255 ordinary endpoint,
  persistent-state, score and beam diagnostic fields are exactly unchanged
  versus the optimizer-enabled run. Free-loss endpoints copy ordinary.
- independent: file 1 event 5, beam ON with diffuse/truth fits enabled.
  All 98 diffuse/truth fields exactly match the beam-OFF run.
- all: beam and both ECAL ON; file 2 events 1,148,166. All three observations
  apply in ordinary and free-loss; free-loss status 2 on all three.
  Six native ECAL covariance closures pass, max normalized error 2.62e-16.
  Ordinary/free likelihood quadratic and complete RTS score agree within
  1e-7 relative/1e-6 absolute; likelihood normalization checks pass.
- Across the eight distinct beam-ON tracks, affine versus nonlinear smoothed
  beam drho differs by at most 4.60e-8 mm. This is focused numerical evidence,
  not a general linearization or performance guarantee.
- Focused event indices 11/16/17 were rerun but are not all hard-loss tracks
  in this input. File 2's 1/148/166 provide the explicit hard-loss/ECAL tests.

Test/build logs: /tmp/bp-beam-kf-{ctest,batch-tests,build-final}-20261024.log.
Regression helpers: tests/check_shared_ecal_outputs.py --retired-beam,
tests/check_score_tuple.py, tests/check_ecal_transport_dump.py.
The exact ECAL comparison allows only the documented error-text rename.

Built plugin SHA256:
ffdcf337e4984f7a1e87ddb1675f40d4d6e7975921dd592f7bcd4396fb3f6c33.
Installed plugin remains:
72164a985e9fb7fc8df77601991bc4d9735d4450a27e84c68f01595d7e6e2df5.
The new card needs the new plugin/configurable database installed together.

## Limits and next step

No population resolution claim, no endcap validation, and no new batch.
The known neutral affine/nonlinear energy mismatch remains. Beam transport
omits material and uses one frozen transverse linearization; assess these
limits before precision/default-on use. Ordinary/free endpoints are changed
when BeamSpotConstraint is ON; there is no simultaneous separate beam-guided
family anymore. Old tuples remain readable with their old schema.

## Outgoing current focus, preserved verbatim

Active work on local `breakpoint` is shared optional ECAL energy measurements
in ordinary augmented KF/RTS and free-loss Persistent6D. The common controls
are now `ChargedEcalMeasurement` and `NeutralEcalMeasurement`, both default OFF;
they work for ordinary fitting even with `FreeLossFit=false`. Regenerate cards
using `BP_CHARGED_ECAL_MEASUREMENT` / `BP_NEUTRAL_ECAL_MEASUREMENT`; old free-loss
names are rejected. Existing ordinary/free-loss endpoints carry the constrained
results, without a new output family. Independently executed beam-guided,
diffuse and truth-centered fits remain unchanged. Backward defaults remain OFF.

Both observations use native KalTest transport/update machinery. Charged is at
the ECAL face; neutral is at the post-loss virtual site. Both currently require
a selected breakpoint. Ordinary status/error branches distinguish application,
skips and failed retries. One shared likelihood routine counts each observation
once. The retired cluster-prior/penalty approaches remain retired.

Built/tested but NOT installed: seven CTests pass, all 515 legacy OFF branches
match on eight rows, free-loss ON results are exact on three hard-loss tracks
in all switch combinations, and ordinary fitting works with free-loss disabled.
Native covariance closure passes. One unrelated batch test disagrees with the
user-modified submission-wrapper default and is left unchanged.
This does not fix or validate the known neutral affine/nonlinear energy mismatch.
Next: resolve that limitation and assess cluster ownership, calibration and
held-out performance before default-on steering. Full evidence, install hashes
and outgoing focus: `agents_record/2026-10-24-shared-ordinary-free-ecal-controls.md`.
