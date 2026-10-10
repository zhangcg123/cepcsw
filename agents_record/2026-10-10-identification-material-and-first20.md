# Baseline interval identification: first 20 tracks and reconstructed t/X0

Input: `sim_large_barrel_20261001/rec-barrel-1.root`, zero-based entries
0--17, 20 CompleteTracks rows, 4,194 hits / 4,174 adjacent intervals.
Events 2 and 13 have two tracks each and are control events, excluded from
the single-track observations below. These are NOT the earlier 20 ECAL-study
tracks drawn from other seeds.

## Identifiability observations

Sixteen single-track events remain: nine have positive recorded eBrem momentum
loss within the hit span; seven have none within that span. No topology-clean
population selection or external efficiency validation is claimed here.
Loss percentages below divide the largest interval's truth momentum loss by
the associated generator particle momentum. Native local chi2 is the backend
measurement-update score, not a bremsstrahlung probability. For interval i to
i+1, forward is scored at i+1 and backward at i. Rank 1 is the largest score
among eligible adjacent-interval scores of that track (not a trained selector).

| Entry | Largest loss % | Interval | Forward chi2 / rank | Backward chi2 / rank | Reco chord t/X0 |
|---:|---:|---|---:|---:|---:|
|3|43.639|229->230|3.49 / 36|0.52 / 174|0.00006075|
|5|19.450|3->4|0.14 / 215|0.70 / 172|0.004065|
|6|0.0511|9->10|0.91 / 138|0.25 / 202|0.014172|
|7|0.7042|5->6|1.59 / 104|8.27 / 4|0.008944|
|9|0.1721|230->231|2.74 / 49|~0 / 231|0.017894|
|14|0.0211|143->144|0.68 / 174|0.08 / 221|0.00004971|
|15|0.00832|231->232|2.91 / 56|~0 / 232|0.015188|
|16|0.0998|8->9|1.09 / 135|8.36 / 2|0.010494|
|17|0.3784|8->9|0.48 / 181|0.20 / 206|0.011288|

The absence of a maximum at the true interval does not mean the sequence has
no information. Entry 5 has backward local chi2=24.00 at hit 4, adjacent to
the truth interval; entry 3 has forward chi2=46.64 at hit 232, after the loss.
Conversely, the seven events without in-span eBrem have maximum local scores
of 8.51--14.59 forward and 9.89--14.20 backward. A simple maximum/threshold
rule is not a demonstrated interval finder. Use neighbouring signed residuals,
curvature, uncertainties and geometry jointly; retain no-loss and multi-track
controls. Independent F/B passes use prefits informed by both endpoints, so
their compatibility is not automatically an independent-evidence probability.

## Added default-on geometry feature

`IdentificationMaterial.h` performs separate outward/inward DD4hep scans
between adjacent reconstructed hit positions. It preserves the starting-volume
coverage repair used by GSF (small inward nudge plus accounted-for cap).
It does not use truth and does not modify the baseline fitter.

Schema 2 adds `interval_chord_tx0`, `interval_chord_reverse_tx0`, and paired
coverage/status/segment-count branches. The chord is not the actual curved
native KF trajectory; KF process-noise matrices remain separate. Invalid paths
are flagged and have NaN t/X0. No direction averaging hides disagreements.
Full contract: `Reconstruction/RecBreakpoint/docs/interval-identification-tuple.md`.

## Verification

- Rebuilt RecBreakpoint in the EL9/LCG105 build tree; no shared install or batch.
- Repeated entries 0--17 with verbose baseline dumps, including 11,16,17.
- Output `/tmp/bpid-material-on.root`; truth-disabled paired run
  `/tmp/bpid-material-off.root`. Both terminate successfully.
- All 4,174 outward and inward material scans valid, full chord coverage.
  Outward status 1/2 counts: 2,318/1,856; inward: 1,742/2,432.
- Maximum outward/inward relative t/X0 difference 4.163e-12; median 2.08e-14.
- Every pre-existing tuple branch exactly equals `/tmp/bpid-validated-on.root`
  (NaN-aware comparison); no tracking or truth-label regression.
- Schema/matrix/ownership checks pass, and every non-truth branch is exactly
  equal between the new truth-on and truth-off runs.

This establishes mechanical correctness on this sample, not general endcap
navigation correctness or interval-identification performance.
