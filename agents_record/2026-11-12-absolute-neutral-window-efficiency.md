# Actual angular collection efficiencies within resolved neutrals, 2026-11-12

User asked for collection efficiency by energy and neutral-cluster count, and
explicitly requested the real50mrad situation. Previous10/50ratios made50mrad
100% by reference normalization and did not answer that request. This study
uses ALL-ANGLE resolved-neutral populations as denominators instead.

Inputs: all500 REC_ECALTRUTH files,100000events. Reuse the previous reader with
one optional all_angles analysis mode and a separate cache under
TrackingPerformanceStudies/pre_ecal_absolute_efficiency_20261112/. Retain all
eligible neutral pre-reclustering ECAL clusters regardless of angular window;
classify direct saved MC links using the same selected-electron generator FSR
and exact G4-birth material-eBrem roots plus MC descendants. Existing window
study files/plots and source REC are not modified by this read-only expansion.

Keep the fixed55.75722694396976GeV cap from the user's prior upper2% trim of
the50mrad pool; do not estimate a different cut on the larger all-angle pool.
Both numerators and denominators have identical finite positive energy,
nonempty-hit and finite nonzero-position requirements. Window numerators use
|deltaPhi|<=200mrad and |deltaTheta|<=10or50mrad around either selected original
KF AtCalorimeter reference. Union clusters once per event.

Energy acceptance: sum(Ecluster*normalized MC category fraction) in window /
same category sum at all angles. Count acceptance: category-dominated neutral
clusters (>50%category MC weight) in window / such clusters at all angles.
FSR/eBrem target categories belong to the selected production electrons. An
ancillary all-neutrals row includes unrelated Higgs activity and is not a
photon recovery efficiency. Count clusters, not individual generator photons.

This is conditional on already reconstructed resolved neutral clusters and
the frozen energy trim. Charged clusters, unreconstructed photons and removed
energy tails are absent from the denominator. MC-weighted energies remain
response proxies, not exact deposits or generated/ECAL-entry photon energy.
Do not call this overall photon detection or total FSR recovery efficiency.

Every original in-window cluster index, energy and MC fraction is cross-checked
against the preceding per-job cache. Full aggregate10/50counts and category
energies must match the trimmed result exactly within rounding. Check valid
association IDs/indices/unique pairs/weights, source identity, category energy
closure and monotonic numerator/denominator counts. The initial2400-event
pilot was only a validation step; final claims use all100000events.

The outgoing complete AGENTS is preserved in
2026-11-12-agents-before-absolute-neutral-efficiency.md. Only current focus
changes; laws and installation remain intact. No history-directory migration.
No native software test/rerun, C++/header/card/library/install or production
job-state change. Analysis scripts/caches/figures remain uncommitted artifacts.

## Completed100000-event result

All500all-angle jobs complete. Exactly500old window caches cross-checked at
individual cluster index/energy/fraction level. Trimmed window counts and MC
category energy sums match previous results. All acceptance ratios obey
0<=epsilon10<=epsilon50<=1; category energy closure and source identities pass.

All-angle eligible neutral denominator:2519885clusters,4098754.181GeV total
cluster energy. FSR proxy denominator275248.113GeV,27714FSR-dominated clusters.
eBrem proxy denominator299081.758GeV,23016eBrem-dominated clusters.

| Target / metric |10mrad|50mrad|
|---|---:|---:|
|FSR MC-weighted neutral energy|49.8504%|60.1861%|
|FSR-dominated neutral-cluster count|35.8880%|49.2639%|
|eBrem MC-weighted neutral energy|88.3677%|90.0845%|
|eBrem-dominated neutral-cluster count|82.3253%|88.2864%|

FSR numerators:137212.321/165661.132GeV (10/50),9946/13653clusters.
eBrem numerators:264291.630/269426.390GeV,18948/20320clusters.
The50mrad window misses39.81%FSR-associated neutral response energy and50.74%
of FSR-dominated neutral clusters, within this trimmed resolved population.
The earlier82.83%/72.85%FSR10-to50retentions were ratios of these acceptances,
not actual50mrad efficiencies. All-neutrals ancillary angular fractions are
15.523/16.702% by energy and1.930/2.465% by count; they include unrelated Higgs
activity and must not be presented as target-photon efficiencies.

Artifacts: summary.json, efficiency_table.csv, all-angle per-job cluster/event
CSVs and source identities, analysis_code_hashes.json, run.log, plot.log,
plots/neutral_window_absolute_efficiency.png and PDF plus plot_files.json.
Figure shows actual10and50values on0-to100%axes; no50=100 normalization.
Main visually checked the figure and verified hashes and full sample size.

Original no-upper-cut window reader was reconstructed and archived under the
preceding study's before_energy_tail_cut/analyze.py, with SHA256 exactly matching
its original recorded hash. The live analysis helper adds only the optional
all-angle mode; its window-mode results are regression-checked above. Both
studies' code-hash manifests are synchronized. No native algorithm change.

Gallery update: plot-gallery dedicated agent added the actual-acceptance figure
as a new first section, keeping prior11figures/4sections intact. Gallery now
validates12PNGs/5sections with exact-once coverage. Homepage and new PNGHTTP200;
PNG SHA256 matches final source, independently rechecked by main. Existing
ailogin002 localhost8000 server PID2122174 updated live, no restart. Manifest
and copied figures remain uncommitted. Earlier energy figures are unchanged.
