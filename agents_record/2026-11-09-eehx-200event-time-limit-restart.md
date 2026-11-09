# Smaller REC production jobs after the 10-hour time limit

## Observed production outcome

The user submitted all100 previously prepared REC-only jobs (up to1000 events
per job). Before deletion, submitted/started markers existed for all100 and
completed markers for none. The latest log snapshot found39 explicit scheduler
messages: `Job is KILLED: exceeds the 10.0 hour(s) time limit`. Those jobs reached
roughly382--443 events (median415). Another27 logs advanced during the check;
34 statuses remained unresolved. No normal completion was observed. Scheduler
query was unavailable (hep_q could not import htcondor); do not reinterpret
unresolved jobs as confirmed killed. These were confirmed time-limit kills,
not evidence of an out-of-memory cause. ROOT files were partial/unverified.
The user then reported deleting the previous production. All old campaign
directories were absent at inspection. Output deletion alone does not establish
queue cancellation or authorize builds/installations.

## New preparation

The user requested adjusting the per-job count. Prepared only:
`higgs_eeHX_ISRFSRON_REC_500jobs_200evt_20261109/`.

- 500 jobs,200 events each, target100,000 events; original normal-ISR/FSR STDHEP
  file/seed indices1--500 from E240.Pe1e1h_X.e0.p0.whizard195.
- Single-process sim,trk,calodigi,rec only; no breakpoint refit or intermediate
  ROOT files. Compact electron/eBrem/FSR truth and reconstructed ECAL inputs
  remain; the twelve raw calorimeter REC drops remain.
- Memory8000MB, group cms. No code/template, installation, branch or queue-policy
  change. No jobs were submitted by the agent.
- All500 metadata/stage/card checks pass. Prepared-submission dry run verifies
  every frozen checksum and prints500 commands. No new submitted/started marker.
- Generated campaign-summary.json, prepare.log, submission-dry-run.log and cards
  are uncommitted artifacts in the output directory.

Submit from the repository root:

```bash
SAMPLE_REGION=higgs DRY_RUN=0 SUBMIT_GROUP=cms \
./subbreakpointjobs.sh submit higgs_eeHX_ISRFSRON_REC_500jobs_200evt_20261109
```

The200-event choice is based on observed throughput and gives substantially
more margin below10hours; no200-event native runtime benchmark was added.
Keep breakpoint-only fitting separate, reading completed/verified REC later.
The FSR/beam-spot radiation extension is still pending. Original DetCRD scripts
are unchanged. Prior focus is preserved byte-for-byte in
[the snapshot](2026-11-09-agents-before-200event-restart.md).
