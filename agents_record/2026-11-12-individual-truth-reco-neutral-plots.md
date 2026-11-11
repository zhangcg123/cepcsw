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
