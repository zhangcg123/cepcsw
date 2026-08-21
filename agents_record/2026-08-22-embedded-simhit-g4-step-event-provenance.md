# Embedded SimTrackerHit-to-Geant4-step provenance

Date: 2026-08-22

## Decision

The maintained simulation-to-GSF workflow no longer needs a separate
`GsfMaterialStepRecorderAnaElemTool` ROOT tuple to supply the default-off truth
BH-loss oracle.  When explicitly enabled, the normal EDM writer now persists
two PODIO collections in `sim.root` and the downstream `trk`, `calodigi`, and
`rec` stages preserve them:

```text
accepted reconstructed TrackerHit
  -> MCRecoTrackerAssociation
  -> exact SimTrackerHit
  -> GsfSimTrackerHitG4StepLinks
  -> exact GsfG4MaterialSteps range and truth hook
```

The side-file `G4StepTuple` reader and strict prejoined CSV reader remain
available only for historical reproduction and cross-checks.  The side
material recorder remains compiled for compatibility, but it is not active in
the maintained simulation card or `dump_gsftrk.sh` workflow.

This is truth-diagnostic plumbing only.  It is default-off in `RecGsfTracking`,
does not steer production GSF, and does not change the production BH model.

## Persisted event-data contract

The new `Simulation/GsfTruthEventData` PODIO extension defines:

- `GsfG4MaterialSteps`: selected Geant4 steps with stable event-local
  track/parent/step identifiers, pre/post positions and momenta, process
  subtype, momentum loss, retained fraction, deposited and kinetic energy,
  step length, radiation length, `t/X0`, track length, and time.
- `GsfSimTrackerHitG4StepLinks`: a direct relation to the same persisted
  `SimTrackerHit`, relations to the first, last, and hook Geant4 steps, scalar
  copies of their step numbers, the hook fraction/kind, provenance type, and
  completeness status.

The standard `Edm4hepWriterAnaElemTool` owns the output.  Its new controls are
default-off and independently restrict persisted truth by PDG, primary-track
status, and tracker material.  The maintained `DumpGsfTrks/sim.py.bk` enables
the collections for primary electrons, positrons, muons, and antimuons.

Sensitive detectors propagate the event-local Geant4 provenance into their
transient `Geant4Hit`.  One-step detectors use the exact step.  Combined
silicon hits preserve the exact contributing-step range and let the writer
resolve a hook only inside that range.  TPC high-pT pad-row hits preserve the
complete contributing range and use the center-crossing post-step hook.  An
incomplete link is tolerated in the event, but it invalidates the truth scope
if the selected reconstructed track actually uses that hit.

## RecGsfTracking source

`TruthBHLossSource="EventData"` is now an allowed default-off oracle source.
It requires `TruthBHLossInput=""` and reads the current event.  For the one
configured `CompleteTracks` index it:

1. follows standard tracker-hit truth associations to an exact
   provenance-bearing `SimTrackerHit`;
2. requires a unique same-primary-electron, complete, monotonic hook sequence;
3. checks the associated hook against the reconstructed endpoint using the
   configurable 5 mm integrity guard, without using spatial proximity to
   choose the truth hit or Geant4 step;
4. sums process-subtype-3 Geant4 momentum loss in each ordered hook interval;
5. replaces only already executed BH responses with the corresponding exact
   retained fraction and leaves the downstream GSF workflow unchanged.

Missing, ambiguous, incomplete, or nonphysical selected-track provenance uses
ordinary BH for the entire selected track and records the existing negative
truth-scope status.  Old inputs remain usable when the oracle is off because
the maintained GSF card requests the custom collections only when
`TruthBHLossOverride=true`.

## Mechanical validation

A clean one-event 2 GeV, 85 degree electron chain was run from simulation
through tracking and GSF in `/tmp`:

- simulation wrote 629 unique `GsfG4MaterialSteps`, 233
  `GsfSimTrackerHitG4StepLinks`, and 233 standard tracker hits;
- all 233 links had complete status, direct SimTrackerHit relations, and
  mutually consistent scalar and object step references;
- tracking produced one `CompleteTracks` track with the expected tracker-hit
  association collections, and tracking, calorimeter digitization, and
  calorimeter reconstruction each preserved all 629 steps and 233 links;
- with the reconstructed `rec.root` as its input, the EventData oracle exactly
  matched 231 accepted-hit anchors, with a maximum association-hook endpoint
  discrepancy of 2.19813 mm;
- 18 executed BH responses were truth-overridden, the selected-track status
  was `1`, and one `GSFTracks` result was written;
- a forced 0.1 mm endpoint-guard failure completed the event with ordinary BH,
  replaced zero calls, counted one endpoint-distance fallback, and persisted
  status `-2`;
- no side `gsf_material_steps.root` file was produced;
- a truth-off GSF smoke on a historical input without the custom collections
  completed successfully.

The generated dictionaries were also installed with their ROOT PCM/rootmap,
and the complete relevant target set builds successfully.

## Validation boundary and next use

This closes the data-flow mechanism for one representative event.  It does not
validate the BH response, the truth oracle as production logic, or population
momentum performance.  Canonical hard events 11, 16, and 17 were unavailable
for a same-input rerun because their earlier input was deleted.  The next new
batch may use the normal `sim -> trk -> calodigi -> rec -> gsf` chain with the
embedded collections, then validate EventData matching/status rates and the
truth-oracle A/B on focused hard events and held-out clean controls.  The
material/BH branch-local investigation remains the active physics focus.
