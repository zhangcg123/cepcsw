# Neutral isolation as a reconstructed selection — 2026-11-14

The user's clarified target is to identify high-purity neutral clusters using
reconstructed isolation. Added two figures evaluating isolation on ALL eligible
neutral candidates. The previous conditional association plots remain valid
truth diagnostics but do not measure the purity of an implementable isolation
selection, because their population itself requires a nonzero target truth
contribution. No such target filter is used in these new selection figures.
No FSR/eBrem event-category, fit-success or charged-reference-availability gate.

## Sample and definitions

Reuse the validated currentREC_ECALTRUTH metric cache:100000inputs,198081truth-
covered selected-electron tracks and55391baseline unique neutral clusters.
Half-windows theta25/phi200mrad and55.757GeVenergycap unchanged. Missing charged
references remain included. No ROOT read, new fits or reconstruction changes.

Current isolation I=E_N/(E_N+E_other) in a100mrad opening-angle cone. Candidate
and BOTH selected electrons' main charged-cluster sets are excluded from E_other.
Other valid charged/neutral ECAL clusters are included. Larger I means less
other cone energy. This metric does not directly reject energy fragments near
the main charged shower; no alternative isolation definition is silently used.

Per-cluster species purity is the saved native-MC-attributed fraction f_s in
that neutral cluster; combined photon purity is f_FSR+f_eBrem. Cluster density
plots use counts, not energy weights. Overlaid profile means use sum(E*f_s)/
sum(E), so they are not unweighted means of the visible cluster-count density.
All zero-target clusters remain. Native weights provide reconstructed-energy
attribution proxies, not exact deposited-energy truth or photon birth energy.

For a reconstructed threshold I>=t, ALL surviving candidates provide the same
full-energy denominator D(t)=sum(E). Numerators are sum(E*f_FSR),sum(E*f_eBrem)
and their sum; therefore these selected-sample energy fractions are additive.
They differ deliberately from the separate truth-associated diagnostic cohorts.
Target-energy retention divides each numerator by its own already-collected
baseline target energy at t=0; it is not relative to all born photons. Count
retention divides surviving neutral count by55391. No selected threshold.

## Figures

1. neutral_cluster_purity_vs_isolation:three count-density panels for per-cluster
FSR,eBrem and combined fractions versus I, with a common logarithmic cluster-
count scale. White lines show energy-weighted means. Every panel includes all
55391clusters, including f_s=0 and I=1. Twenty uniform bins per axis.
2. neutral_isolation_cut_purity_retention:threshold scan top selected-sample
FSR/eBrem/combined energy purity; bottom retained FSR/eBrem/combined energy and
neutral count. These are descriptive curves, without new uncertainty bands.

## Results

| Minimum I | Clusters | FSR share | eBrem share | Combined purity | Photon-energy retention |
|---|---:|---:|---:|---:|---:|
|0|55391|22.97%|40.34%|63.31%|100%|
|0.5|37143|25.63%|38.78%|64.41%|83.90%|
|0.9|30264|29.35%|34.16%|63.51%|63.86%|
|1|27352|30.09%|32.36%|62.45%|53.06%|

At I>=0.9, FSR/eBrem energy retentions are81.34/53.90%. Isolation increases
FSR's share while decreasing eBrem's share; combined purity stays nearlyflat.
Thus THIS isolation alone does not yield a high-purity neutral sample. The
large I=1 population still contains many zero-target and mixed clusters. The
exclusion of main charged clusters is a known limitation, not a proven causal
explanation without a separate same-cohort definition comparison. No cut is
adopted and no recoil improvement is claimed.

## Verification

MetricsSHA matches validated parent. Synthetic check includes a zero-target
cluster in the purity denominator. All3histogramcounts sum to55391. All bin
counts,energy denominators and energy-weighted means reproduce originalcommon-
sample isolation profiles; all cuts bounded and count-monotonic, baselinecut
keeps all candidates. Source/code/plot hashes and summary/CSV/validation saved.
Both final renders reviewed. Existing conditional plots, distancecomposition
panels and all recoil figures are preserved. No source/card/build/install or
production changes. Artifacts:TrackingPerformanceStudies/neutral_isolation_selection_20261114/.

## Gallery publication

Added two PNG/PDFpairs as a new section “Neutral isolation as a reconstructed
selection”. Final75PNGs/17sections/exact-once,27PDFs; all98priorgalleryimages
and16priorsections preserved byte-for-byte. Removed combined-purity section
remains absent. Page/bothnewPNGsHTTP200;source/local/servedhashesmatch. Same
ailogin002PID443262/localhost8000, no restart. Delegatedverification saved in
study/gallery_verification.json. Main independently checked all98oldimagehashes,
16unchangedsections and4newcopies, and reviewed both finalrenders. Global
AGENTS prefix preserved; complete outgoing focus is in the dated snapshot.
