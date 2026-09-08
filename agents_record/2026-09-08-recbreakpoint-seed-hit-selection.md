# RecBreakpoint configurable three-hit seed selection

## Contract

User requested configurable seed hits with first/middle/last as default.
`RecBreakpoint.SeedHitSelection` now accepts `FirstMiddleLast` (compiled and
dedicated-card default) or `FirstThree` (previous behavior). Selection operates
on usable two-dimensional hits in outward radius order. The middle is ordinal
`N/2`, choosing the upper middle for even N, not the radial midpoint.
One-dimensional hits are skipped. Fewer than three usable hits fails the track;
an unknown mode fails initialization. No truth-dependent fallback is used.

The native three-hit prefit, loose FullLDCTracking-style covariance scaled by
`SeedScale`, first-hit update, subsequent filtering and RTS equations are
unchanged. The independent native-KF reference uses the same selected seed.
Flat branches `seed_hit_selection` and `seed_hit_indices` record the choice.
The dedicated card exposes `BP_SEED_HIT_SELECTION` as an optional environment
override. Existing GSF/shared KF sources and maintained GSF cards were untouched.

## Mechanical checks

Built and installed RecBreakpoint in the EL9/LCG105 environment; generated
configurable reports `FirstMiddleLast`. Standalone transport/selection CTest
passed, including odd/even hit counts, skipped-index lists, minimum hit count,
legacy selection and invalid input. Runtime `MiddleOnly` initialization fails
with the expected allowed-values error.

Same-code verbose runs used seed 12 from
`gsf_doublebhoff_freshseed_diagnostic/trk-e--2.0-85-12.root`, zero-based events
3/11/16/17, unchanged loss prior mean 0 and sigma 0.05, SeedScale 1,
multiple scattering on, deterministic energy loss off. Both seed choices were
run for each event. All eight jobs succeeded; legacy results reproduce the
previous stored pT values within 1e-8 GeV (identical printed doubles).

| Event | Breakpoint intervals | Truth pT (GeV) | FirstThree pT | FirstMiddleLast pT |
|---|---|---:|---:|---:|
| 3 | [4] | 42.086566925 | 37.049299230 | 36.970100575 |
| 11 | [10] | 9.151077271 | 9.125469494 | 9.125044417 |
| 16 | [] | 38.360702515 | 38.250953124 | 38.250515169 |
| 17 | [2] | 31.755603790 | 17.481638625 | 17.466338203 |

FirstMiddleLast selects [0,116,231] for event 3 and [0,116,232] for the others;
FirstThree selects [0,1,2]. An additional four-event empty-interval run succeeded
with the new default. Its maximum relative difference from independent native
MarlinTrk pT is approximately 4.49e-5 (event 3), not the tighter historical
three-event gate. Other differences are below 5.2e-7. This is a mechanical
comparison, not proof of numerical identity for every seed and event.

Artifacts, verbose logs and test runner are under
`TrackingPerformanceStudies/recbreakpoint_seed_selection_2026-09-08/` and remain
uncommitted. Twelve successful fit rows across nine jobs were checked for
status and persisted seed indices.

## Interpretation and reproduction

Changing the initial three-hit geometry does not repair event 3's large bias.
With the new default, early updates still produce forward kappa near -0.159055
at hit 4, far from the subsequently smoothed value near -0.0270488. The existing
frozen-linearization limitation is not repaired by this configuration change.
Event 17 is a historical secondary-activity control, not a clean-population
optimization case. No physics improvement claim is made from these examples.

To reproduce earlier dated results, explicitly select `FirstThree`; new
unsteered runs use `FirstMiddleLast`.
