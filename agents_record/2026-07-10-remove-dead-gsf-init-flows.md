---
name: remove-dead-gsf-init-flows
description: Concentrated recovery-status record after pruning ineffective GSF initialization paths
metadata:
  type: change-record
  date: 2026-07-10
---

# GSF Recovery Focus After Flow Cleanup

Removed two diagnostic GSF initialization paths from `RecGsfTracking`:

- `UseCompleteTrackFirstHitInit` / `GSFInitialisationMode = "CompleteTrackFirstHit"`
- `GSFInitialisationMode = "BaselinePrefit"` / `"Baseline"`

They were removed because both only changed the numerical initial state and did not change the early direct-GSF recovery pattern. The active evidence points instead to the direct `TKalTrack::AddAndFilter` update workflow.

## Active Fit Flows

- `FitterMode = "KF"`: pure baseline `KalTestTool/KalTest111::Fit(...)` over all hits.
- `FitterMode = "GSF"`, `GSFInitialisationMode = "Seed"`: manual first site, then direct GSF `TKalTrack::AddAndFilter` updates.
- `FitterMode = "GSF"`, `GSFInitialisationMode = "BaselineEarlyFit"`: MarlinTrk fits the first `GSFInitialisationFitHits` hits, then direct GSF starts from the following hit.

## Recovery Evidence

Fresh comparison after cleanup used `trk-e--2.0-85-1.root`, events `10, 12, 14, 15`. The two GSF flows used `MaxComponents = 1`, `ReductionTargetComponents = 1`, `ReductionMode = "TopN"`.

| workflow | recovery result |
|---|---|
| Pure KF | no recoveries |
| GSF Seed, max1/topN1 | recoveries remain at early VXD hits: event 10 hits 1,2,3; event 12 hits 1,3; event 14 hits 1,2,3; event 15 hit 2 |
| GSF BaselineEarlyFit, max1/topN1 | no direct-GSF recoveries after the early baseline segment with `GSFInitialisationFitHits = 4` |

Logs:

```text
/tmp/codex_current_kf_events_10_12_14_15.log
/tmp/codex_current_gsf_seed_max1_topn1_events_10_12_14_15.log
/tmp/codex_current_gsf_early_max1_topn1_events_10_12_14_15.log
```

## Concentrated Conclusion

The immediate focus is the direct GSF component update path, not BH fitting or performance comparison.

- Pure KF succeeds because it stays inside the baseline `KalTestTool::Fit` workflow.
- GSF Seed fails even with one component, so the base recovery problem is not caused by BH splitting, mixture reduction, or TopN.
- `BaselineEarlyFit` works in max1/topN1 because it bypasses direct `AddAndFilter` for the first VXD hits.
- Full multi-component GSF can still recover later after splitting, so increasing `GSFInitialisationFitHits` is only a safeguard that moves the handoff point; it is not the real fix. The real implementation target is to make the GSF component update itself baseline-compatible, or replace the direct per-component update mechanism with something using the same site/update construction as the baseline workflow.
