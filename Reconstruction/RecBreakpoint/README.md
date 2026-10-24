# RecBreakpoint

Experimental electron breakpoint Kalman refitter on branch `breakpoint`
(renamed from `test_breakpoint`).
Reads `CompleteTracks`. GSF/shared KF sources and maintained batch cards are
unchanged. This is not a physics-validated replacement.

For the separate baseline-KF dataset producer (no breakpoint or ECAL fit), see
[interval-identification tuple](docs/interval-identification-tuple.md) and
`options/run_interval_identification.py`. It preserves raw hits, forward,
native-smoothed and independent-backward diagnostics, and separate truth labels.

The retired LocalMarginal comparison is preserved in the
[historical walkthrough](../../agents_record/2026-10-19-retired-local-marginal-walkthrough.md).
Only Persistent6D remains executable for a selected breakpoint.

## Build and run

From the CEPCSW repository root:

```bash
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target RecBreakpoint -j4
cmake -P build.105.0.0.x86_64-el9-gcc11-opt/Reconstruction/RecBreakpoint/cmake_install.cmake
BP_INPUT=/absolute/path/to/tracker.root \
BP_OUTPUT=/absolute/path/to/new_breakpoint_flat.root \
BP_INTERVAL_SELECTION_MODE=Truth BP_EVENTS=18 BP_SELECTED=11,16,17 BP_VERBOSE=1 \
build.105.0.0.x86_64-el9-gcc11-opt/run \
  gaudirun.py Reconstruction/RecBreakpoint/options/run_breakpoint.py
```

The dedicated card explicitly steers every package property. Existing files
are refused, never overwritten. Its tracker-only TDR_o1_v01 geometry must
match the input. `BackwardMode` is removed: each of the three backward filters
has its own run switch, default off.
The card rejects the retired `BP_BACKWARD_MODE` variable instead of ignoring it.

## RTS and optional backward endpoints

```text
Shared first-pass forward filter
  +--> RTS smoothing           --> BreakpointTracksRTS
  +--> optional backward hit refilter --> BreakpointTracksBackwardFilter
```

These are separate results, not CPU threads. Backward starts from the full
first-pass forward endpoint mean and its full covariance multiplied by
BackwardSeedScale (default100), revisits hits N-2 through 0, and uses
material-aware native IP propagation. RTS never consumes the backward-refiltered
states. Unconstrained RTS and backward endpoints use
`KalmanAdapter::propagateToIP`: native MarlinTrk propagation from the innermost
endpoint to (0,0,0), honoring MSOn/ElossOn. Independently executed truth/diffuse
fits and optional backward refilters retain this operation.
In any cumulative ordinary/free-loss stage containing the beam, RTS smooths back
through the stored native beam-to-first-hit transition and publishes that beam
state at IP. It does not traverse the boundary material a second time. The
optional backward refilter does not perform an additional beam update.
Backward still reuses forward hit evidence; it is NOT an independent Bayesian
smoother. Neither branch adds a beam-to-first-hit breakpoint.

The existing `OutputTracks` property now names the RTS collection (default
`BreakpointTracksRTS`); `OutputTracksBackwardFilter` names the other collection.
Two additional collections are declared:
`BreakpointTracksTruthOverrideRTS` and `BreakpointTracksTruthOverrideBackwardFilter`,
named by `OutputTracksTruthOverrideRTS` and `OutputTracksTruthOverrideBackwardFilter`.
The free-loss collections are also declared: `BreakpointTracksFreeLossRTS` and
`BreakpointTracksFreeLossBackwardFilter`, controlled by `OutputTracksFreeLossRTS`
and `OutputTracksFreeLossBackwardFilter`. It contains independently optimized
results when applied, ordinary copies otherwise where the corresponding
backward fit is enabled. The former separate beam-guided free-loss family is removed.
`BeamSpotConstraint` adds two independent beam-only RTS families.
`BreakpointTracksDiffuseAugmentedRTS` is an independent
exact-diffuse study controlled by `DiffuseAugmentedRTS`. The old neutral
penalty-only and absolute-prior refitters remain retired. The new optional
`NeutralEcalMeasurement` uses an actual scalar KF update in two cumulative neutral-stage
RTS families. Original `BreakpointTracksRTS` / `BreakpointTracksFreeLossRTS`
always remain unconstrained.

### Six cumulative constraint-stage families

Each switch enables one ordinary and one free-loss RTS result. All switches
default OFF. Enabled measurements accumulate in the saved-stage order
**beam -> charged -> neutral**. With all ON, the beam families contain beam;
the charged families contain beam+charged; the neutral families contain all
three. Disabled stages remain NaN and add nothing to later stages.
Each stage refits from the original seed/prior with its accumulated observations,
not from a previous posterior or fitted prior center. Thus hits are not counted
twice. The fitter applies observations at their physical sites (beam, neutral
at the breakpoint, charged at ECAL), not in output-stage order.

| Switch | Ordinary flat prefix | Free-loss flat prefix |
|---|---|---|
| BeamSpotConstraint | beam_spot_ordinary_rts_ | beam_spot_free_loss_rts_ |
| ChargedEcalMeasurement | charged_ecal_ordinary_rts_ | charged_ecal_free_loss_rts_ |
| NeutralEcalMeasurement | neutral_ecal_ordinary_rts_ | neutral_ecal_free_loss_rts_ |

Every prefix has `pt`, `pt_error`, `ip_parameters`, `ip_covariance`, and
`calo_*` fields. Removing the final `rts_` gives the prefix for
per-hit chi2 breakdown, track-state diagnostics, result/index and constraint
status. Ordinary stages also have `likelihood_*` diagnostics. Free-loss stages
save the accepted likelihood only under `optimizer_nll2`, `optimizer_quadratic`
and `optimizer_logdet` (with optimizer status/error and trial diagnostics).
The redundant constrained `*_free_loss_likelihood_*` groups are not written.
The original unconstrained `free_loss_likelihood_*` group is unchanged.
If no optimization runs (OFF, unavailable measurement or ordinary-result copy),
the optimizer likelihood remains unavailable; check optimizer/constraint status.
`constraint_mask`
records applied observations (beam=1, charged=2, neutral=4); the corresponding
`constraint_requested_mask` records enabled observations through that stage.
For all ON and successful, the masks are 1,3,7 for both methods.
The corresponding six collections are `BreakpointTracks{BeamSpot,ChargedEcal,NeutralEcal}{,FreeLoss}RTS`,
explicitly named in the maintained card.

OFF performs no fit: scalar endpoint values are NaN, vectors empty, index -1,
collection empty, `constraint_status=0`. ON but missing a usable observation
or breakpoint copies the preceding accepted stage (`constraint_status=1`);
failed additions also retain that stage, with status -1 and an error. Ordinary
and free-loss retain their own successful measurement sets. Status 2 means
the new measurement was applied. The initial free-loss baseline can itself be an input-KF fallback; its
result status remains explicit. With FreeLossFit OFF or no selected breakpoint,
the enabled beam free variant copies the beam ordinary result. These six are
RTS-only; existing three backward switches continue to control only the
unconstrained ordinary/free-loss/truth families.

The main `breakpoint` tree saves neutral-cluster selection for its own input
track, even with no selected breakpoint or a failed tracking fit. These fields
also supply the optional neutral-energy KF measurement; collection uses no truth.

| Track-row branch | Meaning |
|---|---|
| collected_neutral_ecal_status | 0: no input AtCalorimeter reference; 1: valid reference, no qualifying cluster; 2: clusters selected; -1: collection error |
| collected_neutral_ecal_error | Error text for status -1 |
| collected_neutral_ecal_cluster_count | Number of selected unique hit-supported neutral ECAL clusters |
| collected_neutral_ecal_cluster_indices | EcalCluster indices, linking to the event-level neutral_pfos tree |
| collected_neutral_ecal_cluster_energy | Aligned per-cluster energies in GeV |
| collected_neutral_ecal_cluster_energy_error | Aligned per-cluster errors in GeV using the configured resolution |
| collected_neutral_ecal_energy | Sum of selected cluster energies in GeV |
| collected_neutral_ecal_energy_error | Quadrature sum of selected cluster errors in GeV |

No selection gives empty vectors, count zero and zero energy/error; status
distinguishes a missing track reference from an empty window. A collection
error gives empty vectors/count zero and NaN summed energy/error. Each track
uses its input KF ECAL reference point with the configured theta/phi windows.
Only neutral PFO clusters belonging to EcalCluster with actual calorimeter
hits, finite positive energy and valid position qualify; a cluster is counted
once within a track. Different tracks may independently select the same cluster
when their windows overlap: this is not exclusive event-wide ownership.
The per-event `neutral_pfos` tree remains separate and is unchanged.
All track-output names must differ
from each other and the input. Each successful result contains IP, first-hit,
last-hit, and, when native extrapolation succeeds, ECAL-face (`AtCalorimeter`)
states and the original ordered hits. The ECAL-face state starts from that
result's own outermost fitted endpoint; it is not copied from `CompleteTracks`
and the last hit is not updated a second time. The same native barrel/endcap
layer propagation and nearest-face choice used by the baseline are applied.
Failure of this extra extrapolation leaves the IP/hit fit intact. Fixed
input-row mappings are
`BreakpointOutputIndex` (RTS) and `BreakpointBackwardOutputIndex`.
`BreakpointStatus` is 1 for a successful ordinary RTS fit, -1 for failed fit and 0 for
excluded input; absent outputs map to -1. Current fit failures fail the pair,
rather than publishing one branch under the other's name.

The flat tuple retains both branch names. With a backward switch off, that
filter is not run: its collection is empty, its index is -1, and its flat
endpoint/chi2 fields are NaN with per-hit vectors empty. Three
`*_backward_filter_enabled` flat fields record the effective switches.
`breakpoint_pt` remains an RTS alias. Every IP momentum endpoint has a
matching `*_pt_error` one-sigma field in GeV: `kf`, `breakpoint`/`rts`,
`backward`, `free_loss_rts`, `free_loss_backward`,
`truth_override_rts`, `truth_override_backward`, and `diffuse_augmented`.
The optional `reference_kf` and `reference_backward_kf` checks have the same
field. These are fit-covariance errors, not empirical resolution estimates.
For an EDM endpoint with curvature `omega` and packed covariance element
`covMatrix[5]=Var(omega)`, the stored value is
`sigma(pT) = pT*sqrt(Var(omega))/abs(omega)`. Missing or invalid endpoint
covariance yields NaN. The removed `backward_mode` branch no
longer misleadingly labels a row that now contains both results.

The calorimeter state is saved for the input KF, the original seven
endpoint families, and the six cumulative constraint-stage families listed above.
The original prefixes are `kf_`, `rts_`, `backward_`,
`free_loss_rts_`, `free_loss_backward_`, `truth_override_rts_`,
`truth_override_backward_`, and `diffuse_augmented_rts_`. Each prefix has
`calo_status`, `calo_error`, `calo_pt`, `calo_pt_error`, `calo_p`, `calo_parameters`,
`calo_covariance`, and `calo_reference_point_mm`. Parameters are the EDM
`(D0, phi, omega, Z0, tanLambda)` at the ECAL face; covariance is the packed
21-entry EDM covariance. Status 1 means native extrapolation succeeded, 2
means an input-KF state was copied (including a fit fallback), 0 means no
output, and -1 means the state was unavailable or extrapolation failed.
`calo_pt_error` uses that family's calorimeter-state covariance and the same
curvature formula. A charged-energy measurement is already incorporated in
its family's updated covariance; no extra shower error is added afterward.
Unavailable numerical fields are NaN/empty. The default card writes only
the flat tuple; add the optional `PodioOutput` from the card to serialize the
EDM track collections themselves. Optional neutral measurements use the existing
`free_loss_*` diagnostics and `free_loss_rts_*` endpoint fields.

## Neutral-energy KF measurement in free-loss RTS

`NeutralEcalMeasurement=false` is the compiled/card default. Enable with
`BP_NEUTRAL_ECAL_MEASUREMENT=1`. This and `ChargedEcalMeasurement` steer BOTH
separate ordinary augmented KF/RTS and free-loss variants with the same selected observations.
They work in ordinary fitting even with `FreeLossFit=false`. Old
`FreeLossChargedEcalMeasurement` / `FreeLossNeutralEcalMeasurement` properties
were renamed; the corresponding old `BP_FREE_LOSS_*_ECAL_MEASUREMENT` environment
variables are rejected, not silently ignored. Regenerate prepared batch cards.
Enabling both charged and neutral produces a charged stage and a later
charged+neutral stage; both also contain beam when that switch is enabled.
The independently executed diffuse and truth-centered fits are unchanged.
`FreeLossBackwardFilter` remains default OFF and affects only the original,
unconstrained result. The six constraint variants have no backward outputs.
Native MarlinTrk tracker-hit updates remain unchanged; this new scalar helper
is needed because its observation depends on the package's sixth loss coordinate.

For each trial of the loss-prior center, the shared Persistent6D fitter:

1. Introduces `b=log(p_before/p_after)` with the existing `SigmaLogLoss` and
   applies the existing loss mapping at the selected breakpoint.
2. At that same position, after the loss but **before** propagation through
   any further material, inserts a scalar neutral-energy measurement site.
3. Updates the full six-dimensional mean/covariance, then continues all native
   tracker updates. If requested, the charged-energy ECAL-face update follows.
4. Smooths back through every site, including the neutral site. The cluster's
   measured energy/error remain fixed; the fitted photon-energy estimate changes.

In KalTest coordinates `(drho, phi0, kappa, dz, tanLambda, b)`, with
`kappa=charge/pT` in GeV^-1, the predicted neutral energy and nonzero derivatives
are (neglecting electron mass):

```text
p_after = sqrt(1 + tanLambda^2) / abs(kappa)
h       = p_after * (exp(b) - 1)
H[kappa]     = -h/kappa
H[tanLambda] = h*tanLambda/(1 + tanLambda^2)
H[b]         = p_after*exp(b)
r = measured_E - h;  R = measured_sigma_E^2
S = H*P*H^T + R;     K = P*H^T/S
x_updated = x_predicted + K*r
P_updated = (I-K*H)*P*(I-K*H)^T + K*R*K^T
```

The measurement is the sum of collected hit-supported neutral ECAL energies.
For each cluster `sigma_E = 0.011*sqrt(E) + 0.004*E` in GeV by default;
independent cluster variances add. Synthetic/no-hit clusters are excluded.
Both charged and neutral scalar energy observations use `EnergyMeasurementSite`,
a six-dimensional `TKalTrackSite` specialization, and native `TVKalSite::Filter()`
just like tracker hits. The specialization supplies only the linearized energy
observation and its derivative; it does not implement a separate Kalman gain or
covariance update. Curvature-sign crossings are rejected for both observations.
The shared full-track Gaussian likelihood includes this scalar observation
exactly once, with its covariance determinant and normalization. No additional
neutral penalty or independently derived Gaussian prior on b is added.
The prior-center optimizer still retains the configured `SigmaLogLoss`.

`neutral_ecal_{ordinary,free_loss}_constraint_status` records OFF (0),
unavailable observation/breakpoint with previous-stage copy (1), applied (2), or
failed with previous-stage copy (-1). Original-family measurement-status fields stay
zero because those original families no longer consume optional observations.

The `neutral_ecal_free_loss_*` branches include optimizer diagnostics, normalized
likelihood, complete per-hit chi2 breakdown, IP and calorimeter pT/errors.
`neutral_ecal_free_loss_optimizer_neutral_ecal_*` retains measured/predicted/filtered/smoothed
photon energy/error, innovation variance/chi2, all three six-dimensional states
and covariances. `smoothed_affine_energy` and `smoothed_chi2` use the frozen
measurement Jacobian for the complete Gaussian score; `smoothed_energy` uses
the nonlinear energy formula at the final state. The measurement term and birth
process penalty of the virtual site are grouped with receiving tracker hit i+1
in the per-hit chi2 lists; native tracker-only residuals remain separate.

This is an experimental constraint, not evidence of better resolution. All
selected neutral energy is assigned to the one selected loss: contamination,
multiple emissions and wrong interval association can bias it. Charged/neutral
energy errors are currently treated as independent despite possible shared
calorimeter reconstruction/calibration correlations.
Trials whose neutral update or smoothed neutral state crosses zero curvature
relative to its prediction are rejected: the energy Jacobian cannot be used
across that momentum pole. The affine and nonlinear photon estimates can still
differ substantially without crossing it; both are saved to expose this limit.

## Loss state and interval ownership

`Persistent6D` uses the Gaussian loss prior configured
by `MeanLogLoss` and `SigmaLogLoss`. The experimental primary `Unconstrained`
and `Fixed` modes, their `LossPriorMode` selector, and their dedicated tuple
diagnostics were removed on 2026-09-29. The card and batch planner explicitly
reject the retired `BP_LOSS_PRIOR_MODE` variable rather than silently changing
a requested fit. Old Gaudi cards assigning `LossPriorMode` must be regenerated.
The separate `FreeLossFit` optimizer and Gaussian truth-prior pair remain.
Since 2026-09-30, free-loss trials optimize the Gaussian prior center while
retaining the SAME `SigmaLogLoss` as ordinary and truth-prior fits.
The retired mathematical contract is preserved in
[the historical record](../../agents_record/2026-09-29-retired-unconstrained-loss-contract.md).

### Exact-diffuse augmented RTS experiment

`DiffuseAugmentedRTS=True` adds a separate result; it never replaces ordinary
RTS, backward filtering, free-loss, or truth-centred outputs. It supports one
selected interval. Truth interval selection supplies the interval index only,
not the loss magnitude. At birth the persistent six-dimensional state adds
`b=log(p_before/p_after)` with an exact rank-one diffuse covariance
`P = P_finite + κ u uᵀ`, `κ → ∞`, rather than any finite Gaussian prior.
`P_finite(b,b)=1` is solely a decomposition reference; it does not constrain
the fitted b. Native material-aware propagation and the 6D loss Jacobian are
shared with Persistent6D. Until a downstream hit identifies b, a special
exact-diffuse scalar measurement update is used; subsequent hits use the
ordinary native KalTest 6D update. A rectangular 5D→6D transition and the
limiting diffuse RTS gain bring downstream information back to inner hits.

`diffuse_augmented_status` is 0 for a disabled ordinary copy, 1 for a
no-interval ordinary copy, 2 for a fitted diffuse result, and -1 for an
underidentified/failed input-KF copy. Its collection index is row-mapped in
`BreakpointDiffuseAugmentedIndex`; the fit status is also in
`BreakpointDiffuseAugmentedStatus`. The flat tuple stores the IP momentum,
parameters/covariance, fitted b and variance, and per-hit filtered/smoothed
b values. Row-aligned six-dimensional predicted/filtered/smoothed means,
finite covariance blocks, transport/noise, and hit indices are also recorded.
For a successful diffuse fit, `diffuse_augmented_eloss` is the signed
momentum-loss estimate in GeV at the first hit after the selected interval:
`p_after * (exp(b)-1)`, with
`p_after = sqrt(1+tanLambda^2)/abs(kappa)` from that hit's smoothed 6D state.
`diffuse_augmented_eloss_error` is its one-sigma linear propagated error,
using the full joint covariance of `(kappa, tanLambda, b)`, including their
off-diagonal correlations. This is a tracker momentum-loss proxy, not an
independently reconstructed photon energy. The two fields are NaN for
disabled/copied or failed diffuse fits.
`diffuse_augmented_predicted_unresolved` and
`diffuse_augmented_filtered_unresolved` mark states whose full covariance is
still infinite; their stored finite block is only `P_finite`, not a physical
total covariance. A negative fitted b is retained as a diagnostic rather
than clipped. The finite innovation chi-square excludes the
diffuse-consuming scalar measurement and is **not** an absolute likelihood
comparable to the Gaussian-prior ordinary/free/truth fits. No diffuse backward
endpoint or calibrated model selection is claimed. This method remains an
unvalidated experiment.

An interval i is radius-ordered `hit[i] -> hit[i+1]`, not a detector layer ID.
IntervalSelectionMode selects the source of the breakpoint list independently
of LossStateMode (how losses are fitted) and TruthOverride (extra truth-centered pair):

| IntervalSelectionMode | Behavior |
|---|---|
| Truth (compiled/card default) | Select the eligible layer-owned breakpoint with the largest summed Geant4 eBrem momentum loss; ties choose innermost |
| Manual | Use BreakpointIntervals exactly; [] is the no-breakpoint reference |
| Auto | Reserved reconstruction-based finder; initialization fails explicitly until implemented |

Truth follows reconstructed-hit associations and exact embedded G4 provenance.
`TruthBHLossEventData::matchBreakpoints` resolves each emission's owner and its
accepted hit. Both halves of a sensitive traversal belong to that same hit;
audited barrel support/cooling follows its physical sensor assembly, never
nearest radius. Shared structures and unresolved/missing/ambiguous hits are
recorded but cannot select a breakpoint. Unknown passive layouts fail closed.
The selected index i means a loss at hit[i], before outward propagation to
hit[i+1]; inward propagation reaches hit[i] before applying inverse loss and
updating measurement i. The outermost hit has no supported outward interval.
Several eligible emissions at the same hit are summed. Select the maximum
positive sum without an additional loss threshold; other losses are not fitted.
Physical hook-to-hook material integrals are unchanged and remain separate.
The ordinary fit receives only the indices: its MeanLogLoss/SigmaLogLoss prior
is NOT replaced by truth. TruthOverride must separately be enabled to set loss
prior centers in the additional pair. All Geant4 metadata remain diagnostic apart
from this explicit location selection and the optional loss override.

Truth with no eligible eBrem selects an empty list and runs the ordinary 5D
reference, with copied extra outputs. This is separately labelled from no
recorded eBrem. Invalid truth fails the affected track
with interval-selection status/error; it never means a guessed empty list.
A nonempty BreakpointIntervals list is rejected outside Manual mode, so it
cannot silently be ignored. Dedicated-card control: BP_INTERVAL_SELECTION_MODE.
The card loads truth collections for Truth selection even if TruthOverride=False.
Previously prepared cards predate this control/default change: regenerate them
in a new output directory. Explicit fixed-list comparisons must now set Manual;
do not patch a checksum-protected prepared card in place.

At most one interval is supported by the ordinary and truth-prior fits.
Negative or duplicate entries fail initialization;
out-of-range entries fail the affected track. This ordering targets outward,
noncurling barrel tracks.

```text
b = log(p_before / p_after)
fractional loss = 1 - exp(-b)
kappa_after = exp(b) * kappa_before
```

Loss is collapsed at the upstream surface. Outward: update upstream hit,
apply loss, propagate, update downstream hit. Backward: propagate to upstream
surface, apply inverse loss, then update its hit. It is not a fitted emission
position inside the interval. No BH mixture is used.

### Persistent6D (compiled/card default)

Before birth the filter is 5D. At the configured upstream hit it introduces
one independent b prior. The full 6D mean/covariance then stays live at EVERY
downstream prediction and native KalTest update, with b applied only once.
The joint RTS recursion crosses the rectangular 6D-to-5D birth boundary.

```text
ordinary transport: J6 = diag(F_track,1), Q6 = diag(Q_track,0)
measurement: H6 = [H_track,0]
```

The zero H_b does not prevent measurement updates of b through track/b cross
covariance. The sixth coordinate is never native KalTest t0. All physical
transport and measurement updates use native interfaces. The parallel
backward continuation uses the established local-joint inward loss treatment
on the common forward endpoint's 5D marginal, not a new persistent inward
six-dimensional implementation.

### TruthOverride

The default diagnostic chain is Truth interval selection + Persistent6D +
TruthOverride=True. The effective interval list is shared by both pairs:

```text
select intervals for this track
  -> ordinary RTS/backward: fit losses using MeanLogLoss/SigmaLogLoss priors
  -> extra RTS/backward: use matched Geant4 b as the prior center, SAME SigmaLogLoss
```

Both pairs use the same fitting code and SigmaLogLoss
controls. Truth changes only the prior center of each selected b; later hits
can update b and its variance. Track covariance, native material/MS and
measurement updates remain active. Manual
selection shares its supplied list in exactly the same way. TruthOverride does
not discover additional intervals. With an empty effective list the extra pair
is copied (result status1), even though the switch is on. With a nonempty list,
successful oracle results have status2. Auto remains an initialization error.

```python
fit.IntervalSelectionMode = "Truth"  # select locations for this track
fit.LossStateMode = "Persistent6D"   # ordinary pair fits the loss
fit.TruthOverride = True           # additional oracle pair; compiled/card default True
fit.BreakpointIntervals = []  # Manual-only; Truth builds the effective list
# Three RTS endpoints; their backward partners require separate run switches.
# With FreeLossFit=False its RTS output copies ordinary RTS.
```

The additional prior centers come from embedded event truth:
reconstructed hit -> MCRecoTrackerAssociation -> SimTrackerHit -> exact
Geant4 step/fraction hook. All ordered hits require unambiguous monotonic
hooks on one primary electron. TruthMaxEndpointDistance validates associated
positions, never chooses a nearest hit.

For each eligible layer-owned breakpoint, sum the recorded momentum losses
of its process-subtype 3 steps. Use the pre-step momentum of the earliest such
emission: `z=1-sum(delta_p_ebrem)/p_before_first_emission`, `b=-log(z)`.
Other process types are excluded, but the recorded pre/post momentum change
of an eBrem step can include continuous loss in that same step. Invalid sums
are marked ineligible, not clipped. No hook-interpolated momentum is reused
as the denominator after changing ownership.
Only MeanLogLoss is replaced per interval for the extra pair. SigmaLogLoss is
identical in both pairs, and the extra fitted b need not equal truth. A Manual
interval with no eBrem gets prior center b=0, not a permanently fixed zero loss.
The fitter accepts generic per-interval prior centers; it has no separate
TruthOverride loss-state mode or truth-specific update. Native material, MS,
ionization steering, seeds and upstream loss placement remain unchanged.

The card conditionally loads the six tracker association/SimTrackerHit sets,
GsfG4MaterialSteps and GsfSimTrackerHitG4StepLinks. It uses no side CSV/ROOT
helper or GSF execution. The module reuses TruthBHLossEventData's event reader
and adds its breakpoint ownership method inside RecBreakpoint. The identification
tupliser uses that same method for all candidate labels. Legacy GSF callers
still use matchTrack; that workflow is not migrated.
Invalid truth fails
only the additional oracle pair; the ordinary outputs remain available.
truth_override_status 0 means off/empty, 1 valid input, -1 invalid
event input, -2 invalid track match, -3 out-of-range interval. There is no
ordinary-loss fallback on truth failure. An input loading failure can occur before tuple output.

With `TruthOverride=False`, the additional pair copies the already computed
ordinary pair (including covariances, hits and chi2); it does not rerun a fit.
Truth interval selection can still read material truth. An empty effective
interval list also copies the ordinary pair.
With True and nonempty intervals, a separate call to the same fitter generates
both truth-centered endpoints with the same loss-state mode, prior sigma, seed
scales, hit selection and material settings. Both pairs are one-pass fits.
No truth-centered state
feeds back into the ordinary pair. The old public `LossStateMode="TruthOverride"` fails clearly:
keep `LossStateMode="Persistent6D"` and set the independent `TruthOverride` bool.
The old internal fixed-loss path has also been removed.
Dedicated-card environment control: `BP_TRUTH_OVERRIDE=1` (default 1).
Set BP_TRUTH_OVERRIDE=0 to copy ordinary results into the extra pair instead.
Already prepared cards preserve their old values/default expressions; regenerate
them in a new output directory to adopt the new default.

`truth_override_result_status` and the input-row-aligned EDM
`BreakpointTruthOverrideStatus` distinguish: 0 absent/not attempted, 1 copied,
2 oracle success, -1/-2/-3 truth input/match/interval failure, -4 oracle fit failure.
Here status2 now means a truth-centered adjustable-prior fit. Historical tuples
without truth_override_loss_treatment retain their old fixed-b meaning; do not
mix them with new results or describe old resolution/chi2 plots as this method.
Index maps are `BreakpointTruthOverrideRTSIndex` and
`BreakpointTruthOverrideBackwardIndex`; missing outputs map to -1. Check this
result status rather than assuming that a truth-named branch used truth.
Flat branches `truth_override_rts_pt`, `truth_override_backward_pt`,
`truth_override_{forward,backward,smoothed}_chi2` and their `_local_chi2` vectors
save the extra pair's results. The smoothed score also has status/error fields.
New metadata: truth_override_loss_treatment="PriorCenter" and
truth_override_prior_sigma_log_loss record the definition and shared prior sigma.
truth_override_{rts,backward}_fitted_log_loss and corresponding
_fitted_log_loss_variance vectors save the updated b posterior; they align with
breakpoint_interval (including copied results). truth_override_log_loss remains
the matched truth input, NOT the fitted output. Invalid extra results have empty
posterior vectors. The treatment/sigma fields describe
configuration; the result status still distinguishes active, copied and failed.
`truth_override_{rts,backward}_ip_parameters` hold `(D0,phi,omega,Z0,tanLambda)`
at the origin; `_ip_covariance` copies all 21 packed EDM covariance elements.
The first 15 describe the fitted five-parameter helix; the trailing time
row/column is carried from EDM and is not a fitted loss coordinate.
Failed extra results have NaN scalar results and empty vectors, not copies.
An ordinary-pair failure currently leaves the extra pair unattempted (status 0).

Truth vectors retain interval, retained_fraction, log_loss, momentum_before,
ebrem_loss, tx0, first_step/last_step and start_fraction/end_fraction under the
truth_override_ prefix; scalar error, G4 track ID and max endpoint distance are
also saved. Truth t/X0 is passive. Manual with an empty list needs no loss truth;
Truth selection needs it to determine whether the effective list is empty.

## One-pass fits and seeds

Each pair runs one forward filter, one RTS smoother and one
optional backward refilter. There is no repeated relinearization loop.
Persistent6D supports at most one interval.
Empty intervals use the ordinary 5D reference. No new fitted-loss positivity
constraint is imposed.

RTS starts at the final forward updated state/covariance and uses buffered
forward transitions. It neither restarts a filter nor consumes the backward
refilter. BackwardSeedScale does not affect RTS. The backward refilter copies
the forward endpoint and scales its full covariance once; it still reuses
forward evidence and is not an independent Bayesian smoother.
backward_seed_forward_chi2 records that pair's forward bookkeeping.

MaxFitIterations and RelinearizationTolerance have been removed.
BP_MAX_ITERATIONS and BP_ITERATION_TOLERANCE are rejected by the maintained
card and batch preparation, not silently ignored. Old prepared cards assigning
the removed Gaudi properties must be regenerated; existing tuples are unchanged.
Iteration-only flat fields are removed: one_pass_pt, fit_iterations,
iteration_*, backward_fit_iterations, backward_iteration_* and
truth_override_{rts,backward}_fit_iterations. The original four endpoints, fitted loss means
and variances, per-hit states, and all three per-hit chi2 lists/totals remain.
The original iteration contract is preserved in
agents_record/2026-09-10-breakpoint-readme-before-iteration-removal.md.

FirstMiddleLast selects first/middle/last usable 2D hits (N//2 middle).
FirstThree restores the older prefit. SeedScale uniformly scales native loose
FullLDCTracking-style variances. The seed uses hit positions as a starting
estimate; do not interpret its fitted covariance as independent data.
VerifyKFReference evaluates BOTH native no-breakpoint references; empty-list
pT must match within 1e-4 relatively for RTS and for backward when
BackwardSeedScale=1. The native backward reference remains unscaled: at other
scales its pT is saved as a comparison, without imposing an equality gate.
Stored CompleteTracks can differ because pattern recognition, merging,
retries and steering are not rerun here.

## Three default-on chi2 lists

All vectors are indexed in outward hit order, length hit_count:

| Flat vector | Definition |
|---|---|
| forward_local_chi2 | Native forward update increments; local_chi2 is its retained alias |
| backward_local_chi2 | Native inward update increments; last hit is 0 because it is a copied seed, not another update |
| smoothed_local_chi2 | Final RTS complete quadratic objective assigned to each hit |

The smoothed score uses the SAME last-pass affine model as the KF/RTS:
measurement penalties use original V and measurement derivatives at the
filter's reference states. Incoming process penalties use the stored Q,
including the independent loss birth prior once. The initial seed penalty is
added once at hit 0. No extra b penalty is added after it was included at birth.
It is a complete quadratic objective, not a calibrated probability or a
normalized likelihood for comparing models with different covariances.

Components are persisted as smoothed_measurement_chi2, smoothed_process_chi2
and scalar smoothed_seed_chi2. smoothed_native_measurement_chi2 separately
evaluates the nonlinear native measurement at the final smoothed mean, to
expose differences from the affine objective. It is hit-only and does not
replace the complete score.

This complete-score breakdown is always written for ordinary, free-loss and
truth-override RTS results, with no prefix, `free_loss_` and
`truth_override_`, respectively. No card switch is needed. Each prefix has
`smoothed_measurement_chi2`, `smoothed_process_chi2` and
`smoothed_native_measurement_chi2` vectors in outward hit order, plus scalar
`smoothed_seed_chi2`. For a valid score:

```text
smoothed_local_chi2[i] = smoothed_measurement_chi2[i]
                      + smoothed_process_chi2[i]
                      + (i == 0 ? smoothed_seed_chi2 : 0)
smoothed_chi2 = sum(smoothed_local_chi2)
```

The native measurement vector is not added to that sum. Disabled extra fits
copy the ordinary breakdown along with their ordinary endpoints. Absent/failed
extra fits and an input-KF fallback have empty breakdown vectors and a NaN seed
term; the existing status/error fields distinguish these cases. A failed score
evaluation can retain partial diagnostics: only use the complete decomposition
when `smoothed_chi2_status == 1`. These are RTS contributions, not a new
decomposition of the backward innovation score or of stored CompleteTracks.
Older tuples lack the two extra-prefix breakdowns and require refitting to
obtain them.

For an RTS transition, let d=x_smoothed_target-x_predicted_target and
u=P_predicted_target^-1*d. The conditional mean of its process deviation is
w=Q*u; its penalty is u^T Q u. This avoids inverting singular scattering Q or
injecting fictitious noise into a deterministic/static b. It also handles the
rectangular birth boundary. The score is evaluated at RTS means, not arbitrary
candidate states where this conditional identity would not hold.

Totals forward_chi2 (alias filter_chi2), backward_chi2 and smoothed_chi2 equal
their corresponding vector sums. smoothed_chi2_status is 1 valid, 0 not
evaluated, -1 evaluation failed; an error string accompanies failure.
Diagnostics do not alter the fitted state: a score failure retains endpoints
but marks total smoothed chi2 NaN. The RTS EDM track carries smoothed_chi2;
the backward EDM track carries backward_chi2. Their retained NDF bookkeeping
is total forward measurement dimensions minus five, not a calibrated
degrees-of-freedom prescription for fitted losses and priors.

Do not add the forward and backward totals: their evidence overlaps.
For a consistent exact linear-Gaussian model, the complete RTS objective
equals the innovation quadratic sum. Native nonlinear post-update residual
evaluation and numerical approximations can produce differences in this
extended KF; the separate terms make those differences auditable.

## Properties

Optional normalized-likelihood free-loss fitting is now implemented in this
package, not in an external prototype. Set `FreeLossFit=True` to add a separate
FreeLoss RTS result with the shared Gaussian fitter at the optimized prior center. The ordinary
RTS result and its tuple fields are never replaced. The
compiled compatibility default is false; the maintained standalone/batch card
defaults to true. Set `BP_FREE_LOSS_FIT=0` to disable it for newly prepared jobs.
Previously prepared cards are not changed. This is an implementation promotion, not a
claim of physics validation. See [Free-loss fitting](docs/free-loss-fit.md)
for the model, code organization, output contract and limitations.

For each prior-center trial, the base Minuit optimization uses the existing RTS pass directly:
`complete smoothed chi2 + log det S_all + M log(2*pi)`. The first term includes
measurement, process and seed chi2; `S_all` is the joint measurement covariance,
not the smoothed state covariance. No reference-trajectory iteration is
introduced. See [the exact objective](docs/smoothed-objective.md).
`ChargedEcalMeasurement` (compiled/card default **false**;
`BP_CHARGED_ECAL_MEASUREMENT=1`) is an experimental measurement for both
ordinary and free-loss fits. With exactly one charged CyberPFOPID linked to
the input track, it sums its hit-supported `EcalCluster` energies. Native
propagation takes the outer tracker state to the ECAL face; the propagated
state, transport Jacobian and process noise come together from the same
`TKalDetCradle::Transport` machinery used by tracker-hit propagation. These are
lifted to six dimensions with unchanged b, retaining the loss coordinate and
all its correlations. No finite-difference Jacobian or covariance-subtraction
estimate of process noise is used. The scalar energy measurement updates that
ECAL-face state. RTS first traverses this ECAL-to-last-tracker edge, then the
ordinary tracker edges inward. For a successful ON fit, the updated ECAL-face
state is also the published `charged_ecal_ordinary_rts_calo_*` /
`charged_ecal_free_loss_rts_calo_*` state. No cluster position enters the
measurement. Each free-loss trial scores the tracker likelihood plus the
conditional ECAL-energy likelihood.
The provisional per-cluster error is
`a*sqrt(E/GeV) GeV + c*E`, with independent variances summed (`a=0.011`,
`c=0.004` by default). The energy derivative at ECAL is analytic in curvature
and tan(lambda); the last-tracker-to-ECAL transport uses native double-precision
state, Jacobian and process noise at the same surface pivot. Verbose output dumps
the source/predicted/filtered six-dimensional states and covariances and the
ECAL edge F/Q for closure checks. This is not yet a calibrated calorimeter
measurement. CyberPFA may have used the input track
momentum to construct the charged shower, so the observation may not be
independent of the tracker fit. The flat tuple records
`free_loss_charged_ecal_measurement_status` (0 off, 1 no usable cluster,
2 selected for the fit, 3 ambiguous charged PFO), measured energy/error, predicted energy,
innovation variance and chi2. Original `free_loss_rts_*` and `rts_*` endpoints
remain unconstrained for every switch combination. ECAL variants currently
require a selected breakpoint; unavailable/failed additions preserve the preceding
accepted constraints with an explicit status. The legacy `free_loss_charged_ecal_*` selection
fields remain the shared charged observation (status 2 means selected, not success).
Ordinary and free-loss likelihoods use the same score routine, including neutral
in the captured tracker model and the charged conditional likelihood once.

### Shared beam-spot KF measurement

`BeamSpotConstraint=false` (card/environment `BP_BEAM_SPOT_CONSTRAINT=0`) is
independent of the charged/neutral switches and `FreeLossFit`. It enables the
beam-only ordinary/free-loss families, including tracks without a breakpoint.
The old beam-guided optimizer, output collections, branches and backward
switch are removed. Regenerate prepared cards; retired environment names fail
explicitly rather than silently selecting another behavior.

The loose three-hit prefit is expressed at `(BeamSpotX, BeamSpotY, 0)`.
This is an explicit first measurement site: measured `drho=0`,
`H=(1,0,0,0,0)`. Native `TVKalSite::Filter()` updates all five seed coordinates
and their covariance there. The native IP vacuum layer supplies the navigation
anchor, not an additional measurement. Native `AddAndFilter()` then propagates
to the first tracker hit through the configured detector material and updates
that hit exactly once. Its native `F` and `Q` are retained; `MSOn` and `ElossOn`
control material effects as for tracker-to-tracker transport.
RTS returns through that boundary transition to the beam site. The published
IP state/covariance come from the smoothed beam state; only a coordinate
repivot to the origin is needed for an offset beam mean. Do not propagate
again from the innermost hit and count boundary material twice. There is no z hit.
The scalar beam variance is
`cos(phi0_beam)^2*sigmaX^2 + sin(phi0_beam)^2*sigmaY^2`, frozen at the seed.
No external beam score is appended after RTS.

Likelihood accounting uses
`p(beam,hits,ECAL) = p(beam|original seed) * p(hits,ECAL|beam)`.
The complete RTS breakdown uses the original beam seed penalty, the beam
measurement residual (in the first hit's measurement contribution), and the
beam-to-hit process penalty (in the first hit's process contribution).
The transported beam-conditioned covariance at hit 0 is used for the conditional
Gaussian model. Its normalization is combined with the initial beam innovation
variance, not another smoothed covariance or a repeated beam update.
Beam-only output state covariances therefore include this measurement.
The beam width is projected once using the seed direction, not an iterated
vertex fit. Affine and nonlinear drho fields now coincide at the beam pivot.
Boundary process chi2 and covariance closure are saved; verbose output contains
the native source/predicted/filtered states, `F` and `Q`.

| Property | Compiled default | Meaning |
|---|---|---|
| InputTracks | CompleteTracks | Input hit-list tracks |
| OutputTracks | BreakpointTracksRTS | RTS collection |
| OutputTracksBackwardFilter | BreakpointTracksBackwardFilter | Optional inward-filter collection; empty when disabled |
| OutputTracksFreeLossRTS | BreakpointTracksFreeLossRTS | Optimized RTS, or exact ordinary RTS copy |
| OutputTracksFreeLossBackwardFilter | BreakpointTracksFreeLossBackwardFilter | Optional optimized backward filter; empty when disabled |
| OutputTracksTruthOverrideRTS | BreakpointTracksTruthOverrideRTS | Oracle RTS or ordinary RTS copy |
| OutputTracksTruthOverrideBackwardFilter | BreakpointTracksTruthOverrideBackwardFilter | Optional oracle backward; empty when disabled |
| OrdinaryBackwardFilter | false | Run the ordinary inward refilter; otherwise leave its output absent |
| FreeLossBackwardFilter | false | Run the accepted free-loss backward refit; optimization trials never run backward |
| TruthOverrideBackwardFilter | false | Run the truth-prior backward refit when TruthOverride applies |
| OutputTracksDiffuseAugmentedRTS | BreakpointTracksDiffuseAugmentedRTS | Exact-diffuse augmented RTS or ordinary copy; input KF fallback on failure |
| TruthOverride | true | Extra pair uses truth b prior centers with SAME SigmaLogLoss/mode; otherwise copy ordinary pair |
| IntervalSelectionMode | Truth | Truth, Manual, or reserved/unimplemented Auto |
| BreakpointIntervals | [] | Manual-only radius-ordered hit intervals; must be empty outside Manual |
| MeanLogLoss | 0 | Finite in [0,5]: ordinary Gaussian prior center; truth-prior pair uses matched truth centers instead |
| SigmaLogLoss | 0.001 | Positive finite Gaussian-prior sigma shared by ordinary, free-loss and truth-prior fits; retained in every optimizer trial and final refit |
| LossStateMode | Persistent6D | Compatibility property for prepared cards; only Persistent6D is accepted. TruthOverride is a separate bool |
| DiffuseAugmentedRTS | true | Independent one-interval, flat-prior 6D KF/RTS fit; ignores SigmaLogLoss |
| NeutralLossThetaWindowMrad, NeutralLossPhiWindowMrad | 10, 200 | Positive angular half-windows about input `AtCalorimeter` direction for reconstructed neutral-cluster selection |
| NeutralLossStochasticError, NeutralLossConstantError | 0.011, 0.004 | Provisional per-cluster `sigma_E = a sqrt(E/GeV) GeV + c E`; selected cluster variances add independently |
| FreeLossFit | false | Card default true; normalized-likelihood optimization of the Gaussian loss-prior center for one Persistent6D interval; input KF fallback on failure/unsupported interval |
| ChargedEcalMeasurement | false | Two cumulative charged-stage RTS families; inherit enabled beam, add ECAL-face energy with one breakpoint and one linked charged PFO |
| NeutralEcalMeasurement | false | Two cumulative neutral-stage RTS families; inherit enabled beam/charged, add a post-loss scalar KF update retaining track/loss correlations |
| ChargedEcalStochasticError, ChargedEcalConstantError | 0.011, 0.004 | Provisional positive per-cluster sigma_E coefficients; `BP_CHARGED_ECAL_STOCHASTIC` and `BP_CHARGED_ECAL_CONSTANT` |
| BeamSpotConstraint | false | Two separate beam-only ordinary/free-loss RTS families, using an initial transverse beam measurement and usable without a breakpoint |
| BeamSpotX, BeamSpotY | 0 mm, 0 mm | Transverse beam mean for the initial scalar KF measurement |
| BeamSpotSigmaX, BeamSpotSigmaY | 0.0145 mm, 0.000036 mm | Positive beam widths; projected into local drho for one Gaussian likelihood term |
| FreeLossMaxLogLoss | 1 | Upper bound on the optimized prior center, finite in (0,5]; lower bound zero. Does not truncate the Gaussian or bound the fitted posterior loss |
| FreeLossMaxCallsPerStart | 180 | Positive maximum Minuit function calls per start; does not include the coarse/local scans |
| FreeLossTolerance | 0.001 | Positive finite MIGRAD tolerance |
| SeedScale | 1 | Positive finite scale of five loose seed variances |
| BackwardSeedScale | 100 | Positive finite scale of the full copied first-forward endpoint covariance; mean and RTS unchanged |
| SeedHitSelection | FirstMiddleLast | FirstMiddleLast or FirstThree |
| MaxChi2PerHit | 1e100 | Positive finite native update acceptance limit |
| MSOn | true | Native multiple-scattering noise |
| ElossOn | false | Native deterministic ionization correction |
| TruthMaxEndpointDistance | 5 | Positive finite mm validation tolerance on associated hooks |
| VerboseDump | false | Full state/covariance dumps |
| VerifyKFReference | false | Native reference checks for both endpoints |
| SelectedEventIndices | [] | Zero-based selected entries; empty means all |
| OutputFile | breakpoint_flat.root | New flat output file |

`truth_pt` now means the pT of the MC particle matched to **this row's**
`CompleteTracks` track by `CompleteTracksParticleAssociation`; it no longer
repeats the event's generator-electron pT on unrelated track rows. The
association producer's weight is a count of reconstructed track hits attributed
to an MC particle. The flat row saves `truth_track_hit_count`,
`truth_best_mc_hit_count`, `truth_linked_hit_count`, and
`truth_match_purity = truth_best_mc_hit_count / truth_track_hit_count`.
`truth_match_status` is 0 for no positive association, 1 for a unique largest
weight, and 2 for a tie. Missing/tied rows have NaN `truth_pt` rather than an
arbitrarily assigned particle. A unique match additionally records
`truth_mc_index`, PDG, generator/simulator status, energy, momentum x/y/z,
charge, vertex/endpoint x/y/z, and parent MC indices. This is a flat snapshot
of the associated MC particle, not a copied PODIO object.
The ratio `truth_linked_hit_count / truth_track_hit_count` distinguishes
unlinked hits from a competing MC particle's linked hits. These are passive
diagnostics; truth interval selection and TruthOverride retain their separate
association-driven contracts. For primary-electron pT resolution, require a
valid unique match to the generator-status primary and retain the existing
topology/track-completeness selection; a short fragment can have purity 1.

Each track row also records all `CyberPFOPID` charged PFOs whose track link is
that exact `CompleteTracks` object: `charged_pfo_count`, PFO index/PDG/energy,
and flattened PID hypothesis PDG/likelihood arrays keyed by PFO index. Its
ECAL-only clusters have `charged_ecal_cluster_count` (the exact count, not a
capped flag), PFO/cluster indices, energy, and x/y/z position arrays. The
PFO count and cluster count are distinct; zero and multiple matches remain
visible without silently selecting one. HCAL clusters are not mixed into these
ECAL arrays.

The same flat ROOT file contains a companion `neutral_pfos` TTree with **one
row per processed event**, including an empty row when no neutral PFO exists.
Its `event_index` joins to the per-track `breakpoint` TTree; neutral PFO
index/PDG/energy and ECAL-cluster PFO/cluster indices, energies, and x/y/z
positions are stored once for the event, not repeated for every charged track.
These fields describe CyberPFO reconstruction ownership, not a neutral-cluster
truth match. The same event row also stores one entry per recorded primary-
electron eBrem photon in parallel `ebrem_photon_*` arrays. The photon index,
Geant4 track ID, parent track ID, and parent step number identify its origin;
`birth_{x,y,z}` (mm) and `birth_{energy,px,py,pz}` (GeV) give its emission
location and four-momentum. `ecal_entry_{x,y,z}` (mm) and
`ecal_entry_{energy,px,py,pz}` (GeV) are the post-point values of the first
recorded photon step entering an ECAL volume, not an extrapolation to a
cluster. `ecal_entry_status=1` means that step exists; status 0 means no
recorded ECAL entry, with step number -1 and entry coordinates/four-momentum
set to NaN. These truth-photon arrays are event-level and do not assert a
match to any neutral PFO or cluster. Older flat files retain the former
event-level meaning of `truth_pt` and have no `neutral_pfos` tree; do not
silently combine their `truth_pt` values with the new track-matched schema.

For every photon, `ebrem_photon_last_step_*` saves its highest-numbered
recorded Geant4 step, whether or not it entered ECAL: step number, process
subtype, pre/post step and track status, volume copy numbers, `post_in_ecal`,
pre/post positions (mm), energies and momentum components (GeV), times (ns),
step length (mm), and energy deposit (GeV). `last_step_status=1` means a step
was found. With no recorded step it is 0, integer step fields are -1, and
floating-point step fields are NaN; the photon's birth record remains intact.
The final-step process and track status help identify absorption, conversion,
or escape, but they are raw Geant4 codes and not a reconstructed PFO match.

The dedicated card exposes `fit.BackwardSeedScale` and optional environment
variable `BP_BACKWARD_SEED_SCALE` (default100). All 25 covariance entries are
scaled, preserving correlation coefficients; standard deviations scale by
sqrt(BackwardSeedScale). The effective value is saved in every flat row as
`backward_seed_scale`. This control does not create a fresh backward seed,
scale the independent breakpoint loss prior, or change the forward fit/RTS.

## Other automatic tuple information

All runs also save the `free_loss_*` fields described in
[the free-loss schema](docs/free-loss-fit.md#flat-tuple-contract).
The retired `beam_guided_free_loss_*` family and free-loss external-beam
penalty diagnostics are no longer written. Common beam configuration is saved
as `beam_spot_constraint_enabled`, position and widths in mm.
`ordinary_beam_spot_*` / `free_loss_beam_spot_*` are diagnostic aliases for the
new beam-only families (not the unconstrained baseline). They retain `applied`,
predicted/filtered/smoothed drho, smoothed affine drho, measurement/innovation
variances, innovation chi2, smoothed measurement chi2,
`boundary_process_chi2` and `boundary_covariance_closure`. Ignore the numeric
fields when `applied=false`; they are not a failed-fit substitute.
The base optimizer diagnostics are inactive/NaN/empty when unused; its additional endpoint
fields then copy the ordinary results exactly. Failed/unsupported optimization
instead copies the original input KF (free_loss_result_status=3). Its stored
score is free_loss_kf_chi2; unavailable breakpoint per-hit vectors are empty and
refit totals NaN. Existing tuple fields and the
four earlier collections retain ordinary/truth-prior meanings regardless of
FreeLossFit. FreeLoss track covariances now include posterior loss uncertainty
and track/loss correlations from the same Gaussian fitter. They still condition
on the optimized prior center; Minuit's error on that center is NOT SigmaLogLoss
and is not added to the covariance. `free_loss_treatment="PriorCenter"` and
`free_loss_prior_sigma_log_loss` distinguish these outputs from historical fixed-b
tuples. `free_loss_prior_mean_log_loss` names the optimized center explicitly;
`free_loss_b` remains its compatibility alias, not the posterior loss.
The legacy `free_loss_covariance_conditional` fixed-b flag is false.
`Track.chi2` retains its existing
quadratic meaning; `free_loss_nll2` separately records the fitting objective.

The flat tuple also records the same normalized full-track likelihood for
each RTS family as `ordinary_likelihood_*`, `free_loss_likelihood_*`,
and `truth_override_likelihood_*`.
Each prefix has `status` (1 valid, 0 absent,
-1 unavailable or evaluation failed), `nll2`, `quadratic`, `logdet`,
`measurement_dimensions`, `latent_dimensions`, and `error`. All use
`nll2 = quadratic + logdet + measurement_dimensions * log(2*pi)`, with the
complete RTS measurement + process + seed score as `quadratic`. Free-loss
optimized rows copy the accepted optimizer objective (also retained under
`free_loss_nll2`). Disabled/empty-interval free and truth rows copy the
ordinary likelihood. A free-loss input-KF fallback has no breakpoint
likelihood. Likelihood failures are passive and do not discard valid tracks.
Persistent6D RTS fits capture the required Gaussian
model. Older tuples lack these new fields and must be regenerated.

The tuple has one row per attempted track, including failures. It retains
ordered hit cells/radii/z, truth/KF pT, reference KF pT, filtered/smoothed kappa
and variance, backward predicted/filtered kappa and variance, and truth
override provenance. fitted_log_loss*, local_log_loss* and smoothed_log_loss*
now always refer to RTS; backward_fitted_log_loss* refers to inward results.

Interval selection saves interval_selection_mode, interval_selection_status
(0 not attempted, 1 Manual, 2 Truth with selected loss, 3 Truth with no recorded
emission, 4 Truth with emissions but no positive eligible loss, -1 invalid event truth, -2 invalid
track association), interval_selection_error and selected_breakpoint_interval.
The effective indices are retained even if the subsequent fit fails. Truth
selection additionally saves interval_selection_truth_ebrem_loss (GeV),
_retained_fraction, _g4_track_id and _max_endpoint_distance (mm). The loss/z
vectors align with the selected intervals and are empty in Manual mode. A valid
empty truth list has status3 or4, not an error. Selection truth failure prevents
ordinary fitting; a later oracle-only failure still preserves ordinary tracks.

Default-on per-emission ownership branches are documented in
[truth-layer-ownership.md](docs/truth-layer-ownership.md). Counts distinguish
no unique material owner from a known owner with no usable reconstructed hit.

Persistent6D additionally stores persistent_hit_index and row-aligned
persistent_{predicted,filtered,smoothed}_mean (6 entries per hit), corresponding
_covariance (36 row-major entries), persistent_transport and
persistent_process_noise (36 entries each). These describe the final RTS
forward pass. The saved 6D birth process noise excludes the independent b
prior already present in its input P; the complete-score boundary construction
includes that prior once when crossing from 5D. Empty-list 5D reference
fits have empty persistent vectors. Enabling the extra truth pair
does not change these ordinary diagnostics. b permanently belongs to
the configured interval, never the current hit or native t0.

Covariance transport closure above 1e-3 fails the affected track. Passive
chi2-score errors are separately tagged. Flat output is default; commented
PodioOutput lines remain in the dedicated card for optional serialization.

## Independent batch workflow

Use the root scripts `subbreakpointjobs.sh` and `dump_breakpoint.sh`.
This workflow is independent of GSF and does not run its fitter.
The shared loss-prior sigma is controlled by BP_SIGMA_LOG_LOSS in
subbreakpointjobs.sh (default0.001). The batch helper defaults the shared
beam/charged/neutral constraints to `111`, with explicit `BP_*` overrides
supported. This is campaign steering only: standalone-card and compiled
constraint defaults remain OFF. Successful 111 jobs save unconstrained,
beam-only, beam+charged, and beam+charged+neutral results for ordinary/free-loss
RTS; all backward defaults remain OFF. Other fit physics remains in
options/run_breakpoint.py. Supported BP_* environment values are frozen at
preparation along with the complete card. The card consumes the submitted
sigma, with the same 0.001 fallback for direct standalone runs and in C++.
Every algorithm-specific Gaudi property and configurable track collection is
explicitly assigned in the maintained card; a source/card audit test checks coverage.
Prepared cards containing the retired likelihood-audit property must be
regenerated before use with the updated plugin; do not edit checksummed cards.
The default Truth selection chooses per-track locations from embedded Geant4
provenance. Auto reconstruction-based selection is not implemented. Manual
uses one configured list for every track; an empty Manual list is the baseline.

The submission helper defaults to `STAGES=breakpoint`, reading existing REC
inputs from INPUT_TUPLEPATH. To run the complete chain, explicitly select
`STAGES=sim,trk,calodigi,rec,breakpoint`. That chain freezes the shared
`DumpGsfTrks/sim.py.bk` together with the downstream cards at preparation.
No existing input tuple is needed for this chain. All outputs go into
OUTPUT_TUPLEPATH; SIM and REC are retained alongside the breakpoint flat tuple.
INPUT_TUPLEPATH is unused for this complete chain, but must still differ from
OUTPUT_TUPLEPATH. Existing outputs/cards are never overwritten.

Prepare a new full-chain campaign without submitting:

```bash
DRY_RUN=1 STAGES=sim,trk,calodigi,rec,breakpoint \
OUTPUT_TUPLEPATH=breakpoint_barrel_full_20261018 \
./subbreakpointjobs.sh
```

For partial chains, explicitly set STAGES. For example, prepare from existing
simulation files without submitting:

```bash
DRY_RUN=1 NEVT=200 SEED_FIRST=1 SEED_LAST=2 \
SAMPLE_REGION=barrel INPUT_TUPLEPATH=sim_large_barrel_20261001 \
OUTPUT_TUPLEPATH=breakpoint_barrel STAGES=trk,calodigi,rec,breakpoint \
./subbreakpointjobs.sh
```

After inspecting the generated cards, submit those exact prepared jobs:

```bash
./subbreakpointjobs.sh submit breakpoint_barrel
```

Omit DRY_RUN=1 on the first command to prepare and submit immediately. Do not
rerun preparation over the same sample/output directory; use `submit` after
a dry run. DRY_RUN=1 also works with `submit` to print commands only.
The scripts use IHEP `hep_sub -g GROUP -mem ... -argu JOB.json`.
Set `SUBMIT_GROUP` in `subbreakpointjobs.sh` (default `cms`), or override it:
`SUBMIT_GROUP=higgs ./subbreakpointjobs.sh`. It is read at submission time,
including `SUBMIT_GROUP=higgs ./subbreakpointjobs.sh submit OUTPUT_DIR` for
already-prepared jobs; cards/checksums do not change.
Scheduler stdout/stderr are preserved in each job's submitted.json.
The worker sources the project's `setup.sh` and launches `gaudirun.py` directly
for every stage, using the standard `InstallArea` runtime like a manual
`source setup.sh; ./run.sh CARD.py` run. It no longer selects a build-directory
launcher. Install verified builds before submission; rebuilding alone does not
update this runtime. Cards are frozen at preparation, but libraries are not.
Do not rebuild/install shared runtime libraries while campaign jobs are active.

| Control | Default | Meaning |
|---|---|---|
| STAGES | sim,trk,calodigi,rec,breakpoint | Any nonduplicated subset of these stages; physical order always used |
| INPUT_TUPLEPATH | sim_large_barrel_20261001 | Existing predecessor tuples, relative to repository or absolute |
| OUTPUT_TUPLEPATH | breakpoint_barrel | Results/cards/logs; existing files or run-card directories block preparation rather than being overwritten |
| NEVT | 200 | Maximum events per job |
| SEED_FIRST / SEED_LAST | 1 / 100 | Inclusive seed/file indices; generated sim/trk cards use that RNG seed |
| SAMPLE_REGION | barrel | `barrel` or `endcap`: use region-and-seed filenames; empty restores legacy naming |
| PARTICLES / THETAS / TRANSVERSE_MOMENTA | e- / 85 / 2.0 | Particle remains active; theta and pT are filename labels only when SAMPLE_REGION is empty |
| MEMORY_MB | 5000 | Scheduler memory request |
| DRY_RUN | 0 | 1 prepares/prints without calling scheduler |
| BP_SIGMA_LOG_LOSS | 0.001 | Finite positive prior sigma of b=-log(z), shared by ordinary, free-loss and truth-assisted fits |
| CEPCSW_BREAKPOINT_DIR | script directory | Project worktree |

For the default full barrel chain, simulation produces `sim-barrel-SEED.root`;
downstream outputs are `trk-barrel-SEED.root`, `calodigi-barrel-SEED.root`,
`rec-barrel-SEED.root`, and `breakpoint_flat-barrel-SEED.root`.
The breakpoint fitter reads **rec**, not trk. The retained rec event contains
`EcalCluster`, `CyberPFO`, digitized ECAL/HCAL hits, `CompleteTracks`, and
the Geant4 tracker-step and bremsstrahlung-photon path provenance. The tracker
steps are used by truth interval selection. ECAL
PFO PID, charged-track ECAL clusters, and event-level neutral ECAL clusters are
now copied into the flat tuple as passive diagnostics; they do not steer the
fit. The full REC event remains available for later external eBrem studies.
`submit` selects only prepared cards matching the current `SAMPLE_REGION`, so
old `e--2.0-85-*` manifests in the same directory are not resubmitted.
For existing reconstructed inputs, set STAGES=breakpoint and point
INPUT_TUPLEPATH at their `rec-barrel-SEED.root` directory. An external
`trk-barrel-SEED.root` can feed `STAGES=calodigi,rec,breakpoint` only if it
retains the simulated calorimeter hit collections. To read older
momentum/theta-named files, set `SAMPLE_REGION=''` and use the legacy labels;
the predecessor file still needs the corresponding stage-name prefix.
To regenerate the current barrel flat tuples with newly added passive photon
fields, keep the existing REC files and use a **new** output directory:

```bash
STAGES=breakpoint INPUT_TUPLEPATH=sim_large_barrel_20261001 \
OUTPUT_TUPLEPATH=breakpoint_barrel_photonlast_20261003 \
SAMPLE_REGION=barrel NEVT=200 SEED_FIRST=1 SEED_LAST=100 \
./subbreakpointjobs.sh
```

The helper skips missing REC seeds; it neither reruns simulation/reconstruction
nor overwrites the old `breakpoint_barrel/` flats. The new tuples, generated
cards, and logs go under the specified output directory. Use `DRY_RUN=1`
first if you want to inspect cards before submission; then submit those frozen
cards with:

```bash
./subbreakpointjobs.sh submit breakpoint_barrel_photonlast_20261003
```

When STAGES includes sim, SAMPLE_REGION also sets its gun
theta range (endcap 10--40 degrees, barrel 40--85 degrees); gun energy remains
configured in `DumpGsfTrks/sim.py.bk`. Examples of optional fit
overrides: `BP_INTERVAL_SELECTION_MODE=Truth BP_TRUTH_OVERRIDE=1`, or
`BP_INTERVAL_SELECTION_MODE=Manual BP_INTERVALS=5 BP_BACKWARD_SEED_SCALE=100`.
Interval5 here is only an example, NOT a recommended automatic truth interval.
All BP_* values supported by the dedicated card except its job I/O/event-count
fields are captured; BP_BACKWARD_MODE remains retired and is not supported.
Unset BP_SELECTED normally means all events; explicit selection is available
for isolated batch smoke tests. Job I/O and NEVT come from workflow controls.

Edit the BP_SIGMA_LOG_LOSS default in subbreakpointjobs.sh, or override it for
one preparation with BP_SIGMA_LOG_LOSS=0.01 ./subbreakpointjobs.sh. Changing this
value does not change already prepared cards: use a new output directory and
prepare new cards. The submit-existing command retains their frozen values.

During preparation, missing or zero-byte external predecessor inputs cause that
sample to be skipped, with its sample label and path printed. Other valid seeds
are still prepared/submitted; the final summary reports the skipped count.
This covers every omitted predecessor: sim for trk, trk for calodigi,
calodigi for rec, and rec for breakpoint. A predecessor generated in the same job needs no
existing input. If all samples are skipped, preparation exits with an error
without creating cards or submitting anything. Other validation errors (such
as existing outputs or template drift) still abort preparation before submission.

Output layout for each sample:

```text
OUTPUT_TUPLEPATH/
  breakpoint_flat-barrel-SEED.root
  rec-barrel-SEED.root                # retained ECAL reconstruction and tracker provenance
  calodigi-barrel-SEED.root           # removed after verified rec, if made in this job
  trk-barrel-SEED.root                # removed after verified calodigi, if made in this job
  sim-barrel-SEED.root                # only if sim selected; retained
  outlog/barrel-SEED.out, .err
  runcards/barrel-SEED/
    job.json, trk.py, calodigi.py, rec.py, breakpoint.py  # selected stages only
    submitted.json                  # successful scheduler submission
    started.json, completed.json    # worker lifecycle
```

The flat tuple contains ordinary RTS/backward and oracle/copied RTS/backward
results. No breakpoint EDM file is written by the default card. Once
`calodigi` passes ROOT/tree/collection verification, the worker removes its
own `trk` tuple. Once `rec` passes verification, it removes its own
`calodigi` tuple. It never removes an external input, the simulation tuple,
or the `rec` tuple; a stage without its immediate consumer keeps its output.
Breakpoint fit failures do not affect this cleanup because the durable `rec`
tuple has already passed verification, and failure tags remain in the flat
tuple. The cleanup checks the exact expected path and its production-time file
identity (device/inode/size/modification time), refuses symlinks or changed
files, and records both outcomes in completed.json under
`intermediate_cleanup`. Removed intermediates can be regenerated from the
retained simulation and frozen cards; they are not moved to trash. Existing
completed campaigns are not cleaned retroactively. A selected stage consumes a predecessor
made in the same job, otherwise an external predecessor from the input path.
Cards are checksum-checked by the worker; never edit a frozen card in place.
Use a new output directory for a changed physics setup. The software/library
is NOT snapshotted: keep the branch/build stable while jobs are queued/running.

The worker verifies readable nonempty ROOT trees and required downstream
collections before deleting an intermediate. It also verifies flat branches
and reports fit-success counts, but fit success is not a cleanup criterion.
Invalid oracle rows remain tagged; inspect truth_override_result_status before analysis. This output
check is not physics validation. Failed jobs retain outputs/started marker for
diagnosis and cannot blindly overwrite/restart; use a new output directory.
Duplicate submissions are rejected once submitted.json exists.

Simulation, tracker, calorimeter-digitization, and calorimeter-reconstruction
cards are read-only templates from DumpGsfTrks. Only generated copies receive
filenames, seed/event count and simulation particle. The generated tracker
card also reads simulated calorimeter hits so they survive into calodigi;
generated calodigi/rec cards carry the reconstructed tracker collections and
Geant4 material/primary-bremsstrahlung-photon provenance through to rec.
Shared GSF templates remain unchanged.
The hard-coded simulation energy/theta ranges are NOT changed by filename
labels; inspect sim.py.bk before selecting sim. The current breakpoint fitter
still assumes outward radius-ordered noncurling barrel tracks. The one-event
local smoke of the new four-stage chain verified a nonempty `EcalCluster` and
`CyberPFO`, retained CompleteTracks and G4 provenance, a successful breakpoint
flat output, and identical KF/RTS/free-loss/diffuse pT for the paired old/new
entry. A second one-event smoke with primary eBrem verified nonempty photon
and photon-step collections in rec (three photons and 1,606 path steps) and
again reproduced the old breakpoint pT. This is an I/O regression gate, not
ECAL-eBrem physics validation.

Earlier syntax/planning tests and local worker smoke results are recorded in
`agents_record/2026-09-09-recbreakpoint-independent-batch.md` (repository root).
No Condor submission was performed for the ECAL-flow change.

## Evidence and limits

The exact pre-change documentation is preserved in
[the outgoing README](../../agents_record/2026-09-09-recbreakpoint-readme-before-parallel.md).
Historical tests in the dated persistent6D, relinearization, backward-iteration
and TruthOverride records retain their original mode-specific meanings.
The paired-publication regression and complete-score gates are recorded in
[the parallel implementation record](../../agents_record/2026-09-09-recbreakpoint-parallel-endpoints-chi2.md).

No reconstruction-based interval discovery, positivity enforcement or exact
within-interval loss placement is implemented. Native material/mass
conventions remain unchanged. Successful execution, convergence or smaller
chi2 is not population validation or proof of better momentum reconstruction.
