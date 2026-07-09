---
name: 2026-07-06-g4step-analysis-script
description: Dedicated G4 step tuple analysis script and baseline 1000-event outputs
metadata:
  type: status
  date: 2026-07-06
---

# G4 Step Tuple Analysis Script

Implemented the dedicated `g4step_tuple` analysis requested in the current GSF TODOs.

## New Script

```text
G4MaterialStepComparison/scripts/analyze_g4step_tuple.py
```

The script reads one or more `GsfMaterialStepRecorderAnaElemTool` ROOT files and produces:

- event-level primary endpoint retained momentum
- event-level all-step and primary-track cumulative loss
- event-level primary `eBrem` loss
- single-step `z = post_p / pre_p`
- single-step loss-tail summaries
- all-step vs primary electron `eBrem` distributions
- material and process breakdown CSVs
- `loss` vs `step_tX0` plots
- electron/muon comparison plots and ratios when multiple samples are passed

## Validated Run

Command:

```bash
G4MaterialStepComparison/scripts/analyze_g4step_tuple.py \
  --sample electron=gsf_material_steps-e--1.0-85-1.root \
  --sample muon=gsf_material_steps-mu--1.0-85-1.root \
  --outdir G4MaterialStepComparison/analysis_e1000_mu1000
```

Output directory:

```text
G4MaterialStepComparison/analysis_e1000_mu1000/
```

Key numbers:

```text
electron events = 1000
muon events = 1000
electron primary eBrem steps = 232
mean electron primary event loss = 0.019868708 GeV
mean muon primary event loss     = 0.00085341156 GeV
electron/muon primary loss ratio = 23.281508
electron/muon endpoint-loss ratio = 26.642769
electron/muon primary t/X0 ratio = 1.1326906
primary endpoint R q01/q50/q99 = 184.195 / 233.940 / 235.354 mm for e-
primary endpoint R q01/q50/q99 = 184.398 / 233.936 / 235.378 mm for mu-
```

Endpoint definition was corrected on 2026-07-06 to use the outermost recorded primary step (`max post_r`) and innermost primary start (`min pre_r`). The endpoint R values are short because the recorder default is `TrackerOnly = true`; these ROOT samples record tracker-region material steps, not full-detector endpoints.

## Next TODO

Use the script on larger electron-only production samples at the baseline point, then fit the primary electron `eBrem` `z` distribution vs `step_tX0` for a CEPC-specific Bethe-Heitler mixture.
