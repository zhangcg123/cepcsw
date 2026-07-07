---
name: current-stage-and-todos
description: Current GSF development stage, validated truth source, and prioritized TODOs
metadata:
  type: status
  date: 2026-07-05
---

# Current Stage and TODOs

## Current Stage

The project has moved from SimHit-level momentum diagnostics to a true Geant4 pre/post-step truth source for CEPC Bethe-Heitler tuning.

Current validated truth source:

```text
GsfMaterialStepRecorderAnaElemTool
tree: g4step_tuple
one entry per event, step data in vector<> branches
key variables: pre_p, post_p, retained, loss, step_tX0, material, process
PrimaryOnly = false (records all tracks)
```

The SimHit-level study remains useful as a detector-level cross-check, but it is no longer the source for final BH parameter fitting because `SimTrackerHit::getMomentum()` is momentum at the sensitive hit position, not a material-step pre/post pair.

## Validated Facts

1. The existing ACTS/ATLAS BH parameterization is not reliable for CEPC thin tracker material. At low `tX0`, it produces unphysical large-loss components.
2. The true G4 pre/post-step recorder builds and runs in CEPCSW.
3. A 5-event electron smoke test produced `gsf_material_steps_test.root` with 335 true G4 steps.
4. A 200 e- vs 200 mu- comparison at 1 GeV, theta=85 deg confirms the electron loss tail with true G4 pre/post-step truth:

```text
electron mean event loss = 0.007116 GeV
muon mean event loss     = 0.000898 GeV
electron/muon loss ratio = 7.92
endpoint loss ratio      = 19.06
mean material t/X0 ratio = 1.03
```

Conclusion: the electron bremsstrahlung / energy-loss tail is real and stronger than muon, now confirmed with the correct truth source.

## Resolved Operational Issues

- The 2026-07-07 electron GSF refit segmentation fault is repaired. `agent_record/2026-07-07-temp-gsf-electron-refit-crash.md` is now provenance only, not an active handoff. Do not spend the next session reinstalling/revalidating that old crash unless a new run reproduces it.

## Current Files To Know

Code:

```text
Simulation/DetSimAna/src/GsfMaterialStepRecorderAnaElemTool.h
Simulation/DetSimAna/src/GsfMaterialStepRecorderAnaElemTool.cpp
Simulation/DetSimAna/CMakeLists.txt
Reconstruction/RecGsfTracking/options/run_gsf_material_step_recorder_test.py
DumpGsfTrks/sim.py.bk  (integrated as of 2026-07-06)
```

Generated comparison material:

```text
G4MaterialStepComparison/
gsf_material_steps_e200.root
gsf_material_steps_mu200.root
```

SimHit cross-check files:

```text
SimHitEnergyLoss/root_files/electron/
SimHitEnergyLoss/root_files/muon/
```

## Prioritized TODOs

### 1. Build a dedicated G4-step analysis script

Read `g4step_tuple` and produce:

- event-level endpoint retained momentum
- event-level cumulative loss
- single-step `z = post_p/pre_p`
- single-step loss tails
- material and process breakdown
- `loss` vs `step_tX0`
- `eBrem`-only distributions vs all-step distributions
- electron vs muon control plots

### 2. Produce larger true G4-step electron samples

The 200-event comparison validates the direction but is not enough for stable BH fitting.

Start with the baseline point:

```text
particle: e-
energy: 1 GeV
theta: 85 deg
truth source: GsfMaterialStepRecorderAnaElemTool
```

Muon is now mainly a control sample; it does not need equal statistics unless needed for validation plots.

### 3. Decide the exact BH fitting target

Likely target:

```text
primary electron eBrem steps only
z = post_p / pre_p
conditioned or binned by step_tX0
```

Open decision: exclude ionization/transportation momentum changes from the BH fit, or model them separately as non-BH effects.

### 4. Fit the CEPC-specific BH mixture

After larger G4-step samples exist:

- fit the CEPC-specific `z` distribution vs `tX0`
- compare to the current ACTS/ATLAS BH parameterization
- produce a CEPC-specific mixture table or replacement model
- validate inside GSF tracking

### 5. Clean output and commit policy

Keep code/documentation changes separate from large generated ROOT outputs. Decide whether ROOT outputs stay in working storage, move to CEFS/AIFS output storage, or are ignored by git.

## Historical / Superseded Work

The previous SimHit analysis and BH toy mixture tests are useful provenance, but they are not the current path for final BH fitting. The current path is true G4 pre/post-step truth -> G4-step analysis -> CEPC-specific BH mixture fit.
