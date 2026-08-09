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

`RecGsfTracking` builds, installs, reads `CompleteTracks`, and writes
`GSFTracks`. Its component measurement updates use the baseline-compatible
MarlinTrk `initialise -> addAndFit` path, exact accepted innovation quantities,
full Gaussian innovation likelihoods, and exact accepted inter-surface
transport Jacobians. Forward filtering, an independent reverse
multi-component refit, a KL reduction-aware experimental smoother, and a
CMSSW-like experimental backward workflow are mechanically operational.

The active production candidate remains the reverse multi-component refit. It
starts from the complete final forward mixture, scales each full covariance by
`ReverseKappaSeedCov` (default 100), repeats measurement updates inward, and
publishes either the highest-weight branch or an optional moment-matched
mixture. It has demonstrated interaction-point momentum recovery in many
hard-bremsstrahlung events and favorable central light/hard performance, but
it also creates clean-track degradation and extreme tails. The KL smoother is
largely LCIO-like and forfeits much of the hard-loss recovery. The CMSSW-like
workflow has a different core/tail tradeoff and remains default-off.

The active defaults are `MaxComponents=12`, `ComponentWeightCutoff=1e-4`,
`SymmetricKL` reduction ranking, identity-lineage protection enabled, and the five-component
`CEPC2GeV85StepConditioned` Bethe-Heitler model. Preserve 24 components as an
explicit comparison setting. The weighted `Runnalls` ranking and the
six-component `CEPC2GeV85StepConditioned6` model remain default-off controls;
neither is validated or approved as a replacement.

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
  whose complete property steering is part of the documentation contract.
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
  --target RecGsfTracking -j4
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

The reverse innovation-likelihood calibration audit is complete and did not
find a reproducible GSF-specific defect. The active question is now whether an
independent reconstructed observable, most naturally calorimeter energy or an
E/p-like constraint, can separate genuine energy-loss recovery from false
tracker-only radiative modes without truth at runtime. This is a design and
scope question; it does not authorize changes outside `RecGsfTracking`.

Freeze the production baseline exactly as committed in
`DumpGsfTrks/gsf.py.bk` and tagged by
`gsf-memory-leak-fixed-2026-08-08`: five-component
`CEPC2GeV85StepConditioned`, `MaxComponents=12`,
`ReductionTargetComponents=0`, `SymmetricKL`, identity-lineage protection,
`ComponentWeightCutoff=1e-4`, forward-posterior reverse weights, reverse full
covariance scale 100, `AggregateWeight`, and reverse `BestBranch` publication.
The 24-component setting has already been tested: it preserves both genuine
recovery and false radiative modes and does not solve selection.

Current boundary evidence:

- Broadening `ReverseKappaSeedCov` from 100 to 1,000,000 repaired none of 19
  selected no-eBrem false radiative tails and preserved all seven selected
  genuine light recoveries. After 100--199 inward updates the median relative
  identity-state pT difference was 0.00129, falling to 0.000156 after 200 or
  more. Long tracks forget the forward seed before the decisive inner hits;
  ACTS-like forward-posterior seeding is not the leading tail cause.
- A first 50-event clean control suggested reverse VXD undercoverage, but an
  independently selected 50-event control gave reverse mean two-dimensional
  innovation chi-square 1.999, forward mean 2.004, and summed-chi-square
  p=0.962. A VXD covariance correction is not supported.
- A mild first-coordinate pull width near 1.17 at silicon radius about 235 mm
  repeats across clean halves, but appears similarly in the forward fit and
  does not predict the selected false-clean tails. It is a common measurement-
  model monitoring item, not a reverse-GSF correction candidate.
- Relative hit indices 5--8 are not fixed physical detector layers across the
  broad sample. They span VXD, silicon, and TPC regions depending on trajectory
  and missing hits; a literal layer correction based on these indices would be
  ill-defined.
- At the often-decisive VXD update, the event-level maximum local radiative-
  versus-identity log-likelihood ratio has median 0.421 and range -0.078--14.529
  for false-clean cases, versus median 1.201 and range 0.216--22.386 for genuine
  light recoveries. The accepted measurements genuinely favor radiative states
  in both groups, and their evidence overlaps without a truth-free threshold.
- The ordinary reverse loop transports from `hits[reverseHit+1]`, applies the
  process transition, updates `hits[reverseHit]` exactly once through baseline
  `addAndFit`, and only then cuts/reduces. No accidental duplicate measurement
  or directional ownership defect was found.
- Earlier studies already reject KL ranking, capacity 24, final mean/median/
  density-mode publication, representative-parent merge state, global identity
  preference, and energy/angle/literal-layer BH expansion as solutions.

Proceed in this order:

1. Keep the frozen tracker-only baseline unchanged and stop internal selection,
   covariance-scale, and component-capacity tuning unless new evidence exposes
   a specific defect.
2. Audit read-only whether CEPCSW already provides a stable, independently
   reconstructed calorimeter-energy or electron E/p observable at the point
   where `RecGsfTracking` could consume it. Define the physics question and
   validation cohorts before proposing any interface or implementation.
3. If broader integration is explicitly authorized, first test the independent
   observable as an offline truth-blind discriminator on the frozen false-clean
   and genuine light/hard cohorts. Do not make a runtime cut or package change
   from the current selected samples alone.
4. Treat a measurement-disjoint two-filter GSF only as a separate default-off
   formal research control. The seed-memory result does not predict that it
   will repair the clean tails.
5. Require a new independent light-tail dataset, focused verbose events,
   hard-loss events 11/16/17, same-code held-out population A/B, and separate
   secondary-topology and broad energy/angle reporting before any candidate is
   promoted.

Success means finding independent, truth-blind information that separates real
energy-loss recovery from tracker-fit fluctuations while preserving the no-
eBrem LCIO core and useful light/hard recovery. If no such observable exists or
it fails held-out validation, record the present cases as an information limit
of the current inputs rather than tuning another internal heuristic.

Current non-goals are changing source code without a new authorized design,
repeating the 24-component study, further final mean/median/mode publication
heuristics, promoting Runnalls, an ad hoc likelihood threshold, reverse seed or
global covariance/process-prior rescaling, VXD/relative-index corrections, an
energy/angle/literal-layer BH table without held-out truth support,
truth-dependent runtime logic, fitting SimHit momentum, or changing shared
tracking packages.

The completed reverse seed, innovation coverage, physical-region, and
measurement-ownership audit is preserved in
`agents_record/2026-08-09-reverse-seed-and-innovation-calibration-audit.md`.

The completed full-posterior contrast, correction of the discarded-correlation
hypothesis, and broad held-out BH dimension audit are preserved in
`agents_record/2026-08-09-full-posterior-and-bh-dimension-audit.md`.

The completed 500-event q/p-marginal density-mode stress test is preserved in
`agents_record/2026-08-08-qoverp-density-mode-500-event-screen.md`.

The completed 500x2 diagnosis and outgoing focus are preserved in
`agents_record/2026-08-08-completed-500x2-diagnostic-and-kl-focus-transition.md`.

The focused merge traces and representative-parent counterfactual are
preserved in
`agents_record/2026-08-08-kl-merge-causality-and-representative-state-counterfactual.md`.

The complete pre-curation live file and its links are preserved in
`agents_record/2026-07-23-AGENTS-pre-curation-snapshot.md`. The migration map
and curation rationale are in
`agents_record/2026-07-23-AGENTS-curation-map.md`.
