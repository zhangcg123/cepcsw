# Retained truth-location endpoints with independently chosen location scans

## User request and scope

The user requires all enabled free-loss approaches to retain their truth-location
results, while independently configuring which approaches scan locations.
They confirmed no active jobs and authorized installation after validation.
No new branch/remote operation, fitter algebra, optimizer acceptance/bound,
truth ownership, PFA, shared KF or measurement model change was requested.
No production campaign submission or existing-data/card overwrite authorized.

## One new option, existing approach switches

```python
fit.IntervalSelectionMode = "Truth"
fit.FreeLossFit = True
fit.BeamSpotConstraint = True
fit.ChargedEcalMeasurement = True
fit.NeutralEcalMeasurement = False
fit.LocationScanSetups = [110]  # [] off; [0,100,110] scans those three
```

LocationScanSetups is a vector of unique integers from0/100/110/111, sorted
canonically. Compiled and standard-card default[] means no additional scans.
Nonempty lists require Truth and FreeLossFit.100 requires beam ON,110 requires
beam+charged,111 requires all three. No option silently enables an approach.
TruthOverride is an independent prior-center oracle, not the location selector.

| Truth-location result | Existing enable switch | Existing flat endpoint |
|---|---|---|
| Free000 | FreeLossFit | free_loss_rts_* |
| Free100 | BeamSpotConstraint, with FreeLossFit | beam_spot_free_loss_rts_* |
| Free110 | ChargedEcalMeasurement, with FreeLossFit | charged_ecal_free_loss_rts_* |
| Free111 | NeutralEcalMeasurement, with FreeLossFit | neutral_ecal_free_loss_rts_* |

The existing cumulative switch/fallback behavior remains unchanged. Disabled
constraint families remain NaN as before. All enabled truth-location families
run first using the original workflow. Optional scans run afterwards, reset
their location-specific settings from original inputs, and only populate
location_scan_* vectors. They never publish a selected-site endpoint, change
selected_breakpoint_interval, or replace normal collections/branches. Scan
exceptions are isolated from already-published normal/truth results.

Only requested setups are fitted at each scan point. An omitted predecessor
supplies its raw configured observations, not a preceding posterior or another
optimization. Missing/failed observations may retain an earlier successful
REQUESTED setup at the same point. When no such result exists the entry stays
invalid/NaN, with no hidden fit of an unrequested fallback. In particular,
111-only has an invalid null (no loss coordinate,110 not requested). Source,
attempt, actual mask and error fields must be honored during analysis.

Null/VXD/ITK/exact TPC-row0/passive truth point definitions, full IP tuple
vectors, largest eligible truth owner and guards are unchanged. A truth site
already among candidates is tagged there; an outside eligible truth site is
appended last. The additional scan-side reference does not replace the normal
truth-location family. Loss is always freely fitted in these scans.

Legacy HitScanAll remains scan-only with its prior000/100/110/(optional111)
ordering and inactive normal endpoint families. Default new list[] leaves
existing Truth/Manual/HitScan and legacy HitScanAll numerical behavior alone.
HitScanAll cannot be combined with a nonempty new list; fail explicitly.

## Cards and batch helpers

Standard run_breakpoint.py explicitly steers the new list, with a compatibility
guard: old runtime remains usable when the list is empty, but a requested scan
fails clearly instead of silently disappearing. BP_LOCATION_SCAN_SETUPS is a
comma-separated list, frozen by batch_breakpoint.py; parser validation catches
unknown/duplicate values, wrong mode and disabled prerequisite approaches.
The worker checks location-scan schema alongside normal endpoint schema.

subbreakpointjobs.sh defaults the new environment control to empty, preserving
its previous ordinary workload. The dedicated subbreakpointhiggsscanjobs.sh now
uses Truth plus BP_LOCATION_SCAN_SETUPS (default0,100,110), retaining normal
truth endpoints. Explicitly empty disables scans; a subset such as110 scans
only that approach. Future default output is higgs_location_scan_truth_20261104,
so the prior campaign's prepared cards/logs/ROOTs are untouched. No actual new
campaign was frozen or submitted. One one-event dry planning fixture under
the evidence directory exercises the actual helper and source card.

The249x200 input campaign size was NOT changed: most old jobs hit the10-hour
limit (169 killed,80 complete). This option is not a performance or chunking
fix; avoid blindly resubmitting large all-setup jobs. Existing analysis files
and plots remain uncommitted outputs, not new production evidence.

## Verification

Evidence: /tmp/bp-truth-selected-scans-mw0vLY.
15 CTests passed (6.53s).61 Python batch tests passed (latest4.542s), including
scan-list freezing, prerequisite/duplicate errors, schema checks and launcher
Truth-mode defaults. Header tests cover empty, single, sorted and invalid lists.

Verbose local direct A/B on barrel seed1 events11/16/17:

- Previous installed project runtime vs new build with scans OFF: every913
  non-scan branches per row exactly agrees (2,739 branch-row values).
- Previous installed project runtime vs new build with only110 scanning:
  the same2,739 branch-row values agree exactly, including all vectors, IP
  states, loss estimates, scores, status/fallback fields and other endpoints.
- Additional scans contain only110:10/11/11 point entries respectively.
- Legacy scan-only event5 reproduces all947 branches exactly against the
  prior three-setup reference; no schema/ordering or numerical changes there.
- Event5 with all approaches enabled and only111 scanning exactly preserves
  all913 non-scan branches from the previous installed111 run. All four
  truth-location free-loss endpoint families remain finite. Eight candidate
  sites succeed with actual separate mask7; one site fails Minuit and stays
  invalid/source-1/NaN, rather than secretly running110. The inapplicable
  111-only null also remains invalid. No optimizer acceptance repair made.

Truth pT and preserved free000/100/110 pT [GeV]:

| Event | Truth | Free000 | Free100 | Free110 |110 scan entries|
|---|---:|---:|---:|---:|---:|
|11|26.7928657532|26.7376685610|26.7389467889|26.6852596538|10|
|16|11.3328819275|11.3089648470|11.3127073834|11.3179382477|11|
|17|35.5288352966|35.5596676027|35.5206636047|35.4604433561|11|

These are regression checks, not population physics validation. The comparison
uses the freely fitted loss at truth locations, not a truth energy override.

## Runtime and deployment

Old default library SHA256:
3f35d34f46cbb39fec1edf119b127e31a77d83b21eaf3e4931b6ff1ae3eb4dbc.
Validated new library SHA256:
cb2300011261d42eb5b4b5cc299bca4aa3c3f1f3113e40b5220546e2334992e4.

The genuine old-runtime baselines use source setup.sh followed by direct
gaudirun.py; the driver logs the resolved library path/hash. Build/run prepends
build/lib, so the first staged110 and legacy tests use the build module, whose
hash is identical to the staged copy. The111 staged test uses the temporary
runtime explicitly with direct gaudirun.py. Do not mislabel the build/run
no-scan run as an old installed baseline; project-truth.root is the genuine
old installed reference. Test sources are the maintained package, not another
temporary fitter implementation.

Only RecBreakpoint is installed with its105 package cmake_install.cmake.
The old module and Python configuration are backed up in the evidence folder;
no full-project/PFA/shared-KF installation. A direct project-runtime EvtMax0
probe uses the actual frozen test card with Truth, all truth approaches ON
and only110 scanned, verifies the property values and output booking, and
writes only a /tmp output. The probe passed initialization/finalization,
property assertions, full normal/scan schema booking and a zero-row output
check. The generated planning card checksum is unchanged and no started or
submitted marker exists. Batch workers continue using project setup.sh;
no /tmp runtime pin is introduced. Existing cards remain immutable.

## Knowledge preservation

Complete outgoing AGENTS is in2026-11-04-agents-before-configurable-truth-scans.md.
Its introduction, active laws and essential commands are retained byte-for-byte;
only Current focus is replaced. No history directory move or deletion, so no
history-move manifest applies. Prior batch preparation, removed200 ROOT files,
80/169 completion/timeout evidence and old scan-only semantics remain in
snapshots/dated records. Unrelated dirty helper/STDHEP/cleanup/default-campaign
edits are preserved, not silently staged into this checkpoint.
