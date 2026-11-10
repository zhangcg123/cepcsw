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
