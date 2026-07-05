---
name: gsf-code-map
description: Complete file map of the GSF tracking code — every file, its purpose, and key contents
metadata:
  type: reference
---

# GSF Code Map

## Source Code (`Reconstruction/RecGsfTracking/src/`)

| File | Lines | Purpose |
|------|-------|---------|
| `GsfAlgorithm.h` | 97 | Algorithm class declaration, TrackSummary struct, configurable properties |
| `GsfAlgorithm.cpp` | 704 | Main algorithm: seed extraction, hit matching, analytical prefit, forward GSF filter, q/p refinement, smoothing, IP extrapolation, output |
| `BetheHeitlerSplitter.h` | 28 | BH splitter interface |
| `BetheHeitlerSplitter.cpp` | 151 | Embedded ACTS BH parameterization data (6×3×6 coefficients), split() implementation |
| `GsfComponent.h` | 28 | Component struct: weight, charge, TKalTrack pointer, clone/helix/cov methods |
| `GsfComponent.cpp` | 85 | Deep clone implementation (copies all sites + states), helixAtLastSite, covAtLastSite |
| `GsfMixture.h` | 19 | Namespace with normalizeWeights() and reduce() |
| `GsfMixture.cpp` | 101 | KL-divergence computation (symmetric, full 5×5 covariance), pairwise reduction loop |
| `GsfFlatTuple.h` | 100 | Flat tuple algorithm class declaration, all branch variables |
| `GsfFlatTuple.cpp` | 297 | Flat tuple: reads CompleteTracks, GSFTracks, MCParticle; writes TTree with 50+ branches |

## Build System
| File | Purpose |
|------|---------|
| `Reconstruction/RecGsfTracking/CMakeLists.txt` | Builds two Gaudi modules (RecGsfTracking, RecGsfFlatTuple) |
| `Reconstruction/CMakeLists.txt` | Line 10: `add_subdirectory(RecGsfTracking)` (was `# add_subdirectory(RecActsTracking)`) |

## Options Files (`Reconstruction/RecGsfTracking/options/`)
| File | Purpose |
|------|---------|
| `run_lcio_e.py` | Full reconstruction chain: sim → digitization → tracking → CompleteTracks |
| `run_gsf_e.py` | Main GSF refit config (1.0 GeV, 135°, seed 1) with latest settings |
| `run_gsf_e_0.2.py` | GSF refit for 0.2 GeV electrons at 135° |
| `run_gsf_e_1.0_85.py` | GSF refit for 1.0 GeV electrons at 85° |
| `run_gsf_e_2.0_135.py` | GSF refit for 2.0 GeV electrons at 135° |
| `run_gsf_e_2.0_135_lowthr.py` | Same but with BHSplitThreshold=1e-4 (lower threshold) |
| `run_gsf_test.py` | **测试专用**：EvtMax=5, VerboseDump=True, 无 flat tuple, OutputLevel=INFO |

## Analysis Scripts (`Reconstruction/RecGsfTracking/scripts/`)
| File | Purpose |
|------|---------|
| `compare_pt.py` | Simple LCIO vs GSF pT comparison from one file |
| `plot_pt_resolution.py` | Single-file resolution plot with overlay |
| `plot_fullgrid_resolution.py` | Multi-file grid scan: per-group plots, RMS summary, overlays, scatter |
| `ptres_*.png` (14 files) | Pre-generated resolution plots from past runs |

## Batch Production (`DumpGsfTrks/`)
| File | Purpose |
|------|---------|
| `sim.py.bk` | Geant4 simulation template (particle gun) |
| `trk.py.bk` | Digitization + tracking chain template |
| `gsf.py.bk` | GSF refit + flat tuple template |
| `runsim-*.py` (12 files) | Generated simulation configs |
| `runtrk-*.py` (12 files) | Generated tracking configs |
| `rungsf-*.py` (12 files) | Generated GSF configs |
| `rungsf----.py` | Stale/placeholder file |

## Analysis Notebook (`Ploter/`)
| File | Purpose |
|------|---------|
| `ploter.ipynb` | Jupyter notebook: hit pattern analysis, GOOD/BAD classification |
| `hit_plots_2GeV_85deg/` | 20 PNG files of hit patterns (10 GOOD, 10 BAD) |

## Batch Submission Scripts (project root)
| File | Purpose |
|------|---------|
| `dump_gsftrk.sh` (43 lines) | 3-stage pipeline runner: sim → trk → gsf |
| `subtrkjobs.sh` (19 lines) | HTCondor job submission via `hep_sub` |

## Other Files
| File | Purpose |
|------|---------|
| `.claude/settings.local.json` | Claude Code permissions (allow build.sh, ROOT import) |
| `MIGRATION.md` | Deleted in working tree (EDM4hep 0.x→1.0 migration, not GSF-related) |

**Why:** Quick reference to find any file in the GSF project.
**How to apply:** Use as a directory when you need to locate specific code. See [[gsf-project-overview]].