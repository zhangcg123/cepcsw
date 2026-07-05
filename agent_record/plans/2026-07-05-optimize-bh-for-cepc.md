---
name: 2026-07-05-optimize-bh-for-cepc
description: Plan to make Bethe-Heitler splitting physically useful for CEPC thin tracker material
metadata:
  type: plan
  status: proposed
---

# Optimize Bethe-Heitler Splitting for CEPC

## Current Diagnosis

The present GSF implementation does not improve electron tracking performance in the tested samples. The 5-event verbose dump and pT comparison show that GSF pT is almost identical to LCIO pT, with only small chi2 changes.

The dominant issue appears to be the Bethe-Heitler parameterization, not the LCIO seed covariance or q/p scan:

- `BHSplitThreshold = 0.01` triggers splitting after accumulated material reaches about 1% X0.
- The parent component has reasonable momentum, e.g. `pT ~= 1 GeV`.
- The ACTS/ATLAS low-x 6-component parameterization creates children with retained momentum fractions near zero, giving `pT ~= 0.01 GeV`.
- The next tracker hit rejects those components with large delta-chi2.
- The no-loss component survives, so the mixture collapses back to an LCIO-like trajectory.

Practical consequence: the GSF behaves close to a standard Kalman refit seeded from `CompleteTracks`, and does not carry useful bremsstrahlung hypotheses.

## Goal

Make the BH split physically sane in the CEPC tracker thin-material regime before further tuning seeds, q/p scans, or mixture reduction.

A correct first target is that, for `tX0 ~= 0.01`, a 1 GeV electron should produce components around the physical scale of about 1% energy loss, not 99% energy loss.

## Proposed Work Plan

### Step 1: Minimal BH Sanity Patch

Modify `Reconstruction/RecGsfTracking/src/BetheHeitlerSplitter.cpp` so the single-Gaussian thin-material approximation is used beyond the current `x < 0.002` boundary.

Test candidate upper limits:

- `x < 0.02`
- `x < 0.05`
- `x < 0.10`

This intentionally bypasses the broken low-x 6-component ACTS/ATLAS parameterization for CEPC.

Expected behavior after patch:

- At `tX0 = 0.01`, retained momentum mean should be about `0.99`.
- Split/refit should not create children at `pT ~= 0.01 GeV` from a `1 GeV` parent.
- Verbose dump should show physically plausible kappa changes.

### Step 2: Rebuild and Run Smoke Test

Use the existing fast workflow:

```bash
cd /aifs/user/data/zhangcg/gsfdev/CEPCSW
source setup.sh
./quick_build.sh
./run.sh Reconstruction/RecGsfTracking/options/run_gsf_test.py
```

Compare the verbose dump table for a few events:

- Truth vs LCIO vs GSF: `pT`, `eta`, `phi`, `d0`, `z0`, `p`, `chi2/ndf`
- GSF diagnostics: `splits`, `peak-comps`, `final-comps`, weights, material
- Post-split component kappa/pT and delta-chi2

Success criterion for this step: split children are no longer immediately unphysical and rejected solely because their momentum is near zero.

### Step 3: Quantitative pT Comparison

Run the existing plotting script:

```bash
python3 Reconstruction/RecGsfTracking/scripts/plot_pt_resolution.py gsf_test.root
```

For broader validation, use existing grid files or rerun selected electron points:

- `0.5 GeV`, `1.0 GeV`, `2.0 GeV`
- theta `85 deg`
- seeds `1,2`

Track metrics:

- LCIO vs GSF fractional pT resolution mean/RMS
- bad-track fraction, e.g. `abs((pT_rec - pT_truth)/pT_truth) > 10%`
- GSF/LCIO correlation
- chi2/ndf changes

### Step 4: If Single-Gaussian Helps, Improve the Model

If the single-Gaussian extension makes the behavior sane, then consider a more principled replacement:

1. Keep single-Gaussian approximation for CEPC thin-material splitting as a controlled baseline.
2. Build a CEPC-specific Gaussian mixture from Geant4 electron energy-loss samples.
3. Fit mixture parameters in the relevant `tX0` range with EM or another stable fitting method.
4. Replace or gate the ACTS/ATLAS low-x parameterization.

### Step 5: Only Then Revisit Other Tuning

After BH behavior is physically reasonable, revisit:

- `KappaSeedCov`
- analytical 3-hit prefit
- q/p refinement scan
- mixture reduction using weighted mean instead of keeping the higher-weight component
- multi-track support

These are secondary until the BH split creates useful hypotheses.

## Notes

This plan is consistent with `gsf-bethe-heitler-model.md`, `DEVELOPMENT.md`, and the latest verbose test output. The priority is to fix the energy-loss model first because current GSF performance is limited by the split model collapsing, not by the absence of more seed tuning.
