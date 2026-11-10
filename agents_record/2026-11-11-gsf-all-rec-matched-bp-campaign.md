# Full current-REC GSF campaign matched to BP

The user requested GSF production at the same scale as the just-prepared BP
campaign. Prepared only, not submitted:
`higgs_eeHX_ISRFSRON_GSF_MATCHBP_500jobs_200evt_20261111/`.
500 jobs x200 events =100,000 events, reading the same500 TPC-fix REC files
and entries0..199 as BP_NOSCAN_PREECAL500x200. The native event inventory from
[the immediately preceding BP preparation](2026-11-11-bp-all-rec-noscan-campaign.md)
is copied; all500 input paths, BP manifest/card identities and ranges agree.

The source is the previous matched200x50 GSF card, copied before its original
campaign files became unavailable during this preparation. All500 generated
cards match that source except input/output names, seed and event count; their
normalized physics and effective reverse properties are verified. The copied
worker.py hash matches the old verified worker exactly. New campaign-local
submission helpers follow the existing frozen-runtime/card guards. No old
campaign file is required at runtime.

Effective physics remains reverse, inward scale-1 (fresh seed), lookahead0,
production-electron selection ON, DD4hepBetweenSurfaces, both BH gates ON,
CEPCRuntimeCategoryAligned9Clear, max10, cutoff1e-4, LocalMeasurement and beam
spot ON. ECAL constraint, truth oracle and material recorder remain OFF.
The same private flat writer provides independent per-track IP endpoint views
(BestBranch/WeightedMean/FullMixtureMode) and the12 pre_ecal snapshot branches.
The ten external runtime files match the previous verified SHA256 values.
No simulation, tracking, fitter/writer source, maintained card, installed
library, GSF property default or branch is modified for this task.

Resources:4000MB/job, cms group, scheduler-default walltime (no -wt flag).
The explicit short30-minute request is removed because previous50-event
jobs32 and36 were killed at that cap; the new jobs process200 events.
No new200-event memory/runtime benchmark or physics validation is claimed.

All500 cards pass Python syntax/hash checks, effective-property checks and
matching-BP input/event-range checks. Shell syntax passes. Submission dry run
verifies14 runtime/helper hashes and emits exactly500 commands, with4000MB and
no explicit walltime flag. The scheduler-ID regex passes a positive self-check.
No submitted/started markers or outputs exist. No jobs were submitted.

Artifacts are uncommitted in the new campaign: frozen cards/job manifests,
worker.py/.sh, submit.py/.sh, runtime-sha256.json, campaign-summary.json,
input-event-inventory.json, preparation-validation.json, README and dry-run log.
Keep the referenced private tuple install and normal GSF dependencies intact
while jobs use them; future development can use an isolated build.

Submission from any directory:

```bash
bash /aifs/user/data/zhangcg/gsfdev/CEPCSW/higgs_eeHX_ISRFSRON_GSF_MATCHBP_500jobs_200evt_20261111/submit.sh --submit
```
