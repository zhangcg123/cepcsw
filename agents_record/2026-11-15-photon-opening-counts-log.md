# Retained photon-ID opening angle: counts and logarithmic y — 2026-11-15

Current presentation uses **3D distance in mm, 10 mm bins and log-y raw counts**.
The former mrad presentation below is preserved history; see the distance update.

User names the “Reconstructed photon identification: shower shape, local activity
and efficiency (6)” section. Keep its FIRST plot photon_id_track_opening.png,
remove its other FIVE figures from the webpage, and change the remaining three
histograms to a common count scale with logarithmic y. No independently
normalized distributions remain in that section.

## Data and plot

Reuse the same55391eligible unique neutral clusters from current500RECfiles/
100000inputs in neutral_photon_id_20261114/features.csv. No native-file reread,
simulation, reconstruction, fitting, physics classification or selection change.
Candidate25/200mrad half-windows/fixed55.757GeVenergycap stay unchanged.
Opening angle is between nominal-IP rays to the candidate ECALcluster centre
and its associated BaselineKF track ECALentrance, as previously saved.

Same three disjoint populations:51760other neutrals,1640FSR and1991eBrem
candidates with native fraction >95%from ONE identified taggedZ-electron photon
and its descendants. Distinct photons and origins never combine for this test.
The labels remain native-response fractions, not independently established
spatial deposited-energy purity. The prior audit did not demonstrate bad links.

All3histograms use unweighted raw counts in exactly the original40common
5mrad bins over0–200mrad. Every one of120counts ANDboth edges of everybin
reproduce the oldfeature_histograms.csv exactly. No own-integral/percentage
normalization, bin-width rescaling or new relative factor. Y-axis is logarithmic,
labelled Neutral clusters /5mrad, lowerlimit.7soonecount remains visible. Zero
bins are omitted only from rendering and retained as exact zeros in histogram.csv.
AGORA simulation, squareaxes, translucentgrid, population legends and the full
original x-range retained. Bothoutputhashes and sourcehashes in verification.json;
plot.pycompiles and finalrenderreviewed. Originalsourcefigures and code stay
unchanged; this script is the maintained presentation override for the one
retained figure. No electron-excluded isolation metric is used or restored.

## Gallery scope

Use the plot-gallery skill through one dedicated agent. Preserve the current
20sectiontitles/order; change only the named section's description/patterns to
this onePNG. Archive its5removedPNG/PDFpairs ANDoldfirstPNG/PDF pair outside
servedtree in this study's previous_gallery/, recording hashes. Sourceplots
remain untouched. All100unrelatedoldPNG/PDFhashes and19othersectionobjects
unchanged. Starting80PNGs/20sections/32PDFs/112images; expected75PNGs/20sections/
27PDFs/102images. No formerly removed section/variant restored. Reusehealthy
localhost8000serverPID443262, verifysectioncontainsonepreview/HTTP/source-local-
servedPNGhash and expectedremovedURLs404. Checksin gallery_verification.json.

Artifacts:plot.py,plot.log,histogram.csv,verification.json,before_gallery.json,
before_gallery_manifest.json,plots/*.png/pdf andprevious_gallery/. Keepanalysis
artifacts uncommitted. No source/card/build/install/default/production changes.

## Project-memory retention

Outgoing AGENTS.md is retained byte-for-byte in
2026-11-15-agents-before-photon-opening-counts-log.md. Only currentfocus changes;
all globalstatus/laws/compilecommands remain identical. Historical feature/cut
search figures and sources remain archived and interpretable, not current
webpage content. No cut-search/selection/reconstruction rerun is involved.

Local publication reviewed by main:75PNGs/20sections/27PDFs/102images. The
targetsection keeps ONLYphoton_id_track_opening.png. All100unrelatedPNG/PDF
hashes and19othersectionobjects/order remain unchanged; both newsource/gallery
hashes match. The12oldservedtargetfiles are archived outside servedtree.
HostHTTP/archivehashverification is recorded in gallery_verification.json.

## Distance in millimetres — 2026-11-15

User requests x-axis inmm. Current maintainedplot.py now uses the saved
track_distance_mm = norm(reconstructed neutral-cluster centre - associated
BaselineKF AtCalorimeter referencePoint). This is a3D Euclidean distance,
including the shower-depth offset, not a surface arc or angular relabelling.
The original feature reader directly checked this norm against native REC.
For all55391rows, distance²=perpendicular²+depthoffset² closes to tolerance.
All3populations, strict one-photon>95%labels and counts remain identical.

Current plot uses60common10mm bins0–600mm (observed3.2296–584.0138mm), raw
unitweight counts and logarithmic yfloor.7. Each bin reproduces an independent
integer-bin count; totalall55391 and51760/1640/1991subgroups close. Zero bins
remain0in histogram.csv but are omitted from log rendering. Labels explicitly
state3Ddistance[mm] and Neutralclusters/10mm. Finalrender/hash/compile checks
pass. Prior mradsource/render/data/checks are preserved in before_distance/
with hashes; before_distance_gallery*.json snapshots the102servedimages and
20sections. Filenamephoton_id_track_opening stays as a legacy URL, while its
axis and currentdescription explicitly use distance. No new REC read/refit.

The gallery agent replaces only the retained PNG/PDF pair, archives the former
served mradpair outside servedtree, and updates only this sectiondescription.
All100otherimagehashes/19othersectionobjects/patterns/order and the five prior
removals stay unchanged. Expectedcounts75PNGs20sections27PDFs102images.
Checks go to distance_gallery_verification.json. Full outgoing AGENTS snapshot:
2026-11-15-agents-before-photon-distance-mm.md. Globalprefix unchanged.

Millimetre local publication verified by main: only retainedPNG/PDFpair changed,
all100otherimagehashes and19othersectionobjects/order/patterns fixed; target
title/pattern unchanged.75PNGs20sections27PDFs102images. Newsource/gallery
hashes match. Native inputdata and all five prior removals remain unchanged.
Final host HTTP/source-local-servedhashproof is in distance_gallery_verification.json.
