# GSF Tracking RAG Index

## Read First: Current Stage

- [Current Stage and TODOs](agent_record/current-stage-and-todos.md) — Current validated truth source, present conclusion, and prioritized next steps
- [2026-07-05 G4 Step Electron/Muon Comparison](agent_record/2026-07-05-g4-step-electron-muon-comparison.md) — 200 e- vs 200 mu- true G4 pre/post-step comparison confirming stronger electron loss tail
- [2026-07-05 G4 Material Step Recorder](agent_record/2026-07-05-g4-material-step-recorder.md) — Real Geant4 pre/post-step momentum recorder for CEPC BH fitting
- [2026-07-05 Measure CEPC Electron Energy Loss](agent_record/plans/2026-07-05-measure-cepc-electron-energy-loss.md) — Active plan for extracting Geant4 truth distributions and fitting a CEPC-specific BH mixture
- [Development Log](DEVELOPMENT.md) — Chronological development log; top section summarizes current status, older sections are historical

## Resolved Handoffs

- [2026-07-07 Electron GSF Refit Crash](agent_record/2026-07-07-temp-gsf-electron-refit-crash.md) — Resolved segmentation-fault repair record; provenance only, not the active next task

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
