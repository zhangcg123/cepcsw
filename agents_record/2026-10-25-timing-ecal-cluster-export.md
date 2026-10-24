# Passive timing-matched ECAL cluster export

## Prominent deployment reminder

**This was a PFA-package C++ source modification, not merely a tuple-analysis
script. Commit `9063895` is committed and pushed on `breakpoint`, but its
`CrystalCaloRec` module is built/tested only and is NOT installed into the
default runtime.** Existing batch jobs therefore do not automatically save
these new products. No installation is authorized by a plot-server request.

The user explicitly requested highlighting and retaining this fact in project
memory. The live warning is in `AGENTS.md` section 2. Its complete preceding
version, including the outgoing cumulative-constraint focus and unchanged
active laws, is preserved in
`2026-10-25-agents-before-timing-ecal-focus.md`.

User requested the internal `EcalCluster` produced by
`EnergyTimeMatchingAlg`, not the final EDM collection with the same old name,
for ten no-eBrem and ten >1% eBrem/no-hit-supported-neutral z-phi examples.
This explicitly authorizes the narrow PFA output change below. No breakpoint,
shared KF, GSF, batch-helper or maintained-card changes were made.

## Implementation and limits

`RecPFACyber` now writes `EcalClusterTiming` and `EcalClusterTimingHits` as
additional passive products. The exporter reads internal `map_CaloCluster`
key `EcalCluster` after the algorithm chain. Later maintained-card stages
clone the objects rather than overwrite this collection; an upstream-only
rerun verifies the timing collection remains unchanged.

Cluster energy/position are the timing result's `getLongiE()` /
`getShowerCenter()`, without later crack correction, track-dependent charged
calibration or PFO energy redistribution. No extra PFO collection is created.
Track associations remain internal and are not yet exported for this
visualization-only product. This is not yet a selectable breakpoint input.

Important finding: `Calo3DCluster::mergeCluster()` appends 2D shower pieces
and longitudinal constituents but does not refresh its cached 3D-hit list.
For example, seed 1/event 3 had a cached hit sum of 0.11277 GeV while its
longitudinal cluster energy was 20.52539 GeV. The exporter refreshes the hit
list on a private clone using existing `setCaloHitsFrom2DCluster()`; its full
constituent hit sum is 20.51572 GeV. It does not repair the original object or
change final PFA output. Seed 1/event 1 similarly changes only the new export
from a 22-hit incomplete cache to 358 constituent hits. Do not interpret the
original cached hit list as a full shower image.

These are reconstructed 3D points from U/V bar matching, not raw crystal bars
or truth deposits. Earlier track-assisted clustering and same-track merging
inside timing matching remain. A single cluster is not proof of complete
electron/photon energy containment. Custom cards that rename or replace the
internal `EcalCluster` require adapting the export contract.

## Validation and deployment

- `CrystalCaloRec` builds in the normal EL9/LCG105 build tree.
- Twenty selected frames were copied from seed 1's existing rec file,
  excluding old PFA/PID products before rerunning. A direct `keep *` rec-on-rec
  smoke was rejected because it creates duplicate old/new ROOT branches.
- Full chain versus a chain stopped just after `EnergyTimeMatchingAlg`:
  **32 timing fields match exactly across all 20 events**.
- New full chain versus the original selected rec entries:
  **199 final-PFA fields match exactly across all 20 events**, including
  existing clusters, reconstructed calorimeter hits, PFOs and relations.
- Finite/nonnegative new energies and unique ROOT branch names pass.
- Reproducible checker:
  `Reconstruction/RecPFACyber/tests/check_timing_cluster_export.py`.

The default installation is **not updated**. Tested build module SHA256:
`c949f1d8f75053d305980fb4a38743a8caf8eac85fe4f21faf9d6ac83659b03d`.
Unchanged installed PFA module SHA256:
`3ba471eecb0b3bc29b1346d67cfa6dee4a41b53c667e103e4ba0c40a214d63ab`.
No batch jobs or simulation were run. Original rec files were not changed.

## Twenty example plots

Directory: `TrackingPerformanceStudies/breakpoint_ecal_timing_hits_20261025/`.
Twenty individual PNGs, two overview sheets, analysis script, selection CSV
and README are uncommitted analysis products. Selection is topology-clear
primary electrons with zero final hit-supported neutral ECAL clusters
anywhere in the event; synthetic hitless neutrals do not veto an event.

- No eBrem, original zero-based entries:
  **0, 1, 4, 8, 10, 11, 18, 20, 21, 22**.
- Truth compound Geant4 eBrem-step momentum loss >1%, entries:
  **3, 26, 38, 41, 48, 54, 57, 60, 69, 78**.
- Each example has one timing cluster. The no-eBrem group contains 9,652
  complete constituent hits; the >1% group contains 12,061.
- Projections are z [mm] versus unwrapped phi [mrad], 5 mm x 2 mrad bins,
  common logarithmic color scale for hit energy per bin; no hit-energy cut
  or event normalization. Red crosses mark cluster centres.

Temporary local inputs, two output ROOTs and logs use the explicit
`/tmp/ecal-timing-*-20261025.*` names documented by the analysis README.
This is an illustrative display, not momentum/theta-matched population
validation, photon ownership validation or a new calibrated energy response.
