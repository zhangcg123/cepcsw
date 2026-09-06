# Archived hit-0 single-loss-bank design

Date: 2026-09-06

This was the active design immediately before the reverse-filter look-ahead
weight experiment replaced it as the current focus. It remains a possible
later direction, but it was not implemented or validated.

The proposal was to replace premature surface-local competition with a
bounded hypothesis bank inside the reverse GSF, without reviving the retired
standalone global-loss profiler. One no-radiation identity backbone would be
kept. At each eligible inward material interval, only that backbone would emit
the configured non-identity BH children. Each child would represent exactly
one radiative interval, would never split again, and would propagate through
every remaining inner measurement to hit 0 without posterior cutoff or KL
merging against other histories.

At hit 0, the identity and every single-loss history would be compared using
one complete path score: the BH log-prior of the interval history plus the
accumulated measurement log-likelihood from all hits. The intended question
was whether a low-prior light-loss child, ambiguous beside the loss interval,
could be recovered by the complete remaining inner-hit evidence. Multiple-
loss histories were deliberately excluded from the first version, bounding
the active bank near
`1 + N_intervals * (N_BH_components - 1)` instead of exponential growth.

Open design requirements were state/weight ownership, the exact loss-operator
interval, full-path prior normalization, endpoint publication, and memory.
Persisting every intermediate lineage node would scale approximately as
`N_intervals^2 * N_BH_components`, so an eventual implementation should keep
only the active states and compact history summaries needed to audit the
hit-0 choice. The retired `RecGsfGlobalLossRefitter` and its historical
flat-tuple/card adapters were explicitly not the proposed implementation base.

The motivating F/B trigger studies did not validate a hit-0 trigger. In 199
topology-clear compiled-double-off events, the naive event-level maximum score
had ROC AUC 0.484 and was dominated by hits 220--230. At exact saved intervals
its AUC was 0.539; a nominal one-sided two-sigma cut gave 8.37% Type-I and
89.68% Type-II error. Direction-local three-hit prefits improved Type-II from
72.92% to 58.33% at approximately unchanged Type-I for a 0.2% truth-loss floor
and plus/minus-one-interval tolerance, but every newly recovered interval was
in the outer boundary group. Hits 5--219 were unchanged. The full 5D score
reduced Type-I from 11.55% to 10.60% at threshold 0.95 but worsened Type-II
from 58.33% to 63.54%; at equal Type-I count it missed 59/96 rather than
56/96. Exact evidence remains in
`2026-09-01-double-off-brem-score-errors.md`,
`2026-09-02-directional-three-hit-gsf-initialization.md`, and
`2026-09-02-forward-backward-5d-brem-score.md`.
