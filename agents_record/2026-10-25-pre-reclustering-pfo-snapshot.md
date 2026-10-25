# Pre-reclustering PFO persistence — built/tested, not installed

## Request and scope

User approved saving the initial PFOs rather than clusters without their
existing track associations, and explicitly requested preservation tests.
Changes are confined to `Reconstruction/RecPFACyber` plus project records.
No refitter, shared tracking, simulation, batch helper, maintained run card,
PFA clustering/calibration rule or default installation was changed.
No batch was submitted. The standalone local tests rerun reconstruction only.

This is an actual PFA-package C++ change. It does not add these products to old
rec files. Default runtime remains the previous installation until separately
approved for deployment. The current `keep *` rec card already retains them.

## Implementation and data contract

The existing algorithm manager accepts an optional read-only stage observer.
`CyberPFAlg` allocates the new output collections each event and exports after
`TrackClusterConnectingAlg`, before reclustering. The callback is immediately
after algorithm execution and before its temporary-list cleanup. If the stage
is omitted, the collections remain empty. Snapshot failure propagates to the
algorithm instead of silently proceeding with an incomplete snapshot.

- `CyberPFOPreReclustering`: charge, energy, momentum, mass, preliminary charged
  PID and the existing PFO's track/cluster links.
- `EcalClusterPreReclustering`, `EcalHitsPreReclustering`: independent ECAL
  energy/position and complete reconstructed 3D constituent hits.
- `HcalClusterPreReclustering`, `HcalHitsPreReclustering`: independent associated
  HCAL clusters and hits, including original HCAL hit metadata.
- `TracksPreReclustering`: empty for ordinary reconstructed input tracks; only
  used for the existing truth-track fallback conversion when necessary.

Ordinary track links point to the original tracks, not copied tracks. All
actual PFO track links are retained; this does not restore extra cluster track
links discarded upstream when `PFOCreating` selects the first associated track.
Shared cluster/hit objects remain shared between PFOs. The snapshot writes
independent scalars/calorimeter objects, so downstream modifications do not
change saved values. ECAL cached hits are refreshed only on a private clone.

Cluster energy is the internal pre-reclustering `getLongiE()` / `getHitsE()`,
including internal scale but excluding final PFO-dependent output calibration.
Cluster energy is not necessarily the constituent-hit sum. No error or fitted
axis is manufactured. Charged PFO kinematics use the standard leading-track IP
momentum and pion mass 0.139570 GeV. Trackless PFO kinematics use neutral ECAL/
HCAL calibration factors and the energy-weighted cluster direction, with zero
mass. Their internal `m_pid` is uninitialized upstream, so snapshot `type=0`
is deliberately retained rather than reading it. Charged `type` is preliminary
PID, not FinalPID. `CyberPFO`, `CyberPFOPID` and `ParticleID` remain unchanged.

The standard final output creator is untouched. The original timing and merged
snapshots remain present, including their old field definitions. The new ECAL
objects duplicate some merged snapshot payload to provide an independent,
linked pre-PFO graph; consumer migration/deduplication is not part of this task.

## Mechanical validation

Build target: `CrystalCaloRec` in `build.105.0.0.x86_64-el9-gcc11-opt`.
Build succeeded; existing clock-skew/compiler and missing-PCM diagnostics remain.
All six event reruns finalized successfully, 20 events each.
Test workspace: `/tmp/pfa-pre-reclustering.xY0wma/`.

Focused test `tests/check_pre_pfo_snapshot.py` passed using the build runtime:
multiple original track links, shared cluster/hit identity, trackless and
track-only PFOs, source PFO list clearing, source cluster/hit energy mutation,
and empty events. Scalar values and links remain intact after mutation.

Real-event comparison uses `tests/check_pre_pfo_output.py`:

1. Old installed runtime full-chain reference.
2. New build full-chain.
3. New build stopped immediately after `TrackClusterConnectingAlg`.

Two samples were tested (40 rows, **32 distinct seed-1 events**):

- First selected sample: entries
  `0,1,3,4,8,10,11,18,20,21,22,26,38,41,48,54,57,60,69,78`.
  Input `/tmp/ecal-timing-input-20261025.root`, files `reference.root`,
  `full.root`, `connected.root` in the test workspace.
- Second sample: seed-1 entries 0–19 from current
  `rec_large_barrel_20261025/rec-barrel-1.root`, copied with the same pre-PFA
  collection set to `input.root`. Files `neutral-reference.root`,
  `neutral-full.root`, `neutral-connected.root`. This includes the requested
  hard-loss regression entries 11,16,17 and two real trackless ECAL PFOs.

For **each** sample:

- All **159** new snapshot scalar/vector/relation fields exactly match the
  run stopped before reclustering.
- All **206** checked existing final PFA/PID fields exactly match the installed
  old runtime, including `ParticleID` values/parameters and relation indices.
- All snapshot PFO→track, PFO→cluster and cluster→hit links resolve to valid
  collections/indices. PFO track links agree with the stage-stopped standard
  output. Energy conventions and positions agree with it after accounting
  for its explicit output calibration.
- Snapshot ECAL cluster energies, positions and complete constituent hits
  agree with the existing `TrkMergedECAL` snapshot.
- The older comparison additionally passes all 231 final/timing fields and
  canonical merged-cluster/hit contents. In the second sample only, merged
  snapshot vector ordering changes at event 13; physical values/linked hits
  are identical. No final PFO field changes.

| Sample | PFOs | Track-associated | Trackless | ECAL clusters | HCAL clusters | ECAL hits | HCAL hits | Events with changed final PFO count |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| Selected 20 | 21 | 20 | 1 | 20 | 125 | 21,713 | 441 | 8 |
| Entries 0–19 | 24 | 22 | 2 | 23 | 111 | 24,386 | 342 | 5 |

The first sample's one trackless PFO is HCAL-only. The second sample's two
trackless PFOs have real ECAL clusters. Track-only/empty cases are covered by
the focused synthetic test, not claimed from these event samples.

Whole-file size increases: selected sample 54,972,264 → 55,204,053 bytes
(0.422%); second sample 51,458,947 → 51,704,566 bytes (0.477%). These include
all retained input collections and are not a general production size estimate.

Library hashes:

```
built libCrystalCaloRec.so:
4e033497189ea8d02f072292843e25556f030fff93ee0e22c738be705c14f6fd
unchanged installed libCrystalCaloRec.so:
686747f7273dcc1c07d10be98ca121f43875edf2aef8858f81bf794c7ae78254
```

## Limits and next work

This is a persistence regression, not a validation of association accuracy,
energy response, shower separation or electron momentum resolution. Endcap,
truth-track production, large-population/multitrack physics and absence of the
connecting stage have not received real-event tests here. Existing rec files
require reconstruction rerun to contain the new products. Deployment remains
pending; existing breakpoint selectors still consume the old sources.

The requested conditional 111 constraint routing and neutral input selector
are separate pending work. No diffuse constraints or free-loss wrapper were
implemented. Entire prior AGENTS is preserved in
`2026-10-25-agents-before-pre-reclustering-pfo.md`; global laws and commands
are unchanged, only the active focus was replaced.
