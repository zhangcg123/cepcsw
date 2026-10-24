# Passive timing-cluster output

`CyberPFAlg` additionally writes `EcalClusterTiming` and
`EcalClusterTimingHits`. They expose the internal `EcalCluster` collection
produced by `EnergyTimeMatchingAlg` with its configured default output name.
The maintained reconstruction card uses this name and `keep *`, so no card
change is needed to persist these additional outputs after rebuilding.

The later `TrackClusterConnectingAlg` and `PFOReclusteringAlg` operate on clones
or replacement PFO clusters. The exporter reads the unchanged upstream
collection after the algorithm chain; it does not export `TrkMergedECAL` or
the final PFO-associated clusters. A full-chain versus timing-only regression
checks this non-mutation contract. Cards that replace the internal collection
or change its output name need an explicit adaptation before relying on this
export.

Cluster energies and positions are the internal `getLongiE()` and
`getShowerCenter()` values. The export rebuilds the hit list on a private clone
from all stored 2D shower constituents: internal `mergeCluster()` appends these
constituents without refreshing its cached 3D-hit list. Thus the new output
contains all constituent 3D hits rather than the potentially incomplete cache.
No input cluster or existing output is changed. Hit positions and energies
are copied directly from these reconstructed 3D hits. No PFO-dependent calibration, track-momentum
rescaling, or additional crack correction is applied. These are reconstructed
3D hit points, not the original digitized crystal bars or Geant4 deposits.
The hit cell ID uses the existing display convention `(module << 5) + layer`;
it is not a DD4hep cell ID. Hit type stores the internal detector-system code.
Cluster-to-hit links are persisted; track associations are not yet exported
for this visualization-only output, and no new PFO collection is created.

This is **not** a track-independent clustering result: earlier track-guided
axis reconstruction and same-track merging within `EnergyTimeMatchingAlg`
remain. It is also not a calibrated new energy measurement for a refitter.
The existing `EcalCluster`, `EcalCore`, `HcalCluster`, hits and PFO outputs are
unchanged. Existing rec files do not gain these collections without rerunning
reconstruction; simulation need not be repeated when digitized inputs survive.

Build without installing into the default runtime:

```bash
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target CrystalCaloRec -j4
```

The integration checker requires Python with uproot, awkward and NumPy. Given
a full-chain output, an otherwise identical run stopped after
`EnergyTimeMatchingAlg`, and the original rec reference, run:

```bash
python3 Reconstruction/RecPFACyber/tests/check_timing_cluster_export.py \
  full.root timing_only.root original_rec.root --entries 0,1,3
```

It checks exact timing fields across the two new runs, exact existing final
PFA fields against the selected original entries, finite nonnegative energies
and absence of duplicate ROOT branches. It does not establish energy-response
or electron/photon-separation performance.
