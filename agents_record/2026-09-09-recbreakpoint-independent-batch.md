# Independent breakpoint batch submission and worker

User requested dedicated sub/dump scripts without overwriting the GSF ones.
Added subbreakpointjobs.sh, dump_breakpoint.sh, package-only Python batch
plumbing and standard-library planning tests. No C++ or fitter/card settings
changed. All existing GSF scripts/cards retain their pre-change hashes.

## Workflow

- STAGES defaults trk,breakpoint; any subset of sim,trk,breakpoint runs in
  physical order. Existing predecessor comes from INPUT_TUPLEPATH; a selected
  predecessor writes/feeds its output in OUTPUT_TUPLEPATH. Paths may be absolute
  or relative to the project. Input/output directories must differ.
- Existing filename labels: particle=e-, transverse label2.0, theta label85,
  seeds1..50 by default, NEVT200, memory5000 MB, group higgs. Labels do not
  override the deliberately hard-coded simulation energy/theta ranges.
- Fit steering remains in run_breakpoint.py. Supported BP_* overrides are
  captured at preparation, including the independent TruthOverride boolean.
  Configured intervals apply to every track; no automatic truth selection.
- Generated cards, checksums and job manifests are stored under output/runcards;
  scheduler logs under output/outlog. Worker clears ambient BP_* before applying
  the captured controls and uses the configured EL9 build runner, not run.sh.
- DRY_RUN=1 prepares cards/prints submission commands without submitting.
  `subbreakpointjobs.sh submit OUTPUT_DIR` submits the exact prepared cards;
  ordinary invocation without DRY_RUN prepares and submits in one operation.
- No existing ROOT output/card/log is overwritten. Submitted/started/completed
  markers prevent blind duplicate submission/execution. Failed jobs retain
  files for diagnosis; use a new output directory for changed setups/retries.
- Default card writes only the four-view flat tuple. All input and newly
  produced simulation/tracker tuples are retained. Nothing is deleted.
- ROOT checks require readable nonempty trees, the four-view flat schema and
  at least one successful ordinary fit. Invalid truth rows are reported and
  retained; this is not momentum or physics validation.
- Card contents are frozen, but installed software is not: keep branch/build
  stable while a campaign is queued/running.

## Verification

`bash -n subbreakpointjobs.sh dump_breakpoint.sh` and Python syntax pass.
Seven standard-library tests cover all seven nonempty stage subsets, physical
ordering/predecessor paths, frozen controls/checksums, template drift, invalid
controls/duplicate labels/ranges, existing-output protection, and mocked
scheduler arguments / submitting a prepared campaign. No real scheduler call
is made by these tests.

Local actual-worker tests:

1. Seed12, first two events from sim_large_barrel_20260823, trk then breakpoint,
   interval5, TruthOverride=true. Tracker wrote two events with embedded truth;
   final flat tuple has two successful ordinary fits and zero invalid oracles.
   Artifact: TrackingPerformanceStudies/breakpoint_batch_smoke_20260909/.
2. Existing tracker input from gsf_doublebhoff_freshseed_diagnostic, seed2:68,
   breakpoint only, interval7, TruthOverride=true. One successful flat row and
   zero invalid oracles. Artifact:
   TrackingPerformanceStudies/breakpoint_batch_existing_smoke_20260909/.

Both jobs exited0, produced completed.json, preserved tracker/simulation input
and used the unchanged installed RecBreakpoint library from commit931a97f.
Separate ROOT inspection confirmed oracle-result status2/input-validity1 for
all three rows and all21 covariance elements. Seed2:68 exactly reproduces all
four previous pT values: ordinary44.97146297895819/44.991563840607256 GeV,
oracle44.676959069487786/44.690907113843544 GeV.
Existing ROOT PCM / detector warnings occurred without aborting either job.
Simulation-stage rendering is planning-tested, not Geant4-smoke-tested in this
change. No real Condor/HEP batch job was submitted. No remote operations.

Existing files verified unchanged by SHA256:

- subtrkjobs.sh: 72f0f34c62abf5003daeea0677493518645de953793743ebba58b67e3e952a41
- dump_gsftrk.sh: 124f20c330c1e2138d9e9f2858298cf415f79ad723e76e4a073eaf29e53da852
- DumpGsfTrks/gsf.py.bk: 27eab95e4cfeeecab6f9d2d94046f318c3cfb47adb56bfda6214f6c8b5a3769d
- DumpGsfTrks/sim.py.bk: 61b8dbf67fa20e6c914958e992e1b060f584e5c283675714bdd97a6e6d1be36e
- DumpGsfTrks/trk.py.bk: ecc444ab2f8af7ee356e9c351652f4a129985e5524eff14afd662b72359aa044

Unrelated pre-existing modifications remain untouched. Dedicated new batch
scripts/helper/tests are the user-requested workflow addition; generated cards,
ROOT results, logs and existing local run cards remain outside the checkpoint.
