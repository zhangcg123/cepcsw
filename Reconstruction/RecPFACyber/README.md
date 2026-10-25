# Passive three-stage ECAL cluster output

`CyberPFAlg` preserves all three stages in the same rec event:

| Stage | Saved cluster collection | Saved hit collection |
|---|---|---|
| `EnergyTimeMatchingAlg` internal `EcalCluster` | `EcalClusterTiming` | `EcalClusterTimingHits` |
| `TrackClusterConnectingAlg` internal `TrkMergedECAL` | `TrkMergedECAL` | `TrkMergedECALHits` |
| Final PFO-associated clusters after `PFOReclusteringAlg` and output conversion | Existing `EcalCluster` | Existing `RecECALBarrel` |

The two passive snapshots use the configured default internal names. The
maintained reconstruction card uses these names and `keep *`, so no card
change is needed to persist the additional collections after installation.

The later `TrackClusterConnectingAlg` and `PFOReclusteringAlg` operate on clones
or replacement PFO clusters. The common snapshot exporter reads each retained
internal collection after the algorithm chain; the existing final output
writer is unchanged. Full-chain versus timing-only and connected-only runs
check that each snapshot is independent of the downstream stages. Cards that
replace these internal collections or change their output names require an
explicit adaptation before relying on the snapshots. A card without a given
stage writes an empty snapshot rather than fabricating its clusters.

Cluster energies and positions are the internal `getLongiE()` and
`getShowerCenter()` values. The export rebuilds the hit list on a private clone
from all stored 2D shower constituents: internal `mergeCluster()` appends these
constituents without refreshing its cached 3D-hit list. Thus the new output
contains all constituent 3D hits rather than the potentially incomplete cache.
No input cluster or existing output is changed. Hit positions and energies
are copied directly from these reconstructed 3D hits. The timing snapshot
precedes the later calorimeter energy correction. The middle snapshot retains
the fragment merging and energy correction already performed by
`TrackClusterConnectingAlg`, but not the final PFO-dependent charged energy
rescaling. The snapshot exporter applies no additional correction to either
clusters or hits; middle cluster energy and raw constituent-hit sum need not
agree because the internal cluster already carries an energy scale.
These are reconstructed
3D hit points, not the original digitized crystal bars or Geant4 deposits.
The hit cell ID uses the existing display convention `(module << 5) + layer`;
it is not a DD4hep cell ID. Hit type stores the internal detector-system code.
Cluster-to-hit links are persisted; track associations are not yet exported
for this visualization-only output, and no new PFO collection is created.

Neither snapshot is a track-independent clustering result: earlier
track-guided axis reconstruction and same-track merging within
`EnergyTimeMatchingAlg` remain; the middle snapshot additionally includes
track-guided fragment absorption. Neither is automatically a validated new
energy measurement for a refitter, and breakpoint input steering is unchanged.
The existing `EcalCluster`, `EcalCore`, `HcalCluster`, hits and PFO outputs are
unchanged. Existing rec files do not gain these collections without rerunning
reconstruction; simulation need not be repeated when digitized inputs survive.

Build without installing into the default runtime:

```bash
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target CrystalCaloRec -j4
```

The integration checker requires Python with uproot, awkward and NumPy. Given
a full-chain output, otherwise identical runs stopped after
`EnergyTimeMatchingAlg` and `TrackClusterConnectingAlg`, and the original rec
reference, run:

```bash
python3 Reconstruction/RecPFACyber/tests/check_timing_cluster_export.py \
  full.root timing_only.root original_rec.root --entries 0,1,3 \
  --connected connected_only.root
```

It checks both snapshots against their stage-local runs, exact existing final
PFA fields against the selected original entries, finite nonnegative energies,
snapshot hit-link targets and absence of duplicate ROOT branches. Omitting
`--connected` preserves the earlier timing-only invocation but skips the
middle-stage identity comparison. These checks do not establish energy
response or electron/photon-separation performance.

To install only this package, without deploying other build-tree development:

```bash
cmake --install build.105.0.0.x86_64-el9-gcc11-opt/Reconstruction/RecPFACyber
```

Stop relevant jobs before replacing shared runtime libraries or stage the
package and atomically deploy its verified files. Keep a rollback backup.

## Event-lifetime regression

`TruthClusteringAlg::ClearAlgorithm()` releases its track and both U/V shower
lists before the owning PODIO input event is destroyed. Keeping those handles
until the next event's initialization can dereference freed PODIO objects,
causing allocator-dependent segmentation faults. The owning `CyberDataCol`
still retains the current event's tracks and published clusters for downstream
algorithms; this cleanup does not remove reconstruction outputs.

Run the focused lifecycle regression in the CEPCSW environment:

```bash
source setup.sh
python3 Reconstruction/RecPFACyber/tests/check_truth_clustering_lifetime.py
```

Pass `--library /absolute/path/to/libCrystalCaloRec.so` to test an isolated
build before installation. The test checks event-track release, empty events
and repeated cleanup. Multi-cluster snapshot ordering is not an identity
contract: compare cluster values with their linked hits, not only vector
indices, when allocation order differs between builds.
