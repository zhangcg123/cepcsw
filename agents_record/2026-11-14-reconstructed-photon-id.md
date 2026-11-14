# Reconstructed clean-photon identification study — 2026-11-14

User said “lets go” after agreeing the per-neutral single-photon objective.
This authorizes analysis, not an online reconstruction/default change. Target
remains >95%of FULL cluster response from ONE identifiable selected-Z->ee
photon and its descendants. Different photons remain separate even within
one origin. Native response-attribution weights are proxies, not exact deposits.

## Inputs and fresh reconstructed measurements

Same55391eligible unique candidates/current500REC_ECALTRUTHfiles/100000events;
same25/200mrad half-windows and fixed55.757GeVupper2%candidatecap. No fit/recoil
or eventradiationcategory gate. All prior fields and truthlabels preserved
exactly. 3631cleanclusters(1640FSR/1991eBrem),51760remaining. Parent rootmapping
coverage and selected-Z scope limitations remain; not general detectorwideID.

Read existing REC only: no simulation,reconstruction,refit or sourcechanges.
All500sizes/mtimes checked before/after. All savedcluster shapevectors are
empty. Derive moments from41921911 saved `EcalHitsPreReclustering` entries
linked to the candidates. Validate nativecollectionIDs/indices/uniquehitlinks,
nonnegativefinitehitE andfinitepositions. Use normalized hitenergies; the
hitenergysum need not equal calibratedclusterE and is not forced equal.

New features are electronopeningangle from nominalIP rays, transverse/parallel
separation relative to IP->trackECALentry ray, nearestcluster openingangle,
nearestnon-electron-cluster openingangle, associatedchargedcluster separation,
shower transverse/longitudinalRMS aboutenergycentroid, core10/core20fractions,
hitcount,effectivehitcount andlargesthitfraction. Showerlongitudinalaxis is
nominalIP->clusterposition; core20 uses transverse radius20mm throughcentroid.
This avoids treating longitudinalshowerdepth as electronangularseparation.

Compare inclusiveisolation to the same cone excluding reconstructedECALclusters
associated with the selected electrons via native PFO->track links. No photon
truth is used for that exclusion. Initial selectedelectron study context still
uses the truth-linked Z->ee pair; not independent reconstruction selection of
that pair. Missingchargedclusters do not drop candidates. All inclusiveI(R)
reproduce for everycandidate at25/50/75/100/150/200/300/400mrad. Old main-excluded
I100 also reproduces. CandidateEcap never trims neighbourenergy.

## Diagnostic observations

Fullsample clean/remaining medians:

|Variable|Clean3631|Remaining51760|
|---|---:|---:|
|Electronopening(mrad)|62.60|22.47|
|TransverseRMS(mm)|22.64|24.22|
|Core20fraction|0.7994|0.7686|
|InclusiveI50|0.6927|0.2281|
|I50excludingelectronclusters|0.9890|1.0000|

Electronseparation is more discriminating here; width/core alone overlap
strongly. Excluding the electron makes both clean and dirty candidates
frequently isolated. External isolation therefore does not directly certify
internal single-photon dominance. No claim that all possible reconstructed
information is insufficient follows from this finite study.

## Search declared before evaluation

Twentyrecofeatures: energy, fourangulargeometryquantities, fourinclusiveI and
fourmain-excludedI (R25/50/100/300), sevenhitshapefeatures. Thresholds use
development10/25/50/75/90%quantiles, withfixedfractionthresholds. Up tothree
cuts. Beam retains candidate rules ranked byFbeta(.25,.5,1,2), plus purity at
20/50%cleanretention. >=100development and>=50calibration selectedclusters.
No ID,origin,MCweight or truthfraction is a feature.

Same sourcejobmod5split:0/1/2development33029,3calibration11018,4evaluation
11344clusters. Events do not crosssplits. This production was already viewed
in earlier studies; evaluation is reserved from this search, NOT freshblind
physicsvalidation. Plans/searchcode precede aggregate evaluation. Freeze
rules in frozen_rules.json before evaluating. No post-evaluation retuning.

For20/50%calibrationcleanretention floors, maximize Wilson95%lower bound on
fraction individually clean. Separately record anycalibrationrule>=95%clean;
NONE found. Chosenrules BOTH requireopening>=39.604093mrad andcore20>=.7.
20%target additionallyI300>=.7;50%targetI100>=.3. Developmentcleanretention
23.30/50.34%,calibration21.47/51.15%; calibrationcleanfraction54.78/32.21%.

## Evaluation and limitations

|Selection|Selected|Clean|Fraction individually clean|Clean count retained|Clean attributed E retained|
|---|---:|---:|---:|---:|---:|
|None|11344|752|6.63%|100%|100%|
|Previous3Ddistance>=300mm/I300>=.7|61|48|78.69%|6.38%|11.79%|
|New20%target|313|171|54.63%|22.74%|44.05%|
|New50%target|1109|384|34.63%|51.06%|72.04%|

Energyretention=sum(E_cluster*p_single)ofSELECTEDtruthcleanclusters dividedby
same sum overALLtruthcleanclusters in evaluationpool. Not birthphotonenergy
collectionefficiency. Countretention denominator752(341FSR/411eBrem).
20%target FSR73/341=21.41%,eBrem98/411=23.84%;50%target185/341=54.25%,
199/411=48.42%. Old48/61 exactly reproduces the parent study control.

Selectedcountcomposition:20%target73cleanFSR+98cleaneBrem+35uncleanphotonmixture
+107other;noelectron-dominated.50%target185cleanFSR+199cleaneBrem+125mixtures
+15electron-dominated+585other. Electron-dominated means mainresponse>=50%.
Photonmixture requires failingone-photon95% but summeduniquephotonresponse>=95%;
it is a CONTAMINATION label, never certifies purity. These groups partition
the selected candidates; bars represent counts, not energycomponents.

No satisfactory95%-clean selector established. New rules increase retention
with lowercleanfraction; do not call this improved purity or recommend a
default. Finite beam/quantile search cannot exclude better rules. No online
selection, new production, source/card/build/install change or recoiltest.

## Verification and artifacts

Artifactroot `TrackingPerformanceStudies/neutral_photon_id_20261114/` contains
read.py/analyze.py/plot.py/verify.py, sourcejobs andderivedfeatures, frozenrules,
selectedevaluationIDs, native/read/selection/verificationJSONs and rawhists.
All oldfields exact, original55391labels fixed,500sourceidentities unchanged,
projectionPythagoreanclosure andmain-exclusionisolationchecks pass. Independent
scalar nativehitmoments on fourjob1clusters(event0/cl8,79/9,66/23,112/24) agree;
rules re-evaluated independently reproduce evaluationcounts. Histograms close,
shapes sum100%, profiles sum55391/3631,12finalplot hashes match. Fourscripts
compile; sixfinalrenders reviewed. Source/analysis outputs remain uncommitted.

Six newplotpairs: angularseparation,showerwidth,corefraction,individualclean
fraction versusisolation,cleanfraction/retentiontradeoff,selectedcountcomposition.
Featuredistributions andisolationprofiles useALLcandidates; tradeoff andbars
use reserved evaluationevents. No energyweightedmeanpurity ordinate.
Gallery adds oneNEWsection,68PNGs/16sections/20PDFs, preserving76priorimages
and15priorsections. Every previously user-removed section remains absent.
See gallery_verification.json for HTTP/source-local-servedhash/manifestaudit.
Full outgoing AGENTS preserved in
`2026-11-14-agents-before-reconstructed-photon-id.md`.
