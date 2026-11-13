# Neutral energy purity versus reconstructed metrics — 2026-11-14

100,000 existing REC_ECALTRUTH events (all500 sources). Same198081 truth-covered
selected-production-electron tracks as the truth-normalized energy bars; no
GSF/BP fit-validity or recoil gate. Same fixed neutral-cluster energy cap
55.75722694396976GeV, from the earlier user-requested upper2% trim. No new energy
trim or truth-angle matching. The 349 tracks lacking a saved electron ECAL
truth entry remain excluded as in the parent bars. Full truth photons are not
the purity denominator here: purity is reconstructed signal energy/full neutral
energy. All neutral candidates are charge-zero, trackless, hit-supported, with
positive finite energy and valid measured position.

Purity = sum(Ecluster*(native FSR fraction + native eBrem fraction)) / sum(Ecluster).
Only radiation associated with the selected Z->ee production pair is signal.
Green is exact selected main-electron MC contribution; other origins/unknown
are black Others. FSR roots are generator-tagged, eBrem roots use the existing
Geant4 emitter/birth matching and MC ancestry. Native cluster MC weights are
response-attribution proxies, not calibrated deposited-energy truth. Ratios
are energy weighted, not averages of individual-cluster purities.

## Reconstructed metrics

Baseline half-windows |deltaTheta|<=25mrad, |deltaPhi|<=200mrad, about each input
CompleteTracks ECAL reference. One neutral counted once per event; overlaps
are assigned to the closest angular reference among baseline-passing tracks
(or wide-passing tracks for the wide map). Origin fractions refer jointly to
the selected pair, not an inferred reconstructed emitting leg.

Distances are three-dimensional Euclidean distances in mm from the neutral's
measured cluster position to (1) its assigned track ECAL reference or (2) the
nearest valid ECAL cluster of the uniquely linked charged PFO for that track.
They include cluster-depth differences, rather than a pure lateral surface
projection. No charged-energy trim is applied to geometry-only references.
Missing/ambiguous/nonusable charged associations remain a separate control;
charged-distance plots/cut comparisons require a usable reference. All other
plots retain these candidates.

Isolation I = Eneutral / (Eneutral + Eother), where Eother is all other valid
pre-ECAL cluster energy in a100mrad OPENING-ANGLE cone around the candidate.
Exclude the candidate and valid charged-PFO ECAL clusters associated with
BOTH selected electrons; keep all other charged and neutral activity, including
other selected-electron shower fragments. Higher I is more isolated; I=1 means
no other included activity. No MC information enters these metrics/assignments.
Four candidates have a local neighbor above240GeV; removing them changes the
global purity negligibly. This sensitivity is recorded, not a new selection.

## Plots

1. Purity/components vs track-ECAL distance.
2. Purity/components vs main charged-cluster distance.
3. Purity/components vs isolation.
4. Purity/components vs neutral-cluster energy.
5. Two-dimensional angular purity map, |deltaTheta| and |deltaPhi|<=200mrad;
   white dashed lines mark the nominal theta half-window25mrad. This wider
   diagnostic map does NOT change the baseline used by the other plots.
6. Joint charged-distance/isolation purity map within the baseline window.
7. Purity vs retained FSR+eBrem energy for simple distance/isolation cuts, on
   the SAME charged-reference-available baseline cohort. Retention is relative
   to its already-collected photon energy, not all generated photon birth E.
8. Purity with/without a usable reconstructed charged-cluster reference.

Profiles/map cells require >=30clusters. Distance profiles display0--500mm;
all original distances and tail/control bins are saved in CSV/JSON. Errorbars
are pointwise16/84percentiles from300Poisson block resamplings of the500source
jobs. These are descriptive uncertainty checks, not detector calibration or
independent held-out validation. Cut curves retain points with >=30clusters;
all thresholds (including tiny surviving tails) remain in cut_tradeoffs.csv.
AGORA simulation is above the axes; colours/legends provide origin definitions.

## Validation and limits

Every current source path/size/mtime matches its native-weight cache, checked
before/after reading. Native measured energies and all six cached angular
masks reproduce for all91482wide candidates; baseline55391cluster energy and
all four origin sums reproduce the prior50mrad-total-width budget at relative
1e-12. Fresh source1 reclassification reads native MC weights/origins directly
and reproduces all200neutral rows/four fractions EXACTLY. No full software,
simulation, tracking, PFA replay or fitting is run. Source ROOT files untouched.
Geometry/isolation self-checks cover cone exclusions, I arithmetic and phi wrap.

Low PHOTON purity from main-electron energy is particularly relevant to adding
neutrals onto a track momentum, where double counting is possible. In BP's
charged+neutral TOTAL energy measurement, that same energy may legitimately
restore electron shower energy missing from the charged cluster. Therefore
these purity cuts are diagnostics, not recommended changes to the BP constraint;
a mass/performance comparison is still required before adopting them.

Full metrics/cell/bin/cut tables, source identities and PNG/PDFs are local
analysis artifacts in this directory. Existing gallery figures are preserved.

## Results (full sample)

Baseline:55391unique neutral clusters from38749events; wide:91482from48721events. Input denominator remains100000events. Baseline energy purity63.31%, main-electron35.84%, Others0.85%.

| Reconstructed condition | Neutral clusters | FSR+eBrem energy purity | Main-electron energy fraction |
|---|---:|---:|---:|
|Charged distance <50mm|8965|47.45%|52.29%|
|Charged distance50–150mm|29441|71.01%|28.43%|
|Charged distance >=150mm|9915|87.54%|9.08%|
|Isolation <0.5|18248|58.12%|39.58%|
|Isolation >=0.9|30264|63.51%|36.15%|

Charged-reference available:48321clusters, purity69.23%; unavailable:7070, purity41.66%, main-electron58.16%. Distance cuts/control curves use ONLY the available cohort for a common comparison.

On that cohort, rejecting neutrals within25mm raises purity69.23->71.02% while retaining98.86% of collected photon energy. A50mm rejection gives75.04% and retains85.57% (FSR89.36%, eBrem82.99%);100mm gives82.01% but retains44.41%. Isolation>=0.5 gives70.28% and retains89.56%;>=0.9 gives70.12% and retains73.03%. Thus local isolation adds little by itself; distance is more informative in this sample. No cut is installed.

GSF momentum can already include a material-loss correction, so high combined FSR+eBrem purity also does not establish that adding every recovered eBrem photon is appropriate for GSF. This study measures origin and collection geometry; it does not change any four-momentum prescription.

## Publication and preserved project state

Task started Nov13 and completed Nov14; artifact directory keeps the start date:
`TrackingPerformanceStudies/neutral_purity_metrics_20261113/`. The delegated
plot-gallery agent appended one section without restarting the healthy server.
All8PNG/PDF pairs copied byte-for-byte into bp_matched_recoil_pt_20261110.
All64pre-existing PNG/PDF hashes unchanged. Final64PNGs/15sections, exact-once
manifest coverage,16PDFs. Gallery page and all8newPNGs HTTP200, source/local/served
SHA256 identical, ailogin002.ihep.ac.cn PID443262, localhost8000. Main independently
reviewed renders, exact-once coverage, all16new copies and all64old image hashes.
Verification: gallery_verification.json and before_gallery.json in the artifacts.

Ordered new filenames (all .png plus .pdf):
- neutral_purity_charged_reference_control
- neutral_purity_vs_track_distance_mm
- neutral_purity_vs_charged_distance_mm
- neutral_purity_vs_isolation
- neutral_purity_vs_energy_GeV
- neutral_purity_angular_map
- neutral_purity_distance_isolation_map
- neutral_purity_selection_tradeoff

No GSF/BP fitting code, run-card template, installed library, MC/PFA reconstruction
or REC input changed. No fit or mass/performance validation. Existing BP/GSF
production jobs were not queried, submitted or stopped. In particular the
recreated H0_F/theta25 batch's subsequent submission state is not established
by this read-only study. Prior native BP products remain deleted; no new BP
ROOT file is needed to reproduce this analysis. All scripts/plots/tables are
uncommitted analysis artifacts as required; only this record, AGENTS and its
outgoing snapshot are checkpointed. Global AGENTS status/laws/commands preserved
byte-for-byte, with complete outgoing focus in
2026-11-14-agents-before-neutral-purity-metrics.md.

Analysis implementation hashes:

- analyze.py: `c63fa5c36654a1d903ed0326a7f9df6ecba913e86427d4c15f03f45f27b108aa`

- plot.py: `6287b5d7c23266dd4abe4f73b4399c60583e9f7fb3cef318f4e3bf710adb487e`
