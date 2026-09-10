# CEPCSW GSF Development

## 1. Introduction and global status

This project develops an electron Gaussian Sum Filter (GSF) refit for CEPCSW.
Its goal is to model tracker-material bremsstrahlung and recover the electron
state at the interaction point more accurately than the standard
`CompleteTracks` result.

The intended physics chain is:

```text
Geant4 pre/post-material-step truth
  -> CEPC step-t/X0-conditioned Bethe-Heitler mixture
  -> multi-component filtering and smoothing
  -> validated interaction-point track parameters
```

`RecGsfTracking` builds, installs, and reads `CompleteTracks`. Smoother and
reverse runs write three row-aligned endpoint views: BestBranch to
`GSFTracksBestBranch`, the moment-matched endpoint to
`GSFTracksWeightedMean`, and the maximum of the complete five-dimensional IP
mixture density to `GSFTracksFullMixtureMode`. FullMixtureMode is
automatic/default-on, has a persisted optimization-status collection and
flat-tuple fields, and is mechanically available but not physics-validated.
Its definition and gates are in
`agents_record/2026-08-24-full-mixture-mode-endpoint.md`; historical CMS-like
extensions remain in dated records only.

Positive-weight final components and complete component lineage are persisted
automatically for smoother and reverse. The `final_mixture_component_*`,
`lineage_node_*`, and `lineage_edge_*` flat vectors retain the final mixture
and every evaluated seed, BH child, measurement result, KL output, cutoff, and
merge. Reverse additionally records same-surface two-filter products
`B_smoothed[i] = F_updated[i] x B_predicted[i]` only at interior surfaces.
Each product is formed inside its reverse-surface step from buffered
`B_predicted[i]` candidates, immediately before the same buffered measurement
results are committed to live `B_updated[i]`.
Direct product candidates persist their pair prior, five-dimensional overlap
chi-square/log-determinant, log weight, normalized pre-pruning posterior,
backward-predicted state, and explicitly named smoothed state.
They also persist signed F/B curvature and pT differences, their approximate
zero-cross-covariance variances, and a passive direction-signed
five-dimensional compatibility score. Its magnitude is the chi-square CDF of
the full F/B state difference and its direction is the signed pT difference.
The score is meaningful on the exact identity pair only; it is not a
calibrated physical posterior and never steers the fit.
They also persist the signed forward/backward differences
`B_predicted-F_updated` in kappa and transverse momentum, with passive
independence-approximation variances; the exact schema contract is in
`agents_record/2026-09-01-forward-backward-delta-diagnostics.md`.
The boundaries reuse live mixtures: `B_smoothed[0] = B_updated[0]` and
`B_smoothed[N-1] = F_updated[N-1]`. Interior product states never propagate or
publish. The compiled/default `InwardWeightMode=LocalMeasurement` also leaves
their weights diagnostic-only. Experimental `SmoothedMarginal` instead sums
the normalized unreduced direct-pair weights over all forward partners for
each backward candidate and attaches that marginal to the corresponding live
`B_updated[i]` state before cutoff, reduction, and further inward propagation.
It deliberately reuses overlapping forward evidence at successive surfaces
and is not a calibrated Bayesian posterior. Its active contract is in
`agents_record/2026-08-31-smoothed-marginal-inward-weighting.md`; the original
passive contracts are in
`agents_record/2026-08-25-final-mixture-component-flat-tuple.md`,
`agents_record/2026-08-25-component-lineage-dag-flat-tuple.md`, and
`agents_record/2026-08-29-smoothed-diagnostic-only-publication.md`; the
explicit boundary correction and focused gate are in
`agents_record/2026-08-29-smoothed-boundary-state-contract.md`, and the inline
construction regression is in
`agents_record/2026-08-29-inline-smoothed-surface-construction.md`; the
surface-local evidence schema is in
`agents_record/2026-08-29-smoothed-surface-local-evidence.md`.

Reverse consumes one `SharedForwardFilterResult` and publishes the terminal
inward mixture `B_updated[0] = measurement[0] x B_predicted[0]`. A positive
`InwardSeedCovarianceScale` copies and scales the final forward population; a
finite value at or below zero builds one fresh standard-KF-style seed, updates
the outermost hit, and first revisits hit `N-2`. The common initializer uses a
direction-local two-dimensional-hit prefit: the three innermost hits outward
and the three outermost hits inward, followed by the loose `FullLDCTracking`
covariance and an explicit boundary-hit MarlinTrk update.
`ForwardSeed=1` and `BackwardSeed=1` independently select the complete
FullLDCTracking-style loose diagonal covariance for the direction-local
prefits. Every finite positive value uniformly scales all five seed variances;
zero and negative values are invalid. `BackwardSeed` is inert when a positive
`InwardSeedCovarianceScale` selects a copied/scaled forward mixture. The
implementation gates and retired curvature-only controls are in
`agents_record/2026-08-28-standard-kf-gsf-initializer.md` and
`agents_record/2026-08-29-fresh-inward-standard-kf-initialization.md`, with
the current scaling contract in
`agents_record/2026-09-06-directional-seed-covariance-scales.md`.

The former CMS-like compatibility alias and `CmsGsfSmoothing` property are
retired because they had become exactly equivalent to reverse while publishing
no distinct endpoint. Historical tuples, source codes 3/4, and dated evidence
remain interpretable; the migration and exact reverse regression gate are in
`agents_record/2026-08-29-cms-like-workflow-retirement.md`. Forward filtering,
an independent reverse multi-component refit, and a KL reduction-aware
experimental smoother remain mechanically operational.
A default-off ECAL component-re-ranking prototype is also mechanically
operational. It preserves `GSFTracksBestBranch` and writes its paired result separately;
its focused evidence is promising only for retained bimodal alternatives and
is not population-validated.
A separate default-off truth BH-loss oracle can replace existing BH-call
responses on explicitly selected tracks while leaving the downstream GSF
workflow unchanged. It is a mechanism diagnostic only and never production
steering.
The normal simulation event can now optionally embed exact
`SimTrackerHit -> Geant4-step` provenance in two PODIO collections. The
default-off oracle's current batch source follows the standard reconstructed-
hit truth associations into those collections, so the maintained workflow no
longer requires or supports a side material tuple or prejoined CSV input.
The active `dump_gsftrk.sh` worker accepts one `STAGES` subset of `sim`, `trk`,
and `gsf` (default `trk,gsf`) and executes selected stages in physical order.
Each stage consumes a predecessor produced in the same job or, when that
predecessor is omitted, the corresponding existing tuple from the input tuple
path. The maintained GSF card writes only the flat tuple; its GSF collections
pass directly in memory to `RecGsfFlatTuple` and are not serialized through
`PodioOutput`. After verifying the flat tuple, the worker removes any tracker
tuple produced by its own `trk` stage; a `gsf`-only job retains its external
tracker input. Calorimeter digitization/reconstruction and `EcalCluster` are
not part of this worker while the ECAL prototype is paused.
A passive interval recorder now persists, in the final
GSF EDM and flat tuple, fractionally integrated Geant4 t/X0/eBrem truth,
DD4hep t/X0 between the same exact truth hooks, and summaries of the actual
forward/reverse component paths. These values never steer the GSF. The
compiled, active reverse-template, and maintained-card default is on.

The active production candidate remains the reverse multi-component refit. It
starts from the complete final forward mixture, scales each full covariance by
`InwardSeedCovarianceScale` (default 100), repeats measurement updates inward,
and publishes the selected branch, moment-matched mixture, and full
joint-density mode in separate row-aligned collections. It has demonstrated
interaction-point momentum recovery in many
hard-bremsstrahlung events and favorable central light/hard performance, but
it also creates clean-track degradation and extreme tails. The KL smoother is
largely LCIO-like and forfeits much of the hard-loss recovery.
The maintained comparison card now deliberately selects
`InwardSeedCovarianceScale=-1` for the fresh-inward-seed campaign. This is
campaign steering only; it does not change the compiled or active-template
default 100. Its reverse branch selects the compiled and active-template
`InwardWeightMode=LocalMeasurement` control; `SmoothedMarginal` remains a
default-off experiment.
Directional BH child creation is independently configurable. The compiled and
inherited active reverse-template defaults are
`ForwardBHSplitting=false, InwardBHSplitting=false`, so an unsteered fit
creates no BH children in either direction. The maintained double-off
diagnostic is historical; the maintained live beam-boundary campaign now
explicitly enables both gates. These gates do not disable
material-path evaluation, passive interval recording, deterministic energy
loss, multiple scattering, propagation, or measurement updates.

A default-off reverse-only live beam-boundary experiment is implemented inside
`RecGsfTracking`, outside the MarlinTrk classes. The forward three-hit prefit
is moved to the beam pivot and constrained before optional beam-to-hit-0 BH
splitting and the first real-hit update. After the ordinary reverse recursion
updates hit 0, the same canonical boundary t/X0 may split again in the inward
direction; each child propagates to the beam, receives the transverse `drho`
likelihood, and enters the ordinary cutoff/KL and endpoint publication. Its
nominal comparison widths are 0.0145 mm horizontally and 3.6e-5 mm vertically.
The exact contract and focused mechanical gate are recorded in
`agents_record/2026-09-07-live-beam-boundary-gsf.md`; the superseded endpoint-
copy prototype remains documented in the earlier dated handoff record.

The active defaults are `MaterialPathMode=DD4hepBetweenSurfaces`,
`ForwardSeed=1` and `BackwardSeed=1` (the complete FullLDCTracking-style
loose covariance for both direction-local prefits), `MaxComponents=10`,
`ComponentWeightCutoff=1e-4`, `SymmetricKL` reduction ranking,
identity-lineage protection enabled, `ForwardBHSplitting=false`,
`InwardBHSplitting=false`, and `CEPCRuntimeCategoryAligned9Clear`. Despite the
retained selector name, this BH model now has one effective identity plus nine
globally fitted radiative Gaussians. Preserve 12 and 24 components and
`CurrentSurface` as explicit comparison settings. `ActsAtlas` is the only
alternative BH model. Neither model is validated for production physics.

Geant4 pre/post-step data is the authoritative energy-loss truth.
SimTrackerHit momentum is only a detector-level cross-check. Existing Geant4
studies establish a real electron loss tail and bounded fractional-loss
transfer compatibility over the tested 2--10 GeV and theta 85--20 degree
samples. None of the available Bethe-Heitler models is validated for general
CEPC tracker steps.

Population studies show real central recovery, especially for losses at
transitions 5--11, but also new extreme tails. Losses at transitions 0--4 are
predominantly information-limited. Forced-electron-hypothesis muon controls do
not show a universal momentum inflation, but do show clean-core broadening and
outliers. This is a research implementation, not a validated production
algorithm. Broad performance claims and mainline integration require clean
track preservation, reproducible tail control, and independent held-out
validation.

Historical evidence, resolved incidents, exact experiment tables, runbooks,
and superseded decisions live under `agents_record/`. Load historical records
only for regression evidence, design rationale, experiment comparison, or
explicit provenance. Historical detail does not override this live status.

### Project laws and work scope

- Keep implementation changes inside `Reconstruction/RecGsfTracking` unless
  the user explicitly authorizes broader scope for a concrete reason.
  Current explicit exception: develop `Reconstruction/RecBreakpoint` and its
  dedicated card/build registration. Keep existing GSF and shared KF sources
  and maintained workflow cards unchanged for this experiment.
- Do not modify KalTest, TrackSystemSvc, MarlinTrk, DDKalTest, or other shared
  CEPCSW packages to compensate for a GSF-specific state-management problem
  unless the user explicitly authorizes a narrow shared interface change.
- Do not hand-code a parallel Kalman measurement update when the baseline
  MarlinTrk interface can provide the required operation.
- Treat Geant4 pre/post-step records as material-energy-loss truth. Do not
  present SimTrackerHit momentum as an exact material transition.
- Do not claim Bethe-Heitler or GSF validation from successful execution,
  finite output, improved chi-square, or selected-sample improvement alone.
  Validation requires interaction-point momentum recovery against generator
  truth in categorized hard-loss events, clean-track safety, and held-out
  population checks.
- Preserve unrelated working-tree changes. Keep source/documentation changes
  separate from generated ROOT files, logs, plots, tables, notebooks, and
  batch cards.
- Use `dev` as the active development branch. Do not switch, create, rename,
  delete, merge, or rebase branches unless the user explicitly requests the
  specific branch operation.
  The user-authorized breakpoint experiment is on `breakpoint` (renamed from
  `test_breakpoint`), branched from `dev`. The user authorized publishing this
  branch to origin; other remote branch changes require a separate request.
- Use Git frequently during development: inspect status and diffs, and create
  focused checkpoint commits after coherent, proportionately verified core
  implementation or project-knowledge changes. Track, commit, and push all
  C/C++ implementation and header changes across the repository, together
  with documentation, `AGENTS.md`, `.agents/` maintenance content, and durable
  `agents_record/` status/history records. This Git rule does not broaden the
  separate implementation-scope law: edits outside
  `Reconstruction/RecGsfTracking` still require explicit authorization. Also
  track the specifically maintained workflow card `DumpGsfTrks/gsf.py.bk`,
  whose complete property steering is part of the documentation contract
  except for the deliberate inherited
  `RecordTruthMaterialIntervals=true` default.
  Keep other run cards/options, analysis scripts, build files, generated ROOT
  files, logs, plots, tables, notebooks, batch cards, and experiment outputs
  uncommitted unless the user explicitly authorizes a specific exception. Do
  not change branches unless the user explicitly requests it.
  Explicit breakpoint publication exception: also track its dedicated
  run_breakpoint.py, subbreakpointjobs.sh, dump_breakpoint.sh, batch helper,
  package build files/registration and the required DumpGsfTrks/trk.py.bk.
  Never stage or commit generated ROOT files for this publication.
- Keep `AGENTS.md` limited to global status, active laws, essential commands,
  and the current focus. Before replacing or removing unique detail, preserve
  it in a dated `agents_record/` entry; replace rather than append focus.
- Whenever a `RecGsfTracking` configurable property is added, removed,
  renamed, or its default/allowed values change, assign a dedicated sub-agent
  to audit the complete option surface. In the same change, synchronize the
  authoritative property reference in
  `Reconstruction/RecGsfTracking/README.md` and the explicit effective
  steering in `DumpGsfTrks/gsf.py.bk`; document intentional historical-card
  differences in `DumpGsfTrks/README.md`.
- Validate every implementation step with comprehensive verbose component
  dumps on a focused event. Once mechanically stable, repeat on hard-loss
  events 11, 16, and 17 before population validation. Build success, finite
  output, or lower chi-square is not a sufficient gate.
- Exclude the stable 133-event secondary-tracker-activity set from
  single-track optimization counts and representative selection, but always
  report it separately as a topology/control population.
- Use same-code direct A/B reruns for final-selection claims; stored outputs
  can drift as the implementation changes.

### Compile and run

Run commands from the repository root. For a complete configured build:

```bash
source setup.sh
./build.sh
```

For the normal focused EL9/LCG 105 development cycle:

```bash
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt \
  --target RecGsfTracking RecGsfFlatTuple -j4
cmake --install build.105.0.0.x86_64-el9-gcc11-opt
```

Run Gaudi options through that environment:

```bash
source setup.sh
build.105.0.0.x86_64-el9-gcc11-opt/run \
  gaudirun.py path/to/options.py
```

Use a small `SelectedEventIndices` list for component diagnostics. Generated
ROOT files and logs are outputs, not status records.

## 2. Current focus

Active work is the independent RecBreakpoint package on local
`breakpoint`, reading CompleteTracks. Shared KF/GSF sources, maintained
GSF cards and unrelated user-owned workflow edits remain out of scope.
Beyond the requested origin/breakpoint publication, further remote operations
and a reconstruction-based interval finder require explicit authorization.
Beam-boundary work remains paused.

The user requested removal of repeated relinearization on 2026-09-10.
Both LocalMarginal (compiled/card default) and Persistent6D now run only one
forward filter, one RTS pass and one backward refilter per pair.
MaxFitIterations, RelinearizationTolerance, their fitter settings and the
iteration-specific propagation/update helpers are removed. The maintained
card and dedicated batch helper reject stale BP_MAX_ITERATIONS and
BP_ITERATION_TOLERANCE requests. Old prepared cards assigning retired Gaudi
properties must be regenerated; no existing tuples/cards are rewritten.
A subsequently user-authorized free-loss prototype is now tested outside the
maintained package under TrackingPerformanceStudies. It calls the existing
compiled BreakpointFitter helpers; there is no alternate filter/smoother.
Each trial fixes b with sigma_b=0 and Minuit2 varies b in [0,1]. It uses
truth-selected intervals, never truth loss amounts to initialize the search.
The maintained package, positive Gaussian loss priors and batch cards have
not changed; free fitting is not integrated into the maintained run card.
The initial 100-event RTS-objective test found severe early-interval tails,
not an overall improvement. The evidence and location audit are recorded in
`agents_record/2026-09-10-breakpoint-free-loss-hundred-events.md`.

Four row-mapped outputs remain: BreakpointTracksRTS,
BreakpointTracksBackwardFilter, BreakpointTracksTruthOverrideRTS and
BreakpointTracksTruthOverrideBackwardFilter. These are paired results, not
CPU threads. RTS uses its own forward posterior and buffered transitions,
not the backward refilter. The backward refilter copies its pair's forward
endpoint mean and scales the full 5x5 covariance once by positive finite
BackwardSeedScale (default100; changed from 1 on 2026-09-10 at user request).
The completed three-objective 100-event study explicitly used 1 and is not
relabeled or rerun by this default change. It reuses forward evidence, so is not an
independent Bayesian smoother. Forward SeedScale defaults1 and
FirstMiddleLast remains the prefit hit selection. BackwardSeedScale affects
neither RTS nor the loss-prior sigma.

LocalMarginal supports multiple selected intervals. Persistent6D keeps b
through every downstream native hit update and joint RTS, for at most one
interval. Empty intervals use the 5D reference. Both publish the paired
local-joint backward continuation. No fitted-loss positivity constraint is
imposed. The ordinary prior remains MeanLogLoss=0 and positive
SigmaLogLoss=0.05 by compiled default; campaign overrides are separate.

TruthOverride remains default-on and uses matched truth as each selected b
PRIOR CENTER, with the SAME positive SigmaLogLoss and one-pass fitting code
as the ordinary fit. It does not fix b or zero its uncertainty.
Only the per-interval prior centers differ. Hits can move the fitted loss
away from truth. z=1-sum(delta_p_eBrem)/p_start and b=-log(z) are derived
through reconstructed-hit associations and exact embedded G4 hooks.
LossStateMode only accepts LocalMarginal/Persistent6D, not TruthOverride.
Historical tuples without truth_override_loss_treatment="PriorCenter"
used fixed loss with zero added variance and must not be relabeled.

IntervalSelectionMode=Truth (compiled/card default) selects every matched
accepted-hit interval with positive G4 eBrem loss and supplies only indices
to the ordinary fit. Multiple emissions can give one breakpoint. There is
no added loss cutoff, and only intervals between accepted hits are covered.
Post-step assignment and upstream loss placement are unchanged.
Manual uses BreakpointIntervals; an empty list means a 5D reference.
Nonempty manual lists are rejected outside Manual. Auto remains reserved
and fails initialization. Persistent6D rejects multiple intervals per track
rather than discarding losses. A selected Manual interval with no eBrem has
truth prior center0 but its fitted loss can move; unselected losses are not
automatically corrected.

TruthOverride=False or empty effective intervals copy ordinary results into
the extra pair. Truth interval selection can still require truth with override
off. Invalid Truth selection fails the affected ordinary track; it is never
interpreted as no loss. Oracle-only failure in Manual preserves ordinary
results with absent/NaN extra outputs and tagged errors. Ordinary failure
leaves the extra pair unattempted. Status remains absent0, copied1,
active truth-assisted2, negative failure, with separate collection row maps.
No side CSV/ROOT reader or GSF execution is involved. TruthMaxEndpointDistance
validates associated hooks rather than finding nearest hits.

The flat tuple retains endpoint pT, extra IP parameters/packed covariances,
truth interval metadata, fitted loss means/variances, complete persistent
states and all three per-hit chi2 lists/totals. Removed iteration-only fields:
one_pass_pt, fit_iterations, iteration_*, backward_fit_iterations,
backward_iteration_* and truth_override_{rts,backward}_fit_iterations.
truth_override_log_loss is the truth INPUT, while
truth_override_{rts,backward}_fitted_log_loss and variance vectors are fitted
posteriors aligned with breakpoint_interval. Treatment and shared prior sigma
remain explicit. Truth t/X0 remains passive.

Forward/backward chi2 are native hit-update increment sums. Complete smoothed
chi2 includes original-V measurement, process/loss-prior and initial-seed
penalties once. Backward's outermost increment is zero. Ordinary decomposition
and nonlinear hit-only diagnostics remain; extra pairs retain complete
per-hit sums without decomposition. Score validity is separate from fit
validity. RTS Track.chi2 uses the complete smoothed score; backward uses its
inward sum. Never add the three scores or treat them as calibrated eBrem
probabilities. NDF remains bookkeeping; smaller chi2 does not guarantee better
truth pT, particularly when comparing different prior centers.

The current sigma_b=0.001 campaign has 8,098 clean valid paired tracks; 3,135
have exactly one truth-selected breakpoint. Single-breakpoint chi2-split plots
are in TrackingPerformanceStudies/breakpoint_barrel_sigma0001_singlebreakpoint_chi2split_20260910.
RTS complete-score categories have 1912 truth-prior-smaller, 1215
ordinary-smaller and 8 numerical ties; backward has 1837/1283/15.
Truth-prior width68 is better in both categories, but that is diagnostic,
not production validation or proof that a prior-free minimizer will improve
the momentum. Source fitting changes require explicit user direction.

Dedicated card: Reconstruction/RecBreakpoint/options/run_breakpoint.py;
BP_TRUTH_OVERRIDE defaults1. Shared SigmaLogLoss can be frozen from
BP_SIGMA_LOG_LOSS in the dedicated submission script. Preserve the user's
campaign choice. Batch workers do not snapshot libraries; do not rebuild
while jobs are using them. The package README is the authoritative option,
schema and build reference.

Removal details and focused before/after gates belong in
`agents_record/2026-09-10-recbreakpoint-iteration-removal.md`.
The complete outgoing status, README and explanatory walkthrough are saved
in dated `2026-09-10-*-before-iteration-removal.md` snapshots under
agents_record (AGENTS snapshot:
`2026-09-10-agents-before-breakpoint-iteration-removal.md`).
The prior truth-prior-center implementation and its 20-job/28-configuration
gate remain historical in
`agents_record/2026-09-10-recbreakpoint-truth-prior-center.md`.
Removal gate passed: nine direct before/after configurations (13 track rows),
all 104 retained flat fields and 21,819 full state/covariance dump lines match
exactly; 19 iteration-only fields are absent. Coverage includes both modes,
truth on/off, empty/zero-loss/multiple intervals, sigma0.001/0.05,
BackwardSeedScale100 and seed12:11/16/17 (17 remains a secondary control).
Package build/install, installed configurable audit, 21 batch tests and the
standalone covariance test passed. These are mechanical regression checks.
Current focus: the user-authorized isolated normalized free-loss likelihood
prototype, TrackingPerformanceStudies/breakpoint_free_loss_likelihood_20260910.
It captures native F/Q/H/V and evaluates ONE frozen affine marginal likelihood
through forward-order QR, reverse-order QR and a joint-smoothed SVD calculation.
These are equivalent formulations, not three independent objectives. The
reverse-order score does not reuse a data-conditioned backward seed. The
published native backward endpoint uses BackwardSeedScale100; native RTS and
ordinary/oracle outputs remain unchanged. No maintained fitter/card/library
was modified by this likelihood experiment.
Analytic singular-noise tests pass. All816 valid trial evaluations agree
within1.32e-6; independent three-formulation minima on12:11 give identical
published endpoints. All13 ordinary rows/104 fields and verbose state dumps
match maintained scale100 references. Five clean events and secondary12:17
were profiled; multi-interval12:16 remains only an ordinary regression control.
Normalization removes the +172% overshoot of4:11, but20% errors persist in
other examples. No population or physics validation is claimed.
The empirical prefit seed and trial-dependent affine approximation remain
explicit caveats. Minuit boundaries/status and conditional-covariance limits
must be preserved; do not add the three likelihoods together.
Next: review population performance and seed/model sensitivity before any
integration request. Contract, formulas, numerical evidence and resumption:
`agents_record/2026-09-10-breakpoint-normalized-free-loss-likelihood.md`.
The preceding100-event/300-fit quadratic-only study remains in
`agents_record/2026-09-10-breakpoint-three-free-loss-objectives.md`.
The complete outgoing AGENTS is preserved in
`agents_record/2026-09-10-agents-before-normalized-likelihood.md`.
