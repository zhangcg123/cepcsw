# Upper2% neutral-cluster energy-tail exclusion, 2026-11-12

User requested exclusion of extreme saved energy outliers, then specified
"exclude the2%tail". This supersedes the preliminary >240GeV-only cut. The
published nominal analysis now removes the upper2% cluster-energy tail, not
2% of events, not both tails and not two independently fitted thresholds.

The98th percentile is computed with NumPy's default linear quantile on all
63390positive-finite neutral clusters in the50mrad/200mrad union, before the
upper cut. Threshold55.75722694396976GeV is applied to both theta10 and theta50
collections. Clusters above it are excluded, not capped. Both windows retain
the same per-generated-input normalization100000. All other selection/truth
categories and nominal-IP window geometry remain unchanged. No REC file,
reconstruction algorithm, library, card, job or photon truth definition changed.

1268wide clusters excluded (2.0003%);1253narrow (2.5117%). The common threshold
is deliberate so theta-window comparisons use identical energy acceptance.
1267events contain excluded clusters; surviving clusters in those events stay.
The cut includes all seven old >100GeV clusters and also removes physical
high-energy clusters, so these are explicitly trimmed-sample results.

| Half-theta / half-phi | Retained clusters | Events with retained clusters | Mean E/input event[GeV] | FSR% | eBrem% | Other selected-electron% | Unrelated% |
|---|---:|---:|---:|---:|---:|---:|---:|
|10/200mrad|48634|36100|6.36248|21.5659|41.5391|36.4049|0.4901|
|50/200mrad|62122|41258|6.84563|24.1995|39.3574|34.9594|1.4836|
|Added10-to50|13488|10181|0.48316|58.8812|10.6275|15.9244|14.5668|

Retained FSR proxy energy137212.321->165661.132GeV, gain20.7334%. Added-region
FSR28448.811GeV, unrelated7038.038GeV. MC energy-response proxies are not exact
Geant4 deposits or total FSR recovery efficiencies. Previously excluded
nonfinite190/249clusters remain excluded, with185/244affected events.

Artifacts: TrackingPerformanceStudies/pre_ecal_window_energy_20261112/.
Raw per-job caches are unchanged. render.py applies the cut to cluster rows
and subtracts the corresponding total and category energy from per-event rows.
New canonical collected_clusters.csv, event_energy.csv, energy_table.csv,
summary.json, excluded_clusters_upper2pct.csv and all3PNG/PDFs use the same cut.
Original no-upper-cut figures/tables/code are archived in before_energy_tail_cut/;
the earlier full-sample result is also preserved in
2026-11-12-direct-pre-ecal-window-energy.md. No100000-event ROOT reread or native
software test/rerun was necessary for this cached-data update.

Assertions verify all retained energies meet the cap, excluded+retained counts
close, removed energies reproduce old-to-new category differences, per-event
subtractions remain nonnegative, and per-event/cluster/window/category sums
close. Plot-file hashes and the62122-row final cluster CSV were checked; plots
were visually inspected. Source study hashes were refreshed. The plot-gallery
skill's dedicated gallery agent handles the3figure replacements and metadata,
preserving all other sections and verifying liveHTTP/hash responses.

Gallery publication verified: all3replacement PNGs and homepageHTTP200, hashes
match final originals,11PNGs/4sections each covered once. Existing managed
localhost8000 server on ailogin002, PID2122174, updated live without restart.
Other plots/sections preserved; first section explicitly labels the common
upper2% trim and its exact reference/cut. Main independently checked copied
hashes and updated manifest after the dedicated agent's HTTP verification.
