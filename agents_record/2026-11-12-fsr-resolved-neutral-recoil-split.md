# FSR-only recoil split by resolved neutral FSR (2026-11-12)

Break down the existing `recoil_gsf_muon_fsr_only_shapes_120_140.png` population. Parent sample: 9,458 events in the current three-method-common KF/GSF/BP110 comparison, with FSR >1 GeV on either selected production-electron leg and no material-eBrem leg sum >1 GeV. Softer material radiation is allowed. Preserve the original inclusive FSR-only plot and all other figures.

## Resolution definition

An event is **resolved** if at least one usable, separately reconstructed neutral, trackless pre-reclustering ECAL cluster ANYWHERE has a positive saved MC-weight contribution from generator-linked FSR belonging to either selected recoil-Z electron. Use the existing all-angle native-weight cache with its nonempty-hit, finite-positive-energy and valid-position requirements. No dominance, purity, additional photon-energy threshold or upper-energy cap is imposed. This is an event-level existence flag: it does not require all FSR photons, or the hardest photon, to be resolved. A tiny/soft FSR contribution can tag a mixed cluster.

**No resolved FSR neutral** means no such usable neutral is identified. It includes FSR merged into charged clusters, unreconstructed or invalid clusters and other untagged situations; it is not a proof that all FSR photons merged. A resolved photon outside the 10/200 mrad collection window remains in the resolved category. Record how many tagged neutral clusters are collected separately. Neither definition is based on merely finding any collected neutral.

## Truth links and exact provenance

Current fit and neutral-four-vector caches come from the original REC_TPCFIX campaign. Native saved pre-ECAL cluster MC associations exist in REC_ECALTRUTH, which rebuilt the PFA clusters from the same saved tracker/digitized input. Event clustering can differ. Before transferring a truth presence label, use the existing cluster-identity checker to require exact full-event agreement in cluster energy/position keys, complete constituent-hit multisets (cellID/type/energy/position), and PFO charge/track ownership, allowing collection-order permutations. Compare every cluster in the event, not just those tagged as FSR. Event mismatches are EXCLUDED from both categories and counted explicitly.

Verify old input identity, replay card input identity, current REC/cache identity, and equality of the selected electron track-to-MC mappings. No angular truth matching or nearest-cluster assignment. No simulation, tracking, PFA reconstruction, refit, build, installation or run-card change.

## Spectra

Within each verified group, use identical pre-window event keys for native Baseline KF + neutrals, GSF BestBranch + neutrals and BP110. Reuse their EXACT existing recoil masses, with no new neutral collection or four-momentum changes. BP110 receives no extra neutral addition. Repeat the same inclusive bare muon Baseline KF reference in both figures. Muon black dashed; all electron curves solid in their existing colors. Independent unit area in 120–140 GeV, 0.1 GeV bins; legend N is each curve's raw finite in-window count. No new fits or luminosity scaling.

The two groups partition only the exact-reproducing subset, which can be biased. They do not sum to all 9,458 parent events when ECAL mismatches are present. Final counts and source/cache identities are in summary.json and per-job JSONs. Classification.csv records every parent event, including unclassified mismatches; events.csv also stores the unchanged recoil masses. Histogram.csv, PNG/PDF files and hash manifests provide the plotting artifacts. Existing assert-based checks verify source identities, unique event keys, group partition, histogram closure and unit normalization.

Artifacts: `TrackingPerformanceStudies/fsr_resolved_recoil_20261112/`.

## Final counts and checks

All 496 source pairs checked. Of the 9,458 parent FSR-only events, 8,879 pass exact ECAL reproduction. Exclude 579: 577 differ in cluster energy/position keys and 2 differ in constituent-hit multisets. Do NOT classify these as unresolved.

| Group | Common selected events | KF+neutral window N | GSF+neutral window N | BP110 window N |
|---|---:|---:|---:|---:|
| FSR neutral resolved | 6,899 | 5,077 | 5,069 | 3,932 |
| No resolved FSR neutral | 1,980 | 1,656 | 1,652 | 1,724 |

Resolved 6,899 + unresolved 1,980 + excluded 579 = parent 9,458. The resolved group has 3,851 events with at least one tagged FSR neutral inside the collector, and 3,048 with tagged FSR neutrals only outside. This is a cluster-contribution existence flag, not a photon-count/energy recovery efficiency. Repeated muon count is 44,236 finite entries in 120–140 GeV. Each curve independently sums to one. The groups reproduce the verified inclusive subset histogram in every bin for all three electron methods. Both final figures were visually inspected; source plot hashes and all seven numerical/provenance checks pass.

Original inclusive FSR-only plot remains on the page, using all 9,458 events. The two new figures use only the 8,879 exact-reproduction events and therefore do not partition that original full histogram. The subgroup populations can have different radiation-energy distributions; the shapes are not a causal or pure-resolution comparison. No detector/tracking/PFA implementation, installed library or batch steering changed.

## Gallery publication and memory preservation

Added the two split figures immediately after the preserved inclusive FSR-only figure in the existing matched-category section. Gallery agent verified 23 PNGs / 9 sections / exact-once, all 21 previous PNG hashes unchanged, page and both new PNGs HTTP 200 with source hash matches. Root independently checked exact-once coverage, two source/copy hashes, six figures in that section and the population/observable semantics. Same ailogin002.ihep.ac.cn server PID 2122174 at http://127.0.0.1:8000/, no restart.

Full outgoing AGENTS.md is preserved in `2026-11-12-agents-before-fsr-resolved-recoil-split.md`. Only the current-focus section changed; global status, active laws, work scope and commands remain byte-identical. Analysis scripts, data, figures, caches and gallery manifest remain uncommitted; no C++ implementation or configuration changes.
