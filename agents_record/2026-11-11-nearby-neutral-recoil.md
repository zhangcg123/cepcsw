# Recoil after nearby neutral-cluster four-momentum addition (2026-11-11)

The user requested a new plot adding surrounding neutral-cluster four-momenta
to the electrons. A separate two-panel figure preserves the original FSR-split
four-method recoil plot. It uses the same72,784 matched valid pairs from496
completed GSF/BP-overlap sources:11,750 no-FSR and61,034 with-FSR pairs. All
four methods share event keys and the same neutral addition within a panel;
no events are removed for collection/reference availability in this sample.

## Definition and source

Saved `collected_neutral_ecal_cluster_indices` identifies the actual original
BP neutral collection, recorded independently of fit success and with neutral
measurement steering off. Frozen cards use PreReclustering, theta half-width
10mrad and wrapped phi half-width200mrad. Native NeutralLossCandidate.cpp
centers the window on CompleteTracks AtCalorimeter reference position, selecting
ECAL clusters of charge-zero trackless PFOs, positive finite energy, nonempty
hits and valid position. The analysis does not rerun any reconstruction/PFA.
Pre-ECAL PFO links independently verify neutral ownership. Recorded cluster
indices/energy sums reproduce exact original pre_ecal_cluster_energy/x/y/z.

Each unique cluster contributes `(E,E*r/|r|)` using its reconstructed centroid
relative to nominal IP(0,0,0). Form the union of both electrons' selected
clusters, counted once per event; none are shared between legs in this sample.
Add this four-vector to the electron-pair sum for every method before computing
recoil against nominal(240,0,0,0)GeV. This is equivalent for recoil to adding
each unique cluster to one electron; no arbitrary shared-cluster allocation
can change the pair sum. Each method keeps its own original fitted electron
four-vectors. No electron mass-shell reset after addition. BP100 andBP110 use
truth-selected locations/fitted losses; ECALon only for110. No charged-cluster
energy is added. No generator recoil is drawn; truth FSR tags only split panels,
never select which clusters to add. No extra eBrem requirement.

Window/bins remain120--130GeV/0.1GeV, shared y-axis. Full common panel counts
include undefined and out-of-window masses. No-cluster events remain with zero
addition. Any unavailable collection/reference would remove an event from all
curves and be reported; here there are none.

## Counts

| Category | Common pairs | With neutrals | Added clusters | Added energy sum [GeV] |
| --- | ---: | ---: | ---: | ---: |
| No FSR on either electron | 11,750 | 2,376 | 2,788 | 33,269.79 |
| FSR on at least one electron | 61,034 | 20,374 | 24,892 | 302,066.62 |

In-window KF/BP100/GSF/110 counts: no-FSR8,294/8,623/8,735/8,637;
with-FSR35,402/37,428/37,893/37,304. Undefined counts29/127/139/26 and
156/617/647/162. Cluster totals refer to unique per-event indices, not truth
photons. Events with neutral addition are20.22% and33.38% of the two samples.

This is blind geometric neutral addition: selected clusters may contain FSR,
material eBrem, neutral hadrons or unrelated activity. IP track fits can already
recover material loss, so this operation can double count material-radiation
response. BP110's existing charged ECAL constraint and any charged-cluster FSR
contamination remain unchanged. This is a diagnostic, not a validated/calibrated
FSR recovery or a fitter development.

## Checks and artifacts

All496 input ROOT size/mtime identities match before/after reading, input CSV
and manifest hashes agree, event order and selected-track MC IDs agree. Native
PFO/cluster identity, neutral charge/no-track ownership, hit count, positive
energy, per-track cluster-energy closure, deduplication and photon masslessness
pass. Cached bare electron four-vectors reproduce all prior recoil masses.
Zero addition reproduces old results; event categories and histogram counts
close. Fresh REC/BP job1 window enumeration independently reproduces all399
selected-track collection index sets at10/200mrad. Scalar neutral-sum and
recoil checks are in validation.json: all72,784 event neutral sums close exactly,
27,680 added clusters are unique, and160 independent scalar recoil calculations
agree within6.40e-14GeV. New layout visually inspected.

Uncommitted artifacts: TrackingPerformanceStudies/recoil_neutral_addition_20261111/
contains analyze.py, README, analysis.log, snapshot.json, neutral_observations.csv,
added_clusters.csv, pairs_neutral_added.csv, summary.json, validation.json,
validation_window.json, preserved_gallery_sha256.json, PLOTS_READY and the
PNG/PDF. Copies are in the existing bp_matched_recoil_pt_20261110 gallery.
All40 preceding PNG/PDF files remain byte-identical. Personal plot-gallery
agent verified the new figure first,21 PNGs across7 sections exactly once, live
selection/count text and served PNG SHA256 matching disk. Parent reviewed the
manifest, rehashed all40 prior artifacts and verified both new plot hashes.
Server remains ailogin002 PID2122174, http://127.0.0.1:8000/.
No source tuple, original card, reconstruction/PFA/fitter implementation,
installed library, branch or batch state is changed. Complete outgoing status:
[the snapshot](2026-11-11-agents-before-neutral-recoil.md); original four-method
FSR split and GSF source/failure/QA details remain in
[the GSF plotting record](2026-11-11-completed-gsf-recoil-pt-plots.md).
