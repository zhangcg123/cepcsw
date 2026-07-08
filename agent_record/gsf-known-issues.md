---
name: gsf-known-issues
description: Known problems, limitations, and areas needing improvement in the GSF code
metadata:
  type: project
---

# GSF Known Issues & Limitations

## Current Limitations

### 1. Single Track Per Event
The code assumes exactly one track per event (uses `trk[0]`, `mcp[0]`). Multi-track events are not handled. This is fine for the current single-particle-gun studies but would need generalization for physics events.

### 2. Curvature Seed Sensitivity
- The analytical 3-hit prefit was added specifically because the LCIO curvature seed is often wrong after bremsstrahlung
- The multi-pass q/p refinement was added because even the 3-hit prefit sometimes fails
- Events with severe early bremsstrahlung (VTX layers) remain the hardest cases

### 3. BAD Track Rate (~8.5%)
- For 2.0 GeV electrons at 85°, ~8.5% of tracks have >10% fractional pT resolution
- Some BAD tracks have very few hits (e.g., 6 instead of 233) — likely a tracking failure upstream
- Some BAD tracks have extreme resolution errors (-93% to +143%)

### 4. Mixture Reduction Quality
- KL-divergence pair selection still uses full 5x5 covariance at the last active site.
- As of 2026-07-08, reduction does moment matching for the selected pair and overwrites the representative component's last-site mean/covariance.
- This is better than the old higher-weight-only merge, but it is still an approximation because the earlier KalTest track history is not a true per-site Gaussian mixture history.
- The O(N²) pairwise search is fine for N<=12 but does not scale to very large mixtures.

### 5. Material IP Extrapolation
- `MaterialIPExtrapolation` is disabled by default (false)
- The geometric extrapolation (`MoveTo`) doesn't account for material between the innermost hit and the IP
- Enabling material extrapolation requires the cradle Transport which is more expensive

### 6. No ACTS Integration
- The GSF uses DDKalTest/KalTest (standalone KF), not ACTS
- The ACTS `RecActsTracking` was commented out in CMakeLists.txt and replaced by GSF
- The BH parameterization data was copied from ACTS to avoid Eigen/Boost dependency

### 7. Hit Matching Fallback
- The cellID-based lookup is primary (O(1)), but the radius-based fallback has a ±25mm tolerance
- This could match hits to wrong layers if the detector geometry has closely-spaced layers

### 8. Muon Studies
- Muon samples are being produced but the BH splitting is disabled for muons (ElectronHypothesis=false)
- The muon samples serve as a control: GSF should match LCIO for non-radiating particles

## Areas for Future Development
1. **Multi-track support** for physics events
2. **Better initial curvature seed** — the 3-hit prefit and q/p refinement are workarounds for a fundamental problem
3. **Component merging** — could use a weighted mean of states instead of keeping the higher-weight component
4. **Outlier rejection** — handle the ~8.5% BAD tracks more robustly
5. **Physics validation** — compare with full Geant4 bremsstrahlung, not just the BH parameterization
6. **Performance** — the multi-pass q/p refinement runs up to 7 full forward passes per track

**Why:** Track what needs to be fixed or improved as development continues.
**How to apply:** Prioritize work based on this list. See [[gsf-algorithm-flow]], [[gsf-analysis-tools]].