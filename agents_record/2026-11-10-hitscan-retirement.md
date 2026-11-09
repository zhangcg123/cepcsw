# Ranked HitScan retirement (2026-11-10)

The user authorized removal of HitScan and freshly confirmed that no jobs
were running. This supersedes the previous active-job installation freeze.
Work stays on breakpoint-delayed-neutral; the detached GSF worktree and
its private install are preserved. No simulation/shared-package changes,
production resubmission or truth-hook repair are part of this change.

## Implementation and compatibility

- Removed the ranked HitLocationScan class, its post-selection refit entry
  path, HitScanMinNllGain property, BP_HIT_SCAN_MIN_NLL_GAIN control, and 36
  obsolete hit_scan_* tuple fields.
- Retained LocationFitScan/HitScanAll and its location_scan_* schema,
  reconstructed candidates, true 5D null, optional truth reference, fitted
  losses, objectives and complete saved IP states/covariances. Retained
  helper files are named LocationFitScan.h/.cpp. No fitting physics changes.
- Truth and Manual output families and Truth-plus-LocationScanSetups remain
  available. Removing HitScan does not remove their outputs and does not
  decouple additional Truth-mode scans from the initial truth match.
- Retired mode/threshold requests fail explicitly in the standalone card and
  batch preparation. The runtime rejects the removed mode/property. Frozen
  breakpoint cards assigning HitScanMinNllGain must be regenerated, including
  old Truth/Manual cards that explicitly assigned the former default. Frozen
  REC-only cards and existing ROOT files are untouched.
- Live README, option steering, batch verification, candidate-point tests,
  charged-energy helper tests and explicit-property audit are synchronized.
  The remaining 52 custom algorithm properties are explicitly steered by the
  card and named in the README. No RecGsfTracking configurable was changed.
- Existing dirty batch/test changes are excluded from the checkpoint using
  task-only index content. Other unrelated edits/deletions are preserved.

## Validation

Artifacts: `build.gsf-selection-dev/validation/remove-hitscan-20261110/`.
All native checks use the new production's rec-higgs-4.root; older historical
REC fixtures are not included.

The Python suite passes 73 tests; the C++ suite passes 17 tests. RecBreakpoint
and the changed test targets build successfully. The renamed CMake test target
needed a fresh build invocation after regeneration. Shared-filesystem clock
skew warnings occur; generated property/schema checks and native runs verify
the actual resulting plugin rather than treating warnings as proof of failure.
The retained Truth fits were rerun with VerboseDump on at new-production job4
source events11,16,17, plus event1 for both truth-hook failures. All four jobs
exit0. Each compares exactly against its corresponding pre-change tuple in
883 non-scan track branches and114 event-level branches, across34,39,48,40
track/control rows respectively. The34 location_scan_* fields are excluded
because these checks intentionally disable the optional extra scan; they
are compared separately in the direct HitScanAll A/B. The saved pre-change
library hash matches the runtime fingerprint of the earlier reference tuples.
All three strict truth failures reproduce exactly, including the7.78976mm
message and statuses28/30. A direct native HitScanAll A/B on new job4/event0, with both selected electrons
and VerboseDump on, completed before and after the removal. All917 retained
track branches across41 track/control rows and114 event branches across1 row
are EXACTLY identical, including every location-scan state, covariance, score,
failure/copy status and truth-reference field. Only the36 retired branches
are absent. Both runs terminate normally. No source/profile/optimizer change
was needed to achieve agreement.

The package-only install into InstallArea/x86_64-el9-gcc11-opt is complete.
Installed plugin SHA256 equals the staged plugin used in the native tests;
its generated configuration lacks HitScanMinNllGain and retains Truth and
LocationScanSetups=[110] defaults. Other packages and private GSF runtime paths
were not installed over. Only RecBreakpoint's package install script was run.
The complete outgoing AGENTS.md is archived in
2026-11-10-agents-before-hitscan-retirement.md; global instructions and active
laws are retained byte-for-byte. The new current focus replaces the old one.

## Truth-hook issue to resume

The user also requested a recap of the truth-hook issue. Its full diagnosis
and introduction history remain in
[the provenance record](2026-11-10-new-rec-truth-provenance-failures.md):

- Job4/event1/track30/hit9: malformed TPC step range 459->170, status28;
- Job4/event17/track22/hit9: malformed range 490->132, status30;
- Job4/event1/track23/hit85: correct electron hook, but a merged positron/muon
  reconstructed hit lies 7.78976 mm away and fails the absolute 5 mm guard.

Both malformed-range hooks still exist and match their SimTrackerHit
positions exactly. Our August22 provenance addition failed to reset high-pT
TPC pad-row state across Geant4 particles. The distance guard was added then
too; merged-hit smearing is inherited 2020 behavior. Neither problem was
introduced by November9 compact truth/generator-FSR recording. The native
Truth workflow aborts before its additional scan, whereas HitScanAll catches
truth-reference errors and can scan independently. No guard was loosened and
no simulation-recorder fix was made in this retirement.

The last campaign inspection found 497 completed REC jobs and 3 unverified
jobs (69,311,442, all aliws002), as recorded in
[the runtime inspection](2026-11-10-200event-rec-runtime.md). The user's fresh
idle confirmation does not turn their partial files into verified REC inputs.
The five-event comparison and its limitations remain in
[the comparison record](2026-11-09-new-production-gsf-breakpoint-comparison.md).
