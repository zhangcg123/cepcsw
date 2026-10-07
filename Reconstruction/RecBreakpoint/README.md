# RecBreakpoint

Experimental electron breakpoint Kalman refitter on branch `breakpoint`
(renamed from `test_breakpoint`).
Reads `CompleteTracks`. GSF/shared KF sources and maintained batch cards are
unchanged. This is not a physics-validated replacement.

For the complete code-checked mathematical workflow, read
[why Persistent6D and LocalMarginal can give the same result](docs/fixed-linearized-loss-walkthrough.md).
It defines every state/reference/deviation, derives the loss Jacobians A and
g, and follows uncertainty, measurement updates and RTS loss recovery in one
self-contained explanation.

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
match the input. `BackwardMode` is removed: both results are always produced.
The card rejects the retired `BP_BACKWARD_MODE` variable instead of ignoring it.

## Always-paired endpoint workflow

```text
Shared first-pass forward filter
  +--> RTS smoothing           --> BreakpointTracksRTS
  +--> backward hit refilter   --> BreakpointTracksBackwardFilter
```

These are separate results, not CPU threads. Backward starts from the full
first-pass forward endpoint mean and its full covariance multiplied by
BackwardSeedScale (default100), revisits hits N-2 through 0, and uses
material-aware native IP propagation. RTS never consumes the backward-refiltered
states. Both endpoints now use the SAME KalmanAdapter::propagateToIP operation:
initialize from the innermost endpoint and call native MarlinTrk propagation
to (0,0,0), honoring the existing MSOn/ElossOn settings. This applies to ordinary,
free-loss and truth-prior results in both LocalMarginal and Persistent6D modes.
The former purely geometric RTS-only atIP helper has been removed. No beam-spot
measurement or extra breakpoint is introduced by this common IP operation.
Backward still reuses forward hit evidence; it is NOT an independent Bayesian
smoother. Neither branch adds a beam-to-first-hit breakpoint.

The existing `OutputTracks` property now names the RTS collection (default
`BreakpointTracksRTS`); `OutputTracksBackwardFilter` names the other collection.
Two additional collections are always available:
`BreakpointTracksTruthOverrideRTS` and `BreakpointTracksTruthOverrideBackwardFilter`,
named by `OutputTracksTruthOverrideRTS` and `OutputTracksTruthOverrideBackwardFilter`.
The free-loss pair is also always available: `BreakpointTracksFreeLossRTS` and
`BreakpointTracksFreeLossBackwardFilter`, controlled by `OutputTracksFreeLossRTS`
and `OutputTracksFreeLossBackwardFilter`. It contains independently optimized
results when applied, exact ordinary copies otherwise. A second free-loss pair,
`BreakpointTracksBeamGuidedFreeLossRTS` and
`BreakpointTracksBeamGuidedFreeLossBackwardFilter`, optimizes an additional
beam-origin likelihood but uses exactly the same detector-hit KF/RTS refit.
`FreeLossBeamSpotObjective` controls this pair; when disabled it copies the
base free-loss result. A ninth, independent
`BreakpointTracksDiffuseAugmentedRTS` collection is controlled by
`DiffuseAugmentedRTS` (compiled default off; maintained-card default on).
The tenth, `BreakpointTracksAbsoluteNeutralRTS`, is one default-off ECAL
absolute-loss KF/RTS experiment controlled by `EcalLossReferenceMode`. It requires exactly one
selected breakpoint interval and a reconstructed hit-supported neutral ECAL
cluster near the input track's ECAL direction. Its sixth state coordinate is
the absolute loss `L = p_before - p_after` in GeV. Selected cluster energy and
its estimated error initialize `L` once; no separate ECAL measurement update
is made. It uses the existing 6D Kalman/RTS path with an absolute-loss birth
map, leaving all log-loss outputs intact. When disabled or no cluster/interval
qualifies, its collection holds an ordinary RTS copy; on fit failure it also
copies ordinary RTS and records the error. No truth enters cluster selection.
`absolute_neutral_status` is 0 for disabled, 1 for an ordinary copy, 2 for a
fitted absolute-loss result, -1 for fit failure with ordinary copy, and -2
when the ordinary fit failed. The flat row also stores the selected ECAL
cluster indices, prior energy/error, smoothed loss/variance, IP covariance,
and 6D smoothed states. This is a single-pass extended-KF prototype;
nonlinear loss mapping can be biased when the pre-break forward state is far
from the true momentum, so a successful status is not physics validation.
The one controller has four values, with no separate enable/reference switches:

| EcalLossReferenceMode | Loss-map reference for the same absolute-energy fitter |
|---|---|
| Off (compiled and card default) | No ECAL refit; publish an ordinary RTS copy |
| NoReference | Evaluate at the live forward state at hit i; no diffuse reference is supplied |
| PreReference | Use the diffuse smoothed state at hit i |
| PostReference | Start from that same state at i; replace curvature using p_diffuse(i+1) + E_neutral and the charge sign at i+1 |

Set `fit.EcalLossReferenceMode` in the maintained card, or export
`BP_ECAL_LOSS_REFERENCE_MODE` for standalone/batch use. The batch helper freezes
this choice into the generated card. PreReference and PostReference require
`DiffuseAugmentedRTS=true`; NoReference can run without the diffuse fit.
All three active choices share cluster selection, ECAL energy/error initialization,
the six-dimensional forward filter, RTS smoothing, output and fallback code.
Only the reference used for **the breakpoint loss mapping and its Jacobian** differs.
For either diffuse-reference choice, the original forward track mean is transported with
the affine offset
`f(reference) + J(reference)*(live-reference)`; it is not replaced by diffuse.
The original seed, live covariance, and ECAL `L` prior/error remain unchanged.
The diffuse covariance is explicitly discarded. Geometry and measurement
linearizations still follow the new live forward pass: this is a single
loss-map reference test, not a whole-trajectory iteration. Unavailable or
unphysical references trigger the existing status -1 ordinary-copy fallback.
The flat row records reference requested/used flags, the five upstream
reference parameters (`drho,phi0,kappa,dz,tanLambda`, kappa=1/pT), and
`absolute_neutral_forward_p_before`/`absolute_neutral_reference_p_before` in
GeV. The used flag is true only for a successful published referenced fit;
on failure the attempted reference can still be recorded.
PostReference reads the diffuse smoothed state at hit i+1; it does not propagate
that reference backward to i. The loss map itself is still applied at the
upstream pivot i, before native propagation to i+1. Its resulting before-loss
reference momentum is `p_diffuse(i+1) + E_neutral`; ECAL energy supplies the same
independent `L` prior/error as in the other two modes. It is not an additional
ECAL measurement. The tuple records the selected controller value in
`absolute_neutral_reference_source`, and the diffuse downstream reference in
`absolute_neutral_reference_p_after`, to make the choice auditable. An
unavailable or invalid post-break reference produces status -1 and an
ordinary-copy fallback. This option is a controlled one-pass test, not a
full-trajectory relinearization or a physics-validated correction.
The former three properties `AbsoluteNeutralLossRTS`,
`AbsoluteNeutralDiffuseReference`, and `AbsoluteNeutralReferenceSource` are
removed. Their BP_ABSOLUTE_NEUTRAL_* environment controls now fail explicitly.
The old ECAL-to-log-loss-prior recipe is retired; its numerical results and
historical diagnostic artifacts remain evidence in dated records. The ordinary,
free-loss and truth-prior fits still use their own MeanLogLoss/SigmaLogLoss
machinery; it is not part of the ECAL refit controller.

The main `breakpoint` tree also always saves the neutral-cluster selection
**for its own input track**, even with `EcalLossReferenceMode=Off`, no selected
breakpoint, or a failed tracking fit. The collector is called once per track
before fitting; an enabled ECAL refit reuses that same result.

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
All ten names must differ
from each other and the input. Each successful result contains IP, first-hit,
last-hit, and, when native extrapolation succeeds, ECAL-face (`AtCalorimeter`)
states and the original ordered hits. The ECAL-face state starts from that
result's own outermost fitted endpoint; it is not copied from `CompleteTracks`
and the last hit is not updated a second time. The same native barrel/endcap
layer propagation and nearest-face choice used by the baseline are applied.
Failure of this extra extrapolation leaves the IP/hit fit intact. Fixed
input-row mappings are
`BreakpointOutputIndex` (RTS) and `BreakpointBackwardOutputIndex`.
`BreakpointStatus` is 1 for a successful pair, -1 for failed fit and 0 for
excluded input; absent outputs map to -1. Current fit failures fail the pair,
rather than publishing one branch under the other's name.

The flat tuple always records both `rts_pt` and `backward_pt`.
`breakpoint_pt` remains an RTS alias. Every IP momentum endpoint has a
matching `*_pt_error` one-sigma field in GeV: `kf`, `breakpoint`/`rts`,
`backward`, `free_loss_rts`, `free_loss_backward`,
`beam_guided_free_loss_rts`, `beam_guided_free_loss_backward`,
`truth_override_rts`, `truth_override_backward`, and `diffuse_augmented`.
The optional `reference_kf` and `reference_backward_kf` checks have the same
field. These are fit-covariance errors, not empirical resolution estimates.
For an EDM endpoint with curvature `omega` and packed covariance element
`covMatrix[5]=Var(omega)`, the stored value is
`sigma(pT) = pT*sqrt(Var(omega))/abs(omega)`. Missing or invalid endpoint
covariance yields NaN. The removed `backward_mode` branch no
longer misleadingly labels a row that now contains both results.

The calorimeter state is saved in the flat tuple for the input KF and all nine
endpoint families, using prefixes `kf_`, `rts_`, `backward_`,
`free_loss_rts_`, `free_loss_backward_`, `beam_guided_free_loss_rts_`,
`beam_guided_free_loss_backward_`, `truth_override_rts_`,
`truth_override_backward_`, and `diffuse_augmented_rts_`. Each prefix has
`calo_status`, `calo_error`, `calo_pt`, `calo_pt_error`, `calo_p`, `calo_parameters`,
`calo_covariance`, and `calo_reference_point_mm`. Parameters are the EDM
`(D0, phi, omega, Z0, tanLambda)` at the ECAL face; covariance is the packed
21-entry EDM covariance. Status 1 means native extrapolation succeeded, 2
means an input-KF state was copied (including a fit fallback), 0 means no
output, and -1 means the state was unavailable or extrapolation failed.
`calo_pt_error` uses that family's propagated calorimeter-state covariance
and the same curvature formula. It does not include any extra uncertainty
from the calorimeter shower. Unavailable numerical fields are NaN/empty. The default card writes only
the flat tuple; add the optional `PodioOutput` from the card to serialize the
nine EDM track collections themselves.

## Loss state and interval ownership

Both `LocalMarginal` and `Persistent6D` use the Gaussian loss prior configured
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
| Truth (compiled/card default) | Select only the matched runtime interval with the largest summed absolute Geant4 eBrem momentum loss; ties choose innermost |
| Manual | Use BreakpointIntervals exactly; [] is the no-breakpoint reference |
| Auto | Reserved reconstruction-based finder; initialization fails explicitly until implemented |

Truth follows reconstructed-hit associations and exact embedded G4 hooks. It
uses the existing post-step assignment on each (start,end] interval, selecting
the maximum positive `ebremLoss` without an additional loss threshold. Several emissions within
one interval are summed. Other intervals' losses are not fitted. Loss before the first hit or after the last
hit is not covered. Interval bounds and upstream placement are unchanged.
The ordinary fit receives only the indices: its MeanLogLoss/SigmaLogLoss prior
is NOT replaced by truth. TruthOverride must separately be enabled to set loss
prior centers in the additional pair. All Geant4 metadata remain diagnostic apart
from this explicit location selection and the optional loss override.

Truth with no matched eBrem selects an empty list and runs the ordinary 5D
reference, with copied extra outputs. Invalid truth fails the affected track
with interval-selection status/error; it never means a guessed empty list.
A nonempty BreakpointIntervals list is rejected outside Manual mode, so it
cannot silently be ignored. Dedicated-card control: BP_INTERVAL_SELECTION_MODE.
The card loads truth collections for Truth selection even if TruthOverride=False.
Previously prepared cards predate this control/default change: regenerate them
in a new output directory. Explicit fixed-list comparisons must now set Manual;
do not patch a checksum-protected prepared card in place.

Multiple independent
intervals are supported by LocalMarginal and the additional truth pair; Persistent6D
accepts at most one. Negative or duplicate entries fail initialization;
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

### Persistent6D

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

### LocalMarginal (compiled/card default)

The live helix stays 5D. At a selected edge an independent Gaussian b is
marginalized into the helix covariance. Retained joint cross covariances allow
later conditioning of b, including RTS or subsequent backward measurements.
The Gaussian birth Jacobian has derivative kappa_after outward and
-kappa_before inward. Independent local losses allow multiple breakpoints.
RTS covariance uses a positive-sum conditional form to avoid subtraction of
large loose-seed covariances.

### TruthOverride

The default diagnostic chain is Truth interval selection + LocalMarginal +
TruthOverride=True. The effective interval list is shared by both pairs:

```text
select intervals for this track
  -> ordinary RTS/backward: fit losses using MeanLogLoss/SigmaLogLoss priors
  -> extra RTS/backward: use matched Geant4 b as the prior center, SAME SigmaLogLoss
```

Both pairs use the same fitting code, LossStateMode and SigmaLogLoss
controls. Truth changes only the prior center of each selected b; later hits
can update b and its variance. Track covariance, native material/MS and
measurement updates remain active. Manual
selection shares its supplied list in exactly the same way. TruthOverride does
not discover additional intervals. With an empty effective list the extra pair
is copied (result status1), even though the switch is on. With a nonempty list,
successful oracle results have status2. Auto remains an initialization error.

```python
fit.IntervalSelectionMode = "Truth"  # select locations for this track
fit.LossStateMode = "LocalMarginal"   # ordinary pair fits the loss
fit.TruthOverride = True           # additional oracle pair; compiled/card default True
fit.BreakpointIntervals = []  # Manual-only; Truth builds the effective list
# Six endpoints: ordinary, truth-override and free-loss RTS/backward pairs.
# With FreeLossFit=False the free-loss pair copies the ordinary pair.
```

The additional prior centers come from embedded event truth:
reconstructed hit -> MCRecoTrackerAssociation -> SimTrackerHit -> exact
Geant4 step/fraction hook. All ordered hits require unambiguous monotonic
hooks on one primary electron. TruthMaxEndpointDistance validates associated
positions, never chooses a nearest hit.

For each hook-to-hook interval (start,end], eBrem process-subtype 3 momentum
losses are assigned by their post-step points and summed:
`z=1-sum(delta_p_ebrem)/p_at_start_hook`, `b=-log(z)`. This excludes ionization.
Only MeanLogLoss is replaced per interval for the extra pair. SigmaLogLoss is
identical in both pairs, and the extra fitted b need not equal truth. A Manual
interval with no eBrem gets prior center b=0, not a permanently fixed zero loss.
The fitter accepts generic per-interval prior centers; it has no separate
TruthOverride loss-state mode or truth-specific update. Native material, MS,
ionization steering, seeds and upstream loss placement remain unchanged.

The card conditionally loads the six tracker association/SimTrackerHit sets,
GsfG4MaterialSteps and GsfSimTrackerHitG4StepLinks. It uses no side CSV/ROOT
helper or GSF execution. The module reuses unchanged TruthBHLossEventData.cpp
from RecGsfTracking and the existing datamodel libraries. Invalid truth fails
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
feeds back into the ordinary pair. Multi-interval ordinary comparisons require
LocalMarginal. The old public `LossStateMode="TruthOverride"` now fails clearly:
select an ordinary LossStateMode and set the independent `TruthOverride` bool.
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

Both loss-state modes run one forward filter, one RTS smoother and one
backward refilter per pair. There is no repeated relinearization loop.
LocalMarginal supports multiple intervals; Persistent6D supports at most one.
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
FreeLoss pair with the shared Gaussian fitter at the optimized prior center. The ordinary
RTS/backward pair and its tuple fields are never replaced. The
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
The parallel beam-guided optimization adds one predictive term,
`drho_beam^2/(P_drho_beam + sigma_beam^2) + log(P_drho_beam + sigma_beam^2)
+ log(2*pi)`, evaluated from each trial's hit-only RTS IP state at the beam
mean. `sigma_beam^2 = cos(phi0)^2*sigma_x^2 + sin(phi0)^2*sigma_y^2`.
The beam is never passed to the Kalman measurement update; it selects a
different fitted loss-prior center. Thus its published track parameters and
covariance are detector-hit refits conditional on that selected center, not
beam-constrained states. This experimental objective is default-on in the
maintained card, but not physics-validated.

| Property | Compiled default | Meaning |
|---|---|---|
| InputTracks | CompleteTracks | Input hit-list tracks |
| OutputTracks | BreakpointTracksRTS | RTS collection |
| OutputTracksBackwardFilter | BreakpointTracksBackwardFilter | Parallel inward-filter collection |
| OutputTracksFreeLossRTS | BreakpointTracksFreeLossRTS | Optimized RTS, or exact ordinary RTS copy |
| OutputTracksFreeLossBackwardFilter | BreakpointTracksFreeLossBackwardFilter | Optimized backward filter, or exact ordinary backward copy |
| OutputTracksBeamGuidedFreeLossRTS | BreakpointTracksBeamGuidedFreeLossRTS | Beam-objective free-loss RTS; base free-loss copy when disabled, input KF fallback on optimizer failure |
| OutputTracksBeamGuidedFreeLossBackwardFilter | BreakpointTracksBeamGuidedFreeLossBackwardFilter | Matching beam-objective backward endpoint |
| OutputTracksTruthOverrideRTS | BreakpointTracksTruthOverrideRTS | Oracle RTS or ordinary RTS copy |
| OutputTracksTruthOverrideBackwardFilter | BreakpointTracksTruthOverrideBackwardFilter | Oracle backward or ordinary backward copy |
| OutputTracksDiffuseAugmentedRTS | BreakpointTracksDiffuseAugmentedRTS | Exact-diffuse augmented RTS or ordinary copy; input KF fallback on failure |
| OutputTracksAbsoluteNeutralRTS | BreakpointTracksAbsoluteNeutralRTS | Absolute neutral-energy-loss RTS, or ordinary RTS copy |
| TruthOverride | true | Extra pair uses truth b prior centers with SAME SigmaLogLoss/mode; otherwise copy ordinary pair |
| IntervalSelectionMode | Truth | Truth, Manual, or reserved/unimplemented Auto |
| BreakpointIntervals | [] | Manual-only radius-ordered hit intervals; must be empty outside Manual |
| MeanLogLoss | 0 | Finite in [0,5]: ordinary Gaussian prior center; truth-prior pair uses matched truth centers instead |
| SigmaLogLoss | 0.001 | Positive finite Gaussian-prior sigma shared by ordinary, free-loss and truth-prior fits; retained in every optimizer trial and final refit |
| LossStateMode | LocalMarginal | Ordinary pair: Persistent6D or LocalMarginal; TruthOverride is a separate bool |
| DiffuseAugmentedRTS | false | Independent one-interval, flat-prior 6D KF/RTS fit; maintained-card default true (`BP_DIFFUSE_AUGMENTED_RTS=0` disables it); ignores SigmaLogLoss for this extra fit |
| EcalLossReferenceMode | Off | One absolute-loss KF/RTS: Off, NoReference (live forward reference), PreReference (diffuse state i), PostReference (curvature from diffuse momentum i+1 plus ECAL); last two require DiffuseAugmentedRTS; card and batch environment BP_ECAL_LOSS_REFERENCE_MODE |
| NeutralLossThetaWindowMrad, NeutralLossPhiWindowMrad | 10, 200 | Positive angular half-windows about input `AtCalorimeter` direction for reconstructed neutral-cluster selection |
| NeutralLossStochasticError, NeutralLossConstantError | 0.011, 0.004 | Provisional per-cluster `sigma_E = a sqrt(E/GeV) GeV + c E`; selected cluster variances add independently |
| FreeLossFit | false | Card default true; normalized-likelihood optimization of the Gaussian loss-prior center for one LocalMarginal interval; input KF fallback on failure/unsupported mode |
| FreeLossBeamSpotObjective | true | When FreeLossFit is active, run an independent beam-guided free-loss optimization in parallel; when false, copy base free-loss outputs |
| BeamSpotX, BeamSpotY | 0 mm, 0 mm | Beam mean for the objective-only virtual measurement |
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
[the free-loss schema](docs/free-loss-fit.md#flat-tuple-contract). The
`beam_guided_free_loss_*` fields use the same endpoint and optimizer schema
for the independent beam-guided result. Its `nll2` and
`beam_guided_free_loss_likelihood_nll2` are detector-hit likelihoods;
`beam_guided_free_loss_objective_nll2` is their sum with
`beam_guided_free_loss_beam_nll2`. Trial-level hit, beam, and total scores are
saved separately. When the option is off, its endpoints copy the base free-loss
pair; when its optimization fails, only this second pair falls back to the
input KF. The base `free_loss_*` result never changes because of this switch.
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
`beam_guided_free_loss_likelihood_*`, and `truth_override_likelihood_*`.
Each prefix has `status` (1 valid, 0 absent,
-1 unavailable or evaluation failed), `nll2`, `quadratic`, `logdet`,
`measurement_dimensions`, `latent_dimensions`, and `error`. All use
`nll2 = quadratic + logdet + measurement_dimensions * log(2*pi)`, with the
complete RTS measurement + process + seed score as `quadratic`. Free-loss
optimized rows copy the accepted optimizer objective (also retained under
`free_loss_nll2`); disabled/empty-interval free and truth rows copy the
ordinary likelihood. A free-loss input-KF fallback has no breakpoint
likelihood. Likelihood failures are passive and do not discard valid tracks.
Both LocalMarginal and Persistent6D RTS fits capture the required Gaussian
model. Older tuples lack these new fields and must be regenerated.

The tuple has one row per attempted track, including failures. It retains
ordered hit cells/radii/z, truth/KF pT, reference KF pT, filtered/smoothed kappa
and variance, backward predicted/filtered kappa and variance, and truth
override provenance. fitted_log_loss*, local_log_loss* and smoothed_log_loss*
now always refer to RTS; backward_fitted_log_loss* refers to inward results.

Interval selection saves interval_selection_mode, interval_selection_status
(0 not attempted, 1 Manual, 2 valid Truth, -1 invalid event truth, -2 invalid
track association), interval_selection_error and selected_breakpoint_interval.
The effective indices are retained even if the subsequent fit fails. Truth
selection additionally saves interval_selection_truth_ebrem_loss (GeV),
_retained_fraction, _g4_track_id and _max_endpoint_distance (mm). The loss/z
vectors align with the selected intervals and are empty in Manual mode. A valid
empty truth list has status2, not an error. Selection truth failure prevents
ordinary fitting; a later oracle-only failure still preserves ordinary tracks.

Persistent6D additionally stores persistent_hit_index and row-aligned
persistent_{predicted,filtered,smoothed}_mean (6 entries per hit), corresponding
_covariance (36 row-major entries), persistent_transport and
persistent_process_noise (36 entries each). These describe the final RTS
forward pass. The saved 6D birth process noise excludes the independent b
prior already present in its input P; the complete-score boundary construction
includes that prior once when crossing from 5D. Empty-list and LocalMarginal
ordinary fits have empty persistent vectors. Enabling the extra truth pair
does not change these ordinary diagnostics. b permanently belongs to
the configured interval, never the current hit or native t0.

Covariance transport closure above 1e-3 fails the affected track. Passive
chi2-score errors are separately tagged. Flat output is default; commented
PodioOutput lines remain in the dedicated card for optional serialization.

## Independent batch workflow

Use the root scripts `subbreakpointjobs.sh` and `dump_breakpoint.sh`.
This workflow is independent of GSF and does not run its fitter.
The shared loss-prior sigma is controlled by BP_SIGMA_LOG_LOSS in
subbreakpointjobs.sh (default0.001); other fit physics remains in
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

Prepare a campaign from existing simulation files, without submitting:

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
The scripts use the existing IHEP `hep_sub -g cms -mem ... -argu JOB.json`
convention. Scheduler stdout/stderr are preserved in each job's submitted.json.

| Control | Default | Meaning |
|---|---|---|
| STAGES | breakpoint | Any nonduplicated subset of sim,trk,calodigi,rec,breakpoint; physical order always used |
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

For the current barrel sample, the input is `sim-barrel-SEED.root`; generated
outputs are `trk-barrel-SEED.root`, `calodigi-barrel-SEED.root`,
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

No reconstruction-based interval discovery, positivity enforcement, beam spot or exact
within-interval loss placement is implemented. Native material/mass
conventions remain unchanged. Successful execution, convergence or smaller
chi2 is not population validation or proof of better momentum reconstruction.
