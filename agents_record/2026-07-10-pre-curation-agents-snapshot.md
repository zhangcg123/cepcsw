# CEPCSW GSF Development Guide

This file is the authoritative entry point for the Gaussian Sum Filter (GSF)
development work in this checkout. Read it before acting on older notes under
`agent_record/`.

Last consolidated: 2026-07-10  
Branch: `gsf-simhit-energy-loss-tuple-20260705`  
Consolidated through commit: `61fea67`

## 1. Project objective

Develop and validate an electron GSF refit for CEPCSW that can model
bremsstrahlung in tracker material and improve the estimate of the electron
state at the interaction point.

The intended chain is:

```text
Geant4 material-step truth
  -> step-t/X0-conditioned Bethe-Heitler model
  -> multi-component GSF filtering and smoothing
  -> validated IP track parameters
```

The present code is a research implementation, not a validated production
electron reconstruction algorithm.

Longer-term success means demonstrating a material-aware improvement over
`CompleteTracks`, especially for electron momentum tails, then extending beyond
the present single-particle-gun workflow to multi-track physics events. A merge
to the main development line should follow reproducible physics validation, not
only successful execution or lower fit chi-square.

## 2. Current status

### Working and validated

- `RecGsfTracking` builds, installs, and runs in the CEPCSW environment.
- Input is `CompleteTracks`; output is `GSFTracks`.
- Each GSF component now uses the baseline-compatible MarlinTrk sequence:

  ```text
  addHit(previous/reference hit)
    -> initialise(component state)
    -> addAndFit(current hit)
    -> export updated TrackState
  ```

- This replaced the failing direct `TKalTrack::AddAndFilter` workflow.
- Single-component tests on events 10, 12, 14, and 15 showed no recovery
  shortcuts and produced sensible parameters and non-zero chi-square.
- Multi-component tests (`MaxComponents=9`, TopN target 3) on the same events
  also showed zero recoveries and no catastrophic high-momentum smoothing
  output.
- A quiet ten-event regression (events 10-19, `MaxComponents=2`, TopN target
  1) fitted every selected track after the configuration cleanup.
- The true Geant4 pre/post-step recorder is the validated source for material
  energy-loss truth. `SimTrackerHit::getMomentum()` is only a detector-level
  cross-check, not a pre/post-material-step pair.
- At 1 GeV and theta 85 degrees, the validated 200-electron/200-muon G4-step
  comparison found mean event losses of 0.007116 GeV and 0.000898 GeV,
  respectively. The electron/muon loss ratio was 7.92 while their mean material
  budgets agreed within about 3 percent. This establishes that the electron
  tail is physical rather than a material-budget artifact.
- The larger LCIO baseline study at 2 GeV and theta 85 degrees contains 3500
  electron and 3500 muon tracks. Tracker-volume primary eBrem categories are
  `no=1373`, `light=1561`, and `hard=566`. Hard tracker eBrem is the dominant
  source of the electron pT tail.

### Not yet validated

- Neither available Bethe-Heitler model is validated for CEPC tracker steps.
- Broad GSF-versus-LCIO performance conclusions are premature.
- The current implementation has not demonstrated recovery of generated IP
  momentum in known hard-bremsstrahlung events.
- Improved chi-square alone is not evidence of energy-loss recovery. The
  reconstructed IP momentum must be compared to generator truth and categorized
  with tracker-volume primary eBrem truth.
- The persistent GSF history is baseline-compatible at the public MarlinTrk API
  level, but it does not adopt the wrapper's internal `TKalTrackSite` ownership.

### Active blocker

The active blocker is now physics inference and component lifetime, not the old
hit-update recovery crash.

The experimental `GlobalSim2GeV85` model always returns the same global
five-component retained-momentum distribution and ignores the individual step
`t/X0`. Its dominant component is a near-no-loss hypothesis:

| retained fraction `z` | prior weight |
|---:|---:|
| 0.3658 | 0.0242 |
| 0.6785 | 0.0345 |
| 0.9750 | 0.2027 |
| 0.9950 | 0.1593 |
| 0.99995 | 0.5793 |

Immediate TopN target 1 reduction normally keeps the `z=0.99995` component
after only one following hit. Hard-loss alternatives are deleted before enough
outer-hit curvature information accumulates. Consequently GSF follows LCIO in
the known hard-loss cases:

| event | truth pT [GeV] | LCIO pT [GeV] | GSF pT [GeV] |
|---:|---:|---:|---:|
| 11 | 2.000 | 1.793 | 1.793 |
| 16 | 2.000 | 1.812 | 1.812 |
| 17 | 2.000 | 1.579 | 1.579 |

The GSF chi-square improves in these events, but the generated momentum is not
recovered.

## 3. Immediate next work

Proceed in this order:

1. Use hard-loss events 11, 16, and 17 as the focused validation set.
2. Retain 3-5 components across several hits after a material split. Do not use
   immediate TopN target 1 as an energy-loss-recovery test.
3. Add a delayed-reduction policy or an equivalent component-age/minimum-hit
   rule, keeping the change local to `Reconstruction/RecGsfTracking`.
4. Record branch weight, momentum, chi-square, and ancestry until the lower-`z`
   hypotheses either become favored or are conclusively rejected.
5. Expose the true predicted crossing/residual on the current measurement
   surface. A `TrackState.referencePoint` is a parameterization pivot and must
   not be described as the predicted hit position.
6. Replace the global model with a mixture conditioned on the actual material
   step `t/X0`, fitted to primary-electron Geant4 eBrem truth.
7. Represent energy loss as a clear pre-material to post-material transition;
   then validate smoothing back to the IP.
8. Only after the above, run broad GSF-versus-LCIO performance studies.

Do not spend time re-debugging the resolved 2026-07-07 segmentation fault or
the removed direct-`AddAndFilter` initialization variants unless a new run
reproduces those failures in the current code.

## 4. Source-of-truth hierarchy

When records disagree, use this order:

1. Current source code and a fresh reproducible run.
2. This `AGENTS.md` status.
3. `Reconstruction/RecGsfTracking/README.md` for the active property surface.
4. `agent_record/2026-07-10-gsf-topn-energy-loss-status.md` for the latest
   evidence and next direction.
5. `agent_record/current-stage-and-todos.md` for accumulated project context.
6. `DEVELOPMENT.md` as an append-only chronological laboratory notebook.
7. `MEMORY.md` as a legacy index only; verify every link and status statement.
8. Older dated records as historical provenance only.

Never infer the current API from an old run card or dated debug note. Check
`GsfAlgorithm.h` first.

`MEMORY.md` and `DEVELOPMENT.md` do not currently describe the July 9-10 final
state. `MEMORY.md` stops at the earlier GlobalSim2GeV85 failure and links to
several records/plans that are absent or deleted in this working tree.
`DEVELOPMENT.md` has a July 5 “Current Stage” and ends at the July 8 global-model
integration. Their measurements and chronological reasoning remain useful, but
their TODO lists and architecture descriptions are superseded here.

### Status lifecycle

Keep this file centered on the present, not as an append-only development log.

When the project concentration changes:

1. Update the objective, current status, active blocker, and immediate next work
   here in the same change that establishes the new direction.
2. Remove superseded detail from the current sections instead of leaving
   several generations of “current” conclusions in place.
3. Preserve evidence, failed experiments, resolved incidents, and design
   rationale in a dated file under `agent_record/` before removing details that
   are not already recorded elsewhere.
4. Add the dated record to the appropriate current, operational, historical, or
   stale category in the record organization below.
5. Keep only a short historical pointer here when the old issue still explains
   an important constraint in the current design.

Retrieval should follow the same lifecycle:

- load this file first to recover the current concentration;
- inspect current code and the latest decision record when needed;
- do not load historical debug records by default;
- load history only to investigate a regression, recover design rationale,
  compare a previous experiment, or answer an explicit provenance question.

An old failure, workaround, or TODO must not become active merely because it is
described in more detail than the current status.

## 5. Code and data map

### Active GSF implementation

| Path | Role |
|---|---|
| `Reconstruction/RecGsfTracking/src/GsfAlgorithm.{h,cpp}` | Main filtering, component update, reduction, smoothing, output, diagnostics |
| `Reconstruction/RecGsfTracking/src/GsfComponent.{h,cpp}` | Component state, history, cloning, chi-square bookkeeping |
| `Reconstruction/RecGsfTracking/src/GsfMixture.{h,cpp}` | Weight normalization, KL merging, TopN pruning |
| `Reconstruction/RecGsfTracking/src/BetheHeitlerSplitter.{h,cpp}` | Current and experimental BH mixtures and curvature splitting |
| `Reconstruction/RecGsfTracking/src/GsfFlatTuple.{h,cpp}` | Flat analysis tuple |
| `Reconstruction/RecGsfTracking/README.md` | Active configuration reference |

### Truth and studies

| Path | Role |
|---|---|
| `Simulation/DetSimAna/src/GsfMaterialStepRecorderAnaElemTool.{h,cpp}` | Geant4 pre/post-step recorder |
| `G4MaterialStepComparison/` | Material/process truth studies |
| `BHModelComparisonStudies/` | Current and experimental BH comparisons |
| `TrackingPerformanceStudies/` | LCIO tracking studies and eBrem categories |
| `SimHitEnergyLoss/` | SimHit-level cross-check only |

For the 2 GeV, theta 85 degree performance study, interpret `no/light/hard
eBrem` as tracker-volume primary eBrem categories. The all-material category is
recorded but does not usefully separate events at this point.

### Production workflow

| Path | Role |
|---|---|
| `DumpGsfTrks/sim.py.bk` | Simulation template |
| `DumpGsfTrks/trk.py.bk` | Digitization/tracking template |
| `DumpGsfTrks/gsf.py.bk` | GSF/tuple template |
| `dump_gsftrk.sh` | Three-stage worker |
| `subtrkjobs.sh` | Batch submission wrapper |

Generated `runsim-*`, `runtrk-*`, and `rungsf-*` cards are artifacts, not the
configuration source of truth.

## 6. Active configuration policy

The runtime configuration was concentrated in commit `61fea67`. The obsolete
alternate KF fitter, recovery modes, KF seed-error controls, and experimental
initialization modes were removed.

Configuration is grouped into:

- physics/material: `ElectronHypothesis`, `BHModel`, `BHSplitThreshold`,
  `MSOn`, `ElossOn`, `KappaSeedCov`;
- mixture/output: `MaxComponents`, `ReductionTargetComponents`,
  `ReductionMode`, `GSFOutputMode`, `MaterialIPExtrapolation`;
- selection/diagnostics: `SelectedEventIndices`, `VerboseDump`,
  `VerboseSplitDump`, `ComponentDebugDump`, `ComponentDebugMaxHistory`.

Important semantics:

- `MaxComponents` gates whether a split starts; it is not a strict
  instantaneous child-count ceiling. For example, one component can split to
  five even when `MaxComponents=2`, followed by reduction.
- `VerboseDump`, `VerboseSplitDump`, and `ComponentDebugDump` default to false.
- Use full component diagnostics only for a small selected event list.
- `GlobalSim2GeV85` is experimental and must be named explicitly.

## 7. Build and focused validation

Use the configured EL9/LCG 105 build in this checkout:

```bash
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt \
  --target RecGsfTracking -j4
cmake --install build.105.0.0.x86_64-el9-gcc11-opt
```

Run with the installed environment and an explicit option file:

```bash
source setup.sh
build.105.0.0.x86_64-el9-gcc11-opt/run \
  gaudirun.py path/to/options.py
```

For every behavior-changing patch, verify at least:

- the target builds and installs;
- events 10, 12, 14, and 15 still fit without recovery or catastrophic IP
  output;
- hard-loss events 11, 16, and 17 are checked separately;
- output contains full-hit tracks, finite parameters, and non-zero chi-square;
- component-flow logging agrees with the intended split/update/reduce order.

ROOT PCM messages about missing build-tree dictionaries are longstanding noise
when the application otherwise finalizes successfully; do not confuse them
with a GSF fit failure.

## 8. Record organization

The existing `agent_record/` directory is append-only provenance and contains
contradictory snapshots. Treat files as follows.

### Root-level legacy records

- `MEMORY.md` — legacy RAG/navigation index. It contains useful study pointers
  and July 8 statistics, but also broken links to removed plans and records.
  Do not use its “read first” ordering as the current authority order.
- `DEVELOPMENT.md` — chronological development log. Its Findings 1-17 preserve
  the progression from seed hypotheses, through broken low-`tX0` BH behavior,
  SimHit studies, the Geant4 recorder, and GlobalSim2GeV85 integration. Its top
  status and TODOs predate the baseline-style component-update repair.
- `README.md` — generic CEPCSW build entry point, not a GSF status record.

### Current decision records

- `2026-07-10-gsf-topn-energy-loss-status.md`
- `current-stage-and-todos.md` — useful accumulated context, but its earlier
  sections may be superseded by later sections in the same file
- `2026-07-05-g4-material-step-recorder.md`
- `2026-07-06-g4step-analysis-script.md`

### Operational references; verify against current files

- `gsf-build-system.md`
- `gsf-batch-production.md`
- `gsf-data-flow.md`
- `gsf-mixture-reduction.md`
- `gsf-bethe-heitler-model.md`

### Historical debugging provenance

- `2026-07-07-temp-gsf-electron-refit-crash.md` — resolved
- `2026-07-08-global-bh-gsf-run.md`
- `2026-07-08-gsf-cov-reducer-debug.md`
- `2026-07-08-gsf-light-ebrem-component-diagnostics.md`
- `2026-07-09-gsf-hit2-recovery-smoothing-failure.md`
- `2026-07-10-remove-dead-gsf-init-flows.md`

### Stale inventories; do not use as current status

- `gsf-project-overview.md`
- `gsf-algorithm-flow.md`
- `gsf-code-map.md`
- `gsf-analysis-tools.md`
- `gsf-known-issues.md`

These inventories mention removed properties, old direct-update behavior,
deleted analysis scripts, analytical prefit/multi-pass logic, and outdated line
counts. Update or archive them before citing them as current documentation.

Study-directory README files are evidence-specific and should remain with their
outputs. In particular:

- `G4MaterialStepComparison/README.md` documents the validated G4-step inputs
  and electron/muon comparison;
- `BHModelComparisonStudies/*/README.md` documents model construction and
  truth-selection details, but statements that “GSF output is known not to
  work” predate the component-update repair and now mean “physics performance
  remains unvalidated”;
- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/README.md`
  defines the canonical baseline selection and tracker-eBrem categories;
- `SimHitEnergyLoss/root_files/README.md` is an artifact inventory and does not
  promote SimHit momentum to material-step truth.

New records should be one of:

- a short dated decision/evidence record under `agent_record/`;
- a maintained operational reference next to the code it documents; or
- an update to this file when project status, authority, or next priorities
  change.

Avoid appending a new “current status” section to several files for the same
finding.

Before ending a substantial development stage, perform a documentation sweep:

- make the current concentration in this file match the tested code state;
- move newly superseded detail to a dated `agent_record/` entry;
- downgrade resolved records to historical provenance;
- repair or label stale links in indexes;
- avoid copying the same status narrative into multiple maintained files.

## 9. Repository hygiene

This checkout has a large dirty working tree containing generated cards, plots,
tables, notebooks, and study outputs. Assume unrelated modifications belong to
the user.

- Stage files explicitly; never use `git add -A` here.
- Keep source/documentation commits separate from generated ROOT files, logs,
  plots, and batch cards.
- Do not restore or delete unrelated changes during GSF work.
- Prefer `/tmp` for disposable output when it remains visible to the workflow;
  otherwise use a clearly named untracked diagnostics directory.
- Before pushing, run `git diff --cached --check` and inspect the staged file
  list.

## 10. Scope constraints

- Keep GSF fixes local to `Reconstruction/RecGsfTracking` unless a broader
  change is explicitly requested and justified.
- Do not modify KalTest, TrackSystemSvc, MarlinTrk, or DDKalTest to compensate
  for a GSF-specific workflow problem.
- Do not hand-code a parallel Kalman update when the baseline MarlinTrk path can
  provide the required operation.
- Do not describe SimHit momentum as exact material-step energy-loss truth.
- Do not claim the BH model or GSF physics performance is validated until the
  hard-loss recovery and step-conditioned model tests pass.
