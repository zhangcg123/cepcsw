# Prepared radiation-on eeHX batch using the in-memory chain

The user requested rerunning normal ISR/FSR eeHX from HEP generator events,
preparing the batch and providing the submission command. Preparation only;
no jobs were submitted or run, no source/card templates or installation changed.

- Output: `higgs_eeHX_ISRFSRON_inmemory_100k_20261109/` in the reference workspace.
- Maintained campaign size: 500 jobs, file/seed indices 1--500, first 200 events
  per generator file; target 100,000 events. All 500 source files exist/nonempty;
  this is not a new independent event-count or generator-physics validation.
- Generator: `/cefs/higgs/zhangkl/stdhep/E240/higgs/E240.Pe1e1h_X.e0.p0.whizard195/e1e1h_X.e0.p0.{seed:04d}.stdhep`.
  No ISR/FSR-off input is used.
- Every frozen job has `full_chain.py`: SIM -> tracking -> calodigi -> REC ->
  breakpoint in one sequential process. Memory request 8000 MB; group cms.
- Current maintained physics settings: production-electron selection on,
  Truth interval selection and truth endpoints, free-loss on, beam/charged on,
  separate neutral off, ElectronPhoton, PreReclustering, extra 110 scans,
  SigmaLogLoss=0.001; delayed-neutral flag on as in the existing wrapper.
- Preparation cleared ambient BP_* variables and used maintained wrapper defaults.
  All job metadata agree. Prepared-submission dry run verified all frozen card
  checksums and printed exactly 500 hep_sub commands. No submitted/started
  markers and no ROOT outputs exist.
- Generated artifacts (not committed): `campaign-summary.json`, `prepare.log`,
  `submission-dry-run.log` and per-job `runcards/higgs-N/` inside the output.

From the repository root, submit the existing frozen cards:

```bash
OUTPUT_TUPLEPATH=higgs_eeHX_ISRFSRON_inmemory_100k_20261109 \
DRY_RUN=0 SUBMIT_GROUP=cms ./subbreakpointhiggsjobs.sh submit-prepared
```

Do not run preparation again over that directory. Current installed libraries
are not frozen by card checksums; keep the runtime stable while jobs run.
No build/install or batch queue action was performed during preparation.

Superseded for the current request: the user subsequently chose REC production
first, with breakpoint fitting later. Use the new directory/command in
[the REC-first record](2026-11-09-eehx-rec-first-preparation.md). The original
prepared cards remain unchanged and had no submitted/started markers.
