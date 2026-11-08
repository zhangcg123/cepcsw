# eeHX REC first; breakpoint fitting is a separate phase

The user directed decoupling breakpoint fitting from upstream production so REC
can be generated first, while the proposed FSR/beam-spot radiation extension
remains pending. This supersedes the all-five-stage batch preparation in
[the earlier record](2026-11-09-eehx-radiation-on-batch-preparation.md).

Prepared only, without changing source/templates, running events or submitting:

- `higgs_eeHX_ISRFSRON_REC_inmemory_100k_20261109/`.
- 500 jobs x200 events, file/seeds1--500, target100,000 events; normal radiation-on
  `/cefs/higgs/zhangkl/stdhep/E240/higgs/E240.Pe1e1h_X.e0.p0.whizard195/e1e1h_X.e0.p0.{seed:04d}.stdhep`.
- Memory8000MB, group cms. Each `full_chain.py` embeds exactly sim,trk,calodigi,rec.
  No breakpoint card or RecBreakpoint instance is scheduled. Only REC is written;
  no intermediate ROOT files/cleanup. Compact electron/eBrem/FSR truth and ECAL
  reconstructed inputs remain; the twelve raw-calorimeter drops are unchanged.
- All500 stage lists, embedded source dictionaries, source filenames and metadata
  checked. Prepared-submission dry run verifies every frozen checksum and prints
  500 commands. No submitted/started markers or ROOT files exist in this batch.
- Earlier all-five-stage preparation had zero submitted/started markers; it is
  retained, with its generated summary marked superseded for the current request.
  No frozen cards were edited or deleted.
- Generated `campaign-summary.json`, `prepare.log`, `submission-dry-run.log` and
  per-job cards are uncommitted artifacts in the new output directory.

Submit from the repository root:

```bash
SAMPLE_REGION=higgs DRY_RUN=0 SUBMIT_GROUP=cms \
./subbreakpointjobs.sh submit higgs_eeHX_ISRFSRON_REC_inmemory_100k_20261109
```

Later prepare breakpoint-only jobs from these REC files after deciding and
implementing the FSR-aware fit. Do not regenerate detector simulation for fit
iterations. No new native simulation test was run for this preparation; it uses
the existing supported REC-only configuration, without any code change.
