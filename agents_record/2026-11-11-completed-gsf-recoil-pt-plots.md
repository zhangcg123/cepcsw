# Completed GSF recoil and pT comparisons (2026-11-11)

The user requested the same resolution and recoil panels for GSF tracking.
Created eight figures comparing GSF BestBranch only with Baseline KF, retaining
inclusive and isolated no-FSR/eBrem views. Each has pT core/wide and recoil
linear/log-y figures, each split into no+light/hard material eBrem at1%.

## Input and matching

The GSF MATCHBP500x200 campaign has498 worker-completed files and two failures:
55 and421, both exit139. The frozen BP truth/Baseline KF cache has498 files,
missing40 and429. The new analysis uses the496-job intersection,99,200 events.
Completed GSF jobs40/429 are excluded for absent cached truth/KF, not fit quality.
No BP endpoint-success condition is used. Do not directly compare GSF/BP
population widths without rematching all methods and valid-fit populations.

Frozen GSF job/card hashes and BP source manifest identities are checked.
REC paths, size/mtime and event ranges0..199 match the unchanged cache.
Each GSF file has200 sequential event rows. Native per-track BestBranch
vectors have matching lengths, unique original input indices and selected MC
identities; positive finite endpoints obey3T pT/curvature closure. Each recoil
leg uses its own method's pT, phi and tanLambda. No scalar endpoint or
Baseline KF direction substitution. No MC recoil or photon addition.

Effective cards: reverse, fresh inward seed(scale-1), lookahead0, two-electron
selector, DD4hepBetweenSurfaces, both BH gates ON, CEPCRuntimeCategoryAligned9Clear,
max10, cutoff1e-4, LocalMeasurement, live beam spot ON. ECAL constraint,
truth oracle and passive material recorder OFF. Configuration names containing
truth-bh-off refer to the oracle, not the BH-splitting gates.

## Definitions and coverage

Material loss is summed saved G4-parent-owned photon birth energies divided by
the matched stable post-FSR generator electron energy. No+light means<1%;
hard>=1%. Recoil hard if either leg hard. FSR uses the saved generator-origin
ownership tags. The isolated selection requires exactly two opposite-PDG
selected electrons, no FSR on either, and material eBrem on at least one.
Selection precedes fit validity and both legs remain in per-track plots.

All curves in a panel use identical valid tracks or opposite-charge pairs.
Recoil is nominal(240,0,0,0)GeV minus the two electron four-vectors, electron
mass included. Undefined recoil masses remain in full common denominators.
Recoil120--130GeV/0.1GeV bins; pT residual100*(reco/gen-1), core[-5,5]% and
log-y wide[-100,200]%. No window renormalization or extra tail/quantile panels.

| View | Selected tracks | Common tracks | Selected opposite-charge pairs | Common pairs |
| --- | ---: | ---: | ---: | ---: |
| inclusive | 196,844 | 184,073 | 97,517 | 85,327 |
| no_fsr_ebrem | 29,084 | 27,090 | 14,521 | 12,612 |

| View/category | Baseline KF pT sigma68 [%] | GSF BestBranch pT sigma68 [%] |
| --- | ---: | ---: |
| inclusive/no_light | 0.25186 | 0.25768 |
| inclusive/hard | 9.99821 | 4.07531 |
| no_fsr_ebrem/no_light | 0.26538 | 0.27058 |
| no_fsr_ebrem/hard | 10.14412 | 4.03895 |

196,844 selected tracks include184,073 published valid endpoints and12,771
unpublished GSF rows. Zero invalid published endpoints/unknown-FSR tracks.
Inclusive common pT groups:98,037 no+light and86,036 hard; recoil24,647/60,680.
No-FSR/eBrem common pT13,363/13,727; recoil2,872/9,740. Undefined recoil KF/GSF:
inclusive0/27(no+light),26/726(hard); isolated0/2 and3/125. GSF outliers remain:
inclusive no+light pT RMS64.45% despite central68% width0.25768%; hard RMS3590.44%.
Central68% widths are distribution summaries, not Gaussian fits or physics
validation. Do not infer clean-track preservation from improved hard-loss cores.

## Artifacts and verification

Analysis artifacts remain uncommitted under
TrackingPerformanceStudies/gsf_completed_recoil_pt_20261111/: analyze.py,
analysis.log, README, snapshot.json, tracks.csv, pairs_{inclusive,no_fsr_ebrem}.csv,
summary.json, validation.json, validation_baseline.json, PLOTS_READY, preserved
prior-plot hashes and eight PNG/PDF pairs. Figures are copied to the existing
bp_matched_recoil_pt_20261110 gallery; all22 previous PNG/PDF files are unchanged.
All source/event/identity/endpoint/category/histogram gates pass. Fresh native
REC job1 matches all399 cached Baseline KF track states. Fresh native GSF
jobs1/500 agree exactly with743 cached published endpoints.784 independent
recoil evaluations agree within7.11e-14GeV, including per-leg four-vectors and
undefined-mass treatment; details are recorded in validation.json.
Representative inclusive pT and isolated recoil layouts are visually inspected.
The dedicated personal plot-gallery agent verified19 PNGs across5 sections,
each exactly once, with the two new GSF sections first. Parent review confirmed
the counts/definitions and all22 old plot hashes. The healthy existing server
remains on ailogin002.ihep.ac.cn, PID2122174, http://127.0.0.1:8000/. Live
homepage sections and a served GSF PNG hash agree with the disk artifacts.

No simulation/tracking/fitter/writer source, runtime library, maintained card,
batch submission/retry/cancellation or branch change. Preserve both normal and
private runtime dependencies. This is stored-output plotting, not an algorithm
selection or validation claim. Outgoing full status is preserved in
[the pre-update snapshot](2026-11-11-agents-before-completed-gsf-plots.md).
Prior BP and truth geometry definitions/results remain in their dated records.

## Follow-up: one inclusive three-method recoil plot

User requested one120--130GeV plot with BaselineKF,100 andGSF. The new single
panel uses77,817 identical valid pairs across KF/BP100/GSFBestBranch, inclusive
of FSR and all material-loss groups. It joins the existing88,598 BP-common and
85,327 GSF-common pairs by job/event, with exact electron/positron original
input indices, material category and BaselineKFfour-vector equality. Thus the
three curves share the same population; no BP/GSF direction substitution.
BP100 retains truth-selected location/fitted loss, tracker+beam andECALoff.
Recoil definitions unchanged,0.1GeV bins, no photon addition/generator recoil.
Fullcommon-pair normalization retains undefined masses and out-of-view entries.
In-window KF/BP100/GSF36,715/42,057/42,584; undefined22/468/567. This is a
stored-output comparison, not an algorithm-validation claim.

Artifacts in the same GSF study: inclusive_three_methods.py/.log,
recoil_inclusive_kf_bp100_gsf.csv, PNG/PDF and _summary.json. Input hashes,
unique pair keys, exact baseline/leg identities and histogram counts pass;
all38 preceding plot artifacts remain unchanged. Layout visually inspected.
The personal gallery agent verified20 PNGs across6 sections exactly once,
with this plot first; parent manifest/hash review passes. The live page and
served PNG hash match disk at http://127.0.0.1:8000/, ailogin002 PID2122174.
Separate existing panels remain unchanged. No ROOT reread,
reconstruction/fit, installed-library or job-state changes.

## Follow-up: BP110 line for events with neither electron carrying FSR

The user clarified: add BP110 only for no-FSR-on-either-electron events,
not a per-track110/100 hybrid. No additional eBrem requirement. The fourth
green curve is the subset of the77,817 existing common inclusive pairs with
no saved generator FSR on either leg and two valid BP110 endpoints. There
are12,504 no-FSR candidates;11,750 pass both110 endpoints and754 do not. Each
leg uses its own BP110 native momentum and direction, truth-selected location
and fitted loss, ECALon. No FSR inference, photon addition, fallback or fit rerun.

The fourth curve uses its own full11,750-pair denominator, including two
undefined masses and out-of-window entries;8,970 pairs lie in120--130GeV.
The three inclusive curves retain exactly the original77,817 pairs and all
stored recoil values, with unchanged input source hashes. They have a different
FSR population from BP110; peak-height/shape differences are not an isolated
measurement of algorithm performance. Bins remain0.1GeV; extra label and footer
state the differing selection and normalization. Layout visually inspected.
All38 otherPNG/PDFs remain unchanged; original three-curve figure and analysis
artifacts are preserved in the study's before-no-fsr-bp110/ directory. Source
CSV hashes, original curves and overlapping old110 recoil values are checked
in validation_no_fsr_bp110.json. Gallery first section is updated by the
personal gallery agent: live text and new PNG hash agree with disk,20 PNGs
across6sections each exactly once; parent manifest/hash review passes. Healthy
server remains ailogin002 PID2122174, http://127.0.0.1:8000/.
Only uncommitted analysis products and these durable status records change.
