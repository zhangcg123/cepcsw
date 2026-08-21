# 1,000-event material/BH audit campaign steering

Date: 2026-08-21

The current from-scratch material/BH diagnostic requires the comprehensive
runtime audit for every GSF job. `DumpGsfTrks/gsf.py.bk` therefore sets
`MaterialBHAuditCSV` to an input-sample/method-specific filename:

```text
gsf_material_bh_audit_<rec-input-basename>_<output-method>.csv
```

The input basename contains the particle, nominal campaign label, and seed;
the output method distinguishes ordinary reverse and `reverse-truth-bh` runs.
Parallel jobs therefore do not overwrite their audit files. With the current
empty `tuplepath`, the CSV files are written in the project root beside the
paired stage outputs.

This is campaign steering only. The compiled `RecGsfTracking` default and the
active reverse-template default remain empty/off. The superseded forward-only
`MaterialTransitionCSV` property remains removed; it is not being restored.

`subtrkjobs.sh` uses seeds 1 through 10 with 100 events per job, giving exactly
1,000 events. It sets `TRUTH_BH_OVERRIDE=true` explicitly. The maintained
`gsf.py.bk` template keeps an explicit false off-side base value;
`dump_gsftrk.sh` replaces that assignment with true in each generated per-job
card. The flat tuple, EDM tuple, audit CSV, generated card, and batch logs carry
a `truth-bh` tag. The audit records the truth retained fraction actually
consumed by GSF, while the paired `gsf_material_steps-<sample>.root` remains the
authoritative step-level source for eBrem positions, processes, materials, and
volumes.
