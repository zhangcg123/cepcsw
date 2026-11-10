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

## No-FSR + eBrem events isolated

The user requested removing FSR while retaining eBrem and the no/light versus
hard split. Added four *_no_fsr_ebrem PNG/PDF pairs alongside the four inclusive
figures. Select exactly two reconstructed production-electron tracks with
has_fsr=0 on BOTH and has_ebrem=1 on at least one, before fit validity. Unknown
flags and missing selected partners cannot pass. Tags use saved generator-origin
FSR/material-photon records, not angular inference; no extra energy threshold.
The veto concerns the two selected electrons, not unrelated event particles.
Both tracks enter pT, including a zero-eBrem partner. Per-track split remains
f<1% versus f>=1%; recoil event hard if either>=1%, otherwise both<1%.

Selected1507 events/3014 tracks, including619 zero-eBrem partner tracks.
Common2657 tracks: no/light1325/1425 selected; hard1332/1589. Central68 pT
widths(KF/BP100/GSF) are0.26483/0.27508/0.26826% and
9.26315/4.44794/3.40275%. Common recoil1183 pairs: no/light269/311 selected;
hard914/1196. Undefined KF/BP/GSF masses0/0/0 and0/5/8 stay in full common
denominators. Own native directions, BP truth-location assistance and fitted
loss magnitude, beam constraints, ECAL off and no photon addition are unchanged.
ISR remains physical spread; these are not intrinsic-resolution estimates.

Uncommitted no_fsr.py reads existing matched CSV caches only and reuses the
same renderer, with a specific subset label and filename suffix. Outputs are
no_fsr_summary.json, tracks_no_fsr_ebrem.csv, pairs_no_fsr_ebrem.csv and four
new PNG/PDF pairs. Inclusive plot SHA256 hashes are unchanged. No ROOT files,
fits, source/runtime cards, libraries or jobs were modified/rerun. The original
matched_summary.json remains the inclusive summary. Selection boundary/unknown
tests, both-leg common validity and population assertions pass; all four new
figures were visually reviewed.
The dedicated gallery agent verified8 PNGs across8 sections exactly once,
with the four no-FSR figures first. PID384311 remains healthy on ailogin002
localhost8000. Page and new no-FSR core PNG returnHTTP200; main review confirms
the pair-level veto, pre-validity selection, both-leg pT handling and counts.

## Truth-location BP110 tested on isolated events

The user expected truth-location110 to improve these no-FSR/eBrem events.
Added native charged_ecal_free_loss_rts_pt/ip_parameters to the four subset
figures, green, preserving the inclusive four figures byte-for-byte. Read200
existing BP files and joined only the3014 isolated track keys. No fit, source,
runtime card, library or job changes. The previous three-curve subset, its
caches/notes and renderer are preserved at
.analysis_archive/bp100_gsf_no_fsr_before_truth110_20261110T215331/.

BP110 remains at the same selected truth-owned breakpoint/null as100, with
loss amount fitted. It is independent of location_scan_* and is not the
minimum-NLL scan candidate. Require result1/2, constraint_status2, requested/
applied mask3, finite native five-parameter IP state, positive pT and3T
curvature closure. A retained previous stage is not accepted as110. Among3014
selected tracks,2758 satisfy full110;129 unavailable/failed,127 without
successful full110. Valid null controls remain. All four curves use the same
valid intersection,2585 tracks and1117 pairs,72 tracks/66 pairs fewer than
the preceding three-method subset. Only110 receives ECAL;100/GSF are tracker
plus beam. BP is truth-location-assisted and GSF does not receive this location.

| Track category | Selected | Common | KF width[%] | BP100[%] | BP110[%] | GSF[%] |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| no+light | 1425 | 1308 | 0.26464 | 0.27483 | 0.34073 | 0.26629 |
| hard | 1589 | 1277 | 8.82945 | 3.89668 | 2.14204 | 3.30498 |

Widths are full-distribution central68% half-widths, not Gaussian sigmas.
Hard-event recoil widths(KF/100/110/GSF)8.27037/6.30547/4.63055/5.67929GeV;
no/light1.55638/1.41917/1.66617/1.60515GeV. These include ISR physical spread
and are not intrinsic detector resolution. Common no/light pairs259/311
selected; hard858/1196. Undefined masses KF/100/110/GSF0/0/0/0 and0/5/0/7
retain the full common-pair denominators. Results support improved hard-eBrem
performance with110 on this conditional sample, but no/light pT is broadened.
Neither a universal improvement nor production validation is established.

Uncommitted truth110.py writes tracks_no_fsr_truth110.csv,
pairs_no_fsr_truth110.csv and truth110_summary.json and redraws the four
subset figures using the same renderer. The110 summary supersedes
no_fsr_summary.json for the displayed isolated subset. The precursor
no_fsr.py would regenerate the earlier three-curve plots. All four updated
figures were visually reviewed. Independent checks pass for exact original
row/value preservation, common-track masks/statuses, same100/110 location,
both-leg joins, native110 four-vectors/recoil, all-method quantiles, coverage
and inclusive PNG/PDF hashes. The selected-event and radiation definitions
are unchanged. No generator recoil or extra plot was added.
The dedicated gallery agent updated the first four sections for BP110 and
its ECAL-only difference; the final four remain inclusive three-method plots.
Validation confirms8 PNGs in8 sections exactly once. PID384311 remains healthy
on ailogin002 localhost8000; page and updated no-FSR core PNG returnHTTP200.
Main review confirms the four-method counts, mask3 gate and ECAL distinction.

## All recoil displays restricted to120--130GeV

At the user's request, all four active recoil PNG/PDF pairs now show only
120--130GeV with0.1GeV bins: inclusive three-method and isolated four-method,
each linear and log-y. Legacy wide filenames now mean the log-y view at this
same range. Cached recoil_window.py redraws from existing pair CSVs; no ROOT
read or fit. In-window counts update but full common-pair denominators,
undefined-mass accounting and full-distribution summaries remain unchanged.
Hash assertions confirm all pT PNG/PDF files, CSVs and summary JSON unchanged.
Main review visually checked an inclusive linear and isolated log-y figure.
Gallery verification passes8 PNGs/8 sections exactly once; all four recoil
titles now specify120--130GeV and linear/log-y. Existing PID384311 remains
healthy; page and updated no-FSR recoil PNG returnHTTP200. Main manifest
review confirms range and unchanged-normalization descriptions.

## Recoil rebinning only

The user chose0.25GeV bins rather than a larger matched production. Redrew
all four recoil views with40 bins across120--130GeV; same cached events,
selections, full denominators and statistics. This increases counts per bin,
not sample size or statistical precision. No production prepared/submitted.
Hash checks pass; main review inspected the rebinned isolated linear figure.
Gallery agent verified8 PNGs/8 sections exact-once,0.25GeV wording in all
four recoil sections, and HTTP200 for page/updated recoil PNG. PID384311 and
ailogin002 localhost8000 are unchanged; main manifest review passes.
