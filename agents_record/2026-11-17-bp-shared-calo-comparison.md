# One BP pass, shared upstream, two calorimeter result families — 2026-11-17

## User correction and final design

The user explicitly rejects running the complete BP algorithm twice and asks
for shared data/upstream fits with ECAL-only and ECAL+HCAL branches in one tuple.
This supersedes the two-instance/two-file production recorded in
`2026-11-17-bp-paired-calo-production.md`. No production had been submitted.

`BP_RUN_BOTH_CALO=1` now sets native `RunBothCalo=true`; the card schedules one
RecBreakpoint instance. One REC read, hit ordering, truth match, Baseline KF,
tracker000 and beam100 fit sequence, event/cluster/radiation truth records are
shared. Only H0, HF, ordinary/free-loss110/111 and their floating-FSR extensions
are run with each calorimeter observation. A CalorimeterFitTuple bundle keeps
all dependent statuses, masks, errors, likelihoods, optimizer diagnostics,
endpoint/covariance and fitted-loss fields separate. H0/HF remain before truth
matching, including in tracks whose later provenance check fails.

Existing unprefixed branches are ECAL+HCAL. The ECAL-only result counterparts
have `ecal_only_` prefixes. Shared upstream fallbacks retain their unprefixed
names. There is one `breakpoint` row per input track and one `neutral_pfos` row
per selected event, with no second file/tree/input copy. No online mode or
hypothesis selection is installed. HCAL scale/resolution, ECAL/HCAL associations,
neutral selection, sum projections, FSR modeling and failure rules are unchanged.

The ECAL-only path independently starts from the same accepted beam/tracker
stage and only inherits its observations, never the other calorimeter posterior
or optimized loss center. It publishes saved flat states only, with no extra EDM
collections and no misleading `*_rts_index` branches. Primary EDM publication
retains the existing 24 configurable writers. The shared calorimeter-state
serializer serves both publication and flat-only results using the same native
propagation operation; there is no independent Kalman update implementation.

`RunBothCalo` defaults false, requires IncludeHcalEnergy true and scans off.
111 remains supported when separately enabled. Existing single-mode scan
workflows retain their settings. The comparison rejects scans explicitly.
New `run_both_calo` metadata identifies this layout; `include_hcal_energy`
describes the primary. Raw ECAL and calibrated HCAL observations are saved once.

The worker checks both family schemas and mode flags in the same file and
complete expected event coverage. Retired two-file manifests are rejected
before submission or a started marker. Cards/manifests were regenerated rather
than edited in place. Only RecBreakpoint and its dedicated card/helper/tests/
build registration/docs changed; unrelated dirty files/hunks are preserved.

## Mechanical and numerical gates

- Build succeeded. All 19 C++ CTests passed through the build-runtime launcher.
  The existing FSR-family test now checks primary/control branch uniqueness,
  absent repeated 000/100/index branches, independent failures and row resets.
  Its target needs the package include path for the shared tuple header.
  Running the newly linked test against the old installed library first exposed
  the expected changed internal FitPairTuple::book symbol; build runtime and
  then the refreshed package resolve it. This was not a fitter failure.
- 71 batch tests, 5 in-memory-chain tests, 2 configuration tests passed.
  These cover one algorithm, frozen comparison controls/resources, one output,
  retired manifest refusal, missing families, wrong mode flags and truncation.
- Same-code verbose native runs: current Higgs REC1 events3,11,16,17, eight
  selected electron legs, 94 track rows and four event rows. These are current
  Higgs cases, not the unavailable historical gun benchmark.
- Fresh single ECAL-only and HCAL runs each exactly reproduce all existing
  fields of their previously tested outputs: 100768 track branch-row values
  plus504 event values =101272 per mode, NaN-aware equality.
- Same-file primary matches its same-code standalone on those same101272
  values; all365 ECAL-only dependent branches match the ECAL-only standalone
  exactly (34310 branch-row values). Four extra secondary RTS index fields are
  deliberately omitted. New metadata is checked separately. Existing failures,
  statuses, scores, covariances and fitted endpoints are preserved.
- An additional111 run on REC1/event3 reproduces the same-code standalone
  HCAL primary (34430 values) and ECAL-only control (11680 values). Track5 has
  no collected neutral and retains110, status3/mask3, in both branches. Track25
  has4.80444574356GeV ECAL neutral energy and successful111 FSR fits,
  status2/mask7, in both branches. This exercises the delayed/separate-neutral
  path with and without a resolved neutral.
- Four-event file:6779610bytes versus10210941bytes for the previous two files,
  a33.6045% reduction. One1438-branch track tree (1072 existing +1 metadata
  +365 control), one126-branch event tree. This sample is not a production
  storage bound. Existing data and outputs remain available under /tmp.
- Four-event run:169.03s user CPU,4.30s system,207.99s wall, maxRSS1330796KiB.
  Previous two-instance focused run had207.15s user and220.59s wall. These are
  descriptive single-run timings with shared-host/I/O differences, not a
  calibrated throughput claim or a physics-resolution validation.

Artifacts, scripts, logs, cards and ROOT outputs (uncommitted):
`/tmp/bp-singlepass-20261117/`; older matched references remain in
`/tmp/bp-paired-20261117/` and `/tmp/bp-hcal-20261117/`.

## Installation and campaign

Only the five RecBreakpoint package runtime files were installed, with their
previous versions backed up in `/tmp/bp-singlepass-20261117/install-backup/`.
Tested/installed library SHA256:
`7e833d9868c72638f3b591e1c73be16f17064f7000d98ae0b40f40196c727fe6`.
Actual installed-runtime `dump_breakpoint.sh` on the frozen REC1/event3 card
completed, verified both schemas/flags and1event/32sharedtrackrows, and wrote
completion with one file. Its entire output matches the tested build event3
exactly:46142branch-row comparisons, including the new metadata/control fields.
Final submit-existing DRY_RUN verified500commands withcms/4000MB/mid;all500
manifest/card checksums and input paths audited. No production submitted/started
markers or ROOT outputs exist. The submit.sh shell syntax also passed.

Campaign remains
`higgs_eeHX_ISRFSRON_BP_ECAL_HCAL_PAIRED_500jobs_200evt_20261117/`:
500 jobs ×200 events =100000 inputs, now **500 flat files total** containing
both result families. Input is all500 current REC_ECALTRUTH Nov12 files.
The old unsubmitted plans were removed after confirming no submitted/started
markers or ROOT outputs. All previous H0F/theta25 controls are frozen exactly,
plus the HCAL/RunBothCalo settings: no scan, truth locations, H0/HF, floating FSR,
ordinary110, optional truth-loss-prior oracle off, ±25/±200mrad window,
HCAL×4 and σ=0.30√E+0.07E per cluster. Resources4000MB,mid,cms.
No production is submitted by this task. Submit from any directory:

```bash
bash /aifs/user/data/zhangcg/gsfdev/CEPCSW/higgs_eeHX_ISRFSRON_BP_ECAL_HCAL_PAIRED_500jobs_200evt_20261117/submit.sh
```

Cards are frozen but runtime libraries are shared; avoid installing over active
campaign jobs. This change consolidates calculation/storage; the prior HCAL
study's calibration and physics-performance limitations still apply.

## Knowledge preservation

Only Current focus is replaced. Global status, active laws and compile/run
instructions retain their exact bytes. Complete outgoing AGENTS is preserved in
`2026-11-17-agents-before-shared-calo-comparison.md`. No history files or gallery
content are moved/removed. No new plots or server changes were requested.
