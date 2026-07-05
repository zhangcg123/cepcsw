# GSF Tracking RAG Index

- [GSF Project Overview](gsf-project-overview.md) — What the GSF tracking project is, its purpose, architecture, and current state
- [GSF Algorithm Flow](gsf-algorithm-flow.md) — Detailed step-by-step flow of the GsfAlgorithm, configurable properties
- [Bethe-Heitler Model](gsf-bethe-heitler-model.md) — The bremsstrahlung parameterization: regimes, data format, split operation
- [GSF Mixture Reduction](gsf-mixture-reduction.md) — KL-divergence-based Gaussian mixture pruning
- [GSF Build System](gsf-build-system.md) — CMake configuration, dependencies, how to build
- [GSF Data Flow](gsf-data-flow.md) — The 3-stage pipeline, input/output collections, file naming conventions
- [GSF Batch Production](gsf-batch-production.md) — How batch jobs are submitted and run on CEFS/HTC
- [GSF Analysis Tools](gsf-analysis-tools.md) — Analysis scripts, plotting tools, pre-generated plots
- [GSF Known Issues](gsf-known-issues.md) — Known problems, limitations, areas needing improvement
- [GSF Code Map](gsf-code-map.md) — Complete file map of every file and its purpose
- [Development Log](DEVELOPMENT.md) — 总纲：当前状态、目标、已完成、下一步、待解决问题
- [Dev Log Principle](dev-log-principle.md) — 开发日志记录原则：累积追加，不覆盖删除

## Plans
- [2026-07-05 Rollback Prefit & QP Refinement](plans/2026-07-05-rollback-prefit-qpref.md) — 回滚 3-hit 预拟合和多轮 q/p 扫描
- [2026-07-05 Optimize BH for CEPC](plans/2026-07-05-optimize-bh-for-cepc.md) — Plan to bypass/fix the broken low-x Bethe-Heitler parameterization for CEPC thin tracker material
