# Main-electron MC contribution in neutral energy bars, 2026-11-12

User requested a new plot beside the current bar plot: keep only main-electron
contributions in green, migrate other electron descendants to black Others.
Original plot retained without modification. Analysis/output only; no native
source, installation, card, job, simulation, PFA or tracking changes.

Read saved native EcalClusterPreReclusteringParticleAssociation weights from
the SAME500current REC_ECALTRUTH sources. Exact source size/mtime match the
original study, checked before/after reads. Same100000events/198430selected
recoil-production electron tracks and48634current-window neutral clusters.
No refit-success cut; common selected-track denominator includes zero collection.
Same10mradtheta halfwidth(20total)/200mradphi halfwidth,55.75722694396976GeV
cluster cap, FSR/eBrem positive-fraction cluster tags and overlapping samples.

Green accepts ONLY association MC indices exactly equal to either selected
final stable recoil-electron MC index from the original selection cache;
selected PDG/status verified. No angular match, momentum match or descendant
ancestry accepted in green. Each cluster's direct fraction must be <=oldgreen;
move its remaining oldgreen fraction to Others, retaining all other fractions.
Normalize by same native weight sum; new black Others is original other origin
+unknown+non-main remainder of old electron-associated category.

Original green231625.51906595638GeV, direct main-electron MC label
231567.18898118002GeV=99.9748170732605%. Move58.33008477636031GeV to Others,
0.0251829267395%of oldgreen. Newinclusive main1.1669968703380538GeV/track,
Others0.01600887481291394GeV/track; total3.2064092136989797unchanged.
FSR-tagged green0.5335388969151248,Others0.004318045451797676,total1.6819947344648525.
eBrem-tagged green0.9187607572682969,Others0.008862213174514535,total2.7056449169527754.
AllFSR/eBrem energies, cluster counts and totals unchanged. Rounded green
percentages31.72/33.96/36.40%remain visually the same. Black has no labels.
Legend explicitly Main electron (MC label). A direct MC label can include shower
contributions attributed to that electron; this split does not isolate energy
deposited only by the charged electron itself. No charged clusters included.

Artifacts: TrackingPerformanceStudies/pre_ecal_main_electron_bars_20261112/:
analyze.py,500resumable jobs/*.json,main_electron_cluster_split.csv.gz,
summary.json,validation.json,analysis_code_hashes.json,plot_files.json,
plots/neutral_window_energy_budget_main_electron.png/pdf. Old renderer reused
in memory with strict green fractions and legend; original source files/plots
unchanged. Allsource identities/selected counts,48634unique keys, per-cluster
weight/range checks, migration closure, per-bar closure, unchanged FSR/eBrem
fractions/counts/totals, source/new plot hashes pass; layout viewed.

Gallery published: new PNG direct_pre_ecal_neutral_window_energy_budget_main_electron.png
immediately follows original direct_pre_ecal_neutral_window_energy_budget.png
in the SAME energy section, before the per-event distribution.14PNGs/6sections
exact-once. All13original PNG hashes unchanged. Page and both bar URLs HTTP200,
new copied hashmatches source. Main independently checked all13baseline hashes,
new source hash, category coverage, adjacency and description. Same localhost8000,
ailogin002 PID2122174, no restart. Previous sectionJSON in gallery_before/ in
new study directory; gallery_verification.json records checks.
