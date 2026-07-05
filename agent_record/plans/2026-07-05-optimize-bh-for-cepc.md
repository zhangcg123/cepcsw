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

## Experiment Log

### 2026-07-05: Thin-Gaussian Sanity Patch

Branch: `gsf-bh-thin-gaussian-20260705`

Changes started:

- Use the single-Gaussian thin-material approximation for `x < 0.1` in `BetheHeitlerSplitter.cpp`.
- Update kappa for every returned split component, including `child[0]` which reuses the parent object.

Reason:

- The previous low-x 6-component parameterization produced near-zero retained momentum at `tX0 ~= 0.01`.
- The previous implementation also failed to apply the computed `newKappa` to `child[0]`, so a single-component thin-Gaussian branch would otherwise have no effect.

Validation to run:

```bash
source setup.sh
./quick_build.sh
./run.sh Reconstruction/RecGsfTracking/options/run_gsf_test.py
python3 Reconstruction/RecGsfTracking/scripts/plot_pt_resolution.py gsf_test.root
```


### 2026-07-05: Thin-Gaussian Smoke-Test Result

Validation completed:

```bash
./quick_build.sh
./run.sh Reconstruction/RecGsfTracking/options/run_gsf_test.py
python3 Reconstruction/RecGsfTracking/scripts/plot_pt_resolution.py gsf_test.root
```

Result summary:

- Build: success.
- Run: success, 5 events processed.
- BH scale: fixed. At `tX0 ~= 0.01`, split components now retain about 99% momentum instead of collapsing to about 1% momentum.
- Performance: not improved yet. Chi2 worsens for normal tracks because the model is now a single forced energy-loss branch, not a mixture of competing hypotheses.
- pT summary:
  - LCIO `(rec-gen)/gen`: mean `-3.3899%`, RMS `6.6364%`
  - GSF `(rec-gen)/gen`: mean `-3.3184%`, RMS `6.6739%`

Conclusion:

The first patch validates the core diagnosis: the previous BH scale was wrong for CEPC thin material. However, the one-component replacement is not sufficient as a tracking model. The next step should introduce a small CEPC-oriented mixture, for example a dominant no/small-loss component plus one or more moderate-loss tail components, rather than forcing every split to shift kappa by the mean loss.

### 2026-07-05: Two-Component CEPC Toy Mixture Started

Branch: `gsf-bh-cepc-two-component-20260705`

Change:

- Replace the forced one-component thin-Gaussian model with a two-component toy mixture for `x < 0.1`.
- Component 0: dominant no-loss branch, `mean = 1.0`.
- Component 1: moderate-loss tail branch, with weight/mean chosen so the mixture expectation remains approximately `exp(-x)`.

Reason:

The one-component model fixed the energy-loss scale but degraded chi2 because it forced every track to lose momentum. A useful GSF needs competing hypotheses; this toy mixture is the smallest test of that idea.


### 2026-07-05: Two-Component CEPC Toy Mixture Result

Validation completed:

```bash
./quick_build.sh
./run.sh Reconstruction/RecGsfTracking/options/run_gsf_test.py
python3 Reconstruction/RecGsfTracking/scripts/plot_pt_resolution.py gsf_test.root
```

Result summary:

- Build: success.
- Run: success, 5 events processed.
- BH hypotheses: physically plausible. At `tX0 ~= 0.01`, the model creates a no-loss branch around `pT ~= 1.0 GeV` and a moderate-loss branch around `pT ~= 0.9 GeV`, rather than pathological `pT ~= 0.01 GeV` branches.
- Chi2: returns to baseline-like behavior; the no-loss branch remains available and is usually selected.
- pT summary:
  - LCIO `(rec-gen)/gen`: mean `-3.3899%`, RMS `6.6364%`
  - GSF `(rec-gen)/gen`: mean `-3.4015%`, RMS `6.6325%`

Conclusion:

This is a better sanity baseline than the one-component model: it fixes the scale and avoids forcing every track to lose energy. It still does not improve tracking performance in the current small smoke test because the selected component remains no-loss for normal events. The next step should use truth energy-loss information or bad/high-bremsstrahlung events to tune the tail component(s).
