# GSF Tracking RAG Index

## Read First: Current Stage

- [Current Stage and TODOs](agent_record/current-stage-and-todos.md) — Current validated truth source, present conclusion, and prioritized next steps
- [2026-07-05 G4 Step Electron/Muon Comparison](agent_record/2026-07-05-g4-step-electron-muon-comparison.md) — 200 e- vs 200 mu- true G4 pre/post-step comparison confirming stronger electron loss tail
- [2026-07-05 G4 Material Step Recorder](agent_record/2026-07-05-g4-material-step-recorder.md) — Real Geant4 pre/post-step momentum recorder for CEPC BH fitting
- [2026-07-05 Measure CEPC Electron Energy Loss](agent_record/plans/2026-07-05-measure-cepc-electron-energy-loss.md) — Active plan for extracting Geant4 truth distributions and fitting a CEPC-specific BH mixture
- [Development Log](DEVELOPMENT.md) — Chronological development log; top section summarizes current status, older sections are historical

## Resolved Handoffs

- [2026-07-07 Electron GSF Refit Crash](agent_record/2026-07-07-temp-gsf-electron-refit-crash.md) — Resolved segmentation-fault repair record; provenance only, not the active next task


## Current Study Directories

- [BH Model Comparison Studies](BHModelComparisonStudies/README.md) — Direct comparison area for the current `BetheHeitlerSplitter.cpp` BH model versus true G4 material-step `z = post_p/pre_p`. Use this before GSF validation, because current GSF output is known not to work properly. First prepared point: `current_bh_vs_g4step_2p0_theta85/`, with model-only weighted-Gaussian BH curves already generated. Current finding: the current BH model is not validated; the `tX0=0.1` transition jumps from mean `z~0.951` at `tX0=0.05` to `z~0.144` at `tX0=0.10`, so next work should tune/smooth it against G4 tracker-volume primary eBrem truth. New study: `globalBHmodelfromSim@2GeV85Degree/` records the 2 GeV, theta=85 deg tracker eBrem `E_f/E_i` histogram split (`<0.9 = 698/3774`, `>0.9 = 3076/3774`) and a bounded fit workflow using a 3-beta-mixture smooth function plus a truncated-Gaussian mixture mimic normalized on `[0,1]`.
- [G4 Material Step Studies](G4MaterialStepComparison/studies/README.md) — Organized true Geant4 `g4step_tuple` material-step studies by point. Current important subdirectories include `e1p0_theta85/` and `e2p0_theta85/`; these hold ROOT macros, summaries, and plots for primary eBrem, tracker process losses, and electron/muon controls.
- [Tracking Performance Studies](TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/README.md) — LCIO `CompleteTracks` resolution study for 2 GeV, theta=85 deg. Its meaningful no/light/hard eBrem split uses **tracker-volume primary eBrem** (`pre_volume` containing VXD/ITK/TPC/OTK/SIT/SET); all-material eBrem is recorded but does not separate this point. This is a separate top-level study tree from the G4 material plots; read it before drawing tracking-performance conclusions or extending LCIO-vs-GSF comparisons. Regenerated with seeds 1..15: no-tracker-eBrem electron pT core is close to muon (`median 0.0542%`, q16/q84 `-0.0884/0.1737%` vs muon `median 0.0668%`, q16/q84 `-0.0569/0.1826%`), but electron no-tracker-eBrem tails remain larger (`|pT residual|>10%`: `13/1373` vs muon `2/3500`).

## Current Cross-Checks

- [2026-07-05 SimTrackerHit Momentum Sufficiency](agent_record/2026-07-05-simtrackerhit-momentum-sufficiency.md) — Why SimHit momentum is useful for qualitative checks but insufficient for final BH fitting
- [2026-07-06 SimHit Energy Loss First Analysis](agent_record/2026-07-06-simhit-energy-loss-first-analysis.md) — Large-statistic SimHit e-/mu- comparison; superseded as final truth source by G4 pre/post-step recorder
- [SimHit ROOT File Grouping](SimHitEnergyLoss/root_files/README.md) — Location of grouped SimHit-level electron and muon ROOT files for future cross-checks
- [G4 Material Step Comparison Files](G4MaterialStepComparison/README.md) — Reproducibility notes for the 200 e-/mu- true G4-step comparison

## Core GSF References

- [GSF Project Overview](agent_record/gsf-project-overview.md) — Project purpose, architecture, and state
- [GSF Algorithm Flow](agent_record/gsf-algorithm-flow.md) — Step-by-step flow of `GsfAlgorithm`
- [Bethe-Heitler Model](agent_record/gsf-bethe-heitler-model.md) — Current BH parameterization and split operation
- [GSF Mixture Reduction](agent_record/gsf-mixture-reduction.md) — KL-divergence-based Gaussian mixture pruning
- [GSF Data Flow](agent_record/gsf-data-flow.md) — Input/output collections and file naming
- [GSF Build System](agent_record/gsf-build-system.md) — Build dependencies and commands
- [GSF Analysis Tools](agent_record/gsf-analysis-tools.md) — Existing analysis scripts and plotting tools
- [GSF Known Issues](agent_record/gsf-known-issues.md) — Known limitations and risks
- [GSF Code Map](agent_record/gsf-code-map.md) — File map

## Historical Plans And Provenance

- [2026-07-05 Optimize BH for CEPC](agent_record/plans/2026-07-05-optimize-bh-for-cepc.md) — Historical low-material BH mitigation plan before true G4-step truth existed
- [2026-07-05 Rollback Prefit & QP Refinement](agent_record/plans/2026-07-05-rollback-prefit-qpref.md) — Historical rollback record for 3-hit prefit and q/p scan work
- [GSF Batch Production](agent_record/gsf-batch-production.md) — Batch production notes
- [AI Analyst Identity](agent_record/ai-analyst-identity.md) — Provenance for earlier DeepSeek SimHit analysis
- [Dev Log Principle](agent_record/dev-log-principle.md) — Logging convention


## 2026-07-08 GlobalSim2GeV85 GSF status

- Encoded `BetheHeitlerSplitter::Model::{Current, GlobalSim2GeV85}` and `RecGsfTracking.BHModel` property. Default is `Current`; set `gsf.BHModel = "GlobalSim2GeV85"` to use the simulation-derived global model.
- `GlobalSim2GeV85` intentionally ignores `tX0` while keeping the `split(parent, tX0, bz)` interface.
- Added `VerboseSplitDump` to suppress split-by-split logs while keeping `VerboseDump` final fit tables, and `SelectedEventIndices` to run only requested events.
- Build passed with `source setup.sh && ./quick_build.sh`.
- Light tracker eBrem GSF test completed seeds 1-5 only: `235/235` selected events parsed into `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/gsf_light_global_bh_fit_parameters_completed_seed1_5.csv`. Result is not yet good: `40/235` have `gsf_p > 10 GeV`, `186/235` have zero GSF chi2.
- Full run recipe: `agent_record/2026-07-08-global-bh-gsf-run.md`.
