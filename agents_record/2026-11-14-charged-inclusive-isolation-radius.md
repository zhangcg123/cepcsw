# Charged-inclusive isolation radius and held-out working point — 2026-11-14

User authorized recomputing isolation INCLUDING main charged clusters and
choosing a suitable cone radius. Fresh read-only geometry extraction from all
500currentREC_ECALTRUTH files (100000events); no simulation,tracking,PFA,BP or
GSF rerun. Source paths/sizes/mtimes match validated inputs before/after every
read. Same55391baseline neutral candidates and savedMC-origin fractions as
previous study; no truth-origin,fit-success,event-radiation or mass-window gate.

## Isolation and estimator

For angularradius R, I(R)=E_candidate/[E_candidate+sum(E_other)], where other
means ALL other valid pre-reclustering ECAL clusters with angularseparation
alpha<=R from the candidate's position direction relative to nominalIP.
Include BOTH selected electrons' charged clusters and othercharged/neutral
activity; exclude only the candidate itself. Neighborenergy has no newtrim.
Validity:positivefiniteE,finite nonzero position andnonempty savedhitlist.
Candidate25/200mradhalf-windows andoriginal55.757GeVcap are unchanged.
R is the angularradius, not a full-apex cone width. Radii25,50,75,100,150,200,
300,400mrad. New score depends only on reconstructed cluster energies/positions.
Old main-cluster associations are read solely for native old-isolation A/B
validation and passive main-energy decomposition, never to exclude newneighbors.

Perclusterpurity p_s is the same savedMC-origin fraction E_s/E_cluster. Mean
purity is arithmetic sum(p_s)/N, equal weight per cluster. High-purity label
means p_FSR+p_eBrem>=0.90 for selected Z->ee radiation. This label is used only
to evaluate/train the workingpoint, not to preselect candidates. Energy
retention is summed already-collected target-attributed reconstructed energy,
not photon birthenergy efficiency. NativeMCweights remain attribution proxies.

## Predeclared choice and split

Before examining performance, declared objective: maximize fraction of selected
clusters whose photonpurity>=90%, subject to retaining>=50% EACH of collected
FSR energy and eBrem energy and>=500selected developmentclusters. Scan lower
isolation cuts0..1 in0.01steps. Use250even-sourcejobs(50000inputevents,
27559candidates) for development. Freeze the choice tochoice_development.json
before evaluating250odd-sourcejobs(50000inputevents,27832candidates). Eventlegs
and all its clusters remain on one side; no overlapping(source,event)keys.
Ties favor greater photonenergyretention, then smaller R and lower cut.
Held-out labels never choose the radius/cut. Reruns assert the frozen decision
is unchanged. Held-out threshold curves are diagnostic, not a second tuning.

Fair controls independently tuned on development with the same objective and
retention constraints: old100mrad isolation excluding mainchargedclusters,
and candidateenergyonly. Oldmetric's best cut is0(no selection). Energyonly
chooses E>=23.693385GeV. Selection is not changed after reading held-out results.

## Chosen analysis workingpoint

R=300mrad and I>=0.46. Development highpurityfraction34.760%,meanphotonpurity
68.112%,4859clusters; retainedFSR/eBremenergy51.445/50.327%. Development radius
scan is broad: R200/300/400 giveshighpurityfractions34.42/34.76/34.26% with the
specified retention constraint. R300 is a reasonable workingchoice, not a
sharply determined universal detector optimum. No runcard/default is changed.

Held-out results:
| Selection | Clusters | Fraction p_photon>=90% | Mean cluster photon purity | Retained FSR E | Retained eBrem E |
|---|---:|---:|---:|---:|---:|
|Baseline / oldmetric best|27832|16.495%|55.201%|100.000%|100.000%|
|Energy-only control|4762|30.365%|65.812%|51.428%|50.857%|
|R300,I>=0.46|4929|33.983%|68.422%|50.614%|51.072%|

Chosen point retains17.71%ofcandidatecounts,36.48%ofhigh-purity-clustercounts
and50.91%ofcollectedphotonenergy. Selected1675/4929clusters have>=90%photonpurity.
Source-block300Poisson resamplings givepointwise16/84percentile intervals:
highpurityfraction33.34–34.65%; meanphotonpurity67.91–68.89%; FSRenergyretention
49.75–51.36%,eBrem50.55–51.60%. Paired gains vsenergyonly:highpurityfraction
+3.21–4.01percentagepoints,meanphotonpurity+2.36–2.88points. These are descriptive
sample uncertainty checks, not calibration or broad physics validation.

## Remaining failure mode

At this R, veryhigh I>=0.9 keeps662held-outclusters with only14.06%meanphoton
purity and85.88%meanmain-electron fraction;47/662(7.10%)pass90%photonpurity.
Thus stronger isolation is NOT monotonically better. A main-electron-dominated
neutral candidate can be isolated because its own energy is excluded from the
neighbor sum. This is a truth-attribution observation; the precise upstream
clustering/association mechanism was not debugged or changed. No upperIcut or
new veto is tuned on held-out observations. Such a follow-up needs further
validation. Isolation improves discrimination but does not yield a clean photon
sample by itself, and no recoil-mass benefit is established.

Including charged clusters removes old saturation: onheld-outsample,newI100=1
for1294/27832(4.65%) versus old100mrad metric roughly49%; newI300=1 for404/27832
(1.45%). Removing nineheld-outcandidates with cone-otherenergy>240GeV leavesall
4929selectedclusters and theirpurities unchanged; energyretention changes
negligibly. This is a sensitivity control, not an added selection.

## Verification, figures and preserved state

All55391candidatekeys/originalfields exactlypreserved. Freshnative oldI100 and
oldotherenergy reproduce for everycandidate. At100mrad, newotherE=oldotherE+
mainchargedconeE for everycandidate. I nonincreases with R; extraE nondecreases.
Syntheticmain-inclusion tests pass. Independent scalar acos/fSum nativechecks
cover80cone/candidatecases; independentheld-outcounts, arithmeticmeans and
energy sums reproduce summary. Allfour scripts compile. Source/code/data/plot
hashes and frozen choice,800+cutrows,sourceidentities and bootstrap intervals
are saved. FourfinalPNG/PDFpairs reviewed: developmentradius workingpoints,
held-out perclusterpuritydensity, fixed-radius thresholdtradeoff, and mean
clustercomponentbarcomparison. Currentplots kept; two removedsections remain
offpage. No reconstruction source/card/build/install/input or productionchange.

Artifacts:TrackingPerformanceStudies/neutral_isolation_radius_20261114/.

## Gallery publication

Added fourPNG/PDFpairs as “Charged-inclusive isolation: radius scan and held-out
check”, ordereddevelopment,density,tradeoff,components. Final83PNGs/18sections/
exact-once,35PDFs. All110priorgalleryimages and17priorsections unchanged; both
removedsectionsstayoffpage. Page/all4newPNGsHTTP200;source/local/servedhashes
match. Same ailogin002PID443262/localhost8000, no restart. Delegated proof in
study/gallery_verification.json. Main independentlychecked110oldimagehashes,
17unchangedsections and8newcopies, and reviewed allfourfinalrenders. Fulloutgoing
focus preserved;globalAGENTS prefix unchanged. Only durable docs/status/history
committed; analysiscode/generatedoutputs remain uncommitted; ROOTinputs untouched.

## Direct native explanation of I=1 low-purity cases

User asked why100%isolation can coexist with lowphotonpurity. In the held-out
sample, all404I300=1clusters lack a usable mainchargedECALreference;319have
main-electronfraction>=90%,227>=99%. Meanmain fraction88.897%,photon11.062%.
NativeRECchecks on(job1,event179,cluster24,track6) and(job3,event4,cluster15,
track9) confirm chargedPFO carrying the selected electrontrack but zero ECAL
clusters, plus a separate charge-zero tracklessPFO carrying the candidate ECAL
cluster. Specifically job3/event4:chargedPFO0tracks[9],ECAL[];neutralPFO25tracks[],
ECAL[15]. Cluster15E=44.479984GeV,mainfraction99.986684%,FSR0,eBrem0; no other
validECALcluster within300mrad, so I=1. Its position direction is only0.5626mrad
in theta/1.5979mrad in phi from the trackECALentrance, despite122.11mm3Ddistance
(which includes depth). The electron shower itself is the candidate, excluded
from the cone denominator's neighbor sum. Including charged clusters cannot
supply an absent separate charged shower. Trackless/neutral-PFO classification
therefore does not ensure photon origin. Exact upstream clustering/association
cause is NOT established, and no old PFA/tracking/source was changed. Native
link evidence/counts saved in study/isolated_main_electron_native_checks.json.
This supports checking reconstructed track-cluster association quality before
interpreting veryhighI as a photon tag; no new veto is adopted.
