# ECAL native truth-weight audit — 2026-11-15

## Finding and correction

The saved ECAL cluster-to-MC links are native reconstruction response maps,
not independently established spatial 3D energy-deposit purity. During U/V
projection matching, a parent half-cluster can be split into several spatial
clusters while keeping its original MC-fraction vector. Separately reconstructed
clusters can therefore inherit mixed labels from one common projection.

The earlier 3631/55391 (6.6%) passing fraction is the fraction exceeding 95%
for one identified selected-electron photon under this native map. It is not
an established fraction of physically pure photon clusters. Likewise the
reported isolation correlations and photon-ID cut performance describe the
native-map labels; their interpretation as physical purity remains provisional.
Reproducing native links established correct persistence/reading, not that
stronger physical interpretation. The earlier interpretation was overstated.
This audit does not invalidate generator FSR identities or G4 eBrem births,
and does not establish that every cluster map is wrong.

## Read-only source trace

- Digitization/DigiCalo/src/EcalDigiAlg.cpp:313–324 sums contribution energy
  by MC particle over the whole crystal bar; 575–597 writes the same normalized
  deposit-fraction vector to both digitized end hits.
- Reconstruction/RecPFACyber/src/Tools/CaloHitsCreator.cpp imports the bar map.
- Reconstruction/RecPFACyber/src/Algorithm/EnergyTimeMatchingAlg.cpp:166/181
  and 961/978 scale cloned U-projection bar energies but copy the parent's
  half-cluster MC vector unchanged. V has the analogous branch at 1010.
- Reconstruction/RecPFACyber/src/Objects/Calo3DCluster.cc:314 energy-averages
  these U/V half-cluster maps using getLongiE().
- Reconstruction/RecPFACyber/src/Tools/OutputCreator.cpp:33 persists the
  cached native map unchanged. Snapshot 3D hits are reconstructed U/V
  intersections (EnergyTimeMatchingAlg.cpp:1110), not independent original
  simulation contributions with per-deposit MC ownership. Their snapshot cell
  identifier encodes module/layer, not the original crystal-bar cell identifier.
- Maintained and actual job-10 REC cards enable EnergyTimeMatchingAlg for ECAL;
  TruthClusteringAlg is configured for HCAL only (DoECALClustering=0).

The split/map behavior predates our passive exporter: inspected file history
last changes were 77a37e1 (2025-09-01, EnergyTimeMatchingAlg.cpp) and fe868af
(2025-01-10, Calo3DCluster.cc). No edits to these files were made. This is a
truth-label resolution/interpretation limitation, not a demonstrated new
exporter corruption or a quantified explanation for the entire low-purity rate.

## Focused fresh native-data checks

Inputs: current REC_ECALTRUTH campaign, jobs 1, 3, 10, 27. Seven events, eight
selected clusters: 1/0/8, 1/66/22, 1/66/23, 1/79/9, 1/112/24, 3/4/15,
10/30/36 and 27/48/29 (job/event/cluster, zero-based event/cluster).
Source size/mtime matches prior verified cache and remains unchanged after read.

Independent iterative parent traversal reconstructs each cluster's photon-root
fractions from native MC parents, generator FSR tags and uniquely matched G4
eBrem births before electron ECAL entry. All eight cached fractions reproduce;
no ambiguous-root weight was found in those eight. Collection identities,
indices and finite nonnegative weights pass the focused assertions.

39,740 raw digitized end hits form 19,870 paired crystal cells. Every pair has
identical MC-fraction vectors at both ends. Weight sums span
0.9999892115592957–1.0000082783526523; none outside [0.99,1.01]. Of these focused
cells, 9,042 have multiple MC particles, 950 multiple distinct selected photon
roots, and 1,604 both a selected photon root and direct primary-electron MC
contribution. These are diagnostic events, not representative population rates.

A separate check of all 55,391 prior cached cluster weight sums gives
0.9999930816702545–1.0000040296485868, none outside [0.99,1.01]. This is a cache
normalization check, not a fresh all-500-file truth reread this turn.

## Mathematical counterexample and limits

A and B share a U projection but have separate pure V showers. Candidate A's
split U energy and V energy are each 1. If the copied U-parent map is 50%A/50%B
and V_A is 100%A, the native energy average assigns 75%A/25%B even for a
constructed spatially pure A deposit. With a 10%A/90%B U-parent map it assigns
55%A/45%B. audit.py reproduces this formula in Python with assertions; this is
not a native C++ test, simulation run or measurement of the production bias.

The four examined REC schemas lack EcalBarrelCollection,
EcalBarrelContributionCollection, EcalEndcapsCollection,
EcalEndcapsContributionCollection and
EcalHitsPreReclusteringParticleAssociation. Raw digitized bar-to-MC fractions
alone cannot independently establish where each particle deposited energy
along a mixed bar. The saved synthetic 3D hits do not supply this missing truth.
An independent physical validation needs original simulation contributions
with spatial deposit ownership, on controlled and representative events,
before revising photon-ID/purity claims. No simulation or reconstruction was
run and no source, card, installation, default or production was changed.

## Artifacts and presentation

Uncommitted analysis: TrackingPerformanceStudies/ecal_truth_weight_audit_20261115/
audit.py, audit.json, audit.log, before_gallery.json and before_gallery_manifest.json.
The script includes native-read assertions and controlled formula checks; JSON
records source identities and script SHA256. Gallery metadata correction marks
only the latest two purity/identification sections provisional; image data and
all previous removals stay unchanged. Final verification is in
 gallery_verification.json in that analysis directory.

The complete outgoing AGENTS.md is preserved byte-for-byte in
2026-11-15-agents-before-ecal-truth-weight-audit.md. Only the current focus and
one global truth-interpretation status paragraph change; active laws and
compile/run instructions remain intact. No implementation changes were made.
