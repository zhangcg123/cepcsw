---
name: gsf-analysis-tools
description: Analysis scripts, plotting tools, and what they produce
metadata:
  type: reference
---

# GSF Analysis Tools

## Scripts (in `Reconstruction/RecGsfTracking/scripts/`)

### `compare_pt.py`
Simple script: reads one ROOT file, plots LCIO vs GSF pT resolution histograms with Gaussian fits.

### `plot_pt_resolution.py`
Single-file resolution analysis. Reads the EDM4hep events tree, extracts truth/LCIO/GSF pT, produces:
- Two-panel canvas: LCIO resolution + GSF resolution (each with Gaussian fit)
- Overlay canvas: LCIO vs GSF overlaid
- Output: `{filename}_ptres.png` and `{filename}_ptres_overlay.png`

### `plot_fullgrid_resolution.py`
Comprehensive multi-file analysis:
- Scans all `gsf-e--*.root` files in `/cefs/higgs/zhangcg/cepc/20May2026/CEPCSW/`
- Parses filename to extract pT, theta, seed
- Groups by (pT, theta) key
- Produces:
  1. **Per-group resolution plots**: `ptres_pT{pT}_theta{theta}.png` — LCIO vs GSF histograms with Gaussian fits
  2. **Summary RMS vs pT**: `ptres_summary_rms.png` — RMS resolution vs pT, separate curves for θ=85° and θ=135°
  3. **Overlay by theta**: `ptres_overlay_theta{theta}.png` — 4-panel (pT=0.2, 0.5, 1.0, 2.0) overlay of LCIO vs GSF
  4. **Scatter plot**: `ptres_gsf_vs_lcio.png` — GSF pT vs LCIO pT, 2D histograms with diagonal line
  5. **Global stats**: mean pT, correlation coefficient, fractional difference

## Pre-generated Plots (in `scripts/`)
- `ptres_pT0.2_theta135.png` through `ptres_pT2.0_theta85.png` — per-group resolution
- `ptres_overlay_theta85.png`, `ptres_overlay_theta135.png` — 4-panel overlays
- `ptres_summary_rms.png` — RMS vs energy summary
- `ptres_gsf_vs_lcio.png` — GSF vs LCIO scatter

## Jupyter Notebook (in `Ploter/`)
`ploter.ipynb`:
- Uses `uproot` + `awkward` + `matplotlib` (pure Python, no ROOT required)
- Reads flat tuple from `gsf_flat-*.root` files
- Analyzes 2.0 GeV electrons at 85°
- Classifies tracks as GOOD (|pT resolution| < 10%) or BAD (>= 10%)
- Generates XY and RZ hit pattern plots for top 5 GOOD and top 5 BAD
- Hit color coding: blue = high EDep, red = low EDep
- Detector boundaries shown as dashed rings/lines
- Summary table with TPC r-z spread as quality metric
- Output: `hit_plots_2GeV_85deg/` directory with 20 PNG files

## Key Metrics Tracked
- Fractional pT resolution: `(pT_rec - pT_truth) / pT_truth * 100 [%]`
- Chi2/ndf for LCIO and GSF tracks
- Number of hits, number of components
- TPC r-z spread (from hit pattern analysis)

**Why:** Reference for running analysis, understanding what plots exist, and adding new analysis tools.
**How to apply:** Use these scripts as templates for new analysis. See [[gsf-data-flow]], [[gsf-known-issues]].