# All >95%-single-photon neutrals: isolation and electron distance

User request: pick all >95%-pure neutral candidates and inspect reconstructed
isolation and distance to the electron. Select by individual-photon identity,
not summed FSR+eBrem or per-origin sums. No reconstructed cut or held-out split.

## Data and selection

Use the validated `single_photon_purity_20261114/clusters.csv` cache from all
500 current REC_ECALTRUTH files /100000events. SHA256
`02a0169db1512f34b9581494b2dd41052283fdba23a82d6cc735e72686cfc2bc`.
This contains55391eligible unique baseline neutrals. Scope is unchanged:
production Z->ee-associated pre-reclustering charge-zero trackless,
hit-supported clusters; theta/phi half-windows25/200mrad, fixed upper2%energy
cap55.75722694396976GeV. No recoil,fit,eventradiationcategory gate. “All” refers
to that existing collection population, not all detector neutrals.

Require `single_photon_purity >0.95`. Every selected cluster gets >95%of its
full MC-attributed response from ONE identifiable tagged photon and that
photon's descendants. Distinct photons remain separate even within one origin.
No candidate is exactly95%, so all3631keys/fullrows reproduce the previous
>=95%list exactly. Dominant roots:1640FSR,1991eBrem. These belong to3306events
and3327associated electron tracks. Remaining51760clusters form a control.
Native weights remain response-attribution proxies; matching coverage and
selected-Z->ee scope limitations in the parent study still apply.

## Reconstructed quantities

I300=E_candidate/(E_candidate+sum(E_other valid pre-ECAL clusters within angular
radius300mrad)). Includes mainchargedclusters and otherneutral/chargedactivity;
exclude only candidate. Directions are from nominalIP to clusterpositions.

Distance is Euclidean 3D separation between neutralclusterposition and the
associated BaselineKF track's saved ECALentrance. Overlappingwindowassignment
is the existing nearest passing track in openingangle. This distance includes
showerdepth; it is not projected onto a common calorimeter surface. No missing
mainchargedcluster reference excludes a candidate. Charged-cluster distances
are retained in CSV/summary for audit, but the requested plots use trackentry.

## Results on ALL candidates

| Population |Clusters|MedianI300|Mediantrackdistance(mm)|10–90%trackdistance(mm)|
|---|---:|---:|---:|---:|
|SingleFSRphoton>95%|1640|0.3651|207.09|120.55–338.79|
|SingleeBremphoton>95%|1991|0.5022|195.93|111.25–331.59|
|Remaining<=95%neutrals|51760|0.1507|101.30|59.83–207.24|

Allclean combined: medianI300=.4558, mediantrackdistance201.16mm, range
30.815–466.126mm. Entire55391candidate trackdistance range3.230–584.014mm.
The plots cover0–600mm and0–1 isolation, so no hidden overflow or 400mm cutoff.

Of3631cleanclusters,902(24.84%)haveI300>=.7,657(18.09%)distance>=300mm,
and238(6.555%)both. The bothcut splits96FSR/142eBrem. There are66remaining
clusters passing both. This is a full-sample descriptive check, NOT a new
optimization or held-out performance claim. It explains the low efficiency
of earlier cuts. Clean photons tend to be farther/moreisolated than the
control, but neither variable perfectly separates them. A photon cluster can
be internally pure while a nearby electron lowers its inclusive isolation;
isolation near1 is not required by single-photon purity.

## Plots and validation

Artifacts: `TrackingPerformanceStudies/pure_photon_neutral_geometry_20261114/`.
`selected_neutrals.csv` saves all3631fullrows/IDs; histograms and summary are
CSV/JSON. Four separate PNG/PDFpairs: normalized isolation shapes, normalized
trackdistance shapes, FSRclean2Dmap and eBremclean2Dmap. One-dimensional curves
each sum100%by clustercount, no energyweighting. Two-dimensional maps share
binning and logcolour scale, display cleanclusters only; prior300mm/.7cutlines
are guides only and are NOT applied. No inferred origin summation.

Independent checks confirm exact selectedrows against the parenttruthlist,
all500REC sizes/mtimes unchanged, scalar medians, histogramcount closures,
100%shape normalization, full displaycoverage and all8plot hashes. Four final
renders reviewed. Analysis script compiles. No native REC reread/reconstruction,
new cuts/classifier, source/card/build/install change or recoil test.

Gallery adds one four-plot section, retaining all118previous image hashes and
18previous sections unchanged. Expected87PNGs/19sections/39PDFs. All previously
removed sections remain absent. Publication/HTTP/hash audit is saved in the
study's `gallery_verification.json`. Outgoing complete AGENTS focus is preserved
in `2026-11-14-agents-before-pure-neutral-geometry.md`; previous one-photon
definition and reconstructed-search results remain in
`2026-11-14-single-photon-purity.md`.
