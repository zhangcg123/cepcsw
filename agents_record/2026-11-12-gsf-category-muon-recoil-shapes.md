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

## Update: BP110 on all four category plots


Four separate square figures compare electron Baseline KF + neutrals (green dashed), GSF BestBranch + neutrals (category-colored solid), BP110 (red dash-dot), and the maintained inclusive bare muon Baseline KF reference (black solid). Display 120–140 GeV, 0.1 GeV bins. Each curve independently normalizes by its OWN finite entry count in this window; legend N is that raw count, not a luminosity-weighted yield. No fits or luminosity scaling.

## Common sample and categories

All THREE electron methods now use 72,784 IDENTICAL pre-window event keys, requiring both-leg endpoint validity. This adds a BP110 validity gate to the earlier 85,327-event KF/GSF comparison, removing 12,543 pairs; earlier KF/GSF curves therefore change. No BP100 gate. The muon reference remains inclusive (48,636 pairs, 44,236 finite in window, 11 undefined, no FSR veto).

Use the unchanged exclusive >1 GeV truth categories: sum photon birth energy by type per selected production electron, then mark the type present if EITHER electron's sum strictly exceeds 1 GeV. Neither means neither type exceeds threshold; eBrem-only and FSR-only exclude the other type above threshold; Both is AND, possibly on different electrons. Softer radiation can still be present. No truth angular match.

| Category | Common electron pairs | KF+neutral window N | GSF+neutral window N | BP110 window N |
|---|---:|---:|---:|---:|
| neither | 18,078 | 17,197 | 17,097 | 17,288 |
| ebrem only | 30,561 | 26,234 | 25,323 | 26,494 |
| fsr only | 9,458 | 7,187 | 7,176 | 6,041 |
| both | 14,687 | 10,027 | 10,155 | 9,177 |

## Observables and validation

KF and GSF use their native fitted IP states plus the SAME unchanged cached neutral-four-vector union (charge-zero trackless pre-ECAL clusters, 10 mrad theta / 200 mrad phi halfwidth about Baseline KF ECAL references). No new purity/tail cuts or later PFA replay clusters.

BP110 uses native fitted IP states at its truth-selected eligible location or valid null, fitted loss and tracker+beam+ECAL constraints. It is not a location-scan minimum. No EXTRA neutral four-vectors are added to BP110: its ECAL objective already uses the existing measurement sum where eligible.

Cached REC identities, selected track indices, baseline states and truth-cache hashes match. Both prior KF/GSF reconstructed masses close algebraically on all 85,327 parent pairs. BP110 masses reproduce all 11,750 existing no-FSR comparison pairs. All methods' category histograms partition their inclusive spectrum bin-by-bin; each curve sums to one. The muon histogram is unchanged. All four figures were visually reviewed. No native event read, reconstruction/refit, build/install/card or batch change.

The count audit is exact: 98,437 matrix events minus 786 absent-source events, 134 same-KF-charge pairs, 12,190 unavailable GSF pairs, and 12,543 unavailable BP110 pairs equals 72,784 common events. `count_bridge.json/csv` includes each stage by category, plus finite in-window, outside-window and undefined counts. The matrix itself remains unchanged; its denominator precedes fit/window cuts.

These are recoil-spectrum shapes including physics radiation and ISR, not pure detector resolution. BP110 uses truth-location assistance. GSF plus neutral addition may double-count material radiation.

## Files and history

`analyze.py` writes four PNG/PDF pairs, `events_matched_bp110.csv`, `histogram.csv`, `summary.json`, hash manifests and assert-based checks. `count_bridge.py` audits matrix-to-plot counts. The previous three-curve study is preserved under `before_bp110/`; earlier two-curve outputs remain under `before_kf_neutrals_counts/`. Gallery publication preserves all unrelated plots.

BP110 gallery publication: prior four served PNGs and section metadata archived under `before_bp110/gallery/`. New section title is “Matched KF, GSF and BP110 category shapes versus muon Baseline KF.” Gallery agent verified 21 PNGs / 9 sections / exact-once, 17 other image hashes unchanged, page and four updated PNGs HTTP 200 with source hash matches. Root independently verified coverage, four hashes and all count/observable semantics. Same ailogin002 server PID 2122174 at localhost:8000; no restart.

## Illustrating the high-mass tail

User asks how to distinguish FSR/eBrem contributions. Recommend cumulative tail fraction versus recoil threshold, separately by the unchanged >1 GeV category, with the full finite category population as denominator (including masses above 140 GeV). This measures tail propensity; category shares of all tail events separately measure composition. Do not interpret either as causal photon attribution. No new plot or gallery change was requested/performed here.

Direct cached audit of the current 72,784 common endpoint-valid events at M_recoil >130 GeV gives conditional-on-finite tail fractions (KF+neutral / GSF+neutral / BP110): Neither13.7925/13.7868/14.9463%; eBrem-only35.9657/21.1713/26.3664%; FSR-only50.7150/50.1748/66.9345%; Both62.9723/50.6836/65.7880%. All finite masses count, with no upper mass cutoff; the methods exclude their own undefined masses (KF185,GSF786,BP11013) and therefore have slightly different finite denominators. No new reconstruction, fit, or held-out validation.

GSF tail has20,924events: Neither2,483(11.8668%); eBrem-only6,366(30.4244%); FSR-only4,735(22.6295%); Both7,340(35.0793%). FSR-flagged categories thus contain57.7089% of its >130GeV tail, an association, not a fraction proven caused by FSR. Neither permits subthreshold radiation; its residual tail also means the two >1GeV flags do not account for all tail sources.

## Paired bare-to-neutral recovery audit

User says recovery looks ineffective. Direct event-key matched comparison of native bare GSF (`gsf_completed_recoil_pt_20261111/pairs_inclusive.csv`) and the current matched study's `events_matched_bp110.csv`, restricted within each category to events with finite GSF mass BOTH before and after addition:

| Category | Common finite | >130 GeV before | >130 after | 120–130 before | 120–130 after |
|---|---:|---:|---:|---:|---:|
| Neither | 18,010 | 2,528 | 2,483 | 15,383 | 15,164 |
| eBrem only | 30,069 | 9,742 | 6,366 | 19,695 | 20,840 |
| FSR only | 9,437 | 6,937 | 4,735 | 2,490 | 4,545 |
| Both | 14,482 | 11,460 | 7,340 | 2,889 | 6,079 |

In the full 9,458 endpoint-valid FSR-only events, 5,152(54.47%) receive ZERO neutral addition. In the paired-finite subset, 5,150 have zero addition and retain3,306events above130GeV unchanged. Events with additions have3,631tailentries before and1,429after. Thus3,306/4,735(69.82%) of the remaining FSR-only GSF tail receives no neutral correction. Counts concern ANY collected neutral, not truth-confirmed FSR efficiency. Existing collector only adds separately resolved neutral clusters in its window; merged FSR in a charged cluster is absent from a track-only GSF recoil and photons outside the window are also omitted. Assigning those failure causes quantitatively requires further truth-linked matching on the same events; this audit does not do it.

The published category plots compare already-corrected methods and normalize their shapes independently, so they do not directly show bare-to-recovered event migrations. Additional material eBrem in the neutral union can double-count GSF material recovery; merged-FSR tension in BP110 remains a separate hypothesis. No native reconstruction, fit, plot or gallery change was made.

## Styling update: dashed muon reference

User requests the four category figures show muon Baseline KF black dashed and all three electron methods solid, retaining their colors. Only these line styles change; populations, raw histograms, normalization, window counts and bins remain unchanged. Previous figures and study outputs are archived under `before_muon_dashed/`.

Styling verification: histogram CSV bytes and complete numerical summary exactly match `before_muon_dashed/`; representative final plot visually checked. Gallery agent archived prior served figures/section metadata under `before_muon_dashed/gallery/` and verified 21 PNGs / 9 sections / exact-once, all 17 unrelated images unchanged, page and four updated PNGs HTTP 200/source hashes match. Root independently checked coverage, four hashes and description styles. Same ailogin002 PID 2122174 at localhost:8000, no restart.
