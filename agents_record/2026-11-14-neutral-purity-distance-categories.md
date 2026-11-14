# Cluster-mean distance purity split by event radiation category — 2026-11-14

User requested the existing purity-versus-distance plots for FSR-only,
eBrem-only and Both separately. Added six figures: the three eventcategories
for each existing distance metric (main charged cluster and track ECAL entrance).
Inclusive distance figures remain. No fit, reconstruction or isolation change.

## Event categories and provenance

Use the existing strictly>1GeVdefinition. For each selected production Z->ee
leg, sum truth photon BIRTH energies separately for generator-linked FSR and
material eBrem. A radiation type is present in an event if either selected leg's
SUM exceeds1GeV. This is neither an individual-photon threshold nor a sum across
legs. FSR-only=FSRpresent/eBremabsent; eBrem-only=reverse; Both=AND. Soft other
radiation <=1GeV per leg is allowed in “only”. Both can have the two radiation
types on different electron legs. Definitions unchanged from the current event
matrix and new BP recoil study.

Join current validated neutral metriccache to the fresh20261114native BP event
export using(job,event). Recompute each category from its saved per-leg energy
sums and check labels; every matched metriccluster's assigned track belongs to
the selected pair. Read ONLY truth/category identity from this export: no BP/KF
fit-success,endpoint validity,chi2,baselinecharge or recoil-window requirement.
No deleted old BP ROOT file is read or needed.

## Population accounting

| Category | Complete classified events | Events with neutral candidates | Neutral clusters | FSR-associated | eBrem-associated |
|---|---:|---:|---:|---:|---:|
|FSR-only|11517|5881|7282|6589|2985|
|eBrem-only|43739|16574|23451|4810|22115|
|Both AND|21579|14025|21921|16422|18757|

The metric parent has55391baseline clusters from100000currentRECinputs,
198081truth-covered selected tracks. Requested categories contain52654clusters.
Explicit exclusions:2246Neither clusters and491clusters in383events without
a complete-pair category record. They are not labelledNeither or silently
assigned from one surviving leg. All98437completeclassifiedevent records are
available (Neither21602plus requested76835events), before any fit cut.
Missing charged-reference candidate counts per requestedcategory:357/3927/2371.
Those candidates remain in track-distance plots; charged-distance bins require
a finite main charged reference, as before.

## Per-cluster estimator and display

Within each EVENTcategory, preserve the current conditional CLUSTERsamples:
FSR-associated requires savedFSRfraction>0; eBrem-associated requires savedeBrem
fraction>0. A mixed cluster can enter both. Thus the two curves are not additive
and each may show soft non-leading radiation in an “only” eventcategory.
For each sample/bin, plot sum_i(E_s(i)/E_neutral(i))/N with equalclusterweights.
Full measured neutralclusterenergy is each cluster's puritydenominator. MC
weights attribute that energy; no change to truth linking or energyresponse.

The lower F/B bar pair uses exactly the same samples/bins as the upperpoints.
Each origin's segment is its arithmetic mean clusterfraction; FSRorange,
eBremblue, mainselectedelectrongreen, Othersblack. Stacks sum100%, target
component equals its upperpoint. Bins/0–500mmdisplay,>=30associated-cluster
minimum and300Poisson source-block resampling errors are unchanged. Lastpoint
400mm represents300–500mm, not an exactly400mmsample. Distances are3D including
depth. Baseline25/200mradhalf-windows and55.757GeVneutralcap unchanged.

## Verification and limits

Truthflags checked against all98437event records; selectedtrackidentity checked
for every matched metric row. All category bins match direct arithmetic means
and conditional-parentcountbounds; fourcategory partition+unclassified closes
to parent in every distance/species/bin. Stackunity/targetpoint identities pass.
InputCSV/code/PNG/PDFhashes, event/categorycounts, explicitunclassifiedkeys,
all profile/tail bins and componentmeans saved in summary/validation/CSV files.
Allsixfinalrenders reviewed; script compiles. No ROOT reread, newfit,cuts,
isolationdefinition/source/card/build/install or production change. These are
truth-conditioned diagnostics, not calibrated photon classifiers or recoil
improvement evidence. Native MCweights remain attribution proxies.

Artifacts:TrackingPerformanceStudies/neutral_purity_distance_categories_20261114/.
Sourcecategoryexport:TrackingPerformanceStudies/bp_h0f_theta25_recoil_20261114/events.csv.
Parentmetrics:TrackingPerformanceStudies/neutral_purity_metrics_20261113/metrics.csv.
The new figures are added to a separate gallerysection, retaining all prior
figures and keeping both user-removed sections offpage.

## Gallery publication

Added6PNG/PDFpairs as “Neutral-cluster purity versus distance by event radiation
category”, orderedFSR-only,eBrem-only,Both, eachcharged-distance then track-
distance. Final79PNGs/17sections/exact-once,31PDFs. All98priorgalleryimages and
16priorsections remain unchanged; both removedsectionsstayoffpage. Page/all6
newPNGsHTTP200 andsource/local/servedhashesmatch. Same ailogin002PID443262 at
http://127.0.0.1:8000/, no restart. Delegated proof in study/gallery_verification.json;
main independently checked98oldimagehashes,16oldsections and12newcopies, and
reviewed all6finalrenders. GlobalAGENTS prefix unchanged; outgoingfocus saved.
