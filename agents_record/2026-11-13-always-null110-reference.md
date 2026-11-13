# Always retain the genuine no-breakpoint setup-110 reference

User instruction: retain the genuine 5D `kind=0`, setup-110 fit whether the
location scan is enabled or disabled. This implements that instruction using
the existing scan-null fitter. No hypothesis selector or threshold is added.

## Behavior and schema

Each processed/selected track independently attempts the reference before
truth-location matching and ordinary breakpoint fits. The existing
LocationFitScan helper has an internal reference-only path with one null
point and setup110; its tuple writer accepts a branch prefix. RecBreakpoint
books a separate instance as `null_110_*` (34 additional branches), resets it
for each track and saves it alongside the existing outputs. Existing
`location_scan_*` behavior and all configurable properties remain unchanged.

The fit has no material-loss coordinate/prior/Minuit optimization or floating
FSR. It uses the native 5D KF/RTS with the same raw configured beam/ECAL
observations as the scan null. ElectronPhoton plus a collected neutral uses
C+N and hypot(sigma_C,sigma_N); ElectronOnly or no neutral uses C. It does not
consume a previously fitted posterior or truth hooks. Both the beam and charged
ECAL observation are required for this dedicated110 reference. Missing inputs
produce a failed entry, NaN score/state and an explicit error, without a
lower-constraint fallback. Unexpected reference exceptions leave ordinary
endpoint processing intact. Unselected tracks stay status0/empty.

Read `null_110_kind=[0]`, `hit=[-1]`, `setup=[110]`, `fit_status=[2]` and
`actual_constraint_mask=[3]` for a valid reference. `status=1` means records
exist, not fit success. Scores/chi2/log determinants/dimensions, pT/error and
native IP state/covariance are one-entry vectors. Parameter/covariance/pivot
strides are 5/21/3, as in the scan. Prior/loss/variance fields remain NaN and
Minuit status is -99. Truth fields are inactive (truth_status0). No EDM track
collection is added; the reference is flat-tree diagnostic output.

This adds one native reference fit per track. Optional full scans still
calculate their own null, allowing a direct equality check. There is no new
scan, FSR, loss-prior or public enable switch. The maintained scan default
remains [] and FSR remains default-on for110/optional111.

## Verification

Evidence is under `/tmp/bp-null110-20261113/`: before/after/final cards, outputs,
verbose trajectory and endpoint dumps, runtime library fingerprints,
`validation.json`, CTest/batch logs, rollback files and installation metadata.
All jobs read retained REC_ECALTRUTH inputs, never the deleted REC_TPCFIX.

- Final C++ suite: 19/19 tests pass, including the extended prefixed tuple
  serialization/reset test. Batch-helper suite: 65/65 pass.
- Five matching old/new cases compare every pre-existing main/side branch
  exactly, NaN-aware: 197,607 main branch-row values across199 track rows,
  plus882 side branch-row values across7 event rows. No ordinary/floating
  endpoint, old scan value, truth diagnostic or pre-ECAL snapshot changes.
- Current Higgs inputs cover source1 events3/15/112, source2 event135 and
  source4 event1: five unique events/ten selected tracks. Repeated scan and
  missing-input controls are not counted as additional unique events.
- Both selected event3 null references match their same-run scan-null score,
  pT, state/covariance and metadata exactly. Independent references also match
  between scan-on/off and Manual with FreeLossFit/FSR floating both off.
- The seven control groups contain14 successful reference rows and4 deliberate
  missing-beam/ECAL rows (with repeats). Both C-only and C+N cases are covered.
  Normalized score closure and native state strides/finite values pass.
- Source4/event1/track23 retains its original7.78976mm/5mm truth-hook failure,
  while its independent H0 fit succeeds. No provenance guard is loosened.

The initial full implementation passed the five-case numerical A/B and scan
checks with plugin SHA256 ad98a289dc6f4c27f7796cd6878dce2a9971fe05525b5df51843f293bce4635d.
Validation then caught an empty missing-input error string. The final change
only fills that reference-only unavailable-input message. Final plugin
a2f3eaf4a87209cc739cdd18f866416c361a993eb98cbba907f6b547af9e2272
passed all19 CTests and fresh core2, missing-beam, missing-ECAL and Manual/off
runs; the combined checker uses these final outputs in place of the earlier
ones. Both staged revisions and their actual loaded-library paths/hashes are
retained, rather than attributing all runs to the final binary.

## Default installation and limits

Only the RecBreakpoint package installation is refreshed. Before replacement,
no local user process mapped its installed plugin; both known REC_ECALTRUTH
and BP_FSRFLOAT500-job campaigns had500 completion markers. This is a local
map/known-campaign check, not a global scheduler audit. Five previous install
files are backed up under previous-install. The installed binary matches
the tested final plugin. Configurable definitions/defaults are unchanged;
only the generated configurable's timestamp differs.

Original historical particle-gun hard-loss events11/16/17 are unavailable;
current Higgs events are explicitly identified rather than substituted under
those names. This is a mechanical output/regression gate, not population
physics validation or calibrated model selection. No production jobs are
submitted and existing ROOT tuples are unchanged. Rerun only BP to acquire
the new fields; REC files remain usable.

The full outgoing388-line AGENTS.md is preserved in
`2026-11-13-agents-before-always-null110.md`. Only Current focus is replaced;
the global-status/laws/commands prefix is preserved byte-for-byte. No history
directory migration or deletion is performed, so no migration manifest is
needed. Plot/normalization and removed-REC details remain in that snapshot
and their linked dated records.
