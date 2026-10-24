# Independent constrained RTS families — 2026-10-24

## Request and contract

User requested beam/charged/neutral constraints as independent branches for
ordinary augmented KF and free-loss: six families, OFF endpoints NaN.
The original ordinary and free-loss fits now always run without optional
observations. Each enabled switch runs one separate ordinary and one separate
free-loss variant using unchanged BreakpointFitter/FreeLossFitter mathematics.
Multiple ON switches do not combine observations.

| Switch | Ordinary prefix | Free-loss prefix |
|---|---|---|
| BeamSpotConstraint | beam_spot_ordinary_ | beam_spot_free_loss_ |
| ChargedEcalMeasurement | charged_ecal_ordinary_ | charged_ecal_free_loss_ |
| NeutralEcalMeasurement | neutral_ecal_ordinary_ | neutral_ecal_free_loss_ |

Each prefix has rts_pt/error, rts_ip_parameters/covariance, rts_calo_*,
likelihood_*, complete smoothed chi2 breakdown and per-hit states. Free variants
also have optimizer_* diagnostics. Six corresponding collections are
BreakpointTracks{BeamSpot,ChargedEcal,NeutralEcal}{,FreeLoss}RTS. The card names
all six explicitly. Collection-name uniqueness now includes all 13 outputs.

- Default switches remain OFF. OFF does not run another fit: endpoint scalars
  NaN, vectors empty, index -1, collection empty, constraint_status=0.
- ON with usable observation: constraint_status=2.
- ON with missing observation or selected breakpoint: corresponding baseline
  copy, constraint_status=1.
- Failed constrained fit/optimization: corresponding unconstrained baseline
  copy, constraint_status=-1 plus error. If the baseline free fit itself is an
  input-KF fallback, the constrained fallback preserves that provenance.
- FreeLossFit OFF: enabled free variant copies its ordinary constrained result.
  Beam also works without a breakpoint; there is then no outer loss optimization.
- New families are RTS-only. Existing three backward switches remain OFF by
  default and control only original unconstrained ordinary/free/truth endpoints.
- Diffuse and truth-prior fits remain unchanged. No baseline fields are replaced.
- Legacy ordinary_beam_spot_* and free_loss_beam_spot_* scalar diagnostics now
  describe the corresponding beam-only variants. The original-family ECAL
  application flags stay zero; legacy free_loss_charged_ecal_* selection fields
  still describe shared observation selection, not application.
- Same shared fitter, beam boundary RTS, native charged transport and native
  scalar measurement updates. No GSF/shared KF source edits.

The FitPairTuple serializer can now omit backward booking for RTS-only families.
Batch validation requires the six new endpoint/status schemas before accepting
flat output. Existing campaign wrapper edits were preserved.

## Local verification

All runs use the build-tree plugin, not installation; no batch jobs submitted.

- 8/8 numerical CTests pass.
- All-OFF rec-barrel-1 events 0,5,11,12,16,17,25,31 exactly match the previous
  all-OFF tuple: 3320 NaN-aware checks (415 prior fields across eight tracks).
  All six new OFF endpoint families have NaN scalars, empty vectors and -1 indices.
- All-ON rec-barrel-2 events 1,148,166: all six families applied successfully on
  all three tracks, with finite endpoints and valid likelihood decomposition.
- On rec-barrel-2 event 1, separate all-OFF/beam-only/charged-only/neutral-only
  runs verify unchanged original endpoints and exact equality of each family
  whether run alone or with all switches enabled: 179 baseline checks in the
  all-ON comparison; 321/323/316 checks in the single-switch comparisons.
- rec-barrel-1 events 0,11,16,17 with FreeLossFit OFF exercise no-interval and
  unavailable-observation copies; all six ordinary/free endpoint pairs agree
  exactly. Beam ordinary pT/errors reproduce the preceding beam-ON implementation.
- Native covariance closure: six beam edges max 1.93e-16; six charged edges
  max 2.02e-16, including PSD/symmetry checks.
- Batch tests: 38/39 pass; only pre-existing wrapper-default mismatch remains.
  Added explicit rejection test for a missing constrained-family flat branch.
- Schema has no duplicate branch names. All retained outputs pass diff hygiene.
- Final rebuilt one-track all-ON repeat passes 602 exact paired checks against
  the preceding all-ON and baseline runs. Its two beam/two charged edges also
  pass covariance closure. The actual batch verifier accepts the final tuple.

Artifact stems (all uncommitted): /tmp/bp-independent-{off,all,nointerval,
off-single,beam-single,charged-single,neutral-single,final}-20261024.{root,log}.
Checks: tests/check_constraint_families.py, check_shared_ecal_outputs.py,
check_ecal_transport_dump.py. Build/test logs use /tmp/bp-independent-*20261024.log.
Inputs: sim_large_barrel_20261018/rec-barrel-{1,2}.root.

Built plugin SHA256:
33b9e6e815c343cb44de59686cd4065355796963521b78a980b01f409fbbb108
Installed plugin unchanged:
72164a985e9fb7fc8df77601991bc4d9735d4450a27e84c68f01595d7e6e2df5

## Remaining limits

Built/tested, NOT installed. No population/endcap validation or resolution
improvement claim. Enabling all three runs up to three additional Minuit searches
per track, besides unconstrained free-loss, and saves more per-hit diagnostics.
Known neutral affine/nonlinear mismatch and possible charged-PFA/tracker
correlations remain. No new backward families or combined-constraint variants.
A baseline fit failure can prevent subsequent variant execution (status 0,
NaN, with the row's baseline failure status); this preserves existing orchestration.

## Preserved outgoing focus


Active work on local `breakpoint` is the shared optional measurement workflow
for ordinary augmented KF/RTS and free-loss Persistent6D. `BeamSpotConstraint`,
`ChargedEcalMeasurement` and `NeutralEcalMeasurement` are independent and default
OFF. Beam now is an explicit native KalTest measurement site. Native
material-aware transport supplies beam-to-first-hit F/Q, honoring MSOn/ElossOn;
RTS smooths back through that edge and publishes the beam state at IP without
a second boundary-material traversal. Charged remains at the ECAL face and
neutral at the virtual post-loss site. Beam works without a selected breakpoint.

Existing ordinary/free-loss endpoints carry enabled constraints; no separate
beam-guided or neutral output family exists. Independently executed diffuse/
truth fits remain unchanged. All three backward defaults remain OFF; optional
backward refilters retain native IP propagation without a second beam update.
Beam likelihood, complete smoothed score and boundary diagnostics count the
measurement and transport once.

Built/tested but NOT installed: eight CTests pass; beam OFF exactly reproduces
411 retained fields on eight prior tracks. Native boundary covariance closure
passes on 16 beam edges and six charged ECAL edges at about 2e-16; MS/Eloss
switch tests confirm native material effects. One pre-existing batch test
disagrees with the user's intentional breakpoint-only wrapper default.

No population/endcap validation is claimed. Beam remains transverse-only with
its measurement variance linearized at the seed direction. The known neutral
affine/nonlinear energy mismatch remains. Next: install when authorized, then
assess categorized performance and cluster calibration/ownership. Full evidence
and preserved outgoing focus:
`agents_record/2026-10-24-native-beam-hit-boundary.md`.
