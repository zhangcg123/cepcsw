---
name: gsf-project-overview
description: What the GSF tracking project is, its purpose, architecture, and current state
metadata:
  type: project
---

# GSF (Gaussian Sum Filter) Tracking for CEPCSW

## Purpose
The GSF is an electron-specific track refit algorithm for the CEPC detector. Unlike standard Kalman filters (single Gaussian noise model), the GSF models bremsstrahlung energy loss as a **mixture of Gaussian distributions** using the Bethe-Heitler parameterization. This is critical for CEPC because electrons lose significant energy via bremsstrahlung in the tracker material (VTX, SIT, TPC, SET).

## Architecture
- Built as a **Gaudi algorithm** (`RecGsfTracking`) in the CEPCSW framework
- Uses **DDKalTest/KalTest** for the Kalman filter engine (standalone, not ACTS)
- Reads **CompleteTracks** (LCIO-based standard tracks) as input seeds
- Writes **GSFTracks** (edm4hep TrackCollection) as output
- A separate post-processing algorithm **RecGsfFlatTuple** writes a flat ROOT TTree for offline analysis

## Key Components
1. `GsfAlgorithm` - Main algorithm: hit matching, forward filter with BH splitting, smoothing, IP extrapolation
2. `BetheHeitlerSplitter` - Embeds ACTS AtlasBetheHeitlerApprox<6,5> parameterization for bremsstrahlung energy loss
3. `GsfComponent` - One Gaussian component (weight + TKalTrack with full KF state)
4. `GsfMixture` - KL-divergence-based mixture reduction (merge similar components)
5. `GsfFlatTuple` - Flat ROOT TTree writer for offline analysis

## Current State (as of 2026-06-28)
- All code is **uncommitted** (new development, not in git history)
- Build system wired via `Reconstruction/CMakeLists.txt` (added `add_subdirectory(RecGsfTracking)`)
- Batch production running on CEFS at `/cefs/higgs/zhangcg/cepc/28Jun2026/CEPCSW/`
- Active tuning phase: pT resolution studies for electrons and muons at pT=0.5-2.0 GeV, theta=85°/135°
- Recent innovations: analytical 3-hit prefit for curvature seed, multi-pass q/p refinement

## Repository Location
`/aifs/user/data/zhangcg/gsfdev/CEPCSW/` on branch `master`

**Why:** Core context for any developer working on this project. Start here to understand what the code does.
**How to apply:** Read this first before touching any GSF code. See also [[gsf-algorithm-flow]], [[gsf-build-system]], [[gsf-bethe-heitler-model]].