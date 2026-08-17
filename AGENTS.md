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
A default-off ECAL component-re-ranking prototype is also mechanically
operational. It preserves `GSFTracks` and writes its paired result separately;
its focused evidence is promising only for retained bimodal alternatives and
is not population-validated.

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

The active question is whether the default-off ECAL component-re-ranking
prototype improves held-out populations without damaging clean tracks. It is
an experimental paired output, not a production replacement, and does not
authorize changes outside `RecGsfTracking`.

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

- The feature is default-off, uses only GSF states plus `EcalCluster`, changes
  no fitted component, leaves `GSFTracks` exactly unchanged in direct on/off
  tests, and writes `GSFTracksEcalConstrained` only when enabled.
- `RecGsfFlatTuple` now preserves the ordinary `gsf_*` fields and adds parallel
  `ecal_gsf_*` fields, availability/change flags, and a constrained pT
  residual. Default-off jobs write unavailable, zeroed constrained fields.
- The configurable two-sided gate defaults to `max(p/E,E/p)>1.1`. The final
  component likelihood is Gaussian in `log(p/E)` with width 0.15 and a 0.05
  floor, so ECAL can re-rank but cannot arbitrarily overwrite tracker evidence.
- In 26 focused single-track overshoots above 3%, 15 activated and four changed
  branch. All four changes improved the truth residual; three ended within
  0.3%, while one moved from +185.4% to -13.1%. This selected cohort is branch-
  mechanism evidence, not clean-track safety or population validation.
- The broad paired screen now has independent Geant4 and SimHit labels. Of
  40,091 valid events, 35,681 are topology-clear and 4,410 form the secondary-
  tracker-activity control. The clear sample contains 14,924 no-eBrem, 16,638
  light-eBrem, and 4,119 hard-eBrem events under outgoing surface ownership.
- In the topology-clear sample, ECAL changed 150 branches: 15/12 improved/
  worsened in no-eBrem, 32/20 in light-eBrem, and 59/12 in hard-eBrem. It
  reduces greater-than-100% tail counts from 17 to 7, 17 to 4, and 76 to 59,
  respectively, but leaves the central width essentially unchanged.
- Clean-track safety is not established. ECAL changed 35 topology-clear events
  whose ordinary GSF residual was within 1%; only five improved and 30
  worsened. Twenty-two moved outside 1%, two outside 3%, and none outside 10%.
  The largest ordinary GSF overshoot, +42,661%, was unchanged, so surviving-
  component support remains a hard limitation.
- Transition and physical-region results preserve the earlier information
  boundary: hard losses at transitions 0--4 remain poorly recovered, 5--8 are
  mixed, and >=12 is much easier. For hard events, ordinary GSF median/width68
  are -16.5%/24.5% for VXD-owned loss, -2.12%/29.2% for ITK, and
  -0.0745%/1.60% for TPC. ECAL acts most usefully on hard transitions 3--6,
  improving 45 of 55 changes there.
- With the 0.05 likelihood floor, ECAL supplies at most a factor-20 relative
  boost. A competing branch below about 5% of the baseline tracker score can
  never win. The four successful candidates had tracker-score ratios 0.989,
  0.746, 0.194, and 0.0549; three unrepaired extreme overshoots had their
  closest-energy alternatives at ratios 0.00267 or far smaller. The practical
  blocker is surviving posterior support, not merely the activation threshold.
- One available transition-0--4 underestimate also remained unchanged at
  threshold 1.05: its truth-like component had a tracker score about 1265 times
  smaller while both momenta were consistent with reconstructed energy. A
  threshold alone cannot restore missing or negligible posterior support.
- Category/transition labels were derived directly from the matching 100-event
  SimHit and material-step files and mechanically cross-checked against the
  established ROOT formulas and outgoing-owned transition builder. This is
  still a studied sample, not independent held-out validation. The cluster
  projection, 0.10-rad windows, energy width, floor, and threshold remain
  unvalidated.
- Previous reverse seed, innovation, measurement-ownership, KL, capacity,
  publication-mode, and BH-dimension conclusions remain unchanged; ECAL does
  not justify resuming tracker-internal heuristic tuning.

Proceed in this order:

1. Keep the frozen tracker-only baseline unchanged and stop internal selection,
   covariance-scale, and component-capacity tuning unless new evidence exposes
   a specific defect.
2. Keep the ECAL experiment default-off and paired. Diagnose the 44 worsened
   topology-clear changes, especially the 30 changes starting inside the
   ordinary GSF 1% core, and the unrepaired extreme tails before tuning.
3. Audit ECAL matching and energy closure for false changes versus successful
   hard transition-3--6 changes across energy, angle, and detector region.
   Preserve no-eBrem, light, hard, and secondary-control reporting separately.
4. Freeze any proposed selector revision before testing it on a new independent
   broad sample. Repeat focused verbose checks and the required hard events
   after regenerating their deleted canonical input.
5. Treat a measurement-disjoint two-filter GSF only as a separate default-off
   formal research control. The seed-memory result does not predict that it
   will repair the clean tails.

Success means finding independent, truth-blind information that separates real
energy-loss recovery from tracker-fit fluctuations while preserving the no-
eBrem LCIO core and useful light/hard recovery. If no such observable exists or
it fails held-out validation, record the present cases as an information limit
of the current inputs rather than tuning another internal heuristic.

Current non-goals are enabling the ECAL prototype by default, replacing track
parameters with calorimeter energy, promoting threshold 1.1 from the selected
sample, changing source outside `RecGsfTracking`, repeating the 24-component
study, further final mean/median/mode heuristics, promoting Runnalls, reverse
seed or global covariance/process-prior rescaling, VXD/relative-index
corrections, an energy/angle/literal-layer BH table without held-out truth
support, truth-dependent runtime logic, fitting SimHit momentum, or changing
shared tracking packages.

The complete design, property surface, mechanical validation, focused
overshoot and early-transition results, and outgoing focus are preserved in
`agents_record/2026-08-17-default-off-ecal-component-constraint-prototype.md`.

The completed paired ordinary/constrained flat-tuple branch implementation and
enabled/default-off smoke validation are preserved in
`agents_record/2026-08-17-flat-tuple-paired-ecal-track-output.md`.

The expanded 20-event overshoot table, combined 26-event counts, exact branch-
choice inequality, successful and failed competitor evidence, and outgoing
validation gate are preserved in
`agents_record/2026-08-17-expanded-overshoot-branch-choice-diagnosis.md`.

The first 40,091-event broad paired population screen, input audit, global and
changed-subset metrics, clean-core safety failure, and plot inventory are
preserved in
`agents_record/2026-08-17-broad-ecal-component-constraint-population-screen.md`.

The topology-clear/secondary split, authoritative no/light/hard categories,
dominant transition and physical-region results, clean-safety audit, and plot
inventory are preserved in
`agents_record/2026-08-17-topology-clear-ecal-category-transition-screen.md`.

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
