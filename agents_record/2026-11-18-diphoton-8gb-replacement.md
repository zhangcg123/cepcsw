# Fresh 8GB diphoton campaign replaces failed4GB preparation — 2026-11-18

The user requests removing the old things and preparing a new batch, rather
than releasing the memory-held jobs in place. This supersedes the prior
resume_8gb.py recovery proposal and its now-retired command.

## Completed local actions

Prepared `higgs_mumuHgg_CLUSTER_8GB_25jobs_200evt_20261118/` with25jobs x200events
=5,000inputs,8,000MB,mid,cms. Same240GeV mu+mu-H,H->gamma gamma STDHEP files
0001–0025 and first200events/file. Same in-memory sim,trk,calodigi,rec stages,
pre/post-reclustering snapshots and native truth links; no BP/GSF. The earlier
generator-input-check.json is retained for the exact same verified inputs.

All25 jobs pass stage/event/resource/input checks, source/card hashes and Python
compilation. Every frozen detector/full-chain card is exactly equal to its old
counterpart after replacing only the old output directory with the new one.
All25 prepared-submission dry-run commands request8,000MB andmid. Shell syntax
checks pass. No native C++, original tracking/PFA, maintained templates, build,
installation or existing electron production changes.

The instrumented one-event source1 repeat completes successfully, writes a
verified REC and exits0. Maximum resident set=5,939,352KiB (5.66GiB), elapsed
8:34.85. Evidence is hold-memory-time.txt and hold-memory-probe.log under
TrackingPerformanceStudies/higgs_diphoton_clusters_20261118/. This demonstrates
why4GB fails; it is not a200-event memory upper bound or photon validation.

Removed only the failed untracked directory
`higgs_mumuHgg_CLUSTER_25jobs_200evt_20261118/`, including its obsolete resume
script and failed logs/cards, as explicitly requested. Before deletion verified
exact path/no symlink/no tracked files,25old manifests and no completed marker
or detector outputs. Preserved the exact25old scheduler IDs, old/new checksums
and source provenance in the new replacement.json, and the confirmed memory
hold evidence in the preceding durable record. Diagnostic studies remain.

## Queue state and submission

The old25 queue jobs are not yet cancelled: local hep_rm cannot import htcondor.
A direct targeted removal attempt exits1 through the new guard; no new jobs are
submitted. The script catches the wrapper's misleading zero exit on a Python
error. replacement.json explicitly records old_local_directory_removed=true,
old_queue_jobs_cancelled=false,new_jobs_submitted=false as of preparation.

New submit.sh first invokes remove_old_jobs.sh for only IDs84030118–84030143
excluding84030137, then submits the fresh frozen cards through the existing
helper. No blanket user/group removal, duplicate release or source change.
If queue cleanup errors, the new batch is not submitted. The command must run
from the user's working batch-login shell:

```bash
bash /aifs/user/data/zhangcg/gsfdev/CEPCSW/higgs_mumuHgg_CLUSTER_8GB_25jobs_200evt_20261118/submit.sh
```

Generated cards, helpers, JSON/logs and new campaign README remain uncommitted.
Do not use the deleted4GB submit or in-place-resume commands. Both previous
preparation/hold records retain historical evidence; this record is the current
replacement status. No5,000-event reconstruction result is claimed.
