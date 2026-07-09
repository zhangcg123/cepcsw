---
name: 2026-07-05-g4-material-step-recorder
description: Real Geant4 pre/post-step momentum recorder for CEPC GSF Bethe-Heitler tuning
metadata:
  type: reference
  date: 2026-07-05
  updated: 2026-07-06
---

# Geant4 Material Step Recorder for GSF BH Tuning

## Purpose

`SimTrackerHit::getMomentum()` is useful but not sufficient for final CEPC Bethe-Heitler tuning because it stores momentum at sensitive hit positions, not per-material-step pre/post momentum.

To provide the needed truth source, a new `IAnaElemTool` was added:

```text
GsfMaterialStepRecorderAnaElemTool
```

It runs inside Geant4 stepping through `DetSimAlg.AnaElems` and records real `G4Step` pre/post information.

## Output Format (2026-07-06 update)

Default ROOT output:

```text
gsf_material_steps.root
tree: g4step_tuple   — one entry per event (not per step)
```

### Tree structure

```
g4step_tuple — one TTree entry per event

Event-level scalars:
  event_id    (int)    — G4 event ID
  step_count  (int)    — number of steps recorded for this event

Step-level vectors (all same length = step_count):
  track_id, parent_id, pdg, charge                     (vector<int>)
  step_status_pre, step_status_post, process_subtype   (vector<int>)
  pre_volume_copy_no, post_volume_copy_no              (vector<int>)

  pre_x, pre_y, pre_z, pre_r    (vector<float>, mm)
  post_x, post_y, post_z, post_r (vector<float>, mm)
  mid_x, mid_y, mid_z, mid_r    (vector<float>, mm)

  pre_px, pre_py, pre_pz, pre_p, pre_pT    (vector<float>, GeV/c)
  post_px, post_py, post_pz, post_p, post_pT (vector<float>, GeV/c)
  dp, loss, retained                       (vector<float>, GeV / ratio)

  pre_ekin, post_ekin, dekin    (vector<float>, GeV)
  edep, nonion_edep              (vector<float>, GeV)
  step_length                    (vector<float>, mm)
  material_radlen                (vector<float>, mm)
  step_tX0                       (vector<float>, unitless)
  global_time_pre, global_time_post (vector<float>, ns)

  pre_volume, post_volume        (vector<string>)
  material, process              (vector<string>)
```

## Default Configuration

```text
PDGs = [11, -11]
PrimaryOnly = false   (changed from true on 2026-07-06 — record all tracks)
TrackerOnly = true
MinStepLengthMm = 0.0
MinAbsLossGeV = 0.0
RecordZeroLoss = true
```

## Integration into sim.py.bk

As of 2026-07-06, the recorder is integrated into `DumpGsfTrks/sim.py.bk`:

- **line 93**: `from Configurables import GsfMaterialStepRecorderAnaElemTool`
- **lines 94–101**: configuration — `OutputFile = "gsf_material_steps.root"`, PDGs `[11, -11, 13, -13]`, `PrimaryOnly = False`, `TrackerOnly = True`, `MinStepLengthMm = 0.0`, `MinAbsLossGeV = 0.0`, `RecordZeroLoss = True`
- **line 114**: added to `DetSimAlg.AnaElems` list as `"GsfMaterialStepRecorderAnaElemTool"`

This means the recorder runs automatically for every simulation job using this script, producing `gsf_material_steps.root` alongside the standard `sim_v01.root` output.

## How To Use

Use this recorder, not SimTrackerHit momentum, for final CEPC-specific BH mixture fitting. The fit target should be local material-step:

```text
z = post_p / pre_p
loss = 1 - z
tX0 = step_tX0
```

SimTrackerHit analysis remains useful for event-level cross-checks and selecting interesting samples, but fitted BH parameters should come from `g4step_tuple`.

## Analysis Macro

`G4MaterialStepComparison/macros/compare_g4step_e_mu.C` reads the event-per-entry format using `SetBranchAddress` on `vector<>` branches. Updated 2026-07-06 to match the new tree structure.

## Change Log

- **2026-07-05**: Initial creation. Step-per-entry format, PrimaryOnly=true.
- **2026-07-06**: Refactored to event-per-entry format (step data stored in vectors). Changed PrimaryOnly default to false to record all tracks, not just primaries. Updated compare macro to match.
