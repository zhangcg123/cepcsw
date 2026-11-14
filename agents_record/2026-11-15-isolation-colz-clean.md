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
