# Individual truth and reconstruction neutral-cluster figures, 2026-11-12

User requested splitting the existing two-panel truth plot into two independent
plots, and adding analogous reconstructed neutral-cluster figures with eBrem or
FSR energy contributions. Only analysis artifacts and project memory changed.
No native source, card, installation or job was changed; no Gaudi/PFA/fit rerun.

## Inputs and outputs

Same25current REC_ECALTRUTH seeds,200events each,5000events/9916selected recoil
production-electron tracks. Seeds:
1,22,43,63,84,105,126,147,167,188,209,230,250,271,292,313,334,354,375,396,417,438,458,479,500.
Source file size/mtime identities match exactly between truth and reco readers.
Truth cache unchanged; reconstructed positions and origin-weight/owner maps
are newly read from those existing REC files and compared with all-angle cache.

Truth artifacts: TrackingPerformanceStudies/truth_photon_ecal_centered_5k_20261112/.
Previous two-panel code/plots/notes preserved in before_separate_truth_reco/.
New figures: plots/truth_ecal_fsr_centered_5k.png and truth_ecal_ebrem_centered_5k.png,
with PDF counterparts. Truth pairs and denominators unchanged:
FSR3069/6246=49.1354%,eBrem15728/16169=97.2726% inside the10/200mrad box.
All valid original-photon/electron first G4 ECAL entries at all birth energies,
including photons merged in charged clusters; missing original entries excluded.
Rays start at the common truth electron production vertex. Identity definitions
remain in the prior truth5k record and actual-window revision record.

Reco artifacts: TrackingPerformanceStudies/reco_neutral_ecal_centered_5k_20261112/.
New figures: plots/reco_ecal_fsr_centered_5k.png and reco_ecal_ebrem_centered_5k.png,
with PDF counterparts; neutral_cluster_pairs.csv.gz, summary.json, validation.json,
individual_plot_summary.json, plot_files.json and analysis_code_hashes.json.

## Reconstruction definitions

Select native pre-reclustering neutral, trackless PFO clusters with finite
positive energy, nonempty hits and finite nonzero position, reusing the current
all-angle cache. Preserve the previously fixed55.75722694396976GeV cluster-energy
cap; do not re-estimate the upper2% cut on these5000events.

ANY positive labelled FSR/eBrem contribution qualifies; NO >50% dominance cut,
no minimum photon energy. A cluster may appear in both plots. FSR roots follow
saved generator-history associations to selected production electrons. eBrem
roots match MC birth momentum/vertex exactly to saved G4 eBrem records with a
selected final-electron MC parent. Descendant MC ancestry carries these labels.
The origin fractions exactly reproduce the earlier cached classification.
Mixed FSR/eBrem ancestry remains unknown. Soft material photons without saved
MC roots remain untagged; this is a conditional cluster population.

For each cluster/category, center on the selected electron with largest native
MC weight for that category. Use reconstructed cluster centroid and that
Baseline KF electron's first AtCalorimeter reference point, both relative to
nominal IP. MC information labels origin/owner; plotted positions are reconstructed.
No geometry-based truth matching, no nearest-electron owner assignment.
Count once per cluster/category. Multiple-owner clusters:6FSR,12eBrem; no tied
maxima or ambiguous ancestry-owner clusters excluded in this sample.

5401distinct clusters produce6729category points,1328clusters appear in both.

| Type | Cluster count | Within own10/200mrad box | Count fraction | Actual union over both selected electrons |
|---|---:|---:|---:|---:|
| FSR contribution |2871|1203|41.9018%|1210|
| eBrem contribution |3858|1956|50.6998%|1957|

The on-plot epsilon_geom is the OWNER-centered geometric cluster-count fraction,
using all qualifying points including outside the displayed zoom. The actual
collector union differs slightly as shown. It is not the fraction of photons
found as neutral clusters and not an energy-recovery efficiency. Reco and truth
populations are not one-to-one. Any positive weight can be tiny: minimum
fraction1.4041e-11FSR/2.9649e-14eBrem. This explains why the requested any-contribution
population includes impure neutral clusters far from the emitting electron.

## Layout and checks

Four single-axis square figures; x=deltaTheta,y=wrappedDeltaPhi,mrad;
electron implicitly(0,0), no electron marker. Same zoom theta+/-150,phi+/-400mrad.
Dashed actual tuple window theta+/-10mrad(20TOTAL),phi+/-200. AGORA simulation
bold outside upper-left axes. Count/epsilon inside; no bottom captions or footer.
All four layouts viewed. Original truth pair counts/efficiencies unchanged.

Every target cluster's native category fractions match the all-angle cache;
computed union windows at10/50mrad match cached flags. Positive fractions,
fixed energy cap, unique keys, source identities, same events/selected-track
counts, synthetic theta-axis and phi-wrap checks, plot counts and hashes pass.
No ambiguous/tied centers. Analysis code/artifacts remain uncommitted per law.
Outgoing AGENTS preserved completely in
2026-11-12-agents-before-individual-truth-reco-plots.md; its global laws unchanged.

## Gallery publication

Delegated plot-gallery workflow completed; main agent independently reviewed
source/copied hashes, category coverage and manifest descriptions. Same gallery
TrackingPerformanceStudies/bp_matched_recoil_pt_20261110, localhost8000,
ailogin002.ihep.ac.cn PID2122174, no restart.18PNGs/8sections, each PNG exactly
once. First two sections contain2truth and2reco plots; other14PNGs unchanged.
Page and all4newPNG URLs HTTP200 and exact source hashes. Served names prefix
each source filename with direct_. Previous combined served image and old
section JSON preserved outside served root under truth-study
before_separate_truth_reco/gallery/. No uncategorized old panel remains.
Evidence: reco-study gallery_verification.json and native validation.json.

## Follow-up: neutral energy collection fraction on the two reco figures

User confirmed count interpretation and requested energy efficiency below it.
Reco annotations now explicitly show epsilon_N and epsilon_E. The count
populations and fractions are unchanged. Energy weighting is reconstructed
cluster energy times its normalized native FSR/eBrem MC category fraction,
not the full cluster energy merely because it has a positive contribution.

Sum this category energy within the owning-electron10/200mrad window, divide
by its sum over all eligible neutral clusters, including out-of-view points,
with the same fixed55.75722694396976GeV cap. Conditional neutral-cluster energy
fraction; excludes charged-cluster energy and untagged/missing photons.

FSR:6091.458104858079/13094.24627448688GeV =46.52011255%.
eBrem:12851.551541174085/14532.83724659117GeV =88.43112548%.
Displayed below count41.9%/50.7% as46.5%/88.4%. Same cached5000events, no native
read, PFA or tracking rerun; truth figures unchanged. Independent recomputation
from cached rows matches the earlier saved owner_window_energy_fraction to
1e-12relative tolerance. Counts, keys and plot hashes checked; both layouts viewed.
Previous reco plots/code/notes preserved outside served root in
TrackingPerformanceStudies/reco_neutral_ecal_centered_5k_20261112/before_energy_annotation/.
Gallery update verified by delegated plot-gallery workflow: samehost/PID2122174,
no restart,18PNGs/8sections/exact-once. Page and both updated recoPNG URLs HTTP200,
source hashes match, other16PNGs unchanged. Prior served reco images archived in
before_energy_annotation/gallery/. Main rechecked hashes and category coverage;
clarified that ownership means the unique chosen maximum-weight center, since
small contributions from the other selected electron can coexist. Verification
artifact: reco-study energy_annotation_gallery_verification.json.

## Follow-up: energy purity after epsilon_E on the same line

P_E = within-own-window category MC-attributed energy / total energy of the
SAME category-tagged neutral clusters inside that window. FSR numerator
6091.458104858079GeV,denominator15745.961820363998GeV,P_E38.68584323%.
eBrem12851.551541174085/26075.907083965838GeV,P_E49.28515622%.
Shown as epsilon_E46.5%,P_E38.7% and epsilon_E88.4%,P_E49.3% respectively,
same line as requested. Purity conditions on the plotted positive-contribution
neutral sample; neutrals with ZERO category contribution are excluded. This
is not the purity of the whole blind collection. Same5000cached events/window/
cap, no native rerun. Previous plots/code/notes outside served root under
reco-study before_purity_annotation/. All pre-existing count/energy metrics
unchanged; independent inside-window flag recomputation and plot hashes pass,
both layouts viewed. Delegated gallery publication verified: same localhost8000,
ailogin002 PID2122174, no restart.18PNGs/8sections/exact-once; page and2updated
PNG URLs HTTP200, hashes match sources, other16images unchanged. Main reviewed
copied hashes and manifest coverage/wording. Prior served images archived in
before_purity_annotation/gallery/. Artifact: purity_annotation_gallery_verification.json.

## Gallery cleanup requested after the purity annotation

User requested removing the two sections "Where FSR-dominated neutral clusters
fall outside the wide collection window" and "Truth-linked photon-dominated
neutral clusters vs collection window", two images each. Delegate archive of
the four served PNGs and removed-section metadata outside the gallery; preserve
the source analyses and remaining plots. Target14PNGs/6sections, no uncategorized
leftover previews or obsolete intro references. Publication checks recorded below.

Cleanup complete. Four served PNGs and removed-section JSON preserved under
TrackingPerformanceStudies/.analysis_archive/removed_photon_gallery_sections_20261112/.
Removed files: direct_pre_ecal_fsr_neutral_misses_full.png,
direct_pre_ecal_fsr_neutral_misses_zoom.png,
photon_dominated_neutral_collection_window_full.png,
photon_dominated_neutral_collection_window_zoom.png.
Delegated gallery check:14PNGs/6sections/exact-once, other14image hashes unchanged;
pageHTTP200, both removed titles/four preview filenames absent, remainingPNG200.
Same ailogin002 PID2122174 localhost8000; no restart or analysis rerun. Main
independently checked14/6coverage, archive presence and absent active previews.

## Follow-up: one current-window stacked energy bar

User updates "Direct saved-MC-link pre-ECAL neutral energy: common upper-2%
trim": remove the middle cluster-energy-by-origin plot from the page and
replace the first3-bar window comparison with ONE stacked bar showing energy
components inside the current collection. Keep the third per-event plot.

Cached100000event theta10 summary,48634neutral clusters; actual10mradtheta
halfwidth(20total),200mradphi halfwidth, same55.75722694396976GeV cap. Components
GeVperINPUTevent(includingzero-collection events): FSR1.37212320687654,eBrem
2.642916295763257,other electron descendants2.3162551906595636,other origin
0.03118310944350146,unknown0. Total6.362477802742885. Component sum closes
within1e-12relative tolerance. MC-weight energy proxies, not exact deposits.
Same native source/selection, no event/PFA/tracking rerun. Generated only new
bar PNG/PDF; all unrelated source plot hashes unchanged. One stack position
asserted, layout viewed. Summary/code/previous plots and notes preserved in
TrackingPerformanceStudies/pre_ecal_window_energy_20261112/, with former bar
under before_single_collection_bar/. Delegated gallery update verified:13PNG/
6sections/exact-once, page200 and replacementPNG200/sourcehashmatch, other12
PNGhashes unchanged; removed middle image absent from previews. Old first/
middle served images and prior sectionJSON archived under before_single_collection_bar/gallery/.
Same ailogin002 PID2122174 localhost8000; no restart. Main independently checked
coverage, updated section patterns, removed active PNG and copied source hash.
Artifact: single_collection_bar_gallery_verification.json in source-study directory.

## Follow-up: unknown black; FSR-tagged, eBrem-tagged, inclusive bars

User requested two extra bars before inclusive and unknown in black. Budget
now shows three stacks in order FSR-tagged,eBrem-tagged,Inclusive, all for the
current10/200mrad window and unchanged55.75722694396976GeV cap. Any positive
category fraction tags a cluster; samples overlap, not an exclusive partition.
All five energy components are shown in each bar; unknown black, zero content
in this cache, others unchanged. Same100000INPUTevent denominator in each bar.

Counts25036/40505/48634; total3.337582151598607/5.368811208709392/
6.362477802742885GeVperinputevent. Exact component tables in source-study
tagged_collection_bars.json. Cached collected_clusters.csv only; no REC/PFA/fit
rerun. Inclusive matches previous single bar and theta10summary; tagged bars
retain all their category energy. Fraction/energy/cap boundaries, unique keys,
component closure, three stack positions, black unknown RGBA and plot hashes
checked; layout viewed. Other source plots unchanged. Previous code/plots/notes
under source-study before_tagged_collection_bars/. Gallery verified by delegated
workflow:13PNGs/6sections/exact-once, page and replacementPNG200 with source
hashmatch, other12PNGs unchanged. Same ailogin002 PID2122174 localhost8000,
no restart. Main independently reviewed copied source hash, category coverage
and manifest definitions. Prior served plot and sectionJSON archived under
before_tagged_collection_bars/gallery/. Artifact: tagged_collection_bars_gallery_verification.json.

## Follow-up: merged black Others; neutral energy per selected track

User requested merge Other origin+Unknown as black Others, clarify charged
cluster exclusion in y-axis, and evaluate bars per track. Charged clusters
are NOT included: existing trackless neutral pre-PFO cluster selection kept.
Four display components nowFSR,eBrem,other electron descendants,Others.
All raw five-category energy sums/counts unchanged; merged energy closes.

Common denominator198430selected electron tracks for all3bars, including zero
collection; cached100000width10event rows independently sumselected_tracks
andtracks_with_calo to198430, matching audit. It is total unique event-union
neutral energy divided by all selected tracks, not a category-conditioned
track average. Positive-contribution cluster tags still overlap. Y-axis now
Mean collected neutral energy per track [GeV]. TotalsFSR-tagged1.6819947344648525,
eBrem-tagged2.7056449169527754,Inclusive3.2064092136989797GeV/track; mergedOthers
0.0042889504272453855/0.008731486933902223/0.015714916818778138GeV/track.
No REC/PFA/fit rerun. Source tagged_collection_bars.json records raw+display
energies and denominator. Prior files preserved in before_per_track_others_bars/.
Independent denominator, oldrawenergies/counts, merger closure, black RGBA and
four-component stacks, plot hashes pass; layout viewed. Other source plots
unchanged. Delegated gallery verification:13PNGs/6sections/exact-once, page and
replacementPNGHTTP200/sourcehashmatch, other12PNGs unchanged. Same ailogin002
PID2122174 localhost8000, no restart. Main independently reviewed copied PNG
hash, category coverage and section normalization/neutral-only definitions.
Prior served bar and sectionJSON archived in before_per_track_others_bars/gallery/.
Artifact: per_track_others_gallery_verification.json in source-study directory.
