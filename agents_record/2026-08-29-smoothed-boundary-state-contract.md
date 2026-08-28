# Smoothed boundary-state contract

Date: 2026-08-29

## Correction

The reverse two-filter sequence now treats its boundary states explicitly:

```text
B_smoothed[0]   = B_updated[0]
B_smoothed[i]   = F_updated[i] x B_predicted[i],  0 < i < N-1
B_smoothed[N-1] = F_updated[N-1]
```

The inner boundary is the terminal live inward posterior after measurement 0.
It supplies reverse BestBranch, WeightedMean, and FullMixtureMode. The outer
boundary is the final live outward posterior and is also the source population
for positive-scale inward initialization. Neither boundary needs a duplicate
source-3 smoothing node.

Only interior Gaussian products are constructed after the live inward
recursion. They remain passive lineage diagnostics: their cutoff, reduction,
and Gaussian-product algebra cannot influence the live inward states or any
published endpoint.

## Superseded behavior retained for provenance

Before this correction, the materialization loop included hit 0 and constructed
an additional source-3 state
`F_updated[0] x B_predicted[0]`. That state was not the terminal
`B_updated[0]`, was never published, and was deleted after its lineage record
was persisted. The outer boundary `N-1` remained empty in the explicit
smoothed-surface vector. The historical all-successful-inward-surface counts
and exact gates are preserved in
`2026-08-29-smoothed-diagnostic-only-publication.md` and
`2026-08-29-cms-like-workflow-retirement.md`; those counts include the former
hit-0 product and must not be used as expected counts for new tuples.

## Completed gate

The installed EL9/LCG 105 build completed for `RecGsfTracking` and
`RecGsfFlatTuple`. A same-card direct comparison of focused events 11, 16, and
17 against the pre-correction reverse tuple compared 88 endpoint, endpoint-
status, and final-mixture branches at the raw basket-byte level; all 88 were
exactly equal.

The new lineage has the following source-3 population:

| event | accepted hits N | source-3 nodes | operation-5 candidates | hit range | source-3 at 0 | source-3 at N-1 | published source-3 |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 11 | 234 | 20,571 | 19,414 | 1--232 | 0 | 0 | 0 |
| 16 | 224 | 3,214 | 3,191 | 1--222 | 0 | 0 | 0 |
| 17 | 232 | 6,329 | 6,312 | 1--230 | 0 | 0 | 0 |

The corresponding existing boundary populations were unchanged: events
11/16/17 retain respectively 120/106/120 source-2 nodes at hit 0 and
36/5/10 source-1 nodes at hit `N-1`. Relative to the old tuple, only the
redundant 60/56/55 source-3 hit-0 nodes disappeared. A verbose event-11 run
reported explicit smoothed mixtures from hit 232 down through hit 1, with no
hit-0 or hit-233 product, and finalized successfully. These checks establish
the boundary representation and endpoint non-interference; they are not a
physics-performance validation.
