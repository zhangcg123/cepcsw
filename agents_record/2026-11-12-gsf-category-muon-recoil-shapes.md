# GSF radiation-category shapes versus muon Baseline KF (2026-11-12)

Replaces the stacked display at the user's request. Four independent square figures each compare GSF BestBranch + the existing neutral recovery with the SAME inclusive muon Baseline KF reference. Display 120–140 GeV, 0.1 GeV bins. Each curve independently divides by its own finite count in this window, so each histogram sums to one. No luminosity scaling, yield annotations or fits.

## Event definitions and provenance

Reuse the 85,327 inclusive valid GSF pairs from `TrackingPerformanceStudies/gsf_recoil_radiation_stack_20261112/events.csv`. Category flags are the unchanged exclusive >1 GeV matrix flags: sum photon birth energy per type per selected production electron, then mark the type present if either leg's sum STRICTLY exceeds 1 GeV. Neither has neither flag; eBrem-only has eBrem and no FSR flag; FSR-only has FSR and no eBrem flag; Both has both flags (AND), possibly on different legs. Soft radiation is allowed in all groups. No BP fit gate or no-FSR veto is added.

The 48,636 muon pairs are loaded from the maintained cached muon recoil table, with pair energy <240 GeV required for physical recoil energy. Eleven entries are undefined. The inclusive muon reference has no FSR veto; 44,236 entries are finite in 120–140 GeV. It is the same curve in all four figures.

Keep the existing electron observable: native GSF BestBranch IP states plus the unique union of neutral four-vectors collected within the existing 10 mrad theta / 200 mrad phi halfwidth about Baseline KF ECAL references. No collection/purity/energy-tail cut is changed and no later PFA replay clusters are substituted.

| GSF category | Available pairs | Finite 120–140 GeV | Undefined |
|---|---:|---:|---:|
| neither | 20009 | 18438 | 298 |
| ebrem only | 36623 | 28752 | 1152 |
| fsr only | 10659 | 7811 | 114 |
| both | 18036 | 11773 | 475 |

## Verification and limits

All four category histograms partition the inclusive GSF histogram in every bin. The first 100 bins reproduce the removed stack's 120–130 GeV raw histograms exactly. Muon first-100-bin raw counts exactly reproduce the maintained baseline (36,918 entries). Each displayed curve sums to one; muon bins and normalization repeat exactly in all four figures. Input hashes are unchanged. All four figures were visually inspected.

This is a cached spectrum comparison, with no native event read, reconstruction, refit, build, install or run-card change. Category sizes are conditional on GSF availability; they differ from the 98,437-event prefit matrix population. Shapes include physics radiation and ISR; they are not pure detector resolution. GSF material recovery plus blind neutral addition can double-count radiation.

## Artifacts

`analyze.py` reproduces four PNG/PDF pairs, `histogram.csv`, `summary.json`, `plot_files.json` and `analysis_code_hashes.json`. Assert-based checks run during plotting. Gallery publication is recorded separately in the durable project record; the removed stack's source data and plot remain archived.

Study: `TrackingPerformanceStudies/gsf_recoil_category_muon_shapes_20261112/`.

## Gallery publication

Replaced the stack section in `TrackingPerformanceStudies/bp_matched_recoil_pt_20261110/gallery.json` with four figures ordered Neither, eBrem-only, FSR-only, Both. Archived served stack and outgoing section JSON outside the gallery at `TrackingPerformanceStudies/gsf_recoil_radiation_stack_20261112/removed_stack/gallery/`; original stack source/data are intact. Gallery agent verified 21 PNGs / 9 sections / exact-once coverage, all 17 other PNG hashes unchanged, old stack absent, page and all four PNG responses HTTP 200 with source hash matches. Root independently checked coverage, four source/copy hashes and stack absence. Same server: ailogin002.ihep.ac.cn, PID 2122174, http://127.0.0.1:8000/, no restart.

Only the current-focus section of AGENTS.md changed. The complete outgoing AGENTS content is preserved in `2026-11-12-agents-before-gsf-category-muon-shapes.md`; global status, laws and essential commands are byte-identical. No native implementation/configuration changed and no reconstruction or fit ran. Generated plots, scripts and gallery manifest stay uncommitted.

## Update: Baseline KF + neutrals and per-curve event counts

User requested a KF + neutral overlay on each category figure and event numbers. Each now contains THREE curves: muon Baseline KF (black solid), electron Baseline KF + neutrals (green dashed), and GSF + neutrals (category-colored solid). Each legend annotates N in 120–140 GeV: the RAW finite entries used as that curve's own shape-normalization denominator, not a luminosity-weighted yield. The inclusive muon count is 44,236 in every figure. The two electron methods use identical category event keys before the recoil window and identical neutral-four-vector sums; no common in-window gate is imposed, so their displayed counts can differ.

| Category | KF + neutrals N (120–140) | GSF + neutrals N (120–140) |
|---|---:|---:|
| neither | 18,609 | 18,438 |
| ebrem only | 30,007 | 28,752 |
| fsr only | 7,846 | 7,811 |
| both | 11,704 | 11,773 |

The existing KF + neutral cache was joined exactly to all 85,327 category events. For EVERY event, both KF + neutrals and GSF + neutrals were recalculated algebraically from their cached native fitted four-vectors plus the SAME cached neutral four-vector sum, then checked against saved masses (including undefined values). Cluster counts and added energies also agree. No native ROOT event read, reconstruction/refit or neutral selection was rerun. GSF and muon histograms and normalization remain unchanged. Independently normalized curves still sum to one. Previous two-curve study outputs and source script are preserved under `before_kf_neutrals_counts/`.

Gallery refresh verified: prior four served PNGs and section metadata archived under the study's `before_kf_neutrals_counts/gallery/`. Still 21 PNGs / 9 sections / exact-once coverage, all 17 unrelated PNG hashes unchanged. Page and four updated images HTTP 200, source/copy hashes match. Same ailogin002 server PID 2122174, localhost:8000, no restart. Root independently checked exact-once coverage, four hashes and description. Old GSF and muon histogram fields match all 200 prior bins exactly. No implementation, card or installed library changed.

## Count audit against the radiation matrix

User noticed matrix/legend numbers differ. Event-by-event audit in the study's `count_bridge.py`, `count_bridge.json`, `count_bridge.csv` confirms ZERO category disagreements on the 85,327 compared events. The >1 GeV matrix uses 98,437 classifiable events in 500 sources, before endpoint/charge/window selections. Four sources absent from the matched 496-source cache (40,55,421,429) account for 786 matrix events; within matched sources, 134 same-baseline-charge pairs and 12,190 pairs without both valid GSF endpoints are removed. Thus 98,437 - 786 - 134 - 12,190 = 85,327. Source availability is the cache's recorded state; no new batch-status inspection was done.

| Category | Matrix | Common KF/GSF before mass window | KF+neutral 120–140 | GSF+neutral 120–140 |
|---|---:|---:|---:|---:|
| Neither | 21,602 | 20,009 | 18,609 | 18,438 |
| eBrem only | 43,739 | 36,623 | 30,007 | 28,752 |
| FSR only | 11,517 | 10,659 | 7,846 | 7,811 |
| Both (AND) | 21,579 | 18,036 | 11,704 | 11,773 |
| Total | 98,437 | 85,327 | 68,166 | 66,774 |

After the common pre-window sample, KF has 1,013 undefined recoil masses and 16,148 finite values outside the window; GSF has 2,039 undefined and 16,514 outside. Each method's three counts close exactly to 85,327. Matrix counts describe category populations; legend N_120–140 describes raw entries in the plotted window and is also each curve's own shape denominator. Shape normalization does not change these raw counts. No reconstruction, refit, plot or gallery change was made in this audit.
