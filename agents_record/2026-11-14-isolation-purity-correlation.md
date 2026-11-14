# Isolation definition and direct per-cluster purity comparison — 2026-11-14

User asks to print the isolation definition on plots and test whether isolation
equals, approximates or strongly correlates with per-neutral photon purity.
This is a descriptive analysis and annotation update, not a new selection.

## Definitions and fixed inputs

I_R=E_C/(E_C+sum_j E_j), where j is every OTHER valid pre-reclustering ECAL
cluster with angular separation alpha_Cj<R. Directions are from nominalIP to
cluster CENTRES. Exclude C itself; include both charged andneutralactivity,
including electronclusters withinR. The labelled electron-excluded variant
removes the selectedelectrons' reconstructed track-associated ECALclusters.
R is an angularradius in mrad, not a full cone width. No hit-levelcone is used.

Percluster p_C=max_gamma E_gamma->C/E_C, for ONE identifiable selected-Z->ee
FSR/eBrem photon and that photon's descendants. Different photons never sum.
Full candidate measuredE is the denominator. NativeMCweights provide the
attributedenergy response proxy, not exact deposited-energy measurements.

If S is the dominantphoton contribution INSIDE C,B is othercontribution inside
C,andA is energy in otherclusters withinR, then I=(S+B)/(S+B+A),p=S/(S+B).
No identity equates these ratios. A mergedmixture withA=0 hasI=1 despite lowp;
a purecandidate can have lowI due to nearby externalenergy. Correlation is an
empirical question, and different hit-levelobservables are not tested here.

Reuse `neutral_photon_id_20261114/features.csv` unchanged:55391eligible unique
neutrals fromall500CURRENTREC_ECALTRUTHfiles/100000events,25/200mradhalf-windows,
original55.757GeVupper2%candidatecap. No new RECread/reconstruction. All500size/
mtimeidentities still match; featureSHA256 fixed in summary.json. No rec-cut,
puritycut,eventradiationcategory,fit/recoilgate or train/testsplit. 3631p>.95
labels remain unchanged. Everycluster has equalstatisticalweight.

## Direct equality and correlation results

|Definition, all55391clusters|Pearsonr|Spearmanrho|Mean abs(I-p)|Fraction abs(I-p)<=.05|
|---|---:|---:|---:|---:|
|InclusiveR50|.059011|.175317|.367418|9.142%|
|InclusiveR300|.274913|.397462|.334778|12.015%|
|Electron-excludedR50|.005728|-.028012|.441543|8.245%|

R50FSR-leadingphoton cohort18395: r=.2590,rho=.3863;
eBrem-leading33797:r=.1078,rho=.2064. “Leading” means the largest INDIVIDUAL
TAGGEDphoton contribution, not dominance over the electron or allclusterE.
No puritythreshold in these cohorts;otherorigins maycoexist.3199noidentified-
photon candidates remain in theall-inclusiveplots. Do not call these event
FSR-only/eBrem-only categories or confuse them with1640/1991cleanclusters.

AtI50>=.99,7398clusters have mean perclusterp=.413559 and16.748%individually
p>.95. AtI300>=.99,909clusters have meanp=.081871 and1.100%individuallyclean.
Electron-excludedR50>=.99:32600clusters,meanp=.476056,5.537%clean.
An intermediatepositive trend exists, but spread is broad and the highIendpoint
can be verydirty. Equality and strongmonotoniccorrelation do not hold for this
current external-cluster definition. Do not claim every possible isolation
definition ineffective. Coefficients are descriptive, no independentcluster
pvalue or causal/physicsvalidation claim.

## Actual saved-cluster counterexamples

Keys are sourcejob/zero-basedlocalevent/cluster:

- 10/30/36: measuredE37.907276GeV,I50=1,p=.512389. FSRphotonMC20 contributes
  .512389;eBremMC368 .472594;MC369 .001206. Summedphotonfraction.986189 is
  irrelevant to SINGLEphotonpurity. Externalclusterenergy inR50 iszero.
- 27/48/29: measuredE1.169727GeV,I50=.033298,p=1; externalenergy33.959285GeV.
  MainchargedclusterconeE50 iszero, so do not attribute that activity solely
  to the matched charged electron. These are observed examples, not cartoons.

Bothmaxphotonfractions independently reproduce the saved per-photon contribution
tables. Full metrics/counterexamples are retained in summary.json.

## Plots, annotations, checks and preservation

Artifacts:`TrackingPerformanceStudies/neutral_iso_purity_correlation_20261114/`.
SixnewPNG/PDFpairs: directallR50/allR300,FSR-leadingR50,eBrem-leadingR50,
electron-excludedR50 andcorrelationsversusR25/50/75/100/150/200/300/400.
All display isolation ANDpurity definitions. Directfigures use percluster2D
countmaps, redp=Idiagonal, withinIbinmedian of perclusterp, andr/rho. They do
NOT plot fractionpassing95% as energy-purity. Rawbins/profiles are CSV.

Exactly TWOexisting photon-ID pairs also receive printedisolationdefinitions:
photon_id_isolation_clean_fraction andphoton_id_selection_tradeoff. Their
data and ordinate meanings remain fixed; allfourotherpairs remain byteexact.
Previousversions of the4modifiedimagefiles andoldplotcode are archived under
previous_formula_plots/. No classifier/search rerun or threshold change.

Independent scalar Pearsoncovariance, independently tiedranks/Spearman andMAE
match; actualexamples matchindividualphotonCSVweights; allhistogramcounts close;
12new and12parentplot hashes match. All500RECidentities andfeaturehashfixed.
Sixnew+twoannotatedrenders reviewed; threeanalysis scripts compile. No source,
card,fit,build,install/default,production or recoilchange.

Galleryadds1section6plots and replacesonly2existingpairs:74PNGs/17sections/
26PDFs. Preserve84untouchedoldimages/all16priorsectionobjects; alluser-removed
sections stayabsent. Beforemanifest/all88oldhashes saved; HTTP/source-local-
servedhash audit in gallery_verification.json. Same localhost8000server.
Complete outgoing AGENTS preserved in
`2026-11-14-agents-before-isolation-purity-correlation.md`.
