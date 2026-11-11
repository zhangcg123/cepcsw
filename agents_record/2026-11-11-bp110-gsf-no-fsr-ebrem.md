# BP110 and GSF on matched FSR-free events with material eBrem

Select exactly two production-electron tracks with opposite truth PDG,
no generator-tagged FSR on either (unknown FSR excluded), and recorded
material eBrem on at least one. Selection precedes fit validity. Require
valid Baseline KF, BP110 and GSF BestBranch endpoints on both tracks, with
opposite Baseline KF curvature signs. Do not require BP100 success.

All three curves and both figures use the **same 10,804 events**, with both
tracks retained for pT (21,608 tracks). The parent FSR-free/eBrem selection
contains 14,542 events; 21 have same-sign Baseline KF curvature and 3,717
fail at least one required endpoint. No recoil-window cut selects pT tracks.

BP110 uses its saved truth-selected eligible breakpoint or valid null and
fitted loss, with tracker+beam+ECAL constraints. In ElectronPhoton mode,
one breakpoint plus a usable collected neutral enables the charged+neutral
sum observation, even when the separate neutral-measurement switch is off.
Without usable neutrals it uses the charged observation; null fits use
electron-only charged energy. It is not a scan
minimum or a truth loss-energy override. GSF BestBranch has ECAL off. All
methods use each electron's own native fitted pT/phi/tanLambda. No neutral
or other photon four-vectors are added.

Input is the completed 496-file/99,200-event GSF cache and matching BP cache.
The full BP cache SHA256 matches the GSF snapshot's provenance. REC source
path/size/mtime and event counts match; every track joins on job/event/input
track with exact MC/G4 identity, generator pT/energy, Baseline KF state and
radiation classification. No ROOT read, fit, batch or production change.

Figures have inclusive, no+light and hard panels. Material-eBrem fraction is
the recorded material photon birth-energy sum divided by the post-FSR
generator electron energy. No+light is below 1%; hard is at least 1%. pT
panels classify individual tracks (10,979/10,629). Recoil classifies an event
hard if either leg is hard (2,600/8,204). Zero-eBrem partners are retained.

Recoil is 120-130 GeV in 0.1 GeV bins at nominal sqrt(s)=240 GeV. Relative
pT residual is 100*(pT_reco/pT_generator-1), displayed over +/-5%.
Histograms divide by the full common panel population, retaining undefined
and out-of-window masses; they are not peak-area normalized. Inclusive
in-window KF/BP110/GSF counts are 6,703/8,165/7,758, with 1/2/78 undefined.

The full-track central-68% half-widths in relative pT percent are:

| Tracks | Baseline KF | BP110 | GSF BestBranch |
| --- | ---: | ---: | ---: |
| Inclusive selected events | 3.199 | 0.690 | 0.974 |
| No+light | 0.260 | 0.342 | 0.265 |
| Hard | 8.794 | 1.919 | 3.392 |

BP110's gain over GSF is concentrated in hard loss; the no+light population
is broader with BP110. Recoil spectra include physical ISR and sample effects;
their widths are not pure detector mass-resolution measurements. This cached
comparison uses different algorithm inputs (BP110 truth location and ECAL),
and is not a same-code validation of a reconstruction-only location scan.

`analyze.py` checks event-gate edge cases, the 1% boundary, a synthetic recoil
calculation, exact cache joins, unique event keys, two tracks per event,
both-leg endpoint validity and histogram count closure. Source hashes and all
56 preceding PNG/PDF artifacts are unchanged. Layouts inspected. Artifacts
are two PNG/PDF pairs, tracks.csv, pairs.csv, summary.json, analysis.log and
PLOTS_READY. Re-run with:

```bash
OPENBLAS_NUM_THREADS=1 OMP_NUM_THREADS=1 python3 TrackingPerformanceStudies/bp110_gsf_no_fsr_ebrem_20261111/analyze.py
```

The complete outgoing focus is preserved in
[the snapshot](2026-11-11-agents-before-bp110-gsf-no-fsr-ebrem.md).

The dedicated plot-gallery agent added a new first section for these two
figures and preserved all28 previous PNGs and the remaining9 sections in
their existing relative order and with unchanged descriptions. Skill
validation and parent review confirm30PNGs/10sections with exact-once
coverage. Existing ailogin002.ihep.ac.cn server PID2122174 is retained at
http://127.0.0.1:8000/. Homepage and both new image routes return HTTP200;
served image bytes match disk and final summary SHA256s. Both final layouts
were inspected by the parent. No migration of a history directory was needed;
the outgoing AGENTS snapshot is exact and all text before Current focus,
including global status, active laws and compile/run commands, is unchanged.

## Correction: BP110 can already include nearby neutral energy

The earlier explanation that BP110 consumed only charged-cluster energy was
incorrect. In `RecBreakpoint.cpp`, the charged-ECAL stage sets
`resolvedEcalSum` and passes the collected neutral energy when ElectronPhoton,
one selected breakpoint and collected-neutral status2 hold. This deliberately
works with `BP_NEUTRAL_ECAL_MEASUREMENT=0`. Job1's manifest explicitly selects
ElectronPhoton and this separate-neutral-off setting.
`combineResolvedEcalMeasurements` in `BreakpointFitter.h` replaces the two
raw observations with one energy sum and quadrature uncertainty, then clears
the separate neutral observation. Thus mask3/110 is compatible with a
charged+neutral sum constraint. The actual saved endpoint flag is
`charged_ecal_free_loss_rts_resolved_ecal_sum_applied`.

For a summed BP110 endpoint, adding the same nearby neutrals to its fitted
IP momentum would double count them. A later KF/GSF neutral-addition diagnostic
must preserve BP110's existing observation and explain that GSF itself can
already recover material loss. The provisional recovery.py execution failed
before producing any plot (circular import); no recovery figure was added to
the gallery. That attempt was paused during this clarification. All existing
plots and native fits remain unchanged; no numerical result is invalidated by
this correction to the explanation.

## Corrected neutral-recovery comparison

Two additional figures retain BP110 exactly as fitted and add the same saved
nearby neutral observations only to KF and GSF, with dashed bare KF/GSF
controls. All10,804 events/21,608 tracks remain; no observations are missing.
2,325 events have2,734 unique neutral clusters, none shared between legs.
Collection is the existing pre-reclustering charge-zero trackless-PFO ECAL
window10mrad theta/200mrad phi around original KF AtCalorimeter position.
Each massless cluster uses(E,E*r/|r|) from nominal IP. Recoil adds the union
once; pT adds each cluster to its owning electron. No truth cluster matching,
new energy threshold or fit is introduced. Recovered pT is not track-only
resolution. BP110 receives no additional neutral four-vector.

Inclusive pT central68 half-widths: KF+neutrals1.479%, BP1100.690%,
GSF+neutrals0.718%. Hard tracks:3.697%/1.919%/2.333%. Recoil120--130GeV
counts:7,493/8,165/7,942; undefined22/2/132, retaining the full10,804-event
denominator. GSF can already recover material loss; blind neutral addition
can double count it. BP110 retains truth location and ECAL inputs. This
diagnostic does not establish equal information or held-out validation.

Exact provenance/event/leg identity, cluster uniqueness/masslessness,
per-leg/pair vector closure, bare-recoil reproduction, zero-addition
invariance and histogram counts pass. All60 preceding PNG/PDFs and source
caches remain unchanged. Parent inspected both final layouts. Artifacts:
recovery.py/log, summary_neutrals.json, PLOTS_NEUTRALS_READY and
{pt,recoil}_bp110_gsf_no_fsr_ebrem_neutrals PNG/PDFs.

A separate read-only job1 ROOT check verifies the saved sum flag on42/399
selected tracks:41 mask3/result2 and one mask2/result2, excluded by the
comparison gate. This check is inclusive in FSR, not a count of the selected
no-FSR sample. The sum is conditional, not universal across BP110/null fits.
The neutral plot construction reuses caches and does not read ROOT files.

Dedicated plot-gallery agent and parent manifest review confirm32PNGs across
10sections, exact-once coverage; recovery recoil/pT precede the two bare
figures. All30 prior PNGs and other section descriptions/order are preserved.
Existing ailogin002 PID2122174 serves localhost8000; homepage and both new
PNGs return HTTP200 and image bytes match final summary hashes.
