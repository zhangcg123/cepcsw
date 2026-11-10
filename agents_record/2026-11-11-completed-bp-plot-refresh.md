# Updated BP panels from completed scan-off production

The user asked to update the BP plot panels using newly finished tuples.
This is read-only offline analysis: no simulation/reconstruction/fitter rerun,
source/library change, batch submission or job interruption.

## Inputs and preservation

A fixed UTC snapshot at 2026-11-10T20:24:10.918145+00:00 contains all171 then-completed
BP_NOSCAN_PREECAL500x200 files, each200 sequential event rows:34,200 events.
Running/failed/partial files and jobs finishing after the freeze are excluded.
All source sizes/mtimes agree before/after reads and at final verification.
The previous complete41MB plot directory, caches and notes are copied to:
`/aifs/user/data/zhangcg/gsfdev/CEPCSW/.analysis_archive/bp_gsf_before_completed_bp_refresh_20261110T202404Z`.
The outgoing AGENTS is preserved byte-for-byte in
[the snapshot](2026-11-11-agents-before-completed-bp-plot-refresh.md).

The eight existing PNG/PDF figure names are refreshed in
`TrackingPerformanceStudies/bp_matched_recoil_pt_20261110/`.
Inclusive curves are Baseline KF/BP100; no-FSR/eBrem curves additionally use
BP110. Earlier matched GSF overlays used the previous50-event sample and are
archived, rather than mixed with this larger BP-only population. New GSF
matching is a later update, not part of this task.

## Definitions and gates

Baseline KF is the original CompleteTracks AtIP state from the matching REC.
BP100 uses beam_spot_free_loss_rts_*; BP110 uses charged_ecal_free_loss_rts_*.
Truth selects the eligible location or null; loss amount remains fitted.
No scan minimum or truth-loss-energy override. Require result1/2,
constraint_status2, requested AND applied mask1/3, five finite native IP
parameters/positive pT,3T curvature closure and the same selected truth interval.
Input-KF or earlier-stage fallback copies are excluded. Each figure uses one
valid track/pair intersection for all displayed methods. BP110 alone uses ECAL.

Material fraction is recorded primary-electron material-photon birth energy /
matched stable post-FSR generator-electron energy. Parent G4 identity is saved,
not inferred angularly. FSR comes directly from saved generator-origin links.
No photon-energy threshold. pT no+light is f<1%, hard f>=1%; recoil hard if
either leg>=1%. The isolated selection requires exactly two production-electron
tracks, no FSR on either and any eBrem on>=1 leg, before fit validity. Both
legs, including a zero-eBrem partner, remain in pT. No unknown-FSR tracks or
excluded truth-identity/material-owner rows occur in this snapshot.

Recoil uses nominal(240,0,0,0)GeV minus both electron four-vectors (electron
mass included), with each method's own fitted directions. No added photons
or generator-recoil curve. All recoil plots show120--130GeV,0.25GeV bins,
linear/log-y. pT windows remain[-5,5]% and[-100,200]%log-y. Undefined masses and
finite outliers remain in full denominators; no window normalization. No new
tail-fraction, quantile or control figures.

## Coverage

| View | Selected tracks | Common tracks | Selected opposite-charge pairs | Common pairs |
| --- | ---: | ---: | ---: | ---: |
| Inclusive KF/BP100 | 67,819 | 64,481 | 33,562 | 30,340 |
| No FSR+eBrem KF/BP100/BP110 | 9,896 | 9,041 | 4,937 | 4,133 |

| Common population | No+light | Hard |
| --- | ---: | ---: |
| Inclusive pT | 34,064 | 30,417 |
| Inclusive recoil | 8,642 | 21,698 |
| No-FSR/eBrem pT | 4,540 | 4,501 |
| No-FSR/eBrem recoil | 973 | 3,160 |

There are34,198 events with selected tracks;2 have none,577 lack one partner.
The no-FSR/eBrem selection has4,948 two-track events before opposite-charge
selection;11 have the same baseline charge. Conditional samples differ from
the older GSF-valid intersection, so old/new histogram differences are not a
same-code A/B improvement claim. Completion-time sampling and fit-success
conditioning also preclude a population-validation claim.

## Distribution checks and limits

The following are full finite-distribution central68% half-widths, not Gaussian
peak fits or intrinsic recoil resolutions:

| View/category | pT KF/BP100/BP110 [%] | Recoil KF/BP100/BP110 [GeV] |
| --- | --- | --- |
| Inclusive no+light | 0.24791/0.26665/— | 7.09977/6.97091/— |
| Inclusive hard | 14.24282/7.12420/— | 16.40859/13.75060/— |
| No FSR+eBrem no+light | 0.25895/0.27883/0.34070 | 2.29935/2.29392/2.60524 |
| No FSR+eBrem hard | 11.66620/5.79961/3.29908 | 10.51290/7.79236/7.04404 |

All4,080 overlapping old cached tracks agree in truth identity, radiation tags,
energies, generator pT and baseline pT; all41 overlapping REC identities agree.
All171 event trees contain sequential entries0..199; unique track keys,
constraint/interval/curvature gates, synthetic1% boundary and recoil convention,
category sums and histogram counts pass. Four representative layouts were
inspected. Full undefined-mass and rejection counts are preserved in the
current JSON/CSV artifacts. No implementation or native fit verification is
claimed from these offline histogram checks.

Artifacts (uncommitted): refresh_completed_bp.py, snapshot_completed_bp.json,
tracks_completed_bp.csv, pairs_completed_bp_{inclusive,no_fsr_ebrem}.csv,
summary_completed_bp.json, validation_completed_bp.json, completed_bp_refresh.log,
README, eight PNG/PDF pairs and gallery.json. Re-run the new script from the
repository root to reproduce the frozen snapshot; older scripts/caches remain
historical. Server organization/launch verification is delegated to the
personal plot-gallery skill; its verified URL/PID is recorded below once ready.

## Active production and prior work

Both new500x200 BP and GSF campaigns have been submitted/started by the user.
This analysis does not interrupt them or alter their frozen runtime. A prior
status check identified GSF55 exit139 (segmentation fault); no fix/retry here.
Live queue tools remain unavailable locally. Further source/build/install
changes must preserve running jobs' normal/private dependencies.
Previous definitions/results and FSR-membership limits remain in
[the matched comparison record](2026-11-10-matched-gsf-bp-overlays.md) and
[the FSR survey](2026-11-10-fsr-cluster-membership.md).

## Verified gallery

Personal plot-gallery delegated the complete manifest/server workflow to
`bp_completed_gallery_20261111` (gpt-6-luna/high). It updated only gallery.json,
validated eight PNGs covered exactly once, safely replaced the prior managed
server and verified the homepage plus a current PNG whose SHA256 agrees with
disk. Host ailogin002.ihep.ac.cn, PID2122174, localhost port8000, URL
http://127.0.0.1:8000/. The main agent reviewed the manifest against the new
README/summary and inspected four representative image layouts.
