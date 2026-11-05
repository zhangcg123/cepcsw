# Inclusive eeHX full-chain submission helper

## User request and scope

Update the submission helper for inclusive eeHX,500 jobs x200 events, enabling
the truth production-electron selector. Cards are prepared, NOT submitted.
No new C++/PFA/shared-KF/GSF change, installation or remote operation.

## Maintained launcher and frozen campaign

`subbreakpointhiggsjobs.sh` now uses:

- Input pattern: `/cefs/higgs/zhangkl/stdhep/E240/higgs/E240.Pe1e1h_X.e0.p0.whizard195/e1e1h_X.e0.p0.{seed:04d}.stdhep`.
- Files0001-0500, first200 events each:100,000 generated events.
- Full sim -> trk -> calodigi -> rec -> breakpoint chain.
- Output: `higgs_eeHX_full_20261105`, preserving the older invisible samples.
- Group cms,5000MB/job, existing project-default runtime.
- `BP_SELECT_TRUTH_PRODUCTION_ELECTRONS=1`: only the selected production-side
  electron tracks run expensive breakpoint fits/scans; the whole reconstructed
  event and original track identities remain intact.
- Current defaults otherwise unchanged: FreeLossFit/beam/charged ON,
  neutral OFF, ElectronPhoton, LocationScanSetups110, SigmaLogLoss0.001.
  Other standalone-card defaults, including truth/diffuse families and disabled
  backward filters, remain in the frozen card; no hidden campaign reductions.
- Existing verified job-owned SIM/tracker/calodigi cleanup is unchanged;
  REC and breakpoint flat tuples remain. External inputs are retained.

Running the launcher with no arguments/`prepare` defaults to dry preparation.
The500 cards are already prepared. To submit them, explicitly:

```bash
DRY_RUN=0 ./subbreakpointhiggsjobs.sh submit-prepared
```

Preparation rejects overwriting these cards/outputs. It is not necessary to
prepare them again. Environment overrides for input/output/range/BP
controls must be set BEFORE preparation; they do not change frozen fit cards
at submission. Group can be overridden at submission, following existing policy.

## Validation

Evidence: `/tmp/bp-eehx-helper-QcSU3w/`.

- All1000 inclusive STDHEP files are present; every requested1-500 input is
  nonempty. The prepared manifest audit confirms500 jobs/100,000 events,
  full five-stage chain, selection ON and unchanged110 steering for every job.
- All65 batch tests pass, including the new wrapper full-chain/selection and
  dry-submit fixture. `bash -n subbreakpointhiggsjobs.sh` passes.
- A dry `submit-prepared` rechecks all500 frozen cards/checksums and prints
  exactly500 cms submissions. No `submitted.json` markers or jobs were created.
- A generator-only local run reads200 events from inclusive file0001 through
  the normal StdHepRdr/GenAlgo, without simulation or reconstruction. Running
  the standalone selector on MCParticleGen yields exactly one eligible stable
  non-Higgs e-/e+pair in all200 events. Empty dummy track/association inputs
  deliberately return status-3 (no reconstructed tracks), not an ancestry
  failure. This check does NOT claim reconstructed selection efficiency.
- No local full detector-chain run or population physics/CPU validation was
  performed for inclusive eeHX. No rebuild/install was necessary.

The previous invisible-only launcher was untracked; this request promotes the
updated inclusive launcher to a maintained helper. Existing unrelated changes
in batch_breakpoint.py, tests, subbreakpointjobs.sh and other cards are preserved.

## Memory preservation

The complete outgoing AGENTS is in
`2026-11-05-agents-before-eehx-campaign.md`. Global status/laws/commands and
all existing current-focus headings are retained. Only the outgoing next-step
paragraph (select visible sample/check ancestry/enable gate) is superseded by
the prepared campaign above, and its exact text remains in that snapshot.
No history directory moved, no historical file removed; no migration manifest
is required. This is planning/generator evidence, not a fitter change.
