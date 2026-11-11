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
fitted loss, with tracker+beam+charged ECAL constraints. It is not a scan
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
