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

The active question is whether the exact material thickness supplied to the
Bethe-Heitler (BH) splitter at the first wrong branch decision is consistent
with Geant4 energy-loss truth and with the domain of the configured BH mixture.
The investigation must distinguish a material path/ownership defect, an
interval-collapse granularity problem, a BH response mismatch, and a later
measurement/selection effect. The ECAL prototype is paused and remains
default-off; this focus does not authorize changes outside `RecGsfTracking`.

Freeze the production baseline, with the
2026-08-18 material-direction correction as the sole intentional difference
from tag `gsf-memory-leak-fixed-2026-08-08`: five-component
`CEPC2GeV85StepConditioned`, `MaxComponents=12`,
`ReductionTargetComponents=0`, `SymmetricKL`, identity-lineage protection,
`ComponentWeightCutoff=1e-4`, forward-posterior reverse weights, reverse full
covariance scale 100, `AggregateWeight`, and reverse `BestBranch` publication.
`MaterialPathMode=CurrentSurface` now governs both outward and inward
propagation. Before this correction, reverse propagation accidentally always
used `DD4hepBetweenSurfaces`; pre-fix reverse tuples must therefore be rerun for
same-code comparisons.
The 24-component setting has already been tested: it preserves both genuine
recovery and false radiative modes and does not solve selection.

The committed production steering contract is `CurrentSurface`,
`BHSplitThreshold=1e-4`, `ComponentWeightCutoff=1e-4`, and ECAL off. The live
working copy of `DumpGsfTrks/gsf.py.bk` is locally modified to DD4hep, `1e-8`,
`1e-8`, and ECAL on. Treat those as uncommitted experimental steering, not new
defaults, and do not silently use or overwrite them.

Current boundary evidence and definitions:

- A Geant4 pre/post step is truth-side energy-loss information. It is not one
  runtime GSF propagation interval or one BH call.
- `CurrentSurface` supplies surface-owned normal thickness projected by the
  component incidence. `DD4hepBetweenSurfaces` integrates geometry material
  along a component trajectory between bounding measurement surfaces.
- The BH-call population is one executed
  `split(parent, pathTX0, bz[, reverse])` per valid component path above the
  split threshold. Candidate paths, event-hit medians, hit-level split counts,
  and Geant4 transitions are not interchangeable with this population.
- The direction-symmetry defect is fixed: `MaterialPathMode` now governs both
  outward and inward propagation. Pre-fix reverse tuples must be regenerated.
- A selected 30-event DD4hep audit, explicitly using split/cutoff `1e-4` and
  ECAL off, contained 4,297 actual BH parent calls: 1,953 forward (including
  ten initial seed-material calls) and 2,344 reverse. Forty-three calls were
  above the last CEPC BH knot (`0.024494897427831779 t/X0`), 19 were at least
  `0.03 t/X0`, the maximum was `0.0319038626 t/X0`, and none reached 0.05.
- The CEPC model saturates above its last knot by returning the last-knot
  mixture. The selected audit therefore does not support the broad statement
  that “too many” calls exceed the model range, but rare saturated calls can
  still seed the winning lineage and require branch-local diagnosis.
- The seed-216 dispatch check contained much larger DD4hep intervals, including
  about `0.183 t/X0`, but it was not part of the uniform 30-event population.
- The 30 events were deliberately selected good/bad examples across no/light/
  hard eBrem. They are mechanism evidence, not an unbiased rate or validation.

Proceed in this order:

1. Freeze and print exact steering for every run. Keep the production baseline
   unchanged and use temporary cards for material controls.
2. Capture every actual BH call, including the seed path, on an unbiased
   sample. Record event, direction, bounding surfaces, parent identity/weight,
   material composition, `pathTX0`, and the returned BH mixture.
3. In bad events, locate the first surface where a truth-compatible lineage
   loses posterior rank or is removed, and compare matched good controls.
4. At that crossover, compare `CurrentSurface`, DD4hep interval composition,
   and Geant4 pre/post-step truth between the same spatial boundaries. Compare
   directions only for equivalent component states.
5. Check BH energy-loss closure at the exact runtime input against Geant4
   retained-energy fractions across energy, angle, and detector region,
   treating last-knot saturation explicitly.
6. Classify the cause as path/ownership, interval-collapse granularity, BH
   response, or downstream measurement/selection before proposing a change.
   Then apply the focused verbose, hard-event 11/16/17, and held-out clean-track
   validation gates.

Success means identifying a truth-blind discrepancy at or before the first bad
branch decision that separates bad events from matched good controls and
predicts a same-code correction without sacrificing the no-eBrem core. If the
exact path and BH response close correctly at the crossover, record that result
and move downstream instead of retuning material.

Current non-goals are changing source before this diagnosis, promoting
`DD4hepBetweenSurfaces`, tuning split/cutoff thresholds, capacity, KL ranking,
reverse seed covariance, final publication heuristics, or the ECAL selector;
fitting SimHit momentum; truth-dependent runtime logic; changing source outside
`RecGsfTracking`; or changing shared tracking packages.

The exact definitions, selected audit, steering warning, and complete ordered
investigation are preserved in
`agents_record/2026-08-18-runtime-material-path-and-bh-input-consistency.md`.
The direction defect and correction are preserved in
`agents_record/2026-08-18-material-path-mode-direction-symmetry.md`.

The paused ECAL boundary, deferred work, and links to its complete evidence are
preserved in
`agents_record/2026-08-18-ecal-focus-handoff-to-material-bh.md`. Older tracker,
BH, KL, and workflow evidence remains available under `agents_record/` and is
historical unless explicitly reactivated.
