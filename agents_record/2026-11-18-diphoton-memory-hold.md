# Diphoton full-chain memory holds — 2026-11-18

## Confirmed cause

The user submitted the25 prepared mu+mu-H,H->gamma gamma jobs and reports all
held. Every job has submitted.json and started.json, none has completed.json
or a REC output. All25 stdout logs stop at CyberPFAlg initialization, before
event0. The user supplies the scheduler reason:

`Error from slot59@lhws302.ihep.ac.cn: Job has gone over cgroup memory limit of 4016 megabytes. Last measured usage: 4003 megabytes.`

The assistant incorrectly requested4,000MB for the full SIM/TRK/calodigi/PFA
in-memory chain. The maintained full-chain helper defaults to8,000MB, and
2026-11-09-in-memory-breakpoint-chain.md already recorded approximately5.76GiB
resident startup memory. The4GB choice should not have been carried over from
the substantially lighter REC-only BP production. Successful unrestricted local
pilot execution did not validate the scheduler memory limit.

The HCAL uniform-response-map warning occurs in the successful pilot too; it
is not the scheduler hold cause. No evidence here indicates a physics/source
bug, input-file shortage or walltime kill.

## Direct diagnostic and recovery

A new isolated source1/event0 full detector-chain run is instrumented with
`/usr/bin/time -v` under
`TrackingPerformanceStudies/higgs_diphoton_clusters_20261118/hold_memory_probe/`.
A live `/proc` sample of only its gaudirun process records VmRSS=VmHWM=
5,468,972KiB (5.22GiB), VmSize=VmPeak=6,013,772KiB, one thread, during
initialization. This already exceeds the original cgroup limit; final time/RSS
results, when completed, are in hold-memory-time.txt. This one-event sample is
not a200-event production memory upper bound.

Prepared uncommitted campaign-local resume_8gb.py. It derives exact job IDs
from each submitted.json (84030118–84030143 excluding84030137), validates all
frozen-card hashes and absence of outputs/completion, and queries the current
memory holds with the existing hep_q. It refuses to act unless all25 exact
IDs are held for memory. It edits those held requests to8,000MB using hep_edit,
archives original start markers/manifests/stdout/stderr, updates only manifest
memory metadata, and hep_release releases the same jobs. No duplicates, source
or physics-card changes, build or installation. Existing submitted.json remains
the original4GB submission evidence. Focused parser checks cover memory holds,
non-memory holds and multiple rows; all25 local preflight checks pass.

Run from the working batch-login shell:

```bash
python3 /aifs/user/data/zhangcg/gsfdev/CEPCSW/higgs_mumuHgg_CLUSTER_25jobs_200evt_20261118/resume_8gb.py
```

The recovery has not been executed here: the local queue client still lacks
htcondor, and login-host fallback is unavailable. Native production/card
outputs and queue state have not been changed by this investigation. The
campaign README identifies the incorrect initial request and recovery command.
Do not rerun submit.sh or blindly release with stale started.json markers.
