# G4 Material Step Electron/Muon Comparison

Generated on 2026-07-05 in remote CEPCSW.

Samples:

- `gsf_material_steps_e200.root`: 200 primary e-, 1 GeV, theta=85 deg
- `gsf_material_steps_mu200.root`: 200 primary mu-, 1 GeV, theta=85 deg

Both use `GsfMaterialStepRecorderAnaElemTool` and write true Geant4 pre/post-step information to `g4step_tuple`.

Run cards are stored in `options/`. The ROOT comparison macro is stored in `macros/compare_g4step_e_mu.C`. Text output is stored in `compare_e200_mu200_summary.txt`.

Main result: electron has a much stronger event-level energy-loss tail than muon at comparable material budget. In this 200-event check, mean event momentum loss is about 7.9x larger for electron, and mean endpoint loss is about 19.1x larger.

## Dedicated `g4step_tuple` Analysis

A Python/PyROOT analysis script is available at:

```bash
G4MaterialStepComparison/scripts/analyze_g4step_tuple.py
```

Example run for the current 1000-event baseline samples in the CEPCSW top directory:

```bash
G4MaterialStepComparison/scripts/analyze_g4step_tuple.py \
  --sample electron=gsf_material_steps-e--1.0-85-1.root \
  --sample muon=gsf_material_steps-mu--1.0-85-1.root \
  --outdir G4MaterialStepComparison/analysis_e1000_mu1000
```

The script reads `g4step_tuple` and writes:

- `summary.txt`: event-level endpoint retained momentum, cumulative loss, single-step `z`, loss tails, eBrem-only distributions, material and process breakdowns, and electron/muon ratios.
- `event_summary.csv`: one row per event with primary endpoint retained momentum, all-step and primary-track cumulative loss, primary eBrem loss, and material budget.
- `step_summary.csv`: one row per Geant4 step with `z = post_p/pre_p`, loss, `step_tX0`, process, material, and primary-track flag.
- `process_counts.csv` and `material_counts.csv`: breakdown tables.
- PNG/PDF plots for endpoint retention, cumulative loss, single-step `z`, loss tails, eBrem-only distributions, and `loss` vs `step_tX0`.

For the current 1000 e-/1000 mu- baseline run, outputs are in:

```text
G4MaterialStepComparison/analysis_e1000_mu1000/
```

Key 1000-event baseline numbers from `summary.txt`:

```text
electron primary eBrem steps = 232
mean electron primary event loss = 0.019868708 GeV
mean muon primary event loss     = 0.00085341156 GeV
electron/muon primary loss ratio = 23.281508
electron/muon endpoint-loss ratio = 26.642769
electron/muon primary t/X0 ratio = 1.1326906
primary endpoint R q01/q50/q99 = 184.195 / 233.940 / 235.354 mm for e-
primary endpoint R q01/q50/q99 = 184.398 / 233.936 / 235.378 mm for mu-
```

These numbers use primary-track event quantities. The endpoint is now defined by the outermost recorded primary step (`max post_r`) divided by the innermost primary pre-step momentum (`min pre_r`), not by raw tuple order. The endpoint R values are short because `GsfMaterialStepRecorderAnaElemTool` currently defaults to `TrackerOnly = true` and records steps only inside `tracker_region_rmax/zmax`; these samples are therefore tracker-material studies, not full-detector propagation endpoint studies. All-step quantities remain available in the CSVs and summary for control plots, but the likely BH fitting target is the primary electron `eBrem` subset.

## Energy-Point Material-Step Studies

The focused step-wise studies are organized by beam momentum and polar angle under:

```text
G4MaterialStepComparison/studies/
```

Current study points:

- `e1p0_theta85/`: completed 1 GeV, theta=85 deg electron/muon study.
- `e2p0_theta85/`: matching 2 GeV, theta=85 deg electron/muon study, using the same scripts and output conventions.

Each point keeps its own `scripts/`, `plots/`, summary text files, and README. The raw `gsf_material_steps-*.root` tuples remain in the CEPCSW top directory and are not copied into the study folders.

For the completed 1 GeV point, primary `eBrem` almost always continues with the same `track_id=1`. The dominant primary-electron eBrem peak is at `R = 1.8-1.9 m`. A smaller `R ~ 100 mm` peak is from forward/MDI material after the low-pT electron curls back to small radius at `z ~ 0.8-1.0 m`, mainly `BeamPipe_BeforeCryoW` tungsten and LumiCal/anti-solenoid material.
