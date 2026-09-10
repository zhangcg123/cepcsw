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
results when applied, exact ordinary copies otherwise. All six names must differ
from each other and the input. Each successful pair contains IP, first-hit and last-hit
states and the original ordered hits. Fixed input-row mappings are
`BreakpointOutputIndex` (RTS) and `BreakpointBackwardOutputIndex`.
`BreakpointStatus` is 1 for a successful pair, -1 for failed fit and 0 for
excluded input; absent outputs map to -1. Current fit failures fail the pair,
rather than publishing one branch under the other's name.

The flat tuple always records both `rts_pt` and `backward_pt`.
`breakpoint_pt` remains an RTS alias. The removed `backward_mode` branch no
longer misleadingly labels a row that now contains both results.

## Loss state and interval ownership

An interval i is radius-ordered `hit[i] -> hit[i+1]`, not a detector layer ID.
IntervalSelectionMode selects the source of the breakpoint list independently
of LossStateMode (how losses are fitted) and TruthOverride (extra truth-centered pair):

| IntervalSelectionMode | Behavior |
|---|---|
| Truth (compiled/card default) | Select every matched runtime hit interval with positive Geant4 eBrem loss, independently for each track |
| Manual | Use BreakpointIntervals exactly; [] is the no-breakpoint reference |
| Auto | Reserved reconstruction-based finder; initialization fails explicitly until implemented |

Truth follows reconstructed-hit associations and exact embedded G4 hooks. It
uses the existing post-step assignment on each (start,end] interval, selecting
`ebremLoss>0` without an additional loss threshold. Several emissions within
one interval yield one breakpoint. Loss before the first hit or after the last
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
FreeLoss pair with the native conditional fit at the optimized loss. The ordinary
RTS/backward pair and its tuple fields are never replaced. The
compatibility default is false. This is an implementation promotion, not a
claim of physics validation. See [Free-loss fitting](docs/free-loss-fit.md)
for the model, code organization, output contract and limitations.

| Property | Compiled default | Meaning |
|---|---|---|
| InputTracks | CompleteTracks | Input hit-list tracks |
| OutputTracks | BreakpointTracksRTS | RTS collection |
| OutputTracksBackwardFilter | BreakpointTracksBackwardFilter | Parallel inward-filter collection |
| OutputTracksFreeLossRTS | BreakpointTracksFreeLossRTS | Optimized RTS, or exact ordinary RTS copy |
| OutputTracksFreeLossBackwardFilter | BreakpointTracksFreeLossBackwardFilter | Optimized backward filter, or exact ordinary backward copy |
| OutputTracksTruthOverrideRTS | BreakpointTracksTruthOverrideRTS | Oracle RTS or ordinary RTS copy |
| OutputTracksTruthOverrideBackwardFilter | BreakpointTracksTruthOverrideBackwardFilter | Oracle backward or ordinary backward copy |
| TruthOverride | true | Extra pair uses truth b prior centers with SAME SigmaLogLoss/mode; otherwise copy ordinary pair |
| IntervalSelectionMode | Truth | Truth, Manual, or reserved/unimplemented Auto |
| BreakpointIntervals | [] | Manual-only radius-ordered hit intervals; must be empty outside Manual |
| MeanLogLoss | 0 | Ordinary Gaussian b-prior center, finite in [0,5]; oracle ignores it |
| SigmaLogLoss | 0.05 | Positive finite b-prior sigma shared by ordinary and truth-centered fits |
| LossStateMode | LocalMarginal | Ordinary pair: Persistent6D or LocalMarginal; TruthOverride is a separate bool |
| FreeLossFit | false | Optional normalized-likelihood optimization for one selected LocalMarginal interval; tagged ordinary fallback otherwise |
| FreeLossMaxLogLoss | 1 | Upper b bound, finite in (0,5]; lower bound is zero; default maximum fractional loss63.2121% |
| FreeLossMaxCallsPerStart | 180 | Positive maximum Minuit function calls per start; does not include the coarse/local scans |
| FreeLossTolerance | 0.001 | Positive finite MIGRAD tolerance |
| FreeLossCheckLikelihoods | false | Check reverse-order QR and joint-smoothed SVD against the same forward-order likelihood; no alternative objective |
| SeedScale | 1 | Positive finite scale of five loose seed variances |
| BackwardSeedScale | 100 | Positive finite scale of the full copied first-forward endpoint covariance; mean and RTS unchanged |
| SeedHitSelection | FirstMiddleLast | FirstMiddleLast or FirstThree |
| MaxChi2PerHit | 1e100 | Positive finite native update acceptance limit |
| MSOn | true | Native multiple-scattering noise |
| ElossOn | false | Native deterministic ionization correction |
| TruthDiagnostics | false | Generator-electron pT reference only |
| TruthMaxEndpointDistance | 5 | Positive finite mm validation tolerance on associated hooks |
| VerboseDump | false | Full state/covariance dumps |
| VerifyKFReference | false | Native reference checks for both endpoints |
| SelectedEventIndices | [] | Zero-based selected entries; empty means all |
| OutputFile | breakpoint_flat.root | New flat output file |

TruthDiagnostics is enabled by the card. It does not steer a fit. Truth interval
selection uses truth locations; only explicit TruthOverride sets truth loss prior centers.
Ambiguous generator electrons have NaN
truth pT. A scalar reference alone does not establish topology-clear selection.

The dedicated card exposes `fit.BackwardSeedScale` and optional environment
variable `BP_BACKWARD_SEED_SCALE` (default100). All 25 covariance entries are
scaled, preserving correlation coefficients; standard deviations scale by
sqrt(BackwardSeedScale). The effective value is saved in every flat row as
`backward_seed_scale`. This control does not create a fresh backward seed,
scale the independent breakpoint loss prior, or change the forward fit/RTS.

## Other automatic tuple information

All runs also save the `free_loss_*` fields described in
[the free-loss schema](docs/free-loss-fit.md#flat-tuple-contract). They are
optimizer diagnostics are inactive/NaN/empty when unused; the additional endpoint
fields instead copy the ordinary results exactly. Existing tuple fields and the
four earlier collections retain ordinary/truth-prior meanings regardless of
FreeLossFit. With free fitting applied, only the FreeLoss pair's
track covariances are conditional on optimized b; the outer optimizer's b
uncertainty is not propagated into them. `Track.chi2` retains its existing
quadratic meaning; `free_loss_nll2` separately records the fitting objective.

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

Use the new root scripts `subbreakpointjobs.sh` and `dump_breakpoint.sh`.
The existing `subtrkjobs.sh`, `dump_gsftrk.sh` and all GSF cards are unchanged.
The shared loss-prior sigma is controlled by BP_SIGMA_LOG_LOSS in
subbreakpointjobs.sh (default0.05); other fit physics remains in
options/run_breakpoint.py. Supported BP_* environment values are frozen at
preparation along with the complete card. The card consumes the submitted
sigma, retaining its 0.05 fallback only for direct standalone runs.
The default Truth selection chooses per-track locations from embedded Geant4
provenance. Auto reconstruction-based selection is not implemented. Manual
uses one configured list for every track; an empty Manual list is the baseline.

Prepare a campaign from existing simulation files, without submitting:

```bash
DRY_RUN=1 NEVT=200 SEED_FIRST=1 SEED_LAST=50 \
INPUT_TUPLEPATH=sim_large_barrel_20260823 \
OUTPUT_TUPLEPATH=breakpoint_campaign STAGES=trk,breakpoint \
./subbreakpointjobs.sh
```

After inspecting the generated cards, submit those exact prepared jobs:

```bash
./subbreakpointjobs.sh submit breakpoint_campaign
```

Omit DRY_RUN=1 on the first command to prepare and submit immediately. Do not
rerun preparation over the same sample/output directory; use `submit` after
a dry run. DRY_RUN=1 also works with `submit` to print commands only.
The scripts use the existing IHEP `hep_sub -g cms -mem ... -argu JOB.json`
convention. Scheduler stdout/stderr are preserved in each job's submitted.json.

| Control | Default | Meaning |
|---|---|---|
| STAGES | trk,breakpoint | Any nonduplicated subset of sim,trk,breakpoint; physical order always used |
| INPUT_TUPLEPATH | sim_large_barrel_20260823 | Existing predecessor tuples, relative to repository or absolute |
| OUTPUT_TUPLEPATH | breakpoint_barrel | New results/cards/logs; must differ from input |
| NEVT | 200 | Maximum events per job |
| SEED_FIRST / SEED_LAST | 1 / 50 | Inclusive seed/file indices; generated sim/trk cards use that RNG seed |
| PARTICLES / THETAS / TRANSVERSE_MOMENTA | e- / 85 / 2.0 | Comma-separated filename labels |
| MEMORY_MB | 5000 | Scheduler memory request |
| DRY_RUN | 0 | 1 prepares/prints without calling scheduler |
| BP_SIGMA_LOG_LOSS | 0.05 | Finite positive prior sigma of b=-log(z), shared by ordinary and truth-assisted fits |
| CEPCSW_BREAKPOINT_DIR | script directory | Project worktree |

For existing tracker inputs, set STAGES=breakpoint and point INPUT_TUPLEPATH
at their directory. Input names are `sim-e--2.0-85-SEED.root` or
`trk-e--2.0-85-SEED.root` with the selected labels. Examples of optional fit
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
This covers missing sim inputs for trk and missing tracker inputs for a
breakpoint-only stage. A predecessor generated in the same job needs no
existing input. If all samples are skipped, preparation exits with an error
without creating cards or submitting anything. Other validation errors (such
as existing outputs or template drift) still abort preparation before submission.

Output layout for each sample:

```text
OUTPUT_TUPLEPATH/
  breakpoint_flat-e--2.0-85-SEED.root
  trk-e--2.0-85-SEED.root             # intermediate; removed after verified breakpoint success
  sim-e--2.0-85-SEED.root             # only if sim selected; retained
  outlog/e--2.0-85-SEED.out, .err
  runcards/e--2.0-85-SEED/
    job.json, trk.py, breakpoint.py  # only selected stages have cards
    submitted.json                  # successful scheduler submission
    started.json, completed.json    # worker lifecycle
```

The flat tuple contains ordinary RTS/backward and oracle/copied RTS/backward
results. No breakpoint EDM file is written by the default card. After all
selected stages succeed and the flat tuple passes verification, a tracker
tuple produced by this same job is deleted regardless of individual ordinary
or truth-override fit failures, including when every fit fails. Failure tags
remain in the flat tuple. A trk-only job keeps its output, and a breakpoint-only
job never deletes its external tracker input. Simulation files are retained.
The cleanup checks the exact expected path and its production-time file identity
(device/inode/size/modification time), refuses symlinks or changed files, and
records the outcome in completed.json under tracker_cleanup. Removed tracker
tuples can be regenerated from the retained simulation and frozen trk card;
they are not moved to trash. Existing completed campaigns are not cleaned
retroactively. A selected stage consumes a predecessor
made in the same job, otherwise an external predecessor from the input path.
Cards are checksum-checked by the worker; never edit a frozen card in place.
Use a new output directory for a changed physics setup. The software/library
is NOT snapshotted: keep the branch/build stable while jobs are queued/running.

The worker verifies readable nonempty ROOT trees and required flat branches;
it reports fit-success counts but does not require successful fits for cleanup.
Invalid oracle rows remain tagged; inspect truth_override_result_status before analysis. This output
check is not physics validation. Failed jobs retain outputs/started marker for
diagnosis and cannot blindly overwrite/restart; use a new output directory.
Duplicate submissions are rejected once submitted.json exists.

Simulation and tracker cards are read-only templates from DumpGsfTrks. Only
generated copies receive filenames, seed/event count and simulation particle.
The hard-coded simulation energy/theta ranges are NOT changed by filename
labels; inspect sim.py.bk before selecting sim. The current breakpoint fitter
still assumes outward radius-ordered noncurling barrel tracks. Tracker truth
collections are preserved by the existing trk template's keep-all output.

Syntax/planning tests and local worker smoke results are recorded in
`agents_record/2026-09-09-recbreakpoint-independent-batch.md` (repository root).
No real Condor submission was performed for this change.

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
