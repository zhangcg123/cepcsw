# Default-off GSF material recorder with automatic track scope (2026-11-10)

## Authorized scope

The user requested an explanation of the 319/397 strict raw-hit truth check,
then authorized making the passive recorder default-off and automatically
following the tracks processed by GSF without a recorder input-track index.
Material-path and BH-model changes are explicitly deferred for discussion.

The check is not a track fit or a physics selection. For every raw REC hit it
requires exactly one provenance-bearing associated SimTrackerHit, complete
saved link ranges and hook relations, one primary-electron G4 trajectory,
ordered hooks, contiguous individual-step/compact-span interval coverage,
physical momentum/loss integrals and at most 5 mm reconstructed-hit/truth-hook
distance. One failed hit makes the complete track's truth scope unavailable.
The previous one-file inventory used raw REC hits sorted by radius, not GSF's
matched measurement subset. Of 397 selected reconstructed tracks, 319 passed;
64 had malformed ranges (23 status28, 41 status30), 13 had missing
provenance-bearing associations, and one failed the distance guard. These
are truth availability counts, not ordinary GSF fitting efficiency.

## Implementation

- Existing `RecordTruthMaterialIntervals` is false in compiled defaults,
  the active reverse template and maintained `DumpGsfTrks/gsf.py.bk`.
  No new configurable property was added.
- When enabled, prepare embedded truth once per event and attempt recording
  for every track that reaches the ordinary GSF processing loop, after the
  production-electron selector. With the maintained eeHX selector this follows
  the selected electron pair; other cards follow their actual processed tracks.
- `TruthBHLossInputTrackIndex` scopes only the optional, unchanged truth oracle.
  Its nonnegative-index validation and the shared 5 mm integrity guard remain.
- Per-input EDM `GSFTruthMaterialRecordStatus` is mirrored to the new flat
  `truth_material_track_status` vector, indexed by original input ordinal.
  Existing interval input/output maps and material fields are retained.
  Unselected tracks have status2, disabled tracks0, valid recorded tracks1,
  and unavailable/invalid/unusable tracks retain negative status codes.
- Existing scalar scope status/validity fields remain backward compatible:
  they report the first non-NotSelected status, not the validity of both tracks.
  Focused-out events preserve the existing singleton sentinel (-4 enabled,
  0 disabled), without retrieving their input-track collection.
- Maintained cards request embedded G4 step/link/optional compact-span inputs
  only when their diagnostics are enabled. Ordinary default-off fits do not
  request these collections. The maintained card also supports the separately
  enabled oracle input path.
- Complete property documentation and both workflow READMEs are synchronized.
  The required dedicated option audit passed: 44 compiled custom properties,
  44 README entries, 44 explicit maintained-card settings. The former inherited
  recorder-default exception is retired.

## Verification

Focused verification and same-code native recorder-off/on comparisons use
only the newly produced normal-ISR/FSR campaign's completed job4 REC file,
source events0,1,11,16,17 (ten selected electron tracks). Both cards deliberately
set oracle index99999 with oracle off, demonstrating independence from index0.
Comprehensive component, split and lineage dumps are enabled. Comparison checks
all non-material flat fields, per-track statuses, original input/output maps,
interval-vector lengths, disabled output and focused-event sentinels.

Build and package-only installation of RecGsfTracking/RecGsfFlatTuple passed.
Installed generated configurable confirms the compiled recorder default false.
RecGsfTruthElectronSelector and RecBreakpointCompactTruthReader CTests passed.
Both native runs completed successfully. All205 non-material flat fields are
exactly equal for every entry, including full final-component/lineage vectors,
endpoint outputs and selection mappings. Ten selected tracks are published
on each side. Enabled recording writes1553 intervals for seven tracks:

| Source event | Published original inputs | Recorder statuses | Interval rows |
| ---: | --- | --- | ---: |
| 0 |9,18|1,1|465|
| 1 |23,30|-2,-3|0|
| 11 |2,8|1,1|462|
| 16 |3,5|1,1|464|
| 17 |22,23|-3,1|162|

The three unavailable scopes reproduce the established7.78976mm hook mismatch
(event1/track23), invalid status28 range (event1/track30), and status30 range
(event17/track22). All still publish ordinary GSF endpoints. Off-side material
intervals are empty and every status is disabled. On-side unselected statuses
remain2 and per-interval input/output maps are correct. Focused skipped-event
sentinels and all interval-vector lengths pass. The existing production-electron
selection checker also passes all five events and ten final-mixture/lineage maps.
Native off/on durations were380.59/369.72s; these are validation runs with
verbose dumps, not a timing benchmark.

Analysis outputs and native cards/logs/flat tuples remain uncommitted under
`build.gsf-selection-dev/validation/gsf-recorder-auto-scope-20261110/`.
The maintained card's pre-existing common inward/beam and output-level edits,
and the active reverse template's pre-existing compact-reader work, are
preserved separately from this task's checkpoint. The active reverse-template
steering/input changes remain uncommitted under the project's other-run-card
rule. No simulation, provenance,
material-path, BH response, seed, weighting or endpoint-fit code is changed.
The earlier endcap BH saturation and truth-hook producer faults remain open.
