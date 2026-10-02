# Exact-diffuse augmented breakpoint experiment (2026-10-03)

## Outgoing current focus, preserved verbatim before replacement

Active work remains RecBreakpoint on local `breakpoint`. The ordinary,
truth-centred and base free-loss pairs remain available. A new, default-on
`FreeLossBeamSpotObjective` experiment runs a second free-loss optimization:
it adds a beam-origin likelihood to the scalar loss-prior-centre objective,
but never feeds the beam into the KF/RTS measurement updates. It publishes
separate RTS/backward tracks and flat diagnostics. The base free-loss outputs
must remain unchanged; disabling the new control copies them exactly.

The source and installed module passed build/unit/batch-card gates and a
same-input three-track on/off smoke test at entries 11, 16 and 17. The old
non-beam tuple fields agreed and the new objective closed as hit-plus-beam
likelihood. This establishes mechanics only, not improved physics performance.
The focused gate is recorded in
`agents_record/2026-10-02-beam-guided-free-loss-objective.md`.
The earlier 2,000-event free-loss population showed light-loss degradation and
large positive tails. Next, analyze the new beam-guided result on categorized
barrel events, including clean-track safety and tails, before considering it
as a production default. Do not alter the common KF or GSF workflow for this
experiment.

Current ROOT campaigns present in this workspace are
`sim_large_barrel_20261001/` and `breakpoint_barrel/` (100 files each); the
earlier retained `sim_large_*_20260823/` inputs referenced in the outgoing
focus are no longer present. The outgoing text and its evidence links are
preserved in `agents_record/2026-10-02-before-beam-objective.md`.

## New method and controller

`DiffuseAugmentedRTS` is an independent, compiled/default-off boolean. The
maintained card exposes `BP_DIFFUSE_AUGMENTED_RTS=0|1`; the batch helper freezes
the value into generated cards. The extra collection is
`BreakpointTracksDiffuseAugmentedRTS`, with row-aligned status and index
collections and flat-tuple branches. Existing ordinary, free-loss, beam and
truth results are not replaced. Disabled and no-interval rows copy ordinary
RTS; a failed/underidentified selected-interval fit copies input KF with an
explicit negative status.

The single selected interval births a persistent six-dimensional state with
`b=log(p_before/p_after)` and an exact rank-one diffuse covariance
`P=P_finite+κuuᵀ`, `κ→∞`. The finite `P_finite(b,b)=1` is only a decomposition
reference; the test verifies changing it to 100 does not change the posterior.
The same native material/geometry predictor and loss Jacobian used by
Persistent6D transport the state. A special exact-diffuse scalar hit update
is necessary only while b is unidentified; later complete hits use the native
KalTest 6D update. The backward pass uses the limiting diffuse RTS gain and a
Joseph covariance recursion. A direct covariance-difference recursion failed
positive-definiteness in the first smoke run, particularly at the loose seed;
the Joseph form passed the focused tests. No finite Gaussian `SigmaLogLoss`
prior, Minuit scan, beam objective or truth loss magnitude steers this fit.

An absolute normalized likelihood is undefined with the flat b prior. The
saved finite innovation chi-square excludes the coordinate that consumes the
diffuse direction and must not be compared to ordinary Gaussian-prior chi2 or
NLL. Negative fitted b is kept as a diagnostic, not clipped. A diffuse
predicted/filtered state marked unresolved has only its stored finite block
`P_finite`; that block is not the physical total covariance. The method uses
one fixed linearization at b=0 and is not a general nonlinear global optimum.

## Focused gates and limits

- Rebuilt and installed the package-only plugin and configurable database.
  `RecBreakpointTransport`, `RecBreakpointLikelihood`, and
  `RecBreakpointDiffuse` numerical tests pass. The diffuse test covers one-
  and two-coordinate updates, reference-variance invariance, and the
  rectangular-birth RTS gain/Joseph covariance.
- Regenerated a 25-event tracker input from
  `sim_large_barrel_20261001/sim-barrel-4.root` to temporary storage. In a
  full 25-event smoke, all 26 reconstructed track rows had successful
  ordinary fits; 12 selected intervals produced diffuse fits and 14
  no-interval rows copied ordinary RTS. There were no diffuse fallbacks after
  the Joseph correction. The temporary files are not project status records.
- Same-code direct on/off reruns for entries 0, 3, 20, 21 gave bit-identical
  ordinary RTS and backward-filter pT arrays. Disabled extra endpoints copied
  ordinary exactly. The final tuple's 6D vector lengths and unresolved tags
  were checked. Verbose runs also covered entries 11, 16, 17 and the two hard-
  loss examples 20, 21; in this sample only 16/20/21 had selected breakpoints.
- Illustrative truth / ordinary RTS / diffuse pT (GeV): entry 20 track 1:
  15.177 / 10.725 / 15.473; entry 21: 29.952 / 26.006 / 30.231. Conversely
  tiny-loss entry 3 moved from 30.656 to 26.525 against truth 30.719.
  These selected examples **do not** establish population improvement.

Immediate next step: evaluate categorized pT resolution, clean-track safety,
failure/tail rates and fitted-b calibration on independent barrel/endcap
samples. Study the early-breakpoint tiny-loss failures and whether a physical
`b≥0` treatment or a principled model-choice gate is required. Keep all
ordinary and GSF workflows unchanged until those tests justify a change.
