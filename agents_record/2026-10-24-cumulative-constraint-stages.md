# Cumulative optional constraints — 2026-10-24

## Superseding clarification

The user requires inheritance/accumulation, not isolated beam/charged/neutral
fits when multiple switches are enabled. This supersedes the dispatch contract
of a014f5a and agents_record/2026-10-24-independent-constraint-families.md.

Six output families and three existing switches remain. For each of ordinary
and free-loss, enabled output stages accumulate in this order:

    beam -> charged -> neutral

All enabled: beam stage uses beam; charged stage uses beam+charged; neutral
stage uses beam+charged+neutral. OFF stages stay NaN/empty, index -1, and
contribute no measurement. Original ordinary/free-loss endpoints are still
unconstrained; truth/diffuse and existing backward outputs are unchanged.

This is saved-stage ordering, not a change in physical measurement placement:
beam at the beam site, neutral at the selected breakpoint, charged at ECAL.
Each successful stage invokes the existing fitter from the original seed and
Gaussian loss prior with the accumulated observation set. It does NOT reuse
an earlier smoothed trajectory/covariance as a new prior, or replace the
configured ordinary loss center by the previous optimized center. Every
free-loss stage independently optimizes its own accumulated-observation
likelihood, retaining SigmaLogLoss. No KF mathematics was changed.

Ordinary and free-loss maintain their own accepted measurement sets. Missing
new observations copy the preceding accepted result (status 1); failed
additions also preserve it (status -1 with error), rather than dropping earlier
constraints. Successful additions have status 2. A baseline failure can still
prevent stages being reached, as before. Disabled families have status 0.
FreeLossFit OFF uses constrained ordinary copies as before.

Each family now records:
- constraint_requested_mask: enabled observations through this stage;
- constraint_mask: observations actually present in its saved fitted result.

Bits: beam=1, charged=2, neutral=4. Successful all-ON masks are 1,3,7.
No available neutral after successful beam+charged yields requested=7,
applied=3, status=1, and the preceding beam+charged endpoint exactly.
Batch output validation requires both masks.

## Verification

Local build-tree runs only; no installation or batch submission.

- Eight numerical CTests pass.
- All-OFF rec-barrel-1 events 0,5,11,12,16,17,25,31 reproduce all 826
  preceding fields exactly: 6608 NaN-aware comparisons.
- All-ON rec-barrel-2 events 1,148,166: both methods have applied masks 1,3,7.
  Final neutral-stage IP pT/error and ECAL pT/error/state/covariance exactly
  reproduce the previous all-three combined implementation saved in
  /tmp/bp-beam-hit-all-final-20261024.root.
- On event 1, beam+charged fields exactly equal the first two stages with all
  three enabled (294 field checks). Beam-OFF charged+neutral passes masks,
  disabled-stage NaNs, and unchanged baseline checks (179 fields).
- rec-barrel-1 events 0,11,16,17, FreeLossFit OFF: absent observations preserve
  preceding constraints. This covers no selected breakpoint and missing neutral.
- Every valid likelihood has the baseline measurement dimension plus one for
  each applied scalar constraint; normalized NLL decomposition passes.
- Native covariance closure: 18 beam edges, maximum 1.93e-16; 12 charged
  edges, maximum 3.82e-16; symmetry/PSD checks pass.
- Actual batch verifier accepts the three-track cumulative tuple.
- Batch tests: 38/39 pass. The same pre-existing test expects a full-chain
  default while the user's wrapper intentionally selects breakpoint-only.
- No injected constrained-fit failure test, population or endcap validation.

Artifacts /tmp/bp-cumulative-{all,off,nointerval,beam_charged,charged_neutral}-20261024.{root,log}.
Build/test logs /tmp/bp-cumulative-{build,ctest,batch-tests}-20261024.log.
Input directory sim_large_barrel_20261018, rec-barrel-{1,2}.root.
Checkers: check_constraint_families.py, check_shared_ecal_outputs.py,
check_ecal_transport_dump.py. Generated outputs are uncommitted.

Built SHA256:
072f2b0173e8140b5aeca79f97b30a02e4db650a6cb139d39ecb565dd341fde5
Installed unchanged:
72164a985e9fb7fc8df77601991bc4d9735d4450a27e84c68f01595d7e6e2df5

Built/tested, NOT installed. Shared KalTest/MarlinTrk/GSF sources unchanged.
Constraint defaults remain OFF. Up to three extra searches per track if all
stages are enabled. Neutral linearization and charged-cluster independence
limitations remain; no resolution improvement claim.

## Preserved outgoing focus


Active work on local `breakpoint` is independent optional-constraint RTS
publication. Original ordinary augmented KF and free-loss endpoints always
remain unconstrained. `BeamSpotConstraint`, `ChargedEcalMeasurement` and
`NeutralEcalMeasurement` each enable TWO separate RTS families (ordinary and
free-loss); all default OFF. Multiple ON switches run independent variants,
not a combined fit. OFF outputs are NaN/empty with index -1. Missing/failed
observations copy the corresponding baseline with an explicit status/error.
FreeLossFit OFF copies the constrained ordinary endpoint into its free variant.

The six families reuse the existing fitters. Beam uses an explicit native
measurement node and material-aware boundary F/Q, with RTS back to the beam.
Charged stays at ECAL, neutral at the post-loss site. Original diffuse/truth
fits remain unchanged. The three original backward defaults remain OFF;
no extra backward families were introduced. The card explicitly names the
six collections; the batch validator requires their flat branches.

Built/tested but NOT installed: eight CTests pass, eight-track all-OFF outputs
reproduce 3320 prior field values exactly, and all six variants run on three
hard-loss tracks. Single-switch versus all-switches tests establish exact
family independence and unchanged baseline endpoints. Native covariance closure
passes; no-interval and FreeLossFit-OFF copies are checked. One pre-existing
batch test still disagrees with the user's breakpoint-only wrapper default.

No population/endcap performance validation claimed. Neutral linearization and
charged-cluster independence/calibration questions remain. Enabling all switches
adds up to three Minuit searches per track. Next: install when authorized and
compare categorized performance. Evidence and complete outgoing focus:
`agents_record/2026-10-24-independent-constraint-families.md`.
