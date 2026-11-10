# Matched GSF added to BP100 pT and recoil plots

User requested adding the newly completed matched GSF events to the existing
four BP plots. This is offline analysis only: no fit, source/runtime card,
library, job submission, retry or physics file changes. Previous two-method
plots/data/scripts/notes are preserved at
`.analysis_archive/bp100_before_matched_gsf_20261110T213916/`.
Active directory remains
`TrackingPerformanceStudies/bp_matched_recoil_pt_20261110/`.

## Campaign audit and exact joins

GSF campaign `higgs_eeHX_ISRFSRON_GSF_MATCHBP_200jobs_50evt_20261110`
has198 verified completion markers, each50 events; all198 ROOT files were
independently read and confirmed to have50 rows. Jobs32 and36 were killed:
both stderr files explicitly state they exceeded the30-minute time limit.
Neither has a completion marker; their partial ROOT files were not used.
Completed-job duration min/median/max559.5/915.6/1648.4seconds, measured between
worker started/completed markers. No retries were submitted.

All9900 completed events match BP source REC paths and entries0--49.
GSF iev-1 = BP event_index. Track join is original CompleteTracks index;
selected generator MC identities agree exactly. All19656 selected GSF input
keys equal the BP cache keys for those198 jobs, with no duplicate or unmatched
key. All396 BP/REC size/mtime identities match the original snapshot. GSF
size/mtime is checked before/after reading and saved in matched_gsf_snapshot.json.
Each BestBranch endpoint vector is aligned with gsf_input_track_indices and
has a saved validity flag, finite native five-parameter IP state, positive pT,
and3T curvature closure. There are18361 published valid tracks and1295
selected tracks without a published GSF output. The two killed jobs account
for199 additional selected rows in the original200-job coverage denominator.

GSF uses the new native per-track state vectors for both electrons. No legacy
first-track scalar or reconstructed KF direction is substituted. Both tracks'
own GSF directions were independently checked against cached recoil vectors;
7757/7759 common pairs have at least one GSF phi different from its KF value.

## Comparison definition

Exactly four PNG/PDF pairs remain, pT core/wide and recoil peak/wide, each
with Baseline KF(gray), BP100 truth location(blue), GSF BestBranch(orange).
All three curves use one common valid-track intersection; recoil additionally
requires both legs and opposite baseline KF charges. Missing GSF publication,
killed jobs and unavailable BP endpoints are excluded from every curve.
This changes the baseline/BP plotted population relative to the preceding
BP-only intersection. Counts remain relative to original selected tracks/pairs
from200 jobs, so missing-job and fit losses are visible.

BP100 eligibility is unchanged: result1/2, constraint_status2, requested/actual
mask1, finite native state/positive pT and curvature closure; valid null controls
remain, producer input-KF fallbacks do not. Truth selects its largest eligible
layer-owned breakpoint or null; loss magnitude remains fitted. This is a
truth-location-assisted BP benchmark, not a blind location scan. GSF receives
no BP location truth. Both fits use tracker+beam, ECAL off, same beam center
and widths. The frozen reverse GSF card selects fresh inward-1, both BH gates
on, look-ahead0, LocalMeasurement, CEPC BH, DD4hepBetweenSurfaces,
MaxComponents10, oracle/passive material recorder off.

Material fraction and FSR treatment are unchanged: recorded primary-electron
tracker photon birth energy / post-FSR pre-material stable electron energy;
no additional photon threshold. pT categories f<1%(zero included) versus
f>=1%; recoil no/light requires both legs<1%, hard either>=1%.
FSR is retained and excluded from the material numerator. pT reference is the
bare post-FSR electron. Recoil is nominal(240,0,0,0)GeV minus two electron
four-vectors, electron mass included and each method's own fitted directions.
No generator recoil curve or photon addition. Undefined recoil stays in the
full common-pair denominator; no visible-window renormalization.

## Results on the common population

| Track category | Selected | Common | KF central68 width[%] | BP100 width[%] | GSF BestBranch width[%] |
| --- | ---: | ---: | ---: | ---: | ---: |
| no+light | 10224 | 9466 | 0.24567 | 0.26295 | 0.25164 |
| hard | 9631 | 8069 | 9.40882 | 4.29640 | 3.52242 |

Total17535 common tracks. Width=(q84-q16)/2 on full distributions, not Gaussian
fit sigma. Finite extreme outliers remain, including GSF: full residual RMS
is63.16% no/light and2518.33% hard. The requested core/wide windows do not
show every extreme value; in-window counts are printed and full statistics
retained. The conditional comparison does not establish population validation.

| Event category | Selected opposite-charge pairs | Common | Undefined KF/BP/GSF recoil |
| --- | ---: | ---: | ---: |
| no+light | 2646 | 2275 | 0/1/2 |
| hard | 7197 | 5484 | 1/37/50 |

Total7759 common pairs from9843 selected opposite-charge pairs. As before,
145 original events lack one selected partner and12 have same KF charge.
Recoil distribution widths contain physical ISR/FSR spread and must not be
presented as intrinsic detector resolutions.

## Reproduction and validation

Uncommitted analysis matched.py reads198 completed GSF files and the original
BP caches, writing tracks_matched_gsf.csv, pairs_matched_gsf.csv,
matched_summary.json, matched_gsf_snapshot.json and the same four PNG/PDF pairs.
Original BP cached values are exactly preserved. Full independent checks pass
for unique keys, selected-track/MC joins, valid both-leg pair membership,
category boundaries, all three methods' four-vectors/recoil masses, quantiles
and population accounting. All four updated figures were visually reviewed.
The previous analyze.py/recoil.py and their summaries remain identifiable as
BP-only provenance; matched_summary.json is authoritative for the overlays.

The dedicated gallery agent updated the manifest and verified4 PNGs across
4 sections exactly once. Existing PID384311 remained healthy on
ailogin002.ihep.ac.cn, http://127.0.0.1:8000/. Page and overlaid pT core PNG
returnedHTTP200(image/png for the latter); page shows all three methods,
common totals and revised categories. Main review confirmed manifest semantics.
