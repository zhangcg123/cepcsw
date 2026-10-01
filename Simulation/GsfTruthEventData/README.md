# Embedded GSF truth event data

This PODIO extension stores the Geant4 information needed by default-off GSF
mechanism diagnostics inside the ordinary EDM event. It is not a production
tracking input and does not make the normal GSF truth-dependent.

The simulation writes three provenance collections:

- `GsfG4MaterialSteps` (`gsftruth::G4MaterialStep`): selected Geant4 pre/post
  steps, including positions, momenta, process subtype, local material
  thickness, energy deposits, and track-length coordinates;
- `GsfSimTrackerHitG4StepLinks`
  (`gsftruth::SimTrackerHitG4StepLink`): the exact persisted `SimTrackerHit`,
  its first and last contributing Geant4 steps, and its measurement hook.
- `GsfG4BremsPhotons` (`gsftruth::G4BremsPhoton`): every photon returned by
  `GetSecondaryInCurrentStep()` for a selected electron/positron parent whose
  photon creator process has bremsstrahlung subtype 3. The parent's defining
  process is not a filter: it identifies the step limiter, not necessarily
  every process contributing secondaries. The existing PDG, primary-only and
  tracker-envelope selection applies to the parent step. Each photon has a direct
  `parentStep` relation and redundant parent-track/step numbers, its index in
  that step's full secondary list, creation position, momentum, energy, and
  time. These are photon-creation values, not calorimeter or final-state
  values. Geant4 has not necessarily assigned the secondary a usable track ID
  at this point, so the step relation plus secondary index is the identity.

The event-level relation chain is:

```text
reconstructed TrackerHit
  -> edm4hep::MCRecoTrackerAssociation
  -> edm4hep::SimTrackerHit
  -> gsftruth::SimTrackerHitG4StepLink
  -> gsftruth::G4MaterialStep range and hook
```

The detector sensitive code records provenance while the transient hit is
created. Per-step hits use that step directly. Combined silicon hits retain
their exact contributing-step range, and the standard event writer resolves
the hit's traversal midpoint only within that range. TPC pad-row hits retain
the center-crossing step. Low-pT TPC accumulation can expose an unresolved
hook; it is stored with an incomplete status and invalidates only a selected
diagnostic track that actually uses it.

`SimTrackerHitG4StepLink::status` is a bit mask:

| Bit | Meaning |
|---:|---|
| 0 | first/last scalar step bounds are ordered |
| 1 | `firstStep` relation is available |
| 2 | `lastStep` relation is available |
| 3 | scalar hook step is resolved |
| 4 | `hookStep` relation is available |

The complete value is therefore `31`. `hookKind` values are `1` pre-point,
`2` post-point, `3` step midpoint, and `4` traversal midpoint. The
`provenanceType` values are `1` per-step detector hit, `2` combined traversal,
`3` TPC pad row, `4` TPC space point, and `5` TPC low-pT accumulation.

Enable writing on the standard simulation writer:

```python
edm4hep_writer.WriteGsfTruthEventData = True
edm4hep_writer.GsfTruthPDGs = [11, -11, 13, -13]
edm4hep_writer.GsfTruthPrimaryOnly = True
edm4hep_writer.GsfTruthTrackerOnly = True
```

All three collections are written into the same `sim*.root` event and survive the
normal `keep *` chain. ROOT dictionary PCM/rootmap files are installed beside
the generated libraries so generic PODIO readers can deserialize them.
Photons below the generic `MCParticle` secondary threshold are included; do
not assume each photon has a corresponding stored `MCParticle`. The photon
collection is empty when no selected eBrem step emits a photon.
Only explicit Geant4 secondary photons are available: energy loss below the
Geant4 photon-production cut is not converted into synthetic photon records.

`RecGsfTracking` can additionally write
`GSFTruthMaterialIntervals` (`gsftruth::MaterialInterval`) into its final EDM
output when `RecordTruthMaterialIntervals=true`. Each object corresponds to
one consecutive accepted-hit interval on the configured input track and keeps
three quantities side by side:

- fractionally integrated Geant4 step t/X0 between the exact associated truth
  hooks, together with the truth eBrem loss;
- DD4hep t/X0 evaluated between those same truth-hook positions;
- direction-separated candidate, valid, above-threshold, parent-weighted,
  minimum, maximum, and leading-component summaries of the material paths
  already evaluated by the ordinary forward/reverse GSF.

`GSFTruthMaterialRecordStatus` stores one scope code per input track. The
recorder is passive: none of these output fields is read back by propagation,
BH splitting, component weighting, reduction, or final selection. Missing or
invalid provenance therefore produces an empty/invalid diagnostic scope but
does not change an ordinary GSF fit.

For the GSF truth BH-loss oracle, explicitly enable `TruthBHLossOverride`.
Embedded event data is the only supported source; no external input-path or
source-selector property remains. GSF joins through tracker-hit associations;
a distance threshold is used only as an integrity guard after the association,
never to select a truth hit or step.
