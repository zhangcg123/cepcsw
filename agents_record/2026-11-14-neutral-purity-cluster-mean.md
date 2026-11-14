# Equal-cluster mean purity correction — 2026-11-14

The user clarified the estimator: compute purity per neutral cluster, THEN
average with equal weight per cluster. Previous plotted means used ratios of
energy sums, which weight higher-energy clusters more strongly. Corrected all
13currentlyvisible neutral-purity figures, including both isolation-selection
figures, all11conditional diagnostics, distancecompositionbars, maps, controls
and cut-purity ordinates. Earlier outputs remain historical, and old gallery
versions are archived. No sample definition or reconstruction change.

## Authoritative estimator

For cluster i, p_s(i)=E_s(i)/E_neutral(i), represented by the same native saved
MC-origin fraction. The new plotted purity is mean_s=sum_i p_s(i)/N. Every
cluster in the specified sample counts once, independent of its energy.
The former sum_i(E_i*p_s(i))/sum_i(E_i) is an energy-weighted mean, and is
superseded for these cluster-purity plots. A toy pure4GeVFSRcluster plus a
zero-FSR6GeVcluster now averages50%, rather than40%.

Two sample definitions remain explicitly unchanged:
- Conditional diagnostics select targetfraction>0 separately for FSR/eBrem.
  Mixed clusters can enter both. Their means are NOT additive because the
  populations differ. Same counts28501/45050baseline,21359mixed.
- Reconstructed-isolation selector evaluates all55391eligible neutrals,
  including zero-target clusters. FSR/eBrem means use the same population and
  sum to its combined mean photon purity. No truth filter in this selection.
Changing the weighting does not silently change either population.

In composition bars, each segment is now the arithmetic mean of that origin's
per-cluster fraction. Four component means sum to100%; the target mean equals
its upper purity point. Bars no longer describe energy summed over a bin.
Map cells use sum(cluster fractions)/cluster count. Profile uncertainties are
recomputed with300Poisson source-block resamplings, dividing fraction sums by
cluster counts in each resampling. No energy weights in any purity ordinate.
The three2Dcount-density panels already used per-cluster fractions: densities
are unchanged; overlaid means are corrected and labelled“Mean cluster purity”.

Energy retention remains energy-based:sum(E*p_s)aftercut / baseline sum(E*p_s).
All34conditional cut retentions and all10isolation cut-scan energy/count
retentions are numerically unchanged. Count retentions, all profile/map/cut
counts, isolation values, masks, bins, displaylimits and>=30cluster thresholds
are unchanged. Energy collection-efficiency/budget plots are outside this
purity correction and retain their intended energy normalization.

## Same underlying input and limitations

CurrentREC metriccache:100000inputs,198081truth-covered selected tracks,
55391baseline/91482wide neutral candidates. Baseline25/200mradhalf-windows,
fixed55.757GeVcap, nearestangularassignment and3Ddistances includingdepth
unchanged. Angularmaps alone use wide200/200. Isolation100mradcone excludes
candidate andbothmainchargedclusters; this limitation is unchanged. No ROOT
reread, new fit, isolation redefinition or hypothesis/cut selector. Native MC
weights remain attribution proxies, not exact deposited-energy truth.

## Corrected numerical examples

All-candidate per-cluster means:
| Minimum I | Clusters | FSR mean | eBrem mean | Combined mean |
|---|---:|---:|---:|---:|
|0|55391|20.4845%|34.7334%|55.2179%|
|0.5|37143|23.8922%|32.9804%|56.8726%|
|0.9|30264|25.8980%|30.0925%|55.9906%|
|1|27352|25.9041%|29.0117%|54.9158%|

At I>=0.9 energyretention is unchanged:FSR81.34%,eBrem53.90%,combined63.86%.
Conditional associated-cluster inclusive means:FSR39.8111%,eBrem42.7063%.
At charged distance>=150mm:FSR54.0713%,eBrem46.3196%, counts4791/6590.
These replace earlier energy-weighted purity numbers, not the energy-retention
numbers. Current isolation alone remains weak; no new performance claim.

## Verification and artifacts

Shared arithmetic.py computes unweighted profiles with source-block errors.
Copied analysis paths adapt existing conditional/distance/isolation renderers;
original studies and their source figures remain untouched. Every profile bin
is independently checked with math.fsum(fractions)/cluster count. All map
counts/masks and all cut/control/profile counts match old samples exactly.
Composition means sum toone; each target component equals its upper point.
Synthetic checks distinguish arithmetic and energy-weighted means. Native
metricSHA unchanged, all44retention rows unchanged, allfour scripts compile.
All13finalrenders reviewed. Artifacts:
TrackingPerformanceStudies/neutral_purity_cluster_mean_20261114/.
Subdirectoriesconditional/,distance/,isolation/contain tables,summaries and
validation; publication/plots/contains the13finalPNG/PDFpairs. Shared source
and source/plot hashes retained as uncommitted analysis artifacts. No changes
to reconstruction sources/cards/build/install or production files.

## Gallery publication

Replaced13PNG/PDFpairs in the current gallery; archived all26oldversions outside
the servedtree. Final75PNGs/17sections/exact-once,27PDFs; all76unrelated images
and15othersections unchanged. Only conditional-purity/isolation-selection
section descriptions changed to explain equal-cluster means versus separate
energy retention. Page/all13newPNGsHTTP200; source/local/servedhashes match.
Same ailogin002PID443262/localhost8000, no restart. Delegatedproof saved in
study/gallery_verification.json. Main independently checked all76untouched
imagehashes,26archives/replacements and15unchangedsections; all13renders
reviewed. GlobalAGENTS prefix unchanged; full outgoing focus preserved.
