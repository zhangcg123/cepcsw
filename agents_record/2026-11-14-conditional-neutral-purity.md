# Conditional FSR/eBrem neutral-cluster purity — 2026-11-14

The user identified that the earlier separate-species plots only decomposed
one common neutral-energy denominator. That was composition, not purity of
FSR-associated and eBrem-associated cluster populations. This study replaces
all 11 separate-species PNG/PDF pairs on the current gallery, retaining the
original source figures and archiving the replaced gallery copies. Combined
FSR+eBrem purity and all recoil plots remain unchanged.

## Definition and sample

For species s and plotted bin b, select S_s(b) from eligible neutral clusters
whose saved target-origin MC fraction f_s is strictly positive. Then
P_s(b) = sum_{i in S_s(b)} E_i*f_s(i) / sum_{i in S_s(b)} E_i.
The full same-cluster energy is the denominator, retaining all contamination,
including the other photon origin and main-electron/other energy. Mixed
clusters enter both associated samples; the two resulting purities are not
additive, and the samples are not an exclusive cluster classification.
This is a truth-conditioned diagnostic, not a reconstruction-only photon tag,
a collection efficiency, an unweighted mean of cluster purities, or a posterior.
No dominant-species threshold is imposed, which would force purity upwards.

Reuses the complete validated metric cache from the current REC_ECALTRUTH
500 sources: 100,000 inputs, 198,081 truth-covered selected-electron tracks,
55,391 baseline / 91,482 wide unique neutral candidates. No ROOT reread or new
BP/GSF tuples are needed. Selected-pair FSR and eBrem origins use the unchanged
saved-MC-link attribution. Baseline half-windows25/200mrad, fixed55.757GeV
neutral upper-tail cap, nearest reconstructed angular assignment for overlaps,
3D cluster/reference distances including depth, and local100mrad ECAL cone
isolation are unchanged. The isolation cone excludes the candidate and both
main charged-cluster sets. Angular maps alone use the wide200/200sample.
Reference-missing clusters remain in other metrics, with a dedicated control.
No fit-success, event radiation category, photon-energy or new reco-metric cut.

## Figures and numerical results

All 11 figures now use species-dependent associated-cluster counts and energy
denominators: four shared-axis profiles, two angular maps, two joint distance/
isolation maps, two cut-retention curves and a charged-reference control.
At least30associated clusters per shown bin/cell/curvepoint; same300Poisson
source-block resamplings give pointwise16/84percentile profile uncertainties.
Retention denominators are each species' already-collected target energy in
the same reference-available associated cohort, never photon birth energy.

| Condition | FSR clusters | FSR purity | eBrem clusters | eBrem purity |
|---|---:|---:|---:|---:|
| Inclusive | 28,501 | 42.95% | 45,050 | 48.13% |
| Charged distance<50mm | 4,295 | 28.67% | 7,752 | 39.00% |
| Charged distance50–150mm | 16,801 | 49.43% | 24,386 | 49.11% |
| Charged distance>=150mm | 4,791 | 62.43% | 6,590 | 62.10% |
| Charged reference available | 25,887 | 48.28% | 38,728 | 49.36% |
| Charged reference missing | 2,614 | 12.29% | 6,322 | 43.70% |

21,359 baseline clusters have both nonzero origins. Conditional denominators
remove all zero-target clusters but retain clusters with small contributions.
That association choice matters. Threshold sensitivity is saved for >0,
>1e-6, >0.001 and >0.01 fraction, without changing the plotted >0 baseline.
At charged distance>=150mm, >1e-6 yields62.53/62.20% FSR/eBrem purity versus
62.43/62.10% at >0. At >0.01, purity becomes74.45/74.46%, while retaining
99.955/99.939% of that far-group target energy. This increase is partly
selection by truth composition, not a reconstruction improvement. Inclusive
>0.01 gives56.17/55.85%, retaining99.902/99.920%target energy.

## Verification and limits

Synthetic denominator self-check distinguishes composition5/12 from conditional
purity5/6. Every profile bin independently matches direct conditional sums;
all profile/map target numerators match the prior composition calculation,
while all new denominators are subsets of their old denominators. All34cut
target-energy retentions match the prior study. Metrics hash
matches the validated parent. Profile and map CSVs save species-dependent
counts, numerator and denominator, including undisplayed distance tails.
Full-code SHA and source hash are in summary.json; validation.json and
association_threshold_sensitivity.csv preserve the checks. All11renders were
reviewed. No reconstruction source/card/library/install/input changes, new
reconstruction fits, hypothesis selection or physics-performance claim.

Artifacts: TrackingPerformanceStudies/neutral_purity_conditional_20261114/.
Prior composition record: agents_record/2026-11-14-neutral-purity-species.md.
Complete outgoing AGENTS snapshot: agents_record/2026-11-14-agents-before-conditional-neutral-purity.md.

## Gallery publication

Replaced exactly11species PNG/PDFpairs in the existing section, renamed
“Conditional purity of FSR/eBrem-associated neutral clusters”. Final81PNGs/
17sections/exact-once,33PDFs,114galleryimages. Archived all22replaced images
byte-for-byte in study/archive_gallery_before; previous composition source
figures also remain untouched. All92unrelated gallery images, all16other
manifest sections, combined-purity/recoil plots remain unchanged. Page and
all11replacement PNGsHTTP200; source/local/served SHA matches. Reused healthy
ailogin002.ihep.ac.cn PID443262 at http://127.0.0.1:8000/ without restart.
Delegated plot-gallery proof: study/gallery_verification.json. Main separately
checked all92unrelated hashes,22archives and replacements,exact-once coverage
and other16section records. Global AGENTS prefix is preserved byte-for-byte.

## Subsequent requested gallery removal

The user explicitly requested removal of “Neutral energy purity versus
reconstructed distance and isolation (8)” from the webpage. Removed only that
manifest section and moved its8PNG/8PDFgallery copies outside the servedtree
to TrackingPerformanceStudies/neutral_purity_metrics_20261113/gallery_removed_20261114/.
Original sourceplots remain. Final gallery73PNGs/16sections,25PDFs; retained
conditional-purity figures, recoil figures and all other98images unchanged.
This supersedes the above81PNG/17section/33PDFcurrent-gallery counts without
changing either analysis or deleting historical evidence. Delegated gallery
verification is saved in that archive directory; no source/card/install change.
