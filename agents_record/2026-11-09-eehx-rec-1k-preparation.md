# eeHX REC batch: up to 1000 events per input file

The user increased the per-job size to1k and asked whether breakpoint-only
can read the REC tuples. Keep the current roughly100k-event scope, now100 jobs
using original normal-ISR/FSR STDHEP file/seed indices0001--0100. Breakpoint-only
remains supported and separate; no FSR/beam-spot loss extension is implemented.

Input checks used the same LCStdHepRdrNew reader as StdHepRdr, reading up to1000
actual events per file rather than trusting header counts. Sixty files provide
at least1000; forty end after997--999, totaling99,952 readable events up to the
cap. File0002 independently gives998 in a fresh process. The user questioned
why a packing choice was needed; use the ordinary up-to1000 interpretation,
announced with the actual total. No source files were combined, changed or
filtered. Each frozen job NEVT matches its available count, preserving the
existing strict output-event check without code changes.

Prepared `higgs_eeHX_ISRFSRON_REC_100jobs_1k_20261109/`:100jobs,99,952 events,
8000MB,cms; stages exactly sim,trk,calodigi,rec in one process; no breakpoint.
All100 metadata/stage/card checks and prepared-submission checksum checks pass.
Dry run printed100 commands; no jobs submitted, no events simulated. The
previous500x200 preparation remains but its generated summary is superseded.
Generated artifacts include campaign-summary.json (per-seed counts),
generator-input-check.json, prepare.log, submission-dry-run.log and frozen cards.

Submit from the repository root:

```bash
SAMPLE_REGION=higgs DRY_RUN=0 SUBMIT_GROUP=cms \
./subbreakpointjobs.sh submit higgs_eeHX_ISRFSRON_REC_100jobs_1k_20261109
```

After REC production, prepare STAGES=breakpoint with INPUT_TUPLEPATH pointing
here and a fresh OUTPUT_TUPLEPATH. Match each job's actual REC event count;
simulation/tracking/calorimeter reconstruction need not be repeated. Input
compatibility and exact in-memory/file-fit equivalence were already checked in
[the chain regression](2026-11-09-in-memory-breakpoint-chain.md). No new1000-event
native reconstruction or population test was run during this preparation.
