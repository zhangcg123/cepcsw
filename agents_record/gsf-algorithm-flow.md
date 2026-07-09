---
name: gsf-algorithm-flow
description: Detailed step-by-step flow of the GsfAlgorithm, including all configurable properties
metadata:
  type: reference
---

# GsfAlgorithm Execution Flow

## Configurable Properties
| Property | Default | Description |
|----------|---------|-------------|
| `MSOn` | true | Multiple scattering in Kalman filter |
| `ElossOn` | false | Energy loss in cradle (separate from BH) |
| `MaxComponents` | 12 | Max Gaussian mixture components |
| `BHSplitThreshold` | 1e-4 | Accumulated t/X0 before triggering BH split |
| `ElectronHypothesis` | true | Enable BH splitting (false for muons) |
| `MaterialIPExtrapolation` | false | Use cradle Transport to IP (vs geometric helix) |
| `VerboseDump` | true | Per-track diagnostic printout |
| `KappaSeedCov` | 1e-7 | Curvature seed covariance (1e-7=tight, 1e-4=loose) |
| `KappaSeedCov` | 1e-4 | Curvature seed covariance (was 1e-7 — 1000x looser) |
| `AnalyticalPrefit` | true | 3-hit helix prefit using outer-half hits |
| `QPRefinement` | true | Multi-pass q/p scan for bad-chi2 events |
| `QPRefinementChi2Thresh` | 3.0 | Trigger threshold (kappa mismatch OR chi2/ndf) |
| `QPRefinementSteps` | 3 | +/- 1,2,3 steps (i.e., 6 extra passes) |
| `QPRefinementStepSize` | 0.1 | Step size as fraction of kappa (10%) |

## Step-by-Step Flow (execute() method)

### Step 1: Seed Extraction
- Extract LCIO seed parameters (omega, d0, z0, phi, tanLambda) from CompleteTracks
- Find AtFirstHit or AtIP track state
- Return `LcioSeed` struct

### Step 2: Hit Matching
- Iterate over tracker hits associated with the LCIO track
- Match each hit to a DDVMeasLayer using:
  - **Primary**: cellID-based lookup via `std::multimap<int, DDVMeasLayer*>` (O(1) via `IsOnSurface`)
  - **Fallback**: radius-based scan over all cradle layers (within 25mm tolerance)
- Convert LCIO hits to DDKalTest hits via `layer->ConvertLCIOTrkHit()`
- Sort hits by radius (inner to outer)

### Step 3: Curvature Seed Computation
- Convert LCIO omega → kappa via `kappa = omega / (Bz * 2.99792458e-4)`
- If `AnalyticalPrefit` enabled and >= 3 hits:
  - Take 3 hits from the **outer half** of the tracker (indices: n/2, n/2 + (n-n/2)/2, n-1)
  - Compute 3-point helix via `HelixTrack(v1, v2, v3, bz, forwards)`
  - This gives post-bremsstrahlung curvature (avoids pre-brem bias from VTX/SIT)
  - Keep charge sign from LCIO seed

### Step 4: Forward GSF Filter
- Create initial site at innermost hit with the seed helix
- For each subsequent hit (inner → outer):
  - For each component, clone the hit, create a TKalTrackSite, call `AddAndFilter()`
  - If filter succeeds: update component weight via `exp(-0.5 * min(dchi2, 100))`
  - If filter fails: penalize weight by 1e-6, keep if weight > 1e-30
  - Accumulate t/X0 from layer material (inner + outer thickness)
  - If accumulated t/X0 > `BHSplitThreshold` and `ElectronHypothesis` and not at max components:
    - Call `BetheHeitlerSplitter::split()` on each component
    - Reset accumulated t/X0 counter
  - Normalize weights
  - If component count > `MaxComponents`: call `GsfMixture::reduce()` (KL-divergence merging)

### Step 4b: Multi-pass Q/P Refinement
- Triggered when: analytical kappa differs from LCIO kappa by > `QPRefinementChi2Thresh` fraction, OR best component chi2/ndf > threshold
- Scan `kappaSeed * (1 + s * 0.1)` for s = -3..-1, 1..3 (6 extra passes)
- For each scan point: run full forward filter, SmoothAll, pick best by chi2
- Keep the best result

### Step 5: Smoothing and IP Extrapolation
- `SmoothAll()` on all components (backward pass)
- Pick best component by highest weight
- Extrapolate to IP:
  - `MaterialIPExtrapolation=false` (default): geometric helix `MoveTo(IP)` with Jacobian covariance propagation
  - `MaterialIPExtrapolation=true`: cradle `Transport()` with MS/Eloss correction

### Step 6: Output
- Write edm4hep Track with AtIP track state (5-parameter helix + 15-element covariance)
- Fill `TrackSummary` struct with per-track diagnostics
- If `VerboseDump`: pretty-print parameter comparison table and GSF diagnostics

**Why:** You need to understand the algorithm flow to modify it, debug issues, or add new features.
**How to apply:** Reference this when modifying `GsfAlgorithm.cpp`. See [[gsf-project-overview]], [[gsf-bethe-heitler-model]], [[gsf-mixture-reduction]].