# Conditional purity distance profiles with composition panels — 2026-11-14

Added a bottom stacked-bar panel to both existing conditional-purity distance
profiles: distance to main charged cluster and distance to track ECAL entrance.
The top purity points, source bins, associated-cluster selection, target-energy
ratios, and300source-block resampling uncertainties are unchanged. No new ROOT
read, reconstruction fit, reco cut, or truth-association threshold.

For each displayed distance-bin point there are two lower-panel stacks, labelled
F and B: FSR-associated and eBrem-associated clusters, respectively. Each stack
uses exactly the same selected clusters, full energy denominator and distance
bin as its corresponding upper point. Components are saved-MC-attributed FSR
(orange), eBrem(blue), main selected electron(green), Others(black). Each stack
sums to100%; the orange part of F equals the upper FSR purity and the blue part
of B equals the upper eBrem purity. These are full-neutral-energy composition
fractions within each conditional sample, not photon birth-energy efficiencies.
Mixed clusters enter both subsets; neither the two samples nor their purities
are exclusive/additive. F/B bar pairs flank the plotted bin centre, with bar
width and separation proportional to bin width. They describe energy fractions,
not total energy or cluster spatial extent. The last point400mm describes the
300–500mm bin. No additional count/fitvalidity or energy cuts are made.

Reuses the current-REC validated metrics cache:100000inputevents,198081truth-
covered selected tracks,55391baseline unique neutral candidates. Baseline25/200
mrad half-windows and55.757GeVcap unchanged. Distance is3D including depth;
charged distance refers only to the main electron's associated charged cluster,
not the closest other cluster. Each displayed point/bar requires>=30associated
clusters. Both panels share0–500mm x axes and0–100%y axes. Sparse distant bins
are saved to CSV but not displayed.

In the300–500mm charged-distance bin:
- F sample803clusters:FSR65.691%,eBrem30.523%,main2.226%,Others1.560%.
- B sample1050clusters:FSR23.621%,eBrem70.752%,main2.621%,Others3.006%.
The other photon origin is the largest non-target energy attribution there;
this does not by itself explain the physical clustering/link mechanism.

Verification:every bin matches prior species-dependent counts, denominator,
and purity; all stacks sum toone and each target component equals its upper
point. Native metric file SHA matches the parentvalidatedhash. Alloriginal
point values/uncertainties are read unchanged from priorprofiles.csv. Both
renders reviewed. Artifacts:TrackingPerformanceStudies/neutral_purity_distance_bars_20261114/,
includingdistance_composition.csv,plot_files.json,validation.jsonand4PNG/PDFs.
Only the two corresponding gallery imagepairs are replaced; oldcopies archived
outside servedtree. All other figures, including the removed combined-purity
section, remain in their requested state. Analysis source/products uncommitted;
no reconstruction source/cards/build/install changes or performance claims.

## Gallery publication

Replaced only the two distance PNG/PDFpairs in the existing conditional-purity
section. All94other gallery images and15other sections unchanged. Four old
files archived in study/archive_gallery_before/. Final73PNGs/16sections/exact-
once,25PDFs; removed combined-purity section remains absent. Page and both
newPNGsHTTP200;source/local/served SHA matches. Same ailogin002PID443262 at
http://127.0.0.1:8000/, no restart. Delegated proof saved in study/
gallery_verification.json; main independently checked94unchangedhashes,
fourarchives/replacements and visually reviewed both updated renders.
