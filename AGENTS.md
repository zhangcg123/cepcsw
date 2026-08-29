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
merge. Reverse additionally records passive same-surface two-filter products
`B_smoothed[i] = F_updated[i] x B_predicted[i]` only at interior surfaces.
Each product is formed inside its reverse-surface step from buffered
`B_predicted[i]` candidates, immediately before the same buffered measurement
results are committed to live `B_updated[i]`.
Direct product candidates persist their pair prior, five-dimensional overlap
chi-square/log-determinant, log weight, normalized pre-pruning posterior,
backward-predicted state, and explicitly named smoothed state.
The boundaries reuse live mixtures: `B_smoothed[0] = B_updated[0]` and
`B_smoothed[N-1] = F_updated[N-1]`. Interior products never feed recursion or
endpoint publication. Their contracts are in
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
first/middle/last two-dimensional-hit prefit, the loose `FullLDCTracking`
covariance, and an explicit first-hit MarlinTrk update. `KappaSeedCov=-1`
selects `Var(omega)=1e-4`; positive values are diagnostic overrides. The
implementation gates are in
`agents_record/2026-08-28-standard-kf-gsf-initializer.md` and
`agents_record/2026-08-29-fresh-inward-standard-kf-initialization.md`.

The former CMS-like compatibility alias and `CmsGsfSmoothing` property are
retired because they had become exactly equivalent to reverse while publishing
no distinct endpoint. Historical tuples, source codes 3/4, and dated evidence
remain interpretable; the migration and exact reverse regression gate are in
`agents_record/2026-08-29-cms-like-workflow-retirement.md`. Forward filtering,
an independent reverse multi-component refit, and a KL reduction-aware
experimental smoother remain mechanically operational.
The separate experimental `RecGsfGlobalLossRefitter` is also mechanically
available as the third explicit `method="global-loss"` choice in the
maintained `DumpGsfTrks/gsf.py.bk`. It consumes `CompleteTracks`, writes
`GlobalLossTracks`, and is scheduled instead of `RecGsfTracking`; the flat
tuple maps that collection into the existing `gsf_*` schema. This availability
does not validate the method or make it the production candidate. The card
default remains `reverse`, and the two established `RecGsfTracking`
workflows are unchanged.
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
path. Calorimeter digitization/reconstruction and `EcalCluster` are not part of
this worker while the ECAL prototype is paused.
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
default 100.

The active defaults are `MaterialPathMode=DD4hepBetweenSurfaces`,
`KappaSeedCov=-1` (standard `Var(omega)=1e-4` forward prefit),
`MaxComponents=10`, `ComponentWeightCutoff=1e-4`, `SymmetricKL` reduction
ranking, identity-lineage protection enabled, and the five-component
`CEPC2GeV85StepConditioned` Bethe-Heitler model. Preserve 12 and 24 components
and `CurrentSurface` as explicit comparison settings. The weighted `Runnalls`
ranking, the six-component `CEPC2GeV85StepConditioned6` model, and the
runtime-interval `CEPCRuntimeGenericGrid5Clear` and
`CEPCRuntimeCategoryAligned5Clear` five-component models and the finer
`CEPCRuntimeCategoryAligned9Clear` and
`CEPCRuntimeCategoryAligned15Clear` models remain default-off controls; none
is validated or approved as a replacement.

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

The immediate checkpoint is the two-mode reverse inward initialization. A
positive `InwardSeedCovarianceScale` preserves the existing correlated-prior
path by copying and scaling the complete final
forward mixture. A finite value at or below zero constructs one fresh
standard-KF-style seed, updates hit `N-1` once in the backward direction, and
starts live recursion at `N-2`. The fresh lineage is one source-2 seed root
with no forward parent; `ReverseInitialWeightMode` is irrelevant there. This
is independent of the final forward mixture, not statistically independent of
the measurements: the geometric prefit uses first/middle/last hit positions
that the loose-covariance fit later consumes. The implementation contract and
focused gates are in
`agents_record/2026-08-29-fresh-inward-standard-kf-initialization.md`.
The maintained `DumpGsfTrks/gsf.py.bk` now selects `-1` for the fresh mode;
`ReverseInitialWeightMode` is explicitly retained but inert because this mode
has one unit-weight inward root. The steering decision is recorded in
`agents_record/2026-08-29-maintained-fresh-inward-seed-steering.md`.

Focused same-code reverse runs on hard-loss events 11, 16, and 17 establish
that scale 100 reproduces its pre-change endpoint exactly and that
scale 0 and scale -1 are exactly equivalent. Every fresh run contains one
source-2 seed at the outermost hit, no copied-seed edge, and first targets
`N-2`; build, install, verbose component execution, and flat-lineage checks
pass. These are mechanical gates only. Reverse publishes terminal
`B_updated[0]`, which is also `B_smoothed[0]`; the outer boundary
`B_smoothed[N-1]` is the final forward mixture, and only interior smoothed
products remain passive lineage diagnostics.

The pre-retirement exact-alias gate on events 11, 16, and 17 compared 900
endpoint/status scalars and 882 final-component/lineage vectors with zero
reverse/CMS mismatches. Each alias retained 30,285 source-3 smoothed nodes and
58,176 smoothing edges; no source-3 node was published. That evidence is now
historical and explains why the duplicate flag was removed. The exact gate is
preserved in `agents_record/2026-08-29-smoothed-diagnostic-only-publication.md`.

The next required evidence is a same-code topology-clear population rerun of
copied scale 100 versus fresh scale 0 for the common inward workflow, split into
no/light/hard loss and early-transition categories, with the 133-event
secondary-activity set reported separately. Audit clean-track safety, tails,
endpoint-mode failures, and local lineage rank changes before considering the
fresh mode as a default. In particular, investigate the focused event-16
FullMixtureMode fallback rather than treating its finite fallback output as a
physics success.

Freeze production defaults and endpoint definitions during this study:
`DD4hepBetweenSurfaces`, `CEPC2GeV85StepConditioned`, `MaxComponents=10`,
`ComponentWeightCutoff=1e-4`, `SymmetricKL`, identity protection,
`KappaSeedCov=-1`, and maintained fresh inward selection -1; the compiled and
active-template inward default 100 remains the copied-mixture control. The
material/BH consistency and narrowly scoped BH-component-variance questions
remain active; ECAL and global-loss remain paused diagnostics. Historical
detail lives in dated `agents_record/` entries and does not override this live
focus.
