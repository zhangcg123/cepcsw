# GSF forward surface-workflow milestone

Date: 2026-07-10

This record preserves completion of the surface-local forward component
workflow and the evidence used to move the active focus to navigation and
reverse multi-component filtering.

## Implementation

`GsfComponent` now separates two concepts that were previously conflated:

- the Kalman track contains the real filtered measurement history;
- an `edm4hep::TrackState` continuation snapshot contains the state used to
  propagate after a surface-local process transition.

The current forward sequence is now:

```text
continuation state from previous surface
  -> MarlinTrk prediction and measurement update at current hit
  -> preserve filtered measurement state in history
  -> copy filtered state into continuation snapshot
  -> apply the existing BH convolution to continuation snapshots only
  -> low-weight cutoff and current-surface KL reduction
  -> initialize propagation to the next measurement from continuation state
```

`BetheHeitlerSplitter` retains its existing mixture calculations but no longer
rewrites every state stored in the preceding `TKalTrackSite`. Verbose output
prints the unchanged filtered kappa and distinct post-process continuation
kappa for every child.

KL distance and moment merging now use only component states expressed at the
common continuation surface. One real measurement history is kept as the
representative branch; unrelated historical sites are no longer moment-merged.
Periodic phi is unwrapped locally before moment matching. The former absolute
determinant rejection threshold of `1e-12` was removed because valid mixed-unit
five-dimensional track covariances have determinants around `1e-22`; only
non-positive or non-finite determinants are rejected.

A normalized `ComponentWeightCutoff` property was added with default `1e-8`.
It preserves at least the largest component, then renormalizes before component
cap reduction.

## Focused validation

The target builds and installs successfully. Event 11 was first run with
`MaxComponents=30`, preventing reduction through the first two process
transitions. At hit 1 the filtered parent had kappa `-5.5762e-01`; all five
children retained that filtered measurement kappa while their continuation
kappas became:

```text
-1.5246e+00, -8.2185e-01, -5.7192e-01, -5.6042e-01, -5.5765e-01
```

At hit 2 all five components were accepted by the exact MarlinTrk update and
then produced 25 distinct post-process continuation states. No reduction
occurred during this two-step gate.

With the normal `MaxComponents=12` KL policy, event-11 hit-2 reduction produced
finite ordered symmetric KL distances beginning at `3.5487e-04`; the former
`1e30` sentinel sequence was eliminated.

Complete verbose focused results were:

| event | hits | truth pT [GeV] | LCIO pT [GeV] | GSF pT [GeV] | splits | reductions |
|---:|---:|---:|---:|---:|---:|---:|
| 11 | 234/234 | 2.0004 | 1.7934 | 1.7933 | 4 | 3 |
| 16 | 234/234 | 2.0004 | 1.8118 | 1.8118 | 4 | 3 |
| 17 | 234/234 | 2.0004 | 1.5790 | 1.5789 | 4 | 3 |

Each run ended with twelve components and no measurement-update rejection.
This validates forward workflow stability, not interaction-point momentum
recovery or the CEPC material/BH physics model.

## Remaining tracking-workflow work

1. Audit the current radius-based hit ordering against actual signed path order
   for events 11, 16, and 17. Establish explicit surface identity and detect
   repeated/non-monotonic crossings before replacing navigation.
2. Define start/final-surface ownership so process transitions cannot be double
   counted between forward and reverse passes.
3. Implement a genuine reverse multi-component filtering pass initialized from
   the final forward measurement mixture, with reverse hit traversal, exact
   measurement updates, direction-aware process transitions, cutoff, and
   current-surface reduction.
4. Produce and validate the mixture on the IP reference surface. Best-branch
   output remains the safe default until weighted-mixture covariance and fit
   metadata have one coherent definition.
5. Only after the workflow is validated should component-local path-corrected
   material and a CEPC step-conditioned BH model be assessed as physics changes.
