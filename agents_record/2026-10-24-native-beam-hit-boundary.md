# Native beam-hit boundary — 2026-10-24

## Request and implementation

Replace the geometric first-hit seed constraint with an explicit beam measurement
site using the same native transport/filter/RTS machinery as tracker hits.
No shared KalTest, MarlinTrk or GSF code changed. Existing independent controls
remain default OFF: BeamSpotConstraint, ChargedEcalMeasurement,
NeutralEcalMeasurement. No additional result family or controller was added.

1. Re-express the existing loose prefit at the configured beam pivot. This is
   seed initialization, not a physical inward material traversal.
2. Apply a native scalar KalTest measurement: drho=0, H=(1,0,0,0,0), and
   R=cos(phi0)^2 sigmaX^2 + sin(phi0)^2 sigmaY^2, evaluated at the seed direction.
3. Attach the filtered beam state to the existing IP vacuum navigation layer.
   The dummy navigation hit is not filtered and adds no second measurement.
   Native TKalTrack::AddAndFilter transports to the first real tracker hit and
   updates it once; retain the native propagation matrix F and process noise Q.
4. Continue the existing Persistent6D filter and RTS. Extend RTS across the
   retained beam-to-first-hit edge and publish its smoothed beam state at IP.
   An offset beam pivot is geometrically re-expressed at the origin; do not
   traverse tracker-to-IP material again.
5. Preserve tracker hit indices and existing endpoint families. Beam is a
   separate boundary node, not an added CompleteTracks EDM hit. No new
   bremsstrahlung breakpoint is introduced there.

Native transport honors MSOn and ElossOn. It uses the same detector material
navigation as tracker propagation. Optional backward refilters remain default
OFF and keep their existing native IP propagation without a second beam update.
Independently executed diffuse/truth fits remain unchanged; their disabled-family
copy semantics remain unchanged.

## Boundary smoothing and scoring

With P_B the beam-filtered covariance, P_1 the first-hit predicted covariance,
and P_1s its smoothed covariance:

    G = P_B F^T inverse(P_1)
    x_Bs = x_B + G (x_1s - x_1)
    P_Bs = (I-GF) P_B (I-GF)^T + G (Q + P_1s) G^T

The complete smoothed score replaces the marginalized first-hit seed term with
the original beam seed, beam measurement and boundary process terms. For
u=inverse(P_1)(x_1s-x_1), the boundary process contribution is u^T Q u.
The initial beam innovation enters the forward likelihood once. The
factorization is p(beam,hits,ECAL)=p(beam|seed)*p(hits,ECAL|beam).
Existing tracker-indexed score lists carry boundary measurement/process terms
in their first entry; explicit beam fields retain the separate diagnostics.

New ordinary/free beam fields include boundary_process_chi2 and
boundary_covariance_closure. Verbose BeamBoundaryKF output includes beam
filtered state/covariance, first-hit predicted and filtered state/covariance,
F and Q. check_ecal_transport_dump.py --beam audits the native edge.

## Verification (local only; no installation or batches)

- Eight CTests pass. BeamSpotTest includes native scalar update, nonidentity F
  and nonzero Q, independent joint-Gaussian likelihood/RTS checks and complete
  seed+measurement+process chi-square closure.
- Beam OFF exactly reproduces 411 retained fields on eight prior tracks
  (3288 NaN-aware comparisons); four new boundary fields are additional.
- Beam ON: input rec-barrel-1, events 0,5,11,16,17; all successful.
  Three-constraint ON: rec-barrel-2, events 1,148,166; all successful.
  These inputs are under sim_large_barrel_20261018/.
- Ordinary-only beam ON works with FreeLossFit disabled; independently run
  diffuse/truth outputs unchanged on the checked selected-breakpoint event.
- 16 beam-boundary covariance checks: max relative closure 1.93e-16.
  Six charged ECAL checks: 1.94e-16. Symmetry/PSD checks pass.
- Native material switch checks on event 0: MSOff makes all 25 Q entries zero;
  MSOn has nonzero Q. ElossOn changes boundary kappa from
  -0.023151995565996095 to -0.02315232582555979; ElossOff leaves it unchanged.
- Nonzero beam position x=0.1mm,y=-0.02mm executes successfully.
- Published IP drho agrees with the smoothed beam diagnostic on eight tracks.
- Complete smoothed score versus independent conditional Gaussian quadratic
  plus innovations: maximum absolute difference 6.34e-7 on the three ECAL
  tracks (largest total about 1.8e4). NLL decomposition also checked.
- Batch helper tests: 38/39 pass; the pre-existing remaining test expects a
  full-chain default whereas the user's wrapper deliberately selects breakpoint.
  The wrapper was not changed.

Artifacts: /tmp/bp-beam-hit-{off,on,on-final,all,all-final,ordinary,noMS,dEdx,offset}-20261024
with .root/.log outputs; /tmp/bp-beam-hit-{ctest,batch-tests,final-build}-20261024.log.
The focused material-control card is /tmp/bp-beam-hit-material-controls.py.
These generated artifacts are not committed.

Built plugin SHA256:
5e8eb46ad421bb7ad860f5c7f426303e40365f07105cd5307e14f229ce954f6c
Installed plugin remains:
72164a985e9fb7fc8df77601991bc4d9735d4450a27e84c68f01595d7e6e2df5

## Limits and next steps

Built/tested, NOT installed. No population/endcap validation or performance
improvement claim. Beam is transverse-only, with the projected width frozen
at its seed direction. Known neutral affine/nonlinear energy mismatch remains.
No extra beam update was added to optional backward refilters. Shared native
transport retains its existing detector-surface/material conventions.
Install only when authorized; then assess held-out categorized performance.

## Preserved outgoing focus


Active work on local `breakpoint` is the common optional measurement workflow
for ordinary augmented KF/RTS and free-loss Persistent6D. `BeamSpotConstraint`,
`ChargedEcalMeasurement` and `NeutralEcalMeasurement` are independent and default
OFF. The new beam option uses native helix projection and KalTest filtering
before the first tracker hit; it updates the full seed covariance and is used
by forward filtering, RTS and free-loss trials. Beam works without a breakpoint
and with free-loss disabled. Charged remains at the ECAL face; neutral at the
post-loss virtual site; those two still require a selected breakpoint.

The separate beam-guided optimizer, collections, tuple family and backward
switch are removed. Regenerate cards with `BP_BEAM_SPOT_CONSTRAINT`; old beam
controls fail explicitly. Existing ordinary/free-loss endpoints carry whichever
constraints are enabled, without extra endpoint families. Independently executed
diffuse and truth-centered fits remain unchanged. All three backward defaults
remain OFF. Likelihood/complete RTS accounting includes the beam once, with
separate persisted beam diagnostics.

Built/tested but NOT installed: eight CTests pass; beam OFF exactly reproduces
388 retained fields on eight previous tracks. Beam ON works on five control
tracks and three hard-loss ECAL tracks, also with free-loss OFF. Existing ECAL
numerical results remain exact with beam OFF; only a helper error text changed.
Native covariance closure passes. One pre-existing batch test disagrees with
the user's intentional breakpoint-only wrapper default.

No population validation is claimed. Beam projection is geometric/material-free,
transverse-only and linearized once. The known neutral affine/nonlinear energy
mismatch remains. Next: install when authorized, then assess these limitations,
cluster calibration/ownership and held-out barrel/endcap performance before
default-on steering. Full evidence and outgoing focus:
`agents_record/2026-10-24-shared-beam-spot-kf.md`.
