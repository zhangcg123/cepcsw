# Isolation R500/R1000 and angular definition — 2026-11-15

User requests trying500/1000mrad and asks whether the angle is based on
fourmomentafromIP or angular separation at ECAL. Current definition is geometric:
alpha_Cj=acos(r_C·r_j/(|r_C||r_j|)), where r is nominalIP(0,0,0)->reconstructed
ECALclusterCENTRE. It uses ECALpositions but defines the angle atIP. No fitted
orPFOmomenta, localincidencedirections or ECALsurfacearclength is used. If a
masslessneutralp4 is built with this same IPpointing direction, its spatial
direction agrees byconstruction, but such p4 is not an input to isolation.
500mrad=.5rad(28.65deg),1000mrad=1rad(57.30deg); R is conehalf-opening angle.

## Inputs and invariants

Read existingREC ONLYclusterenergy,position andhitcountbranches fromall500
currentREC_ECALTRUTHfiles/100000events. No simulation,reconstruction orrefit.
Candidatekeys identical55391,original3631single-photon>95%labels fixed. Candidate
collectiontheta/phihalf-windows25/200mrad and55.757GeVupper2%energycap unchanged.
Only ISOLATIONneighbourcone expands; allvalidECALneighbours are eligible,
includingcharged electronclusters, without particle/charge filters. Neighbour
energy is not newlytrimmed and is not restricted to the candidatewindow.
Candidateitself omitted. FinitepositiveE/nonemptyhits/validpositions are the
same existing validity requirements.

NewR500/R1000 computed on55391candidates. Everycandidate's oldR300/R400 values
reproduce; I1000<=I500<=I400 for everyrow. Nativefile sizes/mtimes fixed before/
afterall500reads, then independently rechecked. Existingangles,moments,
truthresponseweights,selectedrules andfitoutputs unchanged.

## Descriptive results, all eligible candidates

|Radius(mrad)|Pearsonr|Spearmanrho|Mean abs(I-p)|N(I>=.99)|Mean p atI>=.99|
|---|---:|---:|---:|---:|---:|
|300|.274913|.397462|.334778|909|.081871|
|500|.279248|.399666|.338182|641|.087715|
|1000|.276576|.393122|.353922|198|.092983|

Correlation stays aroundr.28/rho.40 instead of becomingstrong. This is a
descriptive comparison on the same production; no significance claim, new
classifier/cutoptimization,recoilimprovement orphysicsvalidation. NativeMC
response-attribution proxies andselected-Z->ee photon scope limitations remain.

## Verification and display

Artifacts:TrackingPerformanceStudies/neutral_iso_large_radius_20261115/.
read.py obtainsnewcones;analyze.py joins by exactkeys tofixedparentcache;
verify.py independently recomputes acos andneighbourE sums for1/0/8and10/30/36
atbothradii. For10/30/36,otherE is38.849163GeV(R500,7neighbours)and53.535495GeV
(R1000,23neighbours). ScalarPearson agrees; allold8radiusstatistics andold4
profilecounts exact; fullhists close55391/allcleanprofiles3631;8plothashes
match. Three scriptscompile;fourfinalrenders reviewed. The update does not
use/regenerate any charged-electron-excluded metric orcurve.

Addtwo directperclusterpuritymapsR500/R1000 tocurrentcorrelationsection, plus
extendexistingcorrelation-vs-R andcleanfraction-vs-I plots. Newmaps print both
definitions, equalitydiagonal,medianprofile,r/rho;N55391. Updatedcleanfraction
plot includesR25/50/100/300/500/1000,showbinsN>=30withallrawcountsretained.
Source for ALLfournew/updatedpairs is this newstudy directory; previousplot
versions andbeforemanifest/all98imagehashes are archived there.

Expectedgallery75PNGs/17sections/27PDFs;94oldimagehashesunchanged,4filesreplaced,
4newfiles. Onlylatest2sectiondescriptions/patterns change; earlier15objects
unchanged. All user-removed sections/variants stayabsent. Samehealthyserver
localhost8000 ifunchanged; HTTP/source-local-servedhashaudit in
gallery_verification.json. No source/card/build/install/default/production
change. FulloutgoingAGENTS preserved in
2026-11-15-agents-before-isolation-large-radii.md; globalprefix unchanged.
