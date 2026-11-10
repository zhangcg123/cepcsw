# BP panels refreshed to498 completed jobs

User request: update the existing plots with newly finished BP tuples. A fixed
snapshot at 2026-11-10T20:38:52.145482+00:00 contains498 completed files x200 events:
99,600 input events. The two jobs without completion markers are excluded;
no running/partial/failed tuple is read. Files finishing after the snapshot are
not added automatically. All eight existing PNG/PDF figures are regenerated.
The previous171-file gallery, metadata and caches are preserved at:
`/aifs/user/data/zhangcg/gsfdev/CEPCSW/.analysis_archive/bp_completed_refresh_before_20261110T203843Z`.
Complete outgoing AGENTS:
[the snapshot](2026-11-11-agents-before-bp-498-refresh.md).
Earlier definitions and0.1GeV rebin history:
[the preceding refresh record](2026-11-11-completed-bp-plot-refresh.md).

The selections, fit gates, estimator definitions and display settings are
unchanged: inclusive Baseline KF/BP100; no-FSR/eBrem Baseline KF/BP100/BP110,
no+light/hard split at1% material-photon birth energy / post-FSR stable-electron
energy; truth-selected location with fitted loss; ECAL only for BP110.
No generator recoil or photon addition. Each method uses its own native IP
direction. Successful full constraint masks1/3 are required; fallback copies
are excluded. Every panel shares the same valid comparison population.
Recoil remains120--130GeV with100 bins of0.1GeV; no tail/quantile plots.

## Incremental extraction and verification

171 unchanged files reuse the preceding verified track cache after exact
manifest/ROOT/REC identity checks.327 newly completed files are read with all
existing event-order, track-key, identity, truth, endpoint-constraint and
curvature gates. A direct fresh read of job103 agrees exactly with all399
cached rows/fields, including NaNs. Previous extraction counts are preserved
and combined with new counts; all498 files account for200 events each.
There are197,637 selected truth-known tracks in99,593 events;7 events have
none. No truth-identity/material-owner exclusions and0 unknown-FSR tracks.
Saved generator-origin FSR tags are used directly, without angular inference.
All19,757 overlapping historical cached tracks and199 old REC identities agree.
Final source identities, unique keys, category sums and histogram counts pass.
Two representative refreshed layouts are visually inspected.

## Coverage

| View | Selected tracks | Common tracks | Selected opposite-charge pairs | Common pairs |
| --- | ---: | ---: | ---: | ---: |
| Inclusive KF/BP100 | 197,637 | 187,996 | 97,910 | 88,598 |
| No FSR+eBrem KF/BP100/BP110 | 29,172 | 26,632 | 14,565 | 12,148 |

| Common population | No+light | Hard |
| --- | ---: | ---: |
| Inclusive pT | 98,464 | 89,532 |
| Inclusive recoil | 24,769 | 63,829 |
| No FSR+eBrem pT | 13,186 | 13,446 |
| No FSR+eBrem recoil | 2,791 | 9,357 |

Inclusive pairs exclude1,549 missing partners,134 same baseline charges and
9,312 unavailable full BP100 pairs. No-FSR/eBrem selects14,586 two-track events
before charge/fit gates;21 same-charge and2,417 unavailable full BP100/BP110
pairs are excluded. Undefined recoil masses and finite tails remain in full
normalization denominators; there is no visible-window renormalization.
This is a completed-job snapshot conditional on successful fits, not a final
population/physics validation or a new same-code A/B selection claim.

Current uncommitted artifacts in
TrackingPerformanceStudies/bp_matched_recoil_pt_20261110/: updated snapshot,
track/pair CSVs, summary/validation JSON, README, refresh_completed_bp.py,
incremental_cache_check.txt, log, eight PNG/PDF pairs and gallery.json.
The analysis script now reuses a preceding frozen cache only when every
source identity agrees; it is an analysis artifact, not fitter implementation.
The original GSF overlay archive remains preserved separately; old50-event
GSF curves are not mixed with this larger BP population.

No simulation/reconstruction/fitter rerun, production source/library/card
change, batch submission or job interruption. The personal gallery agent
handles the manifest/server validation separately; verified state follows.

## Verified gallery

The personal plot-gallery agent updated only gallery.json to the498-job
snapshot and current category counts, preserving0.1GeV bins and section order.
All eight PNGs are covered exactly once. The main agent reviewed the manifest
against the current summary. Live homepage counts and an updated recoil PNG
SHA256 agree with disk. The healthy existing server remains on
ailogin002.ihep.ac.cn, PID2122174, http://127.0.0.1:8000/.
