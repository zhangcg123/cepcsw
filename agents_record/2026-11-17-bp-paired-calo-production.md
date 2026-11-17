# Paired ECAL-only and ECAL+HCAL BP production — 2026-11-17

## User decision and scope

The user explicitly requests a new production retaining ECAL-only alongside the
new ECAL+HCAL measurement, with code/card updates and a submission command.
This authorizes a matched comparison production despite the preceding 400-event
study's lack of a demonstrated core-resolution gain. That study and its limits
remain in `2026-11-17-bp-hcal-matched-recoil-comparison.md`; this change does not
claim improved physics performance or retune the HCAL response.

## Implementation

`BP_RUN_BOTH_CALO=1 BP_INCLUDE_HCAL=1` clones the fully configured BP algorithm.
Two independent fitter/algorithm instances consume each shared REC event:
ECAL-only control first, ECAL+HCAL primary second. They execute sequentially
within one job; batch jobs can run concurrently. No fit state or result is
passed between instances. Both files retain every normal hypothesis family,
including genuine H0, HF, ordinary BP110 and BP110+floating FSR.

The only C++ change exposes the 11 previously fixed status/index output
DataHandles as configurable properties. Existing defaults are unchanged. All
24 writers (13 track and 11 status/index collections) can now be remapped;
the control appends `_EcalOnly` to each output key. Inputs remain shared.
No measurement, minimizer, energy-error, selection or publication logic changes.
No GSF, PFA, simulation or shared tracking code changes.

Primary `breakpoint_flat-higgs-N.root` is ECAL+HCAL. Companion
`breakpoint_flat-higgs-N_ecal_only.root` is ECAL-only. Both have identical
schemas and `include_hcal_energy` identifies the measurement. Do not combine
both filename sets into one event sample. Local card/helper default paired
mode off, preserving older single-mode workflows; this campaign explicitly
enables it. Paired mode rejects a disabled HCAL primary and older installations.

The helper accepts Higgs REC filenames at preparation and submission, and
freezes both output paths and walltime. Preparation, submission and
worker start reject existing companions. The worker verifies both files,
exact event/track/selection/truth identities, opposite HCAL flags, and complete
expected event coverage before writing completion. Full-chain completion also
lists both outputs. Legacy manifests without walltime retain scheduler defaults;
new non-default walltimes pass `hep_sub -wt FLAVOUR`.

## Verification and installation

- RecBreakpoint build succeeded. All 19 focused C++ CTests passed.
- 71 batch tests, 5 in-memory-chain tests and 3 paired-configuration tests passed.
  Tests cover all 24 writer registrations, clone isolation, conflicts, collision
  refusal, frozen controls/resources, companion verification and rejection of
  mismatched rows, incorrect mode flags and incomplete event coverage.
- Same-code native verbose reruns used current Higgs REC source 1, event indices
  3, 11, 16, 17: 8 selected electron legs, 94 total track rows and 4 event rows.
  These are current Higgs events, not the unavailable historical gun benchmark.
- New standalone ECAL-only/HCAL outputs each exactly reproduce the preceding
  installation's corresponding four-event outputs. Each paired file exactly
  reproduces its new-code standalone counterpart: 101,272 NaN-aware branch-row
  comparisons per file, all branches included. Each file has 1,072 breakpoint
  branches and 126 neutral_pfos branches. Existing failed fits remain unchanged.
- Paired native run: exit 0, 220.59 seconds wall time, maximum RSS 1,370,692 KiB.
  This is a focused memory measurement, not a bound for all 200-event jobs.
- Installed only RecBreakpoint (five package files), with the previous files
  backed up under `/tmp/bp-paired-20261117/install-backup/`. Installed and tested
  build library SHA256:
  `b6eb3ef7dbc9f86873c58904c74f80531494cc5fb98aba7b0e6e80494faaa265`.
- Installed-runtime smoke used the actual frozen batch card and `dump_breakpoint.sh`
  on current source1/event3: both algorithms completed; worker verified 1 event,
  32 aligned track rows, mode flags and complete event coverage, then wrote
  `completed.json` listing both files. Both installed outputs match their tested
  build-run event3 results exactly across all branches: 34,430 comparisons per
  file. Expected disabled-111 warnings remain. Final smoke directory is
  `/tmp/bp-paired-20261117/installed-worker-matched/`.
- Submit-existing DRY_RUN=1 verified all 500 commands with `-g cms -mem 4000
  -wt mid`; no submitted/started marker or ROOT output exists in production.

Generated cards, logs, ROOT files, comparison script and JSON are uncommitted
under `/tmp/bp-paired-20261117/`. Pre-existing unrelated dirty helper/test changes
are preserved and excluded from this checkpoint. The full campaign cards freeze
current working helper/card behavior; runtime uses the project InstallArea.

## Prepared campaign (not submitted)

Directory:
`higgs_eeHX_ISRFSRON_BP_ECAL_HCAL_PAIRED_500jobs_200evt_20261117/`.

Input: all 500 retained current
`higgs_eeHX_ISRFSRON_REC_ECALTRUTH_500jobs_200evt_20261112/rec-higgs-N.root` files.
REC-only stage, no simulation/tracking/PFA rerun, no input deletion.
500 jobs ×200 events =100,000 inputs per method; 1,000 distinct output files.
All 500 manifest/card hashes, input existence, resources, paths and controls
were audited. Every control in the previous H0F/theta25 production manifest is
preserved exactly; only the paired/HCAL switches and HCAL model are added.
In particular `BP_TRUTH_OVERRIDE=0` keeps the optional truth-loss-prior oracle
disabled, independently of truth-assisted location selection. Initial generic
card preparation inherited oracle-on and was regenerated before submission;
no old checksum-protected card was edited or production job launched. No job has been submitted/started; no ROOT output was created in
the production directory. A campaign-local README and manifest hash inventory
are retained as uncommitted artifacts.

Steering: truth production-electron pair selector on; truth-assisted material
locations; no location scans; genuine null110/HF enabled through normal card;
free-loss and floating-FSR on; beam/charged/neutral setup110; ElectronPhoton sum;
delayed-neutral machinery retained; theta half-window25 mrad, phi200 mrad;
HCAL scale4, per-cluster sigma in GeV `0.30 sqrt(E_H) + 0.07 E_H`.
Resources: 4,000 MB, `mid` walltime, cms group. Roughly twice the single-mode fit
CPU and output storage; no core-resolution improvement inferred from execution.

Submit from any directory:

```bash
bash /aifs/user/data/zhangcg/gsfdev/CEPCSW/higgs_eeHX_ISRFSRON_BP_ECAL_HCAL_PAIRED_500jobs_200evt_20261117/submit.sh
```

The wrapper changes to the reference worktree, selects SAMPLE_REGION=higgs,
DRY_RUN=0 and cms, and submits these frozen manifests. Do not use the generator
full-chain Higgs launcher for this REC-only production. Cards are frozen but
runtime libraries are shared; avoid installing over active production jobs.

## Knowledge preservation

Only Current focus is replaced. All global status/laws/commands are retained
byte-for-byte. Complete outgoing AGENTS is preserved in
`2026-11-17-agents-before-paired-calo-production.md`; no historical files moved
or removed. Current focus maps to this record; prior comparison evidence remains
in its original dated record. Gallery pages and existing plots are unchanged.
