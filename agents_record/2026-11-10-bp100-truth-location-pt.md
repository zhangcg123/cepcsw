# BP100 truth-location pT histograms

## User decision

The user accepted using the existing truth-location histograms for setup100.
This supersedes the preceding proposal that a blind100 rerun was required for
the immediate plots. No rerun is needed for this explicitly truth-assisted
benchmark. It does not turn the existing100 fields into blind-scan results.
The user-authorized change is offline plotting only; GSF runs and all runtime
paths remain frozen. No source, card, library, job or physics file is changed.

Previous110 eBrem plots are archived at
`.analysis_archive/bp110_ebrem_pt_before_truth100_20261110T131721Z/`.
Active directory remains `TrackingPerformanceStudies/bp_matched_recoil_pt_20261110/`,
with exactly two PNG/PDF pairs:pt_bp100_truth_ebrem_core/wide. Curves are
Baseline KF versus BP100(truth location), not110 plus calorimeter fallback.

## Endpoint and selection

Read saved beam_spot_free_loss_rts_pt/ip_parameters for each selected input
track from the200 completed BP files. Require result_status1/2, successfully
applied constraint_status2, requested/actual mask1, five finite IP parameters,
positive pT and curvature closure at3T. Unavailable/unapplied stages and producer
input-KF fallbacks are excluded from both curves and counted. Valid no-breakpoint
beam-only ordinary controls are retained. Truth supplies the largest eligible
layer-owned loss location, or null if no eligible owner; loss energy remains
fitted, with no truth-energy override. The constrained family is fit before110,
from fresh beam-only settings without ECAL or a preceding optimized posterior.
Both GSF and BP beam centers/widths are0,0 mm and0.0145,3.6e-5 mm.
This matches external constraints but gives BP truth-location information;
future GSF comparisons must keep this label and use identical valid tracks.

The no/light/hard fraction stays sum(recorded primary-electron tracker eBrem
photon birth energies) / stable generator-electron energy afterFSR,before
material:0,0<f<1%,f>=1%. Cached exact energies/MC identities are unchanged.
FSR is retained and excluded from the numerator. No pair/charge cut. Per-track
pT residual100*(pT_reco/pT_gen-1); full denominators/statistics, core±5%, wide
[-100,200]%log-y. No recoil,110, extra control, survival or quantile plot remains.
Full original100 parameters/statuses and rejection reasons are cached in
tracks_bp100_truth.csv, including excluded selected rows. Original110 cached
columns remain identifiable but never enter the new histograms.

## Results and coverage

| eBrem category | Selected | Common valid | Unavailable | With eligible breakpoint | Null controls | Baseline KF width [%] | BP100 truth-location width [%] |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| no | 5,180 | 5,038 | 142 | 10 | 5,028 | 0.20482 | 0.20298 |
| light | 5,044 | 4,844 | 200 | 3,079 | 1,765 | 0.29460 | 0.38170 |
| hard | 9,631 | 8,986 | 645 | 5,967 | 3,019 | 14.11583 | 7.58419 |

Widths are central68% half-widths on identical valid tracks, not Gaussian
peak-fit sigmas. Total18,868/19,855 pass. The987 exclusions comprise863
unavailable/failed endpoints,121 without the beam-only stage successfully
applied,3 producer input-KF fallbacks. No missing endpoint is silently
replaced by KF. These counts are conditional on selected reconstructed tracks,
not overall tracking efficiency. Material-photon severity and eligible
step-derived breakpoint ownership are separate definitions; the photon
category does not force the fitter to have a breakpoint.

All400 BP/REC source identities are unchanged. Unique joins, original category
assignments, native curvature/pT closure, result/constraint masks and coverage
accounting pass. The BP100 core figure was visually inspected. No fit was rerun.

## Gallery verified

The dedicated plot-gallery agent created a fresh manifest:exactly2 PNGs across
2 sections, each matched once. It reused healthy PID384311 on
ailogin002.ihep.ac.cn,localhost8000. Page HTTP200 lists only the two new100
files and explicitly labels truth-location assistance, tracker+beam, ECAL off,
no fallback and matched-valid coverage. Core PNG returns200 image/png.
The main agent reviewed the core figure and manifest semantics. Old110 file
references are absent. URL:http://127.0.0.1:8000/ on that host.

## None/light panels merged

At the user's request, pooled none and light tracks into one f<1% panel,
keeping hard f>=1% separate in both core/wide figures. Original cached data
and all18868 valid endpoints are unchanged; no ROOT input is reread or fit run.
Merged common/selected9882/10224; hard8986/9631. Recomputed central68% widths
are KF0.24658%,BP1000.26394% in the merged panel; hard14.11583%,7.58419% is
unchanged. The previous three-panel images and renderer/README/manifest/summary
are preserved at .analysis_archive/bp100_three_panels_20261110T132442Z/.
Original three-bin CSV labels remain intact; only display grouping changes.
The main agent visually reviewed the merged core figure and checked population
closure. FSR retention, truth-location assistance, masks/eligibility and absence
of ECAL/fallback are unchanged.
The dedicated gallery agent updated the two-panel descriptions/counts, validated
2 PNGs across2 sections exactly once, and reused healthy PID384311. Page and
core PNG returnHTTP200 and the page contains the merged/hard counts. Main
review confirms the current manifest and figure layout. URL is unchanged.

## Recoil plots added

At the user's request, added recoil peak[115,150]GeV and wide[80,240]GeV
log-y figures comparing Baseline KF and BP100(truth location). Existing four
pT PNG/PDF files are unchanged by SHA256. Recoil uses nominal(240,0,0,0)GeV
minus the two electron four-vectors, including electron mass and each method's
own native directions. No generator recoil curve or photon addition; FSR is
retained and ECAL off. Cached tracks only, no ROOT reread or fit rerun.

No/light events require both fractions<1%; hard requires either>=1%.
From10000 events,145 lack a selected partner and12 have same reconstructed
KF charge. Selected opposite-charge pairs9843; common valid100 pairs8891,
with952 unavailable pairs excluded from both curves. No/light common/selected
2473/2646; hard6418/7197. Undefined KF/BP recoil counts0/1 and1/43 remain
in the full common-pair denominators; no visible-window renormalization.
The distributions include physical ISR/FSR spread and are not intrinsic
detector-resolution estimates. pairs_bp100_truth.csv retains full four-vectors;
recoil_summary.json records accounting/statistics. Boundary/mass assertions,
unique-pair accounting and visual inspection of both new figures pass.
The dedicated gallery agent verified4 PNGs across4 sections, each exactly
once. Existing PID384311 remains healthy on ailogin002 localhost8000; page
and new recoil peak PNG returnHTTP200(image/png for the latter). Main review
confirms manifest semantics and both recoil figures. URL is unchanged.

## Matched GSF overlay supersedes two-curve display

The user subsequently requested adding matched GSF BestBranch to all four
figures. The preceding BP-only results remain historical evidence. Active
curves now share17535 valid tracks and7759 valid pairs across all three
methods, using198 completed GSF jobs and excluding timed-out32/36. Full
coverage, reproduction, limits and archive location are in
[the overlay record](2026-11-10-matched-gsf-bp-overlays.md).
