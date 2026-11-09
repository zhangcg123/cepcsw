# Remove the GSF truth-oracle input-index property (2026-11-10)

The user explicitly requested removing TruthBHLossInputTrackIndex after
clarifying the distinction between the truth-loss oracle and passive material
recorder. This change removes the property, without a compatibility alias or
replacement selector. Material-path and BH-model changes remain deferred.

## Behavior and compatibility

- Deleted the Gaudi property, initialization validation/log steering,
  fixed-index event-status assignment and per-track index comparison.
- TruthBHLossOverride remains default-off. When enabled, it automatically
  follows every track processed by GSF after the existing optional
  SelectTruthProductionElectrons gate. The maintained eeHX selection therefore
  scopes the oracle to its selected reconstructed electron pair.
- Embedded truth is prepared once per event. Every processed track independently
  passes the existing exact hit/provenance, primary-electron, ordered-hook,
  interval-coverage and endpoint-distance checks. Any failed scope uses ordinary
  configured BH for its entire track; a partner's truth validity is independent.
- The retained-fraction map remains keyed by event/original-input/hit/cell IDs,
  so responses cannot leak between tracks. Executed BH replacement semantics,
  strict guards, disabled workflow, beam incompatibility, seeds, weights and
  endpoint publication code are unchanged.
- RecordTruthMaterialIntervals stays false and automatically scoped as already
  implemented; no passive recording is required to enable the oracle.
- Added truth_bh_track_status to the flat tuple, mirroring the existing per-input
  GSFTruthBHLossStatus EDM collection. Original input ordinals remain meaningful
  output/provenance mappings; no input-index steering remains. Focused-out events
  retain the existing singleton sentinel. The legacy scalar oracle status and
  validity still describe original input0 for compatibility and must not be
  interpreted as Higgs-pair status.
- The maintained card no longer assigns the removed property. Older frozen or
  generated cards explicitly assigning it must be regenerated or have that line
  removed. Installed generated configuration rejects the old assignment.
- Complete package/workflow documentation is synchronized. The dedicated audit
  passed43 properties,43 README entries and43 explicit maintained-card settings;
  no surviving configurable defaults changed.

## Verification

Normal RecGsfTracking and RecGsfFlatTuple builds and package-only installation
passed. Installed generated configuration confirms the removed property is
absent, an old assignment is rejected, and both truth switches remain false.
RecGsfTruthElectronSelector and RecBreakpointCompactTruthReader CTests passed.

Same-code native oracle-off/on cards use only the completed new normal-ISR/FSR
job4 REC file, source events0,1,11,16,17. Both use identical beam-off settings
because the optional oracle remains incompatible with the beam boundary;
this is test steering, not a maintained-card default change. Passive recorder
is off on both sides. Comprehensive component/split/lineage dumps are enabled.
The new integration checker verifies per-input oracle statuses, selected-track
scope, focused-event sentinels, changed oracle populations and exact final-
component/lineage fallback rows for tracks whose truth validation fails.

Both native jobs completed successfully and published all ten selected tracks.
Oracle-on validated seven complete truth scopes and replaced154 executed BH
responses. Event1 tracks23/30 and17/22 kept their known statuses-2/-3/-3;
their final-component, node and edge data exactly match oracle-off. Event1,
where both tracks fall back, also has all254 non-oracle flat fields exactly
unchanged. Oracle-valid tracks have changed component populations as expected.
Focused-event sentinels, per-input statuses, and production-electron/final-
mixture/lineage output mappings passed. No population resolution claim follows.

| Source event | Selected and published originals, both runs | Oracle statuses |
| ---: | --- | --- |
| 0 |9,18|1,1|
| 1 |23,30|-2,-3|
| 11 |2,8|1,1|
| 16 |3,5|1,1|
| 17 |22,23|-3,1|

The oracle-off/on runs took377.53/194.29s with verbose dumps; these are not
performance benchmarks. Installed module hashes and exact check results are
in validation.json. Source/control fixtures were not taken from old samples.

All generated cards, logs, flat tuples, comparison/validation JSON and the
option audit remain uncommitted under
build.gsf-selection-dev/validation/gsf-oracle-auto-scope-20261110/.
Existing unrelated card edits, private workspaces/builds/installations and
shared simulation/tracking packages are preserved. No new user batch campaign
was submitted; latest user idle confirmation remains applicable.
