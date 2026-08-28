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

`RecGsfTracking` builds, installs, and reads `CompleteTracks`. Smoother,
reverse, and CMS-like runs write three row-aligned endpoint views: BestBranch to
`GSFTracksBestBranch`, the moment-matched endpoint to
`GSFTracksWeightedMean`, and the maximum of the complete five-dimensional IP
mixture density to `GSFTracksFullMixtureMode`. CMS-like forms all three views
from its hit-1 forward-by-backward product mixture and falls back to its final
backward mixture only when no product can be formed. FullMixtureMode is
automatic/default-on for all three workflows, has a persisted
optimization-status collection and flat-tuple
fields, and is mechanically available but not physics-validated. Its
definition, output/fallback contract, and focused mechanical gates are in
`agents_record/2026-08-24-full-mixture-mode-endpoint.md`. Its
CMS-like extension and exact historical-WeightedMean compatibility gate are
in `agents_record/2026-08-27-cms-like-three-endpoint-publication.md`. Its
underlying positive-weight final components are also persisted automatically
with input/output track mapping, normalized weight, IP kappa, kappa variance,
method source, and validity. The flat `final_mixture_component_*` vectors are
sufficient to reconstruct the one-dimensional pT marginal and are empty for
forward and global-loss; their contract and gates are in
`agents_record/2026-08-25-final-mixture-component-flat-tuple.md`. Its
complete component lineage is now also persisted automatically for smoother,
reverse, and CMS-like jobs. The `lineage_node_*` and `lineage_edge_*` flat vectors keep
every evaluated seed, BH child, measurement result, and KL output, including
states later rejected, cut, or merged. Reverse and CMS-like also record every
same-surface forward-updated×backward-predicted product candidate and its two
source states, while forward and global-loss leave the graph empty.
Split/merge/product structures remain
directed acyclic graphs, and the graph never steers the fit. Its base schema
and reverse gates are in
`agents_record/2026-08-25-component-lineage-dag-flat-tuple.md`; the CMS-like
extension is in
`agents_record/2026-08-27-cms-like-component-lineage.md`. Reverse and CMS-like
now consume one transient `SharedForwardFilterResult` and one common inward
filter: the same post-update, post-cutoff, post-reduction outward mixtures,
the same final filtered seed population, and the same measured-hit backward
recursion. Both first revisit hit `N-2` and use the same
`InwardSeedCovarianceScale`; the duplicate method-specific scale properties
are removed. After the live recursion, the common filter always
materializes the passive `F_updated[i] x B_predicted[i]` side mixture at every
successfully processed inward surface. Reverse still publishes the terminal
backward mixture; CMS-like publishes the hit-1 side mixture and falls back to
the terminal backward mixture when necessary. The former CMS-only
identity-compatibility diagnostic is retired from EDM and flat tuples; its
evidence remains historical. The shared mechanics and gates are in
`agents_record/2026-08-28-shared-forward-reverse-cms-framework.md` and
`agents_record/2026-08-29-common-inward-filter-side-products.md`. Its
component measurement updates use the baseline-compatible
MarlinTrk `initialise -> addAndFit` path, exact accepted innovation quantities,
full Gaussian innovation likelihoods, and exact accepted inter-surface
transport Jacobians. Every forward GSF pass now starts through the dedicated
`GsfTrackInitializer`: a first/middle/last two-dimensional-hit prefit, the full
loose `FullLDCTracking` covariance, and an explicit first-hit MarlinTrk update.
This common start is inherited by reverse, smoother, and CMS-like. The default
`KappaSeedCov=-1` selects exact `Var(omega)=1e-4`; positive values remain a
curvature-only diagnostic override and do not restore the former
`CompleteTracks`-anchored seed. The implementation and focused gate are in
`agents_record/2026-08-28-standard-kf-gsf-initializer.md`. Forward filtering,
an independent reverse
multi-component refit, a KL reduction-aware experimental smoother, and a
CMS-like experimental product-endpoint workflow are mechanically operational.
The separate experimental `RecGsfGlobalLossRefitter` is also mechanically
available as the fourth explicit `method="global-loss"` choice in the
maintained `DumpGsfTrks/gsf.py.bk`. It consumes `CompleteTracks`, writes
`GlobalLossTracks`, and is scheduled instead of `RecGsfTracking`; the flat
tuple maps that collection into the existing `gsf_*` schema. This availability
does not validate the method or make it the production candidate. The card
default remains `reverse`, and the three established `RecGsfTracking`
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
largely LCIO-like and forfeits much of the hard-loss recovery. The CMS-like
workflow has a different core/tail tradeoff and remains default-off.

The active defaults are `MaterialPathMode=DD4hepBetweenSurfaces`,
`KappaSeedCov=-1` (standard `Var(omega)=1e-4` forward prefit),
`MaxComponents=12`, `ComponentWeightCutoff=5e-3`, `SymmetricKL` reduction
ranking, identity-lineage protection enabled, and the five-component
`CEPC2GeV85StepConditioned` Bethe-Heitler model. Preserve 24 components and
`CurrentSurface` as explicit comparison settings. The weighted `Runnalls`
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

The immediate implementation checkpoint is the one common inward GSF path for
reverse and CMS-like. A single live backward recursion now returns the terminal
backward mixture and, after that recursion is complete, unconditionally
materializes the passive `F_updated[i] x B_predicted[i]` side mixture at every
successfully processed inward surface. Reverse publishes the terminal mixture;
CMS-like publishes the hit-1 side mixture with the established terminal
fallback. The side mixtures always enter the default-on lineage EDM and flat
tuple, have no option switch, and never steer the live filter. The exact
contract and focused gates are in
`agents_record/2026-08-29-common-inward-filter-side-products.md`.
The duplicate reverse/CMS seed-covariance properties are retired in favor of
the one common control; its migration and exact focused regression gate are in
`agents_record/2026-08-29-inward-seed-covariance-property-unification.md`.

The focused job-98 entry-15 gate records all 232 inward surfaces and 18,375
product candidates while reproducing the reverse endpoint and all 8,496 live
forward/backward statistical nodes exactly across the refactor. Same-code
five-component runs on hard-loss events 11, 16, and 17 complete for both
methods and retain the same ordered live-filter structure; CMS-like alone
publishes source-3 hit-1 products. This is mechanical validation only. The next
required evidence is a topology-clear population rerun split into no/light/hard
loss and early-transition categories, with the 133-event secondary-activity
control reported separately.

Use the common side record to find the first surface where a truth-compatible
lineage changes rank between the backward-only posterior and the
forward-by-backward product. Attribute the change to BH prior weight, exact
local `dchi2`, and `logDetInnovation`; do not infer it from final weights
alone. Keep product mixtures passive until a reviewed population result
justifies any steering use. The related material/BH consistency and narrowly
scoped BH-component-variance questions remain active; ECAL and global-loss
remain paused diagnostics.

Freeze all production defaults and method endpoints during this study:
`DD4hepBetweenSurfaces`, `CEPC2GeV85StepConditioned`,
`MaxComponents=12`, `ComponentWeightCutoff=5e-3`, `SymmetricKL`,
identity protection, the standard `KappaSeedCov=-1` initializer, and common
inward covariance scale 100. The duplicate historical scale properties are
removed; no flat-tuple schema change belongs to the common-inward checkpoint.
Historical material, BH, truth-oracle, ECAL,
global-loss, lineage, and population evidence lives in its dated
`agents_record/` entries and does not override this live focus.
