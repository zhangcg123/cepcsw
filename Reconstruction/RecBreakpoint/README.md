# RecBreakpoint

Experimental electron breakpoint Kalman refitter on local `test_breakpoint`.
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
BP_INTERVALS=5 BP_EVENTS=18 BP_SELECTED=11,16,17 BP_VERBOSE=1 \
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
BackwardSeedScale (default1), revisits hits N-2 through 0, and uses
material-aware native IP propagation. RTS never consumes
the backward-refiltered states and retains its geometric IP extrapolation.
Backward still reuses forward hit evidence; it is NOT an independent Bayesian
smoother. Neither branch adds a beam-to-first-hit breakpoint.

The existing `OutputTracks` property now names the RTS collection (default
`BreakpointTracksRTS`); `OutputTracksBackwardFilter` names the other collection.
Two additional collections are always available:
`BreakpointTracksTruthOverrideRTS` and `BreakpointTracksTruthOverrideBackwardFilter`,
named by `OutputTracksTruthOverrideRTS` and `OutputTracksTruthOverrideBackwardFilter`.
All four names must differ. Each successful pair contains IP, first-hit and last-hit
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
`BreakpointIntervals=[]` is the no-breakpoint reference. Multiple independent
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
position inside the interval. No BH mixture or automatic truth-based interval
selection is performed.

### Persistent6D (default)

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

### LocalMarginal

The live helix stays 5D. At a selected edge an independent Gaussian b is
marginalized into the helix covariance. Retained joint cross covariances allow
later conditioning of b, including RTS or subsequent backward measurements.
The Gaussian birth Jacobian has derivative kappa_after outward and
-kappa_before inward. Independent local losses allow multiple breakpoints.
RTS covariance uses a positive-sum conditional form to avoid subtraction of
large loose-seed covariances.

### TruthOverride

```python
fit.LossStateMode = "Persistent6D"  # ordinary pair; LocalMarginal is also supported
fit.TruthOverride = True           # additional oracle pair; compiled/card default False
fit.BreakpointIntervals = [5]
# Four endpoints: ordinary RTS/backward, truth-override RTS/backward.
```

Configured losses are fixed from embedded event truth, not guessed or fitted:
reconstructed hit -> MCRecoTrackerAssociation -> SimTrackerHit -> exact
Geant4 step/fraction hook. All ordered hits require unambiguous monotonic
hooks on one primary electron. TruthMaxEndpointDistance validates associated
positions, never chooses a nearest hit.

For each hook-to-hook interval (start,end], eBrem process-subtype 3 momentum
losses are assigned by their post-step points and summed:
`z=1-sum(delta_p_ebrem)/p_at_start_hook`, `b=-log(z)`. This excludes ionization.
Each selected b has zero added variance; MeanLogLoss/SigmaLogLoss are ignored
only for the oracle pair, and continue to steer the ordinary pair.
No singular live 6D fixed-loss covariance is created. Native material, MS,
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
ordinary pair (including covariances, hits and chi2); it does not rerun a fit or
access material truth. An empty interval list also copies the ordinary pair.
With True and nonempty intervals, a separate one-pass fixed-loss fit generates
both oracle endpoints with the same seed scales, hit selection and material
settings. The ordinary pair may still use MaxFitIterations>1. No oracle state
feeds back into the ordinary pair. Multi-interval ordinary comparisons require
LocalMarginal. The old public `LossStateMode="TruthOverride"` now fails clearly:
select an ordinary LossStateMode and set the independent `TruthOverride` bool.
The internal fitter retains its fixed-loss implementation under the old name.
Dedicated-card environment control: `BP_TRUTH_OVERRIDE=1` (default 0).

`truth_override_result_status` and the input-row-aligned EDM
`BreakpointTruthOverrideStatus` distinguish: 0 absent/not attempted, 1 copied,
2 oracle success, -1/-2/-3 truth input/match/interval failure, -4 oracle fit failure.
Index maps are `BreakpointTruthOverrideRTSIndex` and
`BreakpointTruthOverrideBackwardIndex`; missing outputs map to -1. Check this
result status rather than assuming that a truth-named branch used truth.
Flat branches `truth_override_rts_pt`, `truth_override_backward_pt`,
`truth_override_{forward,backward,smoothed}_chi2` and their `_local_chi2` vectors
save the extra pair's results. The smoothed score also has status/error fields.
`truth_override_{rts,backward}_ip_parameters` hold `(D0,phi,omega,Z0,tanLambda)`
at the origin; `_ip_covariance` copies all 21 packed EDM covariance elements.
The first 15 describe the fitted five-parameter helix; the trailing time
row/column is carried from EDM and is not a fitted loss coordinate.
Failed extra results have NaN scalar results and empty vectors, not copies.
An ordinary-pair failure currently leaves the extra pair unattempted (status 0).

Truth vectors retain interval, retained_fraction, log_loss, momentum_before,
ebrem_loss, tx0, first_step/last_step and start_fraction/end_fraction under the
truth_override_ prefix; scalar error, G4 track ID and max endpoint distance are
also saved. Truth t/X0 is passive. An empty interval list needs no loss truth.

## Iterations and seeds

MaxFitIterations=1 preserves one-pass results. For >1 (up to 20), exactly one
ordinary breakpoint is required; TruthOverride never iterates.
Persistent6D iterates RTS and backward separately. LocalMarginal retains its
one-pass RTS and iterates backward only. This preserves the established
methods rather than adding unimplemented LocalMarginal RTS relinearization.

Ordinary RTS starts at the final forward updated state and covariance, then
uses buffered forward predictions, covariances and transitions to smooth
inward. It does not initialize an independent filter or update the hit
measurements a second time. BackwardSeedScale never enters RTS. Only explicitly
enabled Persistent6D relinearization repeats the forward-fit/RTS cycle.

When iterating, the forward/RTS cycle relinearizes native F/Q and measurement
derivatives around the preceding smoothed trajectory, retaining the original
seed and b prior. Backward
iterations relinearize only the inward path, freezing the original FIRST-pass
forward endpoint seed; they do not use the final iterated RTS endpoint.
The same BackwardSeedScale multiplies that frozen covariance once on each
inward pass, never repeatedly across iterations. They also retain the original
b prior. No previous posterior becomes a new independent prior. Each hit is
updated once per newly solved branch pass.

Convergence is maximum standardized change in endpoint coordinates and b
below RelinearizationTolerance. It is not proof of an optimum. There is no
line search, damping, positivity constraint or loss-position adjustment.
Iteration status is 0 one pass, 1 converged, 2 limit, -1 failed extra pass
(last completed result retained). Both branches have separate histories:
RTS retains iteration_* and fit_iterations; backward adds backward_iteration_*
and backward_fit_iterations. backward_seed_forward_chi2 identifies its
unchanged first-pass forward bookkeeping, which may differ from the final
RTS forward_chi2 when RTS iterates.

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

| Property | Compiled default | Meaning |
|---|---|---|
| InputTracks | CompleteTracks | Input hit-list tracks |
| OutputTracks | BreakpointTracksRTS | RTS collection |
| OutputTracksBackwardFilter | BreakpointTracksBackwardFilter | Parallel inward-filter collection |
| OutputTracksTruthOverrideRTS | BreakpointTracksTruthOverrideRTS | Oracle RTS or ordinary RTS copy |
| OutputTracksTruthOverrideBackwardFilter | BreakpointTracksTruthOverrideBackwardFilter | Oracle backward or ordinary backward copy |
| TruthOverride | false | Add fixed-truth-loss pair when true; otherwise copy ordinary pair |
| BreakpointIntervals | [] | Selected radius-ordered hit intervals |
| MeanLogLoss | 0 | Ordinary Gaussian b-prior center, finite in [0,5]; oracle ignores it |
| SigmaLogLoss | 0.05 | Ordinary positive finite b-prior sigma; oracle ignores it |
| LossStateMode | Persistent6D | Ordinary pair: Persistent6D or LocalMarginal; TruthOverride is a separate bool |
| MaxFitIterations | 1 | 1--20; ordinary single-interval iterations as described above |
| RelinearizationTolerance | 0.001 | Positive finite standardized stopping threshold |
| SeedScale | 1 | Positive finite scale of five loose seed variances |
| BackwardSeedScale | 1 | Positive finite scale of the full copied first-forward endpoint covariance; mean and RTS unchanged |
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

TruthDiagnostics is enabled by the card. It does not steer a fit; only explicit
TruthOverride uses material-loss truth. Ambiguous generator electrons have NaN
truth pT. A scalar reference alone does not establish topology-clear selection.

The dedicated card exposes `fit.BackwardSeedScale` and optional environment
variable `BP_BACKWARD_SEED_SCALE` (default1). All 25 covariance entries are
scaled, preserving correlation coefficients; standard deviations scale by
sqrt(BackwardSeedScale). The effective value is saved in every flat row as
`backward_seed_scale`. This control does not create a fresh backward seed,
scale the independent breakpoint loss prior, or change the forward fit/RTS.

## Other automatic tuple information

The tuple has one row per attempted track, including failures. It retains
ordered hit cells/radii/z, truth/KF pT, reference KF pT, filtered/smoothed kappa
and variance, backward predicted/filtered kappa and variance, and truth
override provenance. fitted_log_loss*, local_log_loss* and smoothed_log_loss*
now always refer to RTS; backward_fitted_log_loss* refers to inward results.

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
Fit physics remains in `options/run_breakpoint.py`; optional supported BP_*
environment overrides are frozen at preparation along with the complete card.
There is no automatic per-event breakpoint discovery. The configured interval
list applies to every track; empty means the no-breakpoint reference, even
when TruthOverride is true.

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
The scripts use the existing IHEP `hep_sub -g higgs -mem ... -argu JOB.json`
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
| CEPCSW_BREAKPOINT_DIR | script directory | Project worktree |

For existing tracker inputs, set STAGES=breakpoint and point INPUT_TUPLEPATH
at their directory. Input names are `sim-e--2.0-85-SEED.root` or
`trk-e--2.0-85-SEED.root` with the selected labels. Examples of optional fit
overrides: `BP_INTERVALS=5 BP_TRUTH_OVERRIDE=1 BP_BACKWARD_SEED_SCALE=100`.
Interval5 here is only an example, NOT a recommended automatic truth interval.
All BP_* values supported by the dedicated card except its job I/O/event-count
fields are captured; BP_BACKWARD_MODE remains retired and is not supported.
Unset BP_SELECTED normally means all events; explicit selection is available
for isolated batch smoke tests. Job I/O and NEVT come from workflow controls.

Output layout for each sample:

```text
OUTPUT_TUPLEPATH/
  breakpoint_flat-e--2.0-85-SEED.root
  trk-e--2.0-85-SEED.root             # only if trk selected; retained
  sim-e--2.0-85-SEED.root             # only if sim selected; retained
  outlog/e--2.0-85-SEED.out, .err
  runcards/e--2.0-85-SEED/
    job.json, trk.py, breakpoint.py  # only selected stages have cards
    submitted.json                  # successful scheduler submission
    started.json, completed.json    # worker lifecycle
```

The flat tuple contains ordinary RTS/backward and oracle/copied RTS/backward
results. No breakpoint EDM file is written by the default card. No inputs or
intermediate ROOT files are deleted. A selected stage consumes a predecessor
made in the same job, otherwise an external predecessor from the input path.
Cards are checksum-checked by the worker; never edit a frozen card in place.
Use a new output directory for a changed physics setup. The software/library
is NOT snapshotted: keep the branch/build stable while jobs are queued/running.

The worker verifies readable nonempty ROOT trees, required flat branches, and
at least one successful ordinary fit. Invalid oracle rows are reported and
retained; inspect truth_override_result_status before analysis. This output
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

No automatic interval discovery, positivity enforcement, beam spot or exact
within-interval loss placement is implemented. Native material/mass
conventions remain unchanged. Successful execution, convergence or smaller
chi2 is not population validation or proof of better momentum reconstruction.
