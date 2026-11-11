# Current-gallery label update, 2026-11-12

User requested changing "AGORA full simulation" to "AGORA simulation" on every
plot on the current page. Scope is exactly the16displayed PNGs and their source
PDF companions, the8analysis plotting producers, and visible gallery title/
description wording. Earlier unrelated plots and historical records retain
original wording. No physics, sample, selection, normalization or fit change.

Artifacts: TrackingPerformanceStudies/agora_simulation_label_20261112/.
refresh.py regenerates from saved CSV/JSON plotting data. Original producers,
source PNG/PDFs, served images/manifest and recoil fit reports/curves are
archived under before/. path_map.json maps every gallery PNG to its final
source. Existing per-study code/plot hash manifests are synchronized; pre-edit
code manifests are preserved in before/manifests/.

Six recoil figures use existing histograms and original cached RooCrystalBall
parameters/covariances. The label refresh evaluates those fixed PDFs on the
same700-point grid and invokes no fitTo/minimizer. All cached fit dictionaries
and group/method results are exactly unchanged; reproduced saved curve values
differ by at most4.55e-12expected events per0.1GeV (floating-point evaluation).
Other ten figures use existing truth, trace, acceptance and trimmed-cluster
CSVs/JSONs; no REC/flat ROOT event processing or native reconstruction rerun.

A save-time Matplotlib text gate verifies every PNG/PDF contains the new exact
label and no old phrase. Exactly16unique source PNGs match the complete gallery
filename map. Representative plots visually checked. verification.json records
all16gates,0fit optimizations,12cached curve evaluations and unchanged fit
results. Numeric data, normalizations and annotations retain original values.
The personal plot-gallery skill's dedicated agent publishes all16copies,
preserves categorization/order and checks the live gallery/image hashes.

No C++/header, installed library, run card, batch job or input data change.
Analysis scripts, regenerated figures and gallery artifacts remain uncommitted.

Publication complete: all16served PNGs and8existing served PDF counterparts
updated. Gallery title/description use the new exact label, with16PNGs across
7sections and every PNG categorized once. Homepage and all16PNGURLsHTTP200;
all hashes match regenerated sources. Main independently rechecked all16copy
hashes and absence of old text in the manifest. Same ailogin002 localhost8000
server PID2122174, no restart. Existing section order/content preserved.
