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
  The user-authorized breakpoint experiment is on local `test_breakpoint`,
  branched from `dev`; this does not authorize changing remote branches.
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

Active development remains the independent RecBreakpoint package on local
`test_breakpoint`, reading CompleteTracks. BackwardMode is removed. Each run
now produces ordinary `BreakpointTracksRTS` and
`BreakpointTracksBackwardFilter`, with separate input-row mappings, endpoint
pT, loss results and iteration histories in the flat tuple. These are two
results from a shared first forward pass, not CPU threads. The backward
continuation copies the first-pass forward endpoint mean and scales its full
5x5 track covariance by positive finite `BackwardSeedScale` (default1), then
refilters inward; it is not an independent Bayesian smoother. The dedicated
card exposes this scale; its effective value is saved in `backward_seed_scale`.
RTS starts from the final forward updated state/covariance with no such scale, reuses
the buffered forward transitions, and does not consume the backward refilter.

Three default-on outward-hit-indexed lists are saved:
`forward_local_chi2`, `backward_local_chi2`, and `smoothed_local_chi2`.
The first two are native update increments (backward's outermost entry is
zero because its state is a copied seed). The third is the complete final
RTS affine quadratic objective: original-V measurement penalties, incoming
process/loss-prior penalties and the initial seed penalty once. Separate
measurement/process/seed terms, a native nonlinear hit-only diagnostic, and
score validity/error are persisted. Totals match the list sums. These are
not eBrem probabilities or normalized model-comparison likelihoods; forward
and backward totals must not be added because their evidence overlaps.
RTS Track.chi2 carries the complete smoothed score; backward Track.chi2
carries the inward increment sum. NDF is bookkeeping, not calibrated here.

LocalMarginal/RTS is now the compiled/card default loss treatment, supporting
multiple independently selected intervals. Persistent6D remains available,
retaining b through downstream native updates and joint RTS for one interval.
The paired backward continuation
retains the local-joint inverse-loss implementation. LocalMarginal supports
multiple intervals. With MaxFitIterations>1, Persistent6D iterates RTS and
backward separately; LocalMarginal retains one-pass RTS and iterates backward
only. Both keep original priors, and backward freezes the original first-pass
forward seed. BackwardSeedScale is applied once to a copy of that original
covariance on each inward pass, never compounded across iterations. It does
not scale the independent loss prior or affect forward/RTS. Default
MaxFitIterations remains1; iterations require one
ordinary interval and do not enforce loss positivity.

An independent default-off `TruthOverride` bool adds a second RTS/backward
pair in `BreakpointTracksTruthOverrideRTS` and
`BreakpointTracksTruthOverrideBackwardFilter`. Off, or with empty intervals,
these are copies of the completed ordinary results; Truth interval selection
may still read truth even when fixed-loss override is off.
On, each selected interval receives fixed b from reconstructed-hit associations
and embedded G4 provenance, with zero added loss variance, in a separate
one-pass oracle pair. Ordinary iterations/settings remain unchanged.
The public LossStateMode now selects Persistent6D or LocalMarginal only;
old LossStateMode=TruthOverride fails with migration guidance. The dedicated
card uses BP_TRUTH_OVERRIDE=1, default0, and derives truth-input needs from
both this control and IntervalSelectionMode.
Oracle-only truth failure in Manual mode preserves ordinary results but leaves extra outputs absent/NaN
with tagged errors. Result status distinguishes absent0, copied1, oracle2
and negative failure; separate input-row index maps are always saved.
The flat tuple adds oracle/copied endpoint pT, full packed IP covariances and
parameters, and three chi2 lists/totals. Truth values remain passive except for
Truth location selection and the explicitly enabled fixed-loss oracle. No reconstruction-based interval discovery,
within-interval emission fit, native material, shared KF/GSF or seed changes.

IntervalSelectionMode is now independent of loss-state treatment and fixed-loss
override. Compiled/card Truth selects every matched runtime hit interval with
positive G4 eBrem loss per track; it passes only indices, not truth loss centers,
to the ordinary fit. No-loss tracks receive an empty effective list/5D refit.
Manual retains explicit BreakpointIntervals (empty means baseline); a nonempty
list is rejected outside Manual. Auto is reserved for the developing
reconstruction-based finder and fails initialization explicitly.
Truth selection follows reconstructed-hit associations into exact embedded G4
hooks, includes all positive interval losses without a new cutoff, and covers
only intervals between accepted hits. Invalid truth fails the affected track;
it is not treated as an empty list. Effective indices, selection status/error,
selected loss/z and match metadata are saved even with TruthOverride off.
Persistent6D still allows one interval; iterations still require exactly one,
checked per track for Truth selection. Neither silently discards extra losses.
The new batch helper captures BP_INTERVAL_SELECTION_MODE. Regenerate old
prepared cards in a new output directory; default Truth/LocalMarginal replaces
the previous manual-empty/Persistent6D steering. Current batch shell edits
belong to the user and must be preserved. Exact contract and tests:
`agents_record/2026-09-09-recbreakpoint-truth-interval-selection.md`.
Seven same-code Truth-versus-Manual event comparisons reproduce the ordinary
and oracle outputs exactly, including two multiple-interval tracks. An
additional missing-association case fails with a selection tag rather than
becoming a no-loss track. These are regression gates only.

Mechanical gates cover the ordinary/truth seed2:68 smoke, seed12:11/16/17,
iterations, empty/zero-loss and multi-interval controls, plus the previous
eight 1--5% single-eBrem examples. Paired endpoints reproduce the available
separate-run references within the recorded tolerances. Seed12:17 remains a
secondary-activity control, not a clean optimization count. These are
regressions, not population physics validation. Exact tests, schema and
limitations are in
`agents_record/2026-09-09-recbreakpoint-parallel-endpoints-chi2.md`.
The backward seed scale, unchanged RTS boundary and 25-case scale/regression
gate are recorded in
`agents_record/2026-09-09-recbreakpoint-backward-seed-scale.md`.

Next: use the separately labeled scores and four endpoints on the
same negative fitted-loss and truth-oracle controls, checking momentum truth,
not declaring an optimum from smaller chi2. Preserve clean-track and
categorized population gates. No shared KF/GSF edits, maintained batch
workflow changes, automatic truth steering or remote operations are
authorized. Beam-boundary work remains paused.

Dedicated card: `Reconstruction/RecBreakpoint/options/run_breakpoint.py`.
Its retired BP_BACKWARD_MODE environment control is rejected explicitly.
The package README is the authoritative complete property/schema/build
reference. Complete outgoing status and README were preserved in
`agents_record/2026-09-09-agents-before-parallel-breakpoint.md` and
`agents_record/2026-09-09-recbreakpoint-readme-before-parallel.md`.

The parallel truth-pair contract, regression results and control migration are
recorded in `agents_record/2026-09-09-recbreakpoint-parallel-truth-pair.md`.
Its 18-case gate checks exact ordinary on/off preservation, exact disabled
copies, prior oracle reproduction, covariance/schema consistency, and ordinary
result preservation on deliberately invalid truth. This is mechanical evidence,
not additional physics validation.
The complete outgoing status is preserved in
`agents_record/2026-09-09-agents-before-parallel-truth-pair.md`.
