---
name: 2026-07-05-measure-cepc-electron-energy-loss
description: Plan to extract real CEPC electron energy-loss distributions from simulation and use them to tune the GSF Bethe-Heitler model
metadata:
  type: plan
  status: proposed
---

# Measure CEPC Electron Energy Loss for GSF BH Model

## Motivation

The current BH development showed two important facts:

1. The copied ACTS/ATLAS low-x parameterization is unusable as currently applied in CEPC: at `tX0 ~= 0.01`, it creates near-zero retained momentum components (`pT ~= 0.01 GeV` from a `1 GeV` parent).
2. A hand-made two-component CEPC toy model is physically safer, but it still does not improve pT in the smoke test because the no-loss branch usually wins.

Therefore the next step should be data-driven: measure the actual electron energy-loss distribution in CEPC simulation and fit a CEPC-specific mixture.

## Target Quantity

For each material crossing or detector region, measure:

```text
z = p_after / p_before
loss = 1 - z
tX0 = material thickness / radiation length
```

Useful metadata:

```text
particle: e-
initial p or pT
theta
layer / detector region: VTX, ITK, TPC, OTK, etc.
radius and z position
accumulated tX0
random seed / event id
```

The key question:

```text
For CEPC tracker material, what is the real distribution of z at tX0 ~= 0.001, 0.005, 0.01, 0.02?
```

## Work Plan

### Step 1: Check Existing ROOT Content

Inspect existing files:

```text
sim-e--*.root
trk-e--*.root
gsf-e--*.root
gsf_flat-e--*.root
```

Look for whether any of these already store enough truth information:

- MCParticle only: likely gives generator/final truth but not layer-by-layer loss.
- SimTrackerHits: may contain positions and deposited energy, but may not contain pre/post step momentum.
- TrackerHit associations: useful for geometry, probably not enough for true momentum loss.

Deliverable: short note in `DEVELOPMENT.md` saying whether existing files are sufficient.

### Step 2: If Needed, Add a Geant4 Truth Recorder

If existing files do not contain pre/post momentum per material crossing, add a lightweight simulation-side recorder.

Candidate outputs per step or per selected boundary:

```text
event_id
track_id
pdg
pre_px, pre_py, pre_pz
post_px, post_py, post_pz
pre_p, post_p
z = post_p / pre_p
edep
step_length
material_name
volume_name
x, y, z, r
```

Prefer recording only electron primary-track steps in tracker volumes to keep files small.

### Step 3: Build Energy-Loss Summary Script

Create an analysis script to read the truth recorder output and produce:

- histograms of `z = p_after/p_before`
- histograms of `loss = 1-z`
- cumulative loss vs radius/layer
- loss distributions binned by accumulated `tX0`
- tail probabilities: `P(z < 0.99)`, `P(z < 0.95)`, `P(z < 0.90)`, `P(z < 0.80)`

### Step 4: Fit CEPC-Specific Mixtures

For each relevant `tX0` bin, fit a small mixture model.

Start simple:

```text
2 components: no/small-loss core + moderate-loss tail
3 components: no/small-loss core + moderate-loss tail + hard-brem tail
```

Candidate fit variable:

```text
z = p_after / p_before
```

or transformed variables:

```text
u = -log(z)
logit(z)
```

The fitted mixture must satisfy sanity checks:

- weights sum to 1
- `0 < mean <= 1`
- positive variance
- smooth behavior as `tX0 -> 0`
- expected energy loss roughly scales like `tX0` for thin material
- no dominant near-zero momentum component at `tX0 ~= 0.01`

### Step 5: Implement in `BetheHeitlerSplitter`

Add a CEPC-specific model option after fitting.

Possible future structure:

```text
ACTSAtlasBH      old copied model, kept only for comparison
ThinGaussian     diagnostic baseline
CEPCToy2         current two-component toy model
CEPCFitted       fitted model from Geant4 truth
```

### Step 6: Validate with Tracking

Run the standard validation sequence:

```bash
source setup.sh
./quick_build.sh
./run.sh Reconstruction/RecGsfTracking/options/run_gsf_test.py
python3 Reconstruction/RecGsfTracking/scripts/plot_pt_resolution.py gsf_test.root
```

Then run broader samples:

```text
e- pT = 0.5, 1.0, 2.0 GeV
theta = 85 deg and later 135 deg
seeds = 1,2 or more
```

Metrics:

- LCIO vs GSF pT mean/RMS
- bad-track fraction: `abs((pT_rec - pT_truth)/pT_truth) > 10%`
- GSF chi2/ndf versus LCIO
- component survival after splits
- whether non-no-loss branches win in true hard-bremsstrahlung events

## Success Criteria

A CEPC BH model is useful only if:

1. It matches Geant4 truth energy-loss distributions in the relevant `tX0` range.
2. It avoids unphysical near-zero momentum components for thin material.
3. It does not degrade normal tracks.
4. It improves or recovers electron tracks with real hard bremsstrahlung.
5. It does not degrade muon control samples when electron splitting is disabled.

## Current Recommendation

Do not keep tuning the hand-made toy mixture blindly. First determine whether existing simulation files contain enough truth information. If not, add a dedicated Geant4 step/momentum recorder for primary electrons in tracker material.
