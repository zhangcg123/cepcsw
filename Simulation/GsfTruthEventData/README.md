# Embedded GSF truth event data

This PODIO extension stores the Geant4 information needed by default-off GSF
mechanism diagnostics inside the ordinary EDM event. It is not a production
tracking input and does not make the normal GSF truth-dependent.

With `Edm4hepWriterAnaElemTool.WriteGsfTruthEventData=true`, the simulation
writes the following provenance collections. `CompactTruthRecords=true` is
the default; `false` preserves the previous full electron/photon step histories
for direct regression/debug runs.

- `GsfG4MaterialSteps` (`gsftruth::G4MaterialStep`): selected Geant4 pre/post
  checkpoints, including positions, momenta, process subtype, local material
  thickness, energy deposits, and track-length coordinates, plus each selected
  particle's first ECAL-volume entrance;
- `GsfSimTrackerHitG4StepLinks`
  (`gsftruth::SimTrackerHitG4StepLink`): the exact persisted `SimTrackerHit`,
  its first and last contributing Geant4 steps, and its measurement hook.
- `GsfG4BremsPhotons` (`gsftruth::G4BremsPhoton`): explicit photons returned by
  `GetSecondaryInCurrentStep()` for a selected **primary electron/positron**
  parent, with photon creator-process subtype `fBremsstrahlung` and a creation
  position inside the configured tracker bounds. The parent's step-limiting
  process is not used as a photon filter. Each photon retains its exact
  `parentStep`, secondary-list index, creation kinematics and time, and its
  Geant4 track ID once tracking starts (`-1` if it does not start).
- `GsfG4BremsPhotonSteps` (`gsftruth::G4BremsPhotonStep`): the last recorded
  step of each photon, frozen at first ECAL entry. Full mode keeps every step
  up to and including that entrance. Each step links directly to its photon
  and stores the pre/post position, momentum, total energy and time, process
  subtype, statuses, volume copy numbers, path length and energy deposit.
  `postInEcal=1` marks the first ECAL-entry step. If a photon stops before
  ECAL, its final stored step records that termination; if it never reaches
  ECAL, its final tracked step is retained. ECAL entry is identified from the
  Geant4 touchable volume ancestry, not a cylindrical radius approximation.
  Tracking beyond first ECAL entry and daughter-particle trajectories are not
  recorded in this collection.

- `GsfG4MaterialStepSpans` (`gsftruth::G4MaterialStepSpan`): inclusive contiguous
  ranges of omitted non-eBrem steps, their count, and the double-precision sum
  of their original float `stepTX0` values. Exact first/last/hook steps requested
  by all simulated tracker hits, every subtype-3 emission and eBrem-photon
  parent (even with another step limiter), track boundaries,
  first ECAL entry and invalid material values remain full checkpoints. Real
  gaps are never bridged. The reader combines these spans with full hook steps
  to reproduce fractional hook-to-hook material and eBrem integrals, including
  selections that skip hits. Floating-point regrouping of t/X0 sums can change
  the last rounding bits. Full mode writes an empty span collection.
- `GsfG4FsrPhotons` and `GsfG4FsrPhotonSteps`: the same types and checkpoint
  format as the eBrem photon collections, in separate collections.
- `GsfGeneratorFsrLinks` (`gsftruth::GeneratorPhotonLink`): each FSR photon's
  original `MCParticleGen` index, direct emitting electron-copy index, final
  stable electron index, status and exact photon relation.

FSR identification uses only generator PDG, status and parent relations. Starting
from each generator-status-1 electron/positron, walk backwards through copies
of the same signed PDG, stopping at non-electron production nodes. A generator-
status-1 photon with a direct parent on that outgoing chain is tagged. This does
not cross the production node into the incoming beam chain, and excludes Geant4
photons (generator status 0). There is no energy or opening-angle cut. All final
electrons are considered; downstream analyses may select the production legs.
Status 1 means a unique emitter and final electron; status 2 records ambiguity
instead of guessing. Ambiguous indices are -1. The photon is followed through
Geant4 using the primary converter's exact generator-index-to-G4-track map.

For FSR, `creatorProcessSubtype=0`, `parentStepNumber=-1`,
`secondaryIndexInStep=-1`, and the material `parentStep` relation is absent.
`parentTrackID` identifies the final electron's G4 track when available (-1
otherwise), rather than a fictitious material emission. Birth kinematics come
from the generator; entry/termination kinematics come from Geant4. Neither
format records conversion-descendant shower contributions or assigns ECAL
clusters to a photon.

The original datatype layouts remain unchanged. New readers accept old files
without these companion collections. Compact files require span-aware readers
and pass-through of all companion collections; do not discard spans while
keeping compact material checkpoints. The maintained breakpoint cards discover
the optional collections from input metadata and retain them through tracker,
calorimeter and REC stages. Full transient Geant4 histories are still buffered
inside the writer; this change reduces persisted output, not simulation RAM.

## Selected-particle ECAL entrances (schema 2)

`GsfG4MaterialSteps.firstEcalEntry=1` identifies the first Geant4 step from
outside ECAL to a volume with an ECAL touchable ancestor. Its `postPosition`
is the true entrance location in mm and its `postMomentum` is the entrance
momentum in GeV. `trackID`, `parentID`, `pdg`, `stepNumber`, `postKineticEnergy`
and `postGlobalTime` retain their ordinary meanings. This uses the same ECAL
volume test as the photon recorder, not the nominal tracker radius/z cut.

The feature follows `WriteGsfTruthEventData`, `GsfTruthPDGs` and
`GsfTruthPrimaryOnly`; it is not restricted to electrons or one particle per
event. The maintained selection includes primary e-/e+ and mu-/mu+. Each
track can have at most one first-entry flag. A particle born inside ECAL has
no outside-to-inside entrance, and a particle which never enters ECAL has no
flagged record. IDs are local to the event. There is no extrapolated entrance
or invented zero-valued position for such particles.

`ecalEntryOnly=1` means that this boundary step was outside the configured
material-step selection and was retained solely for its entrance information.
It must **not** be included in tracker material/loss/ownership calculations.
The shared `TruthBHLossEventData` reader excludes these diagnostic records.
If the boundary step already passed the material selection, it is stored
once with `firstEcalEntry=1, ecalEntryOnly=0`. Persisted checkpoints retain
their physical values. In full mode, select `ecalEntryOnly==0` to recover the
former material-step population. Compact mode reproduces the supported
integrals/checkpoints, not every discarded step. Additional records can shift
collection indices: follow the persisted relations, not assumed row numbers.

This adds at most one step per selected particle, rather than recording its
whole ECAL shower. The existing `keep *` workflow preserves the fields from
sim through rec without new collection controls. Existing files cannot gain
missing entrance truth by reconstruction alone. The breakpoint flat tupliser
also exports `truth_ecal_entry_*` position, momentum, identity and status for
each track's uniquely matched truth particle; see the RecBreakpoint README.

Old schema-1 ROOT files remain readable in the tested native PODIO reader;
the added flags default to zero. In an old file, zero means the entrance was
not recorded, not proof that the particle failed to reach ECAL. Rebuild the
data model/dictionary and their consumers together; do not mix old generated
headers with the schema-2 libraries. The installed runtime is not updated by
a source-only build.

Focused validation and deployment state are recorded in
`agents_record/2026-10-26-selected-particle-ecal-entrance.md`. The
flat-tuple validation is in
`agents_record/2026-10-26-breakpoint-truth-ecal-entry-flat.md`, and the installed
runtime/full-chain defaults are recorded in
`agents_record/2026-10-26-truth-ecal-full-chain-deployment.md`. The reusable
`tests/check_ecal_entries.py NEW.root [OLD.root]` checks first-entry uniqueness,
finite coordinates/momenta, photon-parent relations and optional same-seed
preservation of the old material and photon data.

The Geant4 photon ID is not reliable at secondary creation, so the recorder
temporarily associates the secondary `G4Track` pointer with its birth record
and fills the ID at `PreUserTrackingAction`. The pointer is never persisted.
This all-step collection costs more space than birth-only truth: a same-seed
20-event 2 GeV barrel test stored 44 photons and 11,532 pre-ECAL photon steps,
adding about 647 kB (11.9%) to the compressed ROOT file compared with the
birth-only version. This is a sample-specific size measurement, not a general
per-event estimate.

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

All four collections are written into the same `sim*.root` event and survive the
normal `keep *` chain. ROOT dictionary PCM/rootmap files are installed beside
the generated libraries so generic PODIO readers can deserialize them.
Photons below the generic `MCParticle` secondary threshold are included; do
not assume each photon has a corresponding stored `MCParticle`. The photon
collection is empty when no selected eBrem step emits a photon.
Only explicit Geant4 secondary photons are available: energy loss below the
Geant4 photon-production cut is not converted into synthetic photon records.

The ordinary secondary `MCParticle` writer has a separate `SecTrackEk`
property, in MeV. Its compiled default is now `10`; the local maintained
`DumpGsfTrks/sim.py.bk` also sets it explicitly to `10.0` for batch steering.
This does not change which Geant4 secondaries are produced or which photons
enter `GsfG4BremsPhotons`. The ordinary writer still applies its configured
spatial bounds, except for its pre-existing process exceptions. A same-seed
20-event smoke increased ordinary
`MCParticle` entries from 32 at 100 MeV to 59 at 10 MeV, while preserving all
44 dedicated bremsstrahlung-photon records and 11,532 photon steps. A zero-cut
control stored 766 ordinary entries, mostly very-low-energy electrons.

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
