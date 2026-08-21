# Passive truth-material intervals in the final GSF tuple

Date: 2026-08-22

## Decision and scope

The current step is recording only. `RecGsfTracking` may place Geant4,
truth-hook DD4hep, and actual runtime GSF material values side by side in the
final EDM and flat tuple, but none of the new values may be read back into the
GSF workflow. No material path, BH response, split gate, component weight,
measurement update, reduction, reverse decision, or published track is
changed. Physics-impact analysis is deliberately deferred.

The new `RecordTruthMaterialIntervals` property is compiled false and remains
false in the active reverse template. The maintained material-consistency card
sets it true for the current campaign and requests the embedded
`GsfG4MaterialSteps` and `GsfSimTrackerHitG4StepLinks` collections. It reuses
`TruthBHLossInputTrackIndex` and `TruthBHLossMaxEndpointDistance` only to define
the recorded track and the association integrity guard. It is independent of
`TruthBHLossOverride`.

## Final-event contract

`GSFTruthMaterialIntervals` contains one `gsftruth::MaterialInterval` for each
consecutive accepted-hit interval of the selected, successfully fitted input
track. `GSFTruthMaterialRecordStatus` contains one status value per input
track, using the existing truth-scope code table: `1` valid, `0` disabled, and
negative values for unavailable or invalid recording scopes.

Each interval stores:

- accepted-hit, matched-surface, cell, input-track, and output-track bounds;
- exact associated Geant4 track, step bounds, hook fractions, and hook
  positions;
- Geant4 t/X0 integrated from fractional boundary steps plus complete
  intermediate steps;
- Geant4 momentum before the interval, subtype-3 eBrem loss, and retained
  momentum fraction;
- DD4hep validity, segment count, and t/X0 between the same two truth-hook
  positions;
- configured runtime material mode and BH split threshold;
- separate forward and reverse summaries of the actual component paths already
  evaluated by GSF: candidate/valid/above-threshold counts, parent-weighted
  mean t/X0, min/max t/X0, and highest-parent-weight component ID, weight, and
  path t/X0.

The flat tuple mirrors this collection in 50 branches with the
`truth_material_` prefix: three scalar scope/count branches and 47 interval
vectors. These are interval population summaries, not substitutes for the
component-call-level `MaterialBHAuditCSV`.

## Mechanical validation

The generated event-data package, `RecGsfTracking`, and `RecGsfFlatTuple`
built and installed in the EL9/LCG 105 development build. A one-event direct
A/B used the same 2 GeV, 85-degree reconstructed input and production physics
settings, changing only `RecordTruthMaterialIntervals`.

- Recorder on: status `1`, 231 accepted-hit intervals, all 50 flat branches
  present, and every interval vector had length 231.
- Recorder off: status `0`, zero material intervals, and zero-length interval
  vectors.
- The full `GSFTracks` track signature, including track states, covariances,
  fit quality, hit references, and metadata, was exactly equal on/off.
- Every pre-existing flat-tuple branch was exactly equal on/off.
- The recorded event summed to `0.0649686189771` Geant4 t/X0 and
  `0.0648903142985` valid truth-hook DD4hep t/X0; all 231 DD4hep paths were
  valid. One interval contained nonzero eBrem loss.
- Runtime summaries contained 2,001/2,001/83 forward
  candidate/valid/above-threshold paths and 2,230/2,230/105 reverse paths.
- With recording disabled, the pre-existing EventData truth-BH oracle still
  replaced 18 responses and produced an output track exactly equal to its
  pre-recorder reference.

This is a non-interference and persistence check only. It does not establish
that any recorded material quantity predicts GSF performance, and no impact
audit was performed.

## Documentation contract

The full property and branch reference is maintained in
`Reconstruction/RecGsfTracking/README.md`. `DumpGsfTrks/gsf.py.bk` explicitly
steers all 46 package properties and enables this recorder only as current
campaign steering. `DumpGsfTrks/README.md` records the intentional difference
from the compiled and reverse-template false default.
