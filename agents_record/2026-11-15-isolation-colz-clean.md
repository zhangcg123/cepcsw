# Isolation/purity colour maps: remove text and coefficient displays — 2026-11-15

User names “Does reconstructed isolation measure single-photon purity? (7)”.
Retain only AGORA simulation above every plot panel, no formula/mode/definition
lines, no count/coefficient annotationboxes, no coefficient displays; use only
colour density (COLZ) and the existing white profile to show the relationship.
Six density maps remain. The seventh Pearson/Spearman-versus-radius figure
is removed from the served gallery because its entire content is the unwanted
coefficient display. Prior images and statistics stay archived, not destroyed.

## Retained content and verification

Six figures in original order: inclusiveR50/R300/R500/R1000mrad, then
FSR-leadingR50/eBrem-leadingR50. Exactly the original55391eligible candidates,
same features/isolation and cohort masks. Electron charged clusters always
included in isolation. No data, cuts, truth labels, purity definitions, energy
weights or normalization change; native MC maps remain spatial-purity proxies.
No simulation/reconstruction/refit/nativefile read. White curve is the median
per-cluster photon fraction in each isolation bin with at least30candidates,
unchanged. It is not a regression or purity calibration.

All six40x40colour-count arrays match all9600previous bins EXACTLY. Original
logcolour normalizations/vmin/vmax/cmap/zero masking retained (first4source
maps shareonecolourmax; R500/R1000sharetheiroldmax). All median values match
an independent sorted-value calculation; four oldprofiles reproduce
exactly, and large-radius source hashes match the prior verified study.
Original axes ranges0–1 unchanged. Radius appears in x-axis LABEL, not above
the panel. Axislabels/ticks/colourbar retained; only white median curve and
colourdensity in the panel. Red equalitydiagonal and legends removed.

Per-figure asserts verify one headertext(AGORA simulation), one WHITE curve,
no legend/title/other textboxes, count/display closure. Two representative
renders(R50/R1000)reviewed; all six annotation checks and plot hashes pass;
scriptcompiles. verification.json records source/code/image hashes and checks;
profiles.csv retains allraw profiles/counts including suppressedbins.

## Gallery publication

Source newplots from plots/*.png/pdf; preserve source originals. Snapshot
before_gallery.json covers102PNG/PDFs; before_gallery_manifest.json19sections.
Archive all7oldservedPNG/PDFpairs in previous_gallery/ with hashproof, then
replaceonly6map pairs and remove the coefficientpair fromservedtree. Keep the
sectiontitle/order; targetpatterns6inoriginalorder. Updateonlytargetdescription
to concise map/cohort/whiteprofile explanation and native-MC caveat, without
coefficient names/numbers or claims. All88unrelatedimagehashes and18othersection
objects/order fixed; all previous removals stayabsent. Expected74PNGs19sections/
26PDFs100images. Useoneplot-galleryagent, reusehealthy localhost8000PID443262;
verifylivepage/newPNGhashes/coefficientURL404. gallery_verification.json.

Analysis script/data/figures/manifests remain uncommitted. No implementation,
card/build/install/default/production change. Correlation coefficients have not been recalculated or invalidated; only
their current display is removed.

## Project-memory retention

Complete outgoing AGENTS.md retained in
2026-11-15-agents-before-isolation-colz-clean.md. Only currentfocus changes;
globalstatus, active laws and compile/run instructions remain byte-identical.
All historical coefficient data, removed display and former decorated maps
remain available as archived study artifacts. No source/card/installation
change or new fit/physics validation.

Publication verified:74PNGs19sections26PDFs100images, exact-once. All88unrelated
imagehashes/18othersectionobjects unchanged;14oldtargetfiles archived with
matching pre-updatehashes. All6newPNGHTTP200/source-local-servedhashmatch;
coefficient-summaryURL404. Sameailogin002localhost8000PID443262, no restart.
Main independently rechecked metadata/patterns andallsource/new/unrelatedhashes.
No coefficient names/numbers remain in the targetdescription.

## Description: isolation definition and surface scale — 2026-11-15

User asks for the iso definition and ECALsurface equivalents of50/100mrad
in the SECTIONdescription, not on plot panels. No graphics are regenerated.
The exact appended description is in description_definition_addition.txt.

I_R(C)=E_C/[E_C+sum_{j!=C,alpha_Cj<=R}E_j]. Energies are measured pre-reclustering
ECALcluster energies; alpha is the3Dopening angle between nominal-IP rays to
clustercentres. Coneaxis is candidateC's ray; R is half-openingangle. Include
every valid other charged/neutralcluster, including the electronchargedcluster.
OnlyC omitted. Neighbours needfinitepositiveE,nonemptyhits,finite/nonzero
positions, no newenergytrim and no candidate-window restriction. LargerI means
moreisolated;I=1meansnoother validneighbourenergy in the cone. Sourcechecked:
neutral_photon_id_20261114/read.py usesangles<=radius; large-radius read.py
usescosine>=cos(R/1000), equivalentlyalpha<=R. No electron-excludedvariant.

TDR_o1_v01geometry constant Ecal_barrel_inner_radius=1830mm (dimensionsXML177).
Nearbarrelcentre theta~90deg, small-angle footprint radiusd~1830R/1000mm for
Rmrad. R50=>91.5mmradius/183mmdiameter;R100=>183mmradius/366mmdiameter. These
are entrance-surface approximations, not universal distances: incidence/position
and endcaps change the footprint. Conehalfangle differs from fullwidth and the
BPrectangularwindow. Exact numerical arithmetic/chord/axial comparisons are
saved in isolation_definition_geometry.json. This does not define a fixed-mm
selection, change isolation, or use fitted/PFOmomenta.

The originaltargetdescription is retained verbatim then these two paragraphs
appended. Allsixpatterns/title/order andother18sectionobjects remainfixed.
All100PNG/PDFhashes unchanged;74PNGs19sections26PDFs. Snapshots
before_definition_gallery*.json and definition_gallery_verification.json
record metadata-only publication. Plotheaders remain ONLYAGORA; no metrics
or removed coefficient figure is restored. No newdata/fit/source/installchange.
This adds current description detail to the SAMEfocus; no prior AGENTS text
or historical evidence is removed.

Description publication verified by main: exactoldtext+exactrequestedtwo
paragraphs, other18sectionobjects/title/patterns/order fixed, all100imagehashes
unchanged. Livepage/PNGHTTP200, formulaand50/100mradgeometry/caveats visible.
74PNGs19sections26PDFs; sameailogin002localhost8000PID443262, no restart.

## Bold clarification paragraphs — 2026-11-15

User requests BOTH clarification paragraphs bold. Existing description text
was escaped by the personal gallery renderer. Reuse its EXISTING safe Markdown
renderer in the two description-rendering sites (flat/nested galleries), a
two-statement change in ~/.agents/skills/plot-gallery/scripts/gallery.py. No
CEPCSW/PFA/reconstruction implementation changes. Wrap complete isolation and
ECALsurface paragraphs in Markdownbold; firstdescriptionparagraph remainsplain.
All original words/numbers remainexact after strippingformatmarkers.

Beforeinstall, tests verified actualstrongtags/fullpage, all19visibletexts
unchanged, rawHTMLescaped/unsafeURLschemesrejected, and arithmetic * preserved.
Before/afterpersonalrendererbackups/hashes live in the analysisbold_descriptions/
subdirectory. SourcebeforeSHA43190d71570d3f666f8d4676fbec0b00cbbb3a96ad00e7c6eb52507f16c9d690
matched; installedSHA00f1a892c902958269103427cb86a4a40576a9ea6cc8c9f53b2b3034554576af
verified. Guardedplotweb stop/start replacedPID443262 withPID3994352 on
ailogin002, same localhost8000/gallery. No CEPCjobs/build/install affected.

LiveHTML contains two actual<p><strong>paragraphs, firstparagraphplain, no
literalMarkdownmarkers. Counts74PNGs19sections26PDFs; all100imagehashes and
18othersectionobjects fixed. RetainedPNGHTTP200/hashmatch; priorremovedURLs404.
Main independently checked installedsourceSHA, allimages, metadata andtext
preservation. Proof inbold_descriptions/gallery_verification.json; prepared
source/backups/manifest/preview/self-checks are uncommitted analysisartifacts.
