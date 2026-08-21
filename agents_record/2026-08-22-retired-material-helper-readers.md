# Retired material helper readers

Date: 2026-08-22

## Decision

After retiring the side `gsf_material_steps*.root` producer and runtime
material/BH audit CSV, the corresponding truth-oracle file readers are also
removed. `TruthBHLossOverride` now consumes only the embedded event collections
`GsfG4MaterialSteps` and `GsfSimTrackerHitG4StepLinks`, joined through standard
tracker-hit truth associations.

## Removed interface and implementation

- Removed the `TruthBHLossSource` and `TruthBHLossInput` Gaudi properties.
- Removed the strict consecutive-hit CSV parser and its file-backed selected-
  track map from `GsfAlgorithm`.
- Removed the `G4StepTuple` dispatch, matching path, and
  `TruthBHLossTupleReader.{h,cpp}` implementation.
- Removed the tuple reader from `RecGsfTracking/CMakeLists.txt`.
- Removed the obsolete `build_g4_transition_dataset.py` helper that read the
  retired side tuple.
- Removed the two retired properties from maintained run cards and the active
  option reference. The supported property count is now 43.

## Preserved path

`TruthBHLossOverride=false` remains the production default. When explicitly
enabled, it validates the configured `TruthBHLossInputTrackIndex` against the
embedded event provenance with `TruthBHLossMaxEndpointDistance` as an integrity
guard. Invalid event or track provenance falls back to the configured BH model
for the whole selected track and writes the existing status tag.

The default-on passive final-tuple recorder remains unchanged. It uses the same
embedded event provenance to write `GSFTruthMaterialIntervals`,
`GSFTruthMaterialRecordStatus`, and the flat tuple's `truth_material_*`
branches. Historical ROOT/CSV artifacts and dated records remain evidence, but
current runtime code no longer reads those helper files.

## Verification boundary

The focused `RecGsfTracking` build and install succeed after the removal. The
installed runtime exposes neither removed property. A one-event embedded-data
truth-on run validated status `1`, matched one selected track, closed all 18
executed oracle calls, and completed with zero invalid-event, endpoint, or
interval-map fallbacks. Its output directory contained only the requested final
EDM and flat tuple—no side ROOT or CSV file. This is an interface cleanup, not a
new material/BH physics result.
