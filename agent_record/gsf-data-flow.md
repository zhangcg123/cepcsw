---
name: gsf-data-flow
description: The data pipeline — input/output collections, event data model, file naming conventions
metadata:
  type: reference
---

# GSF Data Flow

## Pipeline (3 stages)

### Stage 1: Simulation
- Script: `sim.py.bk` (template) → `runsim-{particle}--{pT}-{theta}-{seed}.py`
- Produces: `sim-{particle}--{pT}-{theta}-{seed}.root`
- Contains: MCParticle, VXDCollection, ITKBarrelCollection, TPCCollection, OTKBarrelCollection, Muon collections
- Particle gun: single particle, configurable type/momentum/theta/seed

### Stage 2: Digitization + Tracking
- Script: `trk.py.bk` (template) → `runtrk-{particle}--{pT}-{theta}-{seed}.py`
- Reads: `sim-{particle}--{pT}-{theta}-{seed}.root`
- Produces: `trk-{particle}--{pT}-{theta}-{seed}.root`
- Pipeline: Digitization (VXD, ITK, OTK, TPC, Muon) → SiliconTracking → Clupatra (TPC) → FullLDCTracking → CompleteTracks
- Contains: all digitized hits + CompleteTracks (LCIO-based standard tracks)

### Stage 3: GSF Refit
- Script: `gsf.py.bk` (template) → `rungsf-{particle}--{pT}-{theta}-{seed}.py`
- Reads: `trk-{particle}--{pT}-{theta}-{seed}.root`
- Produces: `gsf-{particle}--{pT}-{theta}-{seed}.root` (EDM4hep) + `gsf_flat-{particle}--{pT}-{theta}-{seed}.root` (flat TTree)

## Event Data Model (Stage 3)

### Input Collections Read by RecGsfTracking
- `CompleteTracks` (edm4hep::TrackCollection) — LCIO standard tracks (seed + hit association)
- `MCParticle` (edm4hep::MCParticleCollection) — MC truth (first particle = primary)

### Output Collections Written by RecGsfTracking
- `GSFTracks` (edm4hep::TrackCollection) — GSF-refit tracks
  - Each track has: type=2, chi2, ndf, AtIP track state, associated tracker hits

### Output Collections Read by RecGsfFlatTuple
- `CompleteTracks`, `GSFTracks`, `MCParticle`
- Plus configurable hit collection names (for all-hits dump)

### Output File by RecGsfFlatTuple
- Flat ROOT TTree (`gsf_tuple`) with branches:
  - MC truth: pdg, px, py, pz, pT, p, eta, theta, phi, vx, vy, vz
  - LCIO: pT, p, eta, theta, phi, d0, z0, omega, tanl, chi2, ndf, nhits, type
  - GSF: same parameters
  - Resolution: res_pT_gsf, res_pT_lcio (fractional)
  - Per-hit data for LCIO track, GSF track, and all original hit collections

## Coordinate Conventions
- Magnetic field: Bz = 3.0 T (configurable)
- alpha = Bz * 2.99792458e-4 [GeV/(mm·T)]
- pT = |alpha / omega|
- Track state: (drho, phi0, kappa, dz, tanLambda) in KalTest internal convention
- LCIO/EDM4hep: (D0, phi, omega, Z0, tanLambda) with D0 = -drho, phi = phi0 + π/2

## File Naming Convention
`{stage}-{particle}--{pT}-{theta}-{seed}.root`
- stage: sim, trk, gsf, gsf_flat
- particle: e- (electron), mu- (muon)
- pT: 0.5, 1.0, 2.0 (transverse momentum in GeV)
- theta: 85, 135 (polar angle in degrees)
- seed: 1, 2 (random seed for simulation)

## Local Input Files (trk)
项目根目录下可直接用于测试的 trk 文件（仅 θ=85°）：
- `trk-e--0.5-85-{1,2}.root` — 0.5 GeV 电子
- `trk-e--1.0-85-{1,2}.root` — 1.0 GeV 电子
- `trk-e--2.0-85-{1,2}.root` — 2.0 GeV 电子
- `trk-mu--0.5-85-{1,2}.root` — 0.5 GeV μ子
- `trk-mu--1.0-85-{1,2}.root` — 1.0 GeV μ子
- `trk-mu--2.0-85-{1,2}.root` — 2.0 GeV μ子

**Why:** You need to know the data flow to run the pipeline, debug output issues, or understand what's in the files.
**How to apply:** Reference when running batch jobs, reading output files, or modifying the pipeline. See [[gsf-project-overview]], [[gsf-batch-production]].