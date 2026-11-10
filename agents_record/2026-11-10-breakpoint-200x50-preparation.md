# BP REC-to-flat preparation: 200 jobs x 50 events

The user chose200 jobs x50 events after reviewing older scan campaign sizes.
Their earlier instruction remains active: do not submit BP until the running
GSF campaign finishes. Before this preparation, the current focus recorded
that no BP campaign was prepared; this record supersedes that statement only.

Prepared `higgs_eeHX_ISRFSRON_BP_TPCFIX_200jobs_50evt_20261110/`, with source
`higgs_eeHX_ISRFSRON_REC_TPCFIX_500jobs_200evt_20261110/`.

- Jobs/seeds1--200; source entries0--49 from each corresponding REC file.
  Total10,000 requested events. This is not a repartition of all100,000 inputs.
- STAGES=breakpoint only; no simulation/tracking/calorimeter/GSF rerun.
- All200 source jobs have successful REC completion records,200-event input
  manifests, matching paths and nonempty REC files. Existing inputs are retained.
-200 frozen BP cards compile and have verified SHA-256 checksums. Their effective
  settings were evaluated without Gaudi and checked against the requested setup.
- Production-pair selection on; Truth interval selection; only additional110
  scan. Ordinary/free-loss/truth-centred/diffuse RTS stay enabled; beam/charged
  ECAL on, neutral measurement off, PreReclustering/ElectronPhoton. All backward
  filters remain off. Other maintained-card defaults are unchanged, including
  sigma0.001 and delayed-neutral steering inherited from the helper (inert with
  separate111 off). Event-wide eBrem/FSR truth export remains available.
-4000MB/job, group cms, default10-hour walltime. A50-event scan/resource pilot
  has not been run; the resource settings are not a measured upper bound.
- The prepared-submission dry run validates all cards and prints exactly200
  commands with -g cms -mem4000. No submitted/started/completed BP markers exist.

The generated campaign-summary.json, README.md, prepare.log,
submission-dry-run.log and per-job cards/manifests are uncommitted artifacts.
The README/summary explicitly hold submission until GSF finishes. No actual
submission command was executed. The existing helper's later submit mode can
consume these frozen manifests after that hold is released.

This task changes no fitter, source helper, maintained card, library, build,
installation, GSF job or REC output. At the initial bounded preparation check,
GSF had500 started,4 completed and3 failed markers; this is a transient marker
snapshot, not a fresh scheduler inventory. Keep active GSF runtime paths frozen.
