# Global BH Model From Sim: 2 GeV, theta=85 deg

This directory starts a simulation-derived global BH model study from true Geant4 material-step tracker eBrem in the electron 2 GeV, theta=85 deg sample.

Selection:

```text
track_id == 1
parent_id == 0
pdg == 11
process_subtype == 3  # eBrem
pre_volume contains VXD/ITK/TPC/OTK/SIT/SET
```

Variable:

```text
E_f/E_i = post_p / pre_p
```

Split at `E_f/E_i = 0.9`:

```text
total        3774
< 0.9        698  (0.184950)
> 0.9        3076  (0.815050)
>= 0.9       3076  (0.815050)
mean         0.927118
q10/q50/q90  0.754791 / 0.995559 / 0.999912
```

Outputs:

```text
analyze_tracker_ebrem_efei.C
tracker_ebrem_efei_fraction_summary.txt
tracker_ebrem_efei_values.csv
tracker_ebrem_Ef_over_Ei_fraction_split.root
plots/tracker_ebrem_Ef_over_Ei_fraction_split.png
plots/tracker_ebrem_Ef_over_Ei_fraction_split.pdf
```

![tracker eBrem retained fraction split](plots/tracker_ebrem_Ef_over_Ei_fraction_split.png)

## Smooth Fit And Gaussian Mimic

The normalized tracker eBrem histogram was fit with a smooth 3-component beta-mixture function on `0 < E_f/E_i < 1`. A 5-component truncated-Gaussian weighted sum, normalized on `[0,1]`, was then fit to mimic that beta-mixture curve. The overlay plot shows the histogram, beta-mixture function, truncated-Gaussian components, and total truncated-Gaussian weighted sum on one canvas.

```text
tracker_ebrem_efei_fit_summary.txt
tracker_ebrem_efei_beta_fit_components.csv
tracker_ebrem_efei_gaussian_components.csv
plots/tracker_ebrem_Ef_over_Ei_beta_fit_gaussian_mimic.png
plots/tracker_ebrem_Ef_over_Ei_beta_fit_gaussian_mimic.pdf
```

![tracker eBrem beta fit and Gaussian mimic](plots/tracker_ebrem_Ef_over_Ei_beta_fit_gaussian_mimic.png)

## Code Integration

The simulation-derived global model is encoded in:

```text
Reconstruction/RecGsfTracking/src/BetheHeitlerSplitter.cpp
```

Use it from `RecGsfTracking` options with:

```python
gsf.BHModel = "GlobalSim2GeV85"
```

The default remains:

```python
gsf.BHModel = "Current"
```

`GlobalSim2GeV85` deliberately ignores `tX0`, but keeps the existing `split(parent, tX0, bz)` interface unchanged.
