# Current BH Model vs G4 Material Steps: 2 GeV, theta=85 deg

This study compares the actual current BetheHeitlerSplitter.cpp model to true Geant4 material-step truth for the 2 GeV, theta=85 deg electron sample. It does not use GSF tracking output.

## Motivation

The current GSF tracking result is known not to work properly, so checking GSF-vs-LCIO performance is not useful yet. The correct next task is to study the BH input model itself.

The tracking-performance study already shows that the LCIO pT core for no_tracker_ebrem electrons is close to muon LCIO, while pT tails are driven by tracker-volume eBrem. Therefore the BH model should be compared directly to primary electron eBrem in tracker volumes.

## Model Under Test

Source file: Reconstruction/RecGsfTracking/src/BetheHeitlerSplitter.cpp

Important: the .cpp currently differs from the older header/RAG description. For 0.0001 <= tX0 < 0.1, the current implementation uses a CEPC thin-material two-component toy mixture:

- expectedMean = exp(-tX0)
- tailWeight = clamp(10 * tX0, 0.02, 0.20)
- tailMean = clamp((expectedMean - (1 - tailWeight)) / tailWeight, 0.50, 0.999)
- components: (1 - tailWeight, mean=1.0), (tailWeight, mean=tailMean)

The old ACTS low-x six-component branch is still present in the file, but is not reached because kThinGaussianUpperX0 = 0.1.

## Truth Selection

The G4 material-step truth selection is step-wise inside g4step_tuple:

- track_id == 1
- parent_id == 0
- pdg == 11
- process_subtype == 3
- pre_volume contains one of VXD/ITK/TPC/OTK/SIT/SET
- z = post_p / pre_p
- x = step_tX0

This intentionally excludes all-material eBrem and focuses on tracker-volume primary eBrem, the category that drives pT tails in the tracking-performance study.

## Inputs

Expected ROOT tuples in the CEPCSW top directory: gsf_material_steps-e--2.0-85-{1..15}.root

## Reproduction

Run from the CEPCSW top directory:

BHModelComparisonStudies/current_bh_vs_g4step_2p0_theta85/scripts/compare_current_bh_to_g4step.py --first-seed 1 --last-seed 15

Expected outputs:

- truth_step_summary.csv
- bh_model_component_summary.csv
- bh_vs_g4step_bin_summary.csv
- summary.txt
- plots/g4_truth_vs_current_bh_z.png/pdf
- plots/g4_truth_vs_current_bh_z_vs_step_tX0.png/pdf

## Current Status

The study directory and comparison script are prepared. The next action is to run the script, inspect the truth-vs-model z distributions, and decide whether the current two-component thin-material model is acceptable or needs replacement by a CEPC-specific BH mixture/table.

## Current BH Model Visualization

The first diagnostic is model-only, before any G4 truth overlay:

```bash
BHModelComparisonStudies/current_bh_vs_g4step_2p0_theta85/scripts/plot_current_bh_model.py
```

Generated files:

```text
current_bh_components.csv
current_bh_summary.csv
current_bh_model_summary.txt
plots/current_bh_weighted_gaussian_curves.png
plots/current_bh_component_moments_vs_tX0.png
```

How to read the plotted curves: each component is a Gaussian in retained momentum fraction `z`; the displayed model curve is the weighted sum `sum_i w_i * N(z; mean_i, sigma_i)`. The no-loss component has zero variance in the code, so the plotting script gives it a narrow display width only to make the delta-like spike visible.

Current-code behavior to remember:

- `tX0 < 1e-4`: one no-loss component at `z = 1`.
- `1e-4 <= tX0 < 0.1`: two-component CEPC thin-material toy model.
- `tX0 >= 0.1`: six-component high-x ACTS table, capped at `tX0 = 0.2`.

The model-only scan shows a sharp behavior change at `tX0 = 0.1`: the weighted mean is near `z = 0.970` at `tX0 = 0.03`, but about `z = 0.144` at `tX0 = 0.1`. The next truth comparison should check whether real tracker-volume primary eBrem `step_tX0` values ever approach that boundary, and whether the two-component thin branch reproduces the observed G4 `z` tail.

## Current Finding: Tune The Current BH Model Next

The current BH model should not be treated as validated. The thin-material two-component branch is a pragmatic patch, not a confirmed CEPC Bethe-Heitler model.

The main red flag from the model-only curves is the hard transition at `tX0 = 0.1`:

```text
tX0 = 0.05: weighted mean z about 0.951
tX0 = 0.10: weighted mean z about 0.144
```

A physical retained-momentum distribution should evolve smoothly with material thickness. This jump comes from switching from the two-component CEPC thin branch to the six-component high-x ACTS table. Unless G4 truth shows such a sharp behavior, this transition is not credible.

Next time, focus on fine tuning the current BH model:

1. Compare the current model to G4 tracker-volume primary eBrem truth using `z = post_p / pre_p` versus `step_tX0`.
2. Check whether real tracker eBrem `step_tX0` values approach the `0.1` boundary.
3. If the boundary is relevant, replace or smooth the transition.
4. Tune the low/thin-material branch shape against G4 truth, not only its mean `E[z] = exp(-tX0)`.
5. Continue to avoid GSF validation until the BH input model is better understood.

