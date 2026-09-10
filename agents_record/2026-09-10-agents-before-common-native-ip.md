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

Active work is the independent RecBreakpoint package on local `breakpoint`,
reading CompleteTracks. Shared KF/GSF sources, maintained GSF cards and
unrelated user-owned workflow edits remain out of scope. Beyond the requested
origin/breakpoint publication, further remote operations and a reconstruction-
based interval finder require explicit authorization. Beam-boundary work is paused.

The user authorized promotion of the normalized free-loss prototype into the
maintained package on 2026-09-10. It is now an OPTIONAL standard package path:
`FreeLossFit=False` preserves the existing Gaussian-prior method; true runs a
bounded Minuit2 fit of b=log(p_before/p_after) and publishes an ADDITIONAL
conditional RTS/backward pair. Ordinary results are never replaced.
The default remains off pending a different
user choice; promotion into maintained code is not physics validation.
There is no alternate filter implementation or temporary-source runtime loader.
FreeLossFitter calls the existing BreakpointFitter; TrackLikelihood computes
the normalized Gaussian marginal likelihood from native F/Q/H/V. FreeLossTuple
handles optimizer serialization separately; FitPairTuple serializes the added
result pair without another fit. All three pairs use one EDM publication helper. See
`Reconstruction/RecBreakpoint/docs/free-loss-fit.md` for the complete contract.

Free fitting currently supports one selected LocalMarginal interval.
Empty intervals use the 5D reference. Multiple intervals or Persistent6D
retain the ordinary pair with unsupported status; failed searches retain it
with failure status. The ordinary pair must first succeed. The optimizer
accepts no truth-loss amount: Truth selection supplies indices only.
Default b bounds are [0,1], with a blind scan and three MIGRAD starts.
Each conditional trial fixes b with sigma_b=0 and uses the same native fitter.
This is distinct from the removed repeated relinearization controls, which
remain retired. FreeLossCheckLikelihoods optionally checks reverse-order QR
and joint-smoothed SVD against the same forward-order Gaussian objective.
These are equivalent formulations, not three independent physics objectives.

Six row-mapped outputs are always available: ordinary RTS/backward, additional
FreeLoss RTS/backward, and additional truth-assisted RTS/backward. The ordinary
pair and all104 original tuple fields retain their ordinary/truth meanings
regardless of FreeLossFit. With free fitting off, empty intervals, unsupported
settings or search failure, the FreeLoss collections and result branches copy
the ordinary pair exactly, without a rerun. FreeLoss result status is0 absent,
1 ordinary copy,2 optimized; input-row EDM maps retain -1 for absent outputs.
The optimizer's request/outcome status remains separate. The new flat
free_loss_result_status branch identifies this separate-pair schema.

TruthOverride is still default-on: matched G4 loss supplies a PRIOR CENTER
with the configured positive SigmaLogLoss and the established one-pass fitter.
When off, the truth pair ALWAYS copies the ordinary pair, never the FreeLoss
pair. PriorCenter/copy/failure contracts remain. Historical replacement-mode
tuples and CopiedFreeLikelihood tags must not be relabeled. No side truth
CSV/ROOT reader is used.

IntervalSelectionMode=Truth remains default and uses associated G4 hooks at
accepted-hit intervals. Manual uses the exact supplied interval list; empty
means no breakpoints. Auto remains reserved and fails initialization.
Invalid truth selection is never converted to no loss. Truth t/X0 is passive.
LocalMarginal supports multiple ordinary intervals; Persistent6D at most one.
The original positive Gaussian prior remains MeanLogLoss0/SigmaLogLoss0.05.
The dedicated submission campaign sigma is separate and must be preserved.

Forward SeedScale1 and FirstMiddleLast remain defaults. BackwardSeedScale100
scales the copied forward endpoint covariance for the native backward refilter;
RTS does not use this refilter and its likelihood does not use that copied
backward seed. The reverse-order likelihood check also does not use it.
The native backward fit is not an independent Bayesian smoother.

The flat tuple always records the optional free-fit steering, selected b,
local Minuit error/status/EDM, boundaries, objective decomposition, trial
records and explicit applied/fallback statuses. Additional FreeLoss result
branches persist pT, IP parameters/covariances, fitted losses/variances, all
three chi2 totals/lists and all five native per-hit state/covariance sequences.
They copy ordinary values when optimization is off, not NaNs or empty vectors.
Only absent ordinary results yield absent extra pairs. Optimized FreeLoss track
covariances are CONDITIONAL on optimized b: optimizer uncertainty is not injected.
A finite scan winner or non-converged candidate may be published and tagged;
Applied does not certify a physical optimum. Existing forward/backward
innovation chi2 and complete smoothed quadratic retain their definitions;
Track.chi2 is not replaced by -2logL. Never add the three chi2s/likelihoods.
NDF remains bookkeeping. The empirical hit-derived prefit seed and local affine
model are explicit likelihood limitations.

Focused integration gates reproduce all 3,724 prototype state/covariance records
exactly across 2:68, 12:11, secondary-control12:17 and 4:11. Disabled and
fallback cases preserve all104 legacy tuple fields and checked full dumps.
In this exact input sample12:16 has NO selected interval, not multiple
intervals; an explicit Manual=[4,5] control tests multi-interval fallback.
Batch unit tests, singular-noise likelihood tests, covariance transport tests,
private configurable generation and CMake dependency checks pass.
Evidence is in `agents_record/2026-09-10-breakpoint-free-loss-integration.md`.
Neither these mechanical gates nor earlier selected-event improvements establish
population performance. The old100-event quadratic-only study must not be
relabeled as a normalized-likelihood population study.

The separate-pair change is built and package-only installed following the user's
confirmation that no batch jobs were running. Ten local regression jobs produced
14 rows:13 successes and one intentionally invalid track. All104 original fields
and ordinary full dumps agree exactly with the pre-change ordinary reference;
the new free-pair values and full-state arrays agree with the prior optimized
or copied references. Nine ordinary-copy rows have exact EDM track payloads,
including IP/first/last states, covariances, chi2, NDF and hit references.
Input-row status/index maps were checked over452 EDM events, including excluded
and invalid tracks. Two compiled numerical tests and23 batch tests pass; colliding
output names are rejected. Build and installed hashes agree. These are mechanical
regression checks, not new physics validation. Evidence and the preserved outgoing
status are in `agents_record/2026-09-10-breakpoint-free-loss-parallel-pair.md`
and `agents_record/2026-09-10-agents-before-free-loss-parallel-pair.md`.
Batch workers do not snapshot libraries: do not rebuild or install over shared
libraries while jobs are using them. The site queue client still fails to import
htcondor; user confirmation, not that client, established the safe deployment window.

Dedicated card: Reconstruction/RecBreakpoint/options/run_breakpoint.py.
Free fit can now be selected with BP_FREE_LOSS_FIT=1 via the existing dedicated
sub/dump workflow. The package README is authoritative.
Next: categorized population checks and study of
conditional-covariance limitations; no shared-KF/GSF changes or automatic
interval finder are included in this promotion.
The full outgoing status is preserved in
`agents_record/2026-09-10-agents-before-free-loss-integration.md`.
