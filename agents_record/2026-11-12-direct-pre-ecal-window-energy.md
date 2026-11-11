# Direct saved pre-ECAL window energy study, 2026-11-12

User accepted REC-input PFA reruns and asked to repeat the cluster-energy study
after the new production completed. All500jobs have completed.json and
ecal_truth_verified.json, zero completed_before_truth_QA markers;500REC outputs
occupy171.96GiB and contain100000events (200each).

Inputs are only REC_ECALTRUTH files. No full-software event processing, PFA
replay, old-cluster remapping, debug MC fingerprints, old BP/GSF momentum fits
or fit-success gate is used in this analysis. Direct ObjectID associations
belong to the exact same saved pre-reclustering clusters. Thus the previous
929/1000 replay-matching exclusion is not needed for this study.

Artifacts: TrackingPerformanceStudies/pre_ecal_window_energy_20261112/.
Definitions and limitations are recorded in its README.md. Main analysis is
analyze.py; energy spectra/budgets are rendered by render.py. Neutral clusters
are unioned over selected production-electron tracks, once per event. Current
half-window theta10/phi200mrad; compare theta50/phi200mrad, no energy threshold.
Both positive finite energy and nonempty hits are required. Saved generator
FSR links define FSR roots; exact saved G4 birth plus selected-electron parent
defines material eBrem MC roots. Descendants inherit photon categories. Other
selected-electron descendants are separate from unrelated origins. Weights
are normalized for native float-sum drift, not treated as deposited energy.
Soft material photons absent from MC cannot be independently root-tagged and
can remain in the other-electron category. No new fit/recovery steering.

Python selection mirrors TruthElectronSelector.cpp and TrackTruthMatch.h.
Cross-check against saved native BP selector rows for seeds1,22,43,63,84
covers1000events and1988tracks with exact MC/track agreement. BP is used only
for this selection cross-check; its clusters or fit outputs are not inputs.
Selector synthetic assertions cover hit-count ranking, purity tie-breaking,
Higgs ancestry exclusion, and unresolved exact ties. Per-job checks validate
source identity, direct association IDs/ranges/unique pairs/finite nonnegative
weights, cluster ownership, and category energy closure. Aggregation verifies
all500jobs, per-event/cluster totals and 10-to50-window energy closure.

The complete preceding AGENTS is preserved in
2026-11-12-agents-before-window-energy-results.md. Only its current focus will
be replaced with completed production and this study; active laws remain.
No history directory moved or renamed. Reconstruction code, native card/library,
job state, input files, existing plots and plotting server remain unchanged.

## Completed100000-event results

All500direct-input jobs analysed,198430selected tracks:98437events have both,
1556one,7neither. Every selected track has a usable AtCalorimeter reference.
2530400neutral clusters inspected;8203have nonpositive/nonfinite energy and
are excluded, none have empty hits.24801683direct MC links validated; maximum
native cluster-weight sum deviation8.1359e-6. No unknown MC energy remains
among the finite positive collected clusters under these ancestry categories.

| Half-theta / half-phi | Neutral clusters | Events with clusters | Total E[GeV] | Mean E/input event[GeV] | FSR% | eBrem% | Other selected-electron% | Unrelated% |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
|10/200mrad|49887|36927|723830.217|7.23830|20.2123|39.5389|39.8128|0.4361|
|50/200mrad|63390|42004|773060.212|7.73060|22.7064|37.6926|38.2759|1.3251|
|Added10-to50|13503|10189|49229.995|0.49230|59.3786|10.5457|15.6787|14.3970|

The widened window adds29232.084GeV of FSR proxy energy (19.98% relative gain)
and7087.658GeV unrelated proxy energy. The added region is59.38%FSR by energy,
not an overall FSR-pure collection. All percentages are MC response proxies,
not exact deposits or physical recovery efficiencies. Other-electron ancestry
must not be counted as FSR purity; material recovery may double count GSF losses.

Plots: plots/neutral_cluster_energy_by_origin.png,
plots/collected_neutral_energy_per_event.png, plots/neutral_window_energy_budget.png;
PDF counterparts, plot hashes, energy_table.csv, summary.json, per-job CSVs and
source stat identities are saved. Full cluster spectra extend to100GeV with
>=100GeV overflow in the last bin; per-event spectra extend to120GeV with
>=120GeV overflow. No clipping enters energy totals. Means divide by100000;
event-distribution curves use99993events with a selected track, including zeros.
Existing plotting server/gallery and its figures were not changed.

## Finite energy outlier sensitivity

Seven collected clusters exceed100GeV, six exceed the240GeV collision energy;
maximum3180.6968GeV. All seven have exactly matching energy and position in the
previous original REC_TPCFIX files, so these particular outliers predate this
exporter and REC rerun. No origin/algorithm fix is claimed or attempted.
They are retained in nominal no-upper-cut totals and plotted overflow bins.
The added10-to50 region contains none of them, so its energy budget is unchanged.

A diagnostic excluding only clusters above240GeV changes mean collected energy
7.23830->7.16799GeV/input event (theta10),7.73060->7.66029(theta50). FSR fractions
20.2123->20.3668%,22.7064->22.8740%; unrelated0.4361->0.4402%,1.3251->1.3371%.
The widening conclusion is stable, but these saved energies are not validated
calorimeter response. Exact event/cluster identities, original REC matches and
sensitivity totals are saved in finite_energy_outliers.json. No upper cut is
silently applied to the main result. Invalid-energy/window QA is reported
separately from these finite outliers.

## Completed window-energy quality accounting

A separate read-only pass over all500files finds190nonfinite neutral-cluster
energies inside theta10,249inside theta50, affecting185and244events respectively
(0.185%/0.244% of100000). No nonpositive finite or empty-hit cluster is rejected
inside either window. Main sums exclude only unusable clusters; other usable
clusters in affected events remain. The global8203unusable neutral energies
include clusters outside these windows. Exact window counts: window_quality.json.
All source identities, MC-weight/category energy closures,500-file coverage,
plot output hashes and selector cross-checks pass. No native software rerun,
source/card/library edit, input deletion or batch-state change occurred.

The first optional quality pass used threads and was stopped to avoid its
slow Python execution; the completed invalid-only pass used four processes.
All our analysis processes are now finished. Existing gallery unchanged.
The plot/code manifests, nominal and outlier sensitivity summaries and CSV
are final. Scientific spectra are PNG/PDF artifacts, not generated-image output.

## Subsequent gallery publication

User reported the new plots absent from the webpage. The plot-gallery skill's
complete update/validation workflow was delegated to one dedicated agent
(gpt-6-luna/high), then reviewed by the main session. Existing managed gallery
is on ailogin002.ihep.ac.cn, localhost8000, PID2122174, serving
TrackingPerformanceStudies/bp_matched_recoil_pt_20261110. No restart needed.

Copied three PNGs with direct_pre_ecal_ filename prefixes and verified SHA256
identity to study originals. Added a new first manifest section labelled as
the full100000-event direct-link study, preserving all previous plots/sections.
Manifest validates11PNGs across4sections, exactly one category per PNG.
Gallery homepage and all three new images returnHTTP200; main independently
confirmed the live homepage contains all three image links. Manifest and plot
copies remain uncommitted analysis artifacts. No analysis rerun or data/source,
installation, job-state or generic-server-code change occurred.
