# Passive smoothed products move into the reverse surface step

Date: 2026-08-29

## Decision

Interior passive products are now materialized inside the corresponding live
reverse-surface step:

```text
B_updated[i+1]
  -> propagate and BH-split
  -> evaluate and buffer B_predicted[i] measurement candidates
  -> B_smoothed[i] = F_updated[i] x B_predicted[i]   (passive)
  -> commit the buffered measurement candidates
  -> B_updated[i]                                   (live)
```

The smoothed product and live measurement update therefore branch from the
same evaluated prediction. The product is no longer deferred until the full
inward recursion has completed. It remains passive: it does not change a live
component, component weight, cutoff, KL reduction, endpoint selection, or
published track.

## Implementation contract

Each temporary baseline MarlinTrk `addAndFit` operation is still evaluated
once. Its exact predicted state/covariance and updated state are buffered in a
surface-local candidate. The predicted fields build the passive product before
`appendBaselineStateToComponent` commits the updated state to the live reverse
component. No parallel Kalman measurement formula was added.

The smoothing edge from the backward side now correctly originates from the
pre-measurement reverse node representing `B_predicted[i]`: normally the BH
split node at hit `i`, or the preceding reverse-update node at `i+1` when no
split was made. It no longer originates from the measurement node representing
`B_updated[i]`. The forward parent remains the immutable
`F_updated[i]` snapshot.

The boundary contract is unchanged:

```text
B_smoothed[0]   = B_updated[0]
B_smoothed[N-1] = F_updated[N-1]
```

Only `0 < i < N-1` has an explicit product mixture. The product candidates,
cutoff survivors, and KL outputs retain lineage source 3; retained product
survivors remain inward-internal messages and cannot be endpoint members.

No EDM collection, flat branch, configurable property, allowed value, or
compiled default changed.

## Mechanical regression gate

The focused EL9/LCG-105 `RecGsfTracking` and `RecGsfFlatTuple` build and full
install completed successfully. A verbose fresh-seed event-11 run showed each
`SMOOTHED MIXTURE hit=i` record before the first corresponding
`REVERSE UPDATE accept hit=i` record and terminated normally.

Stored pre-change tuples were used only as exact mechanical references. The
new installed binary reran the same inputs and steering:

- maintained five-component model, fresh inward seed `-1`, hard-loss events
  11, 16, and 17 from `trk-e--2.0-85-1.root`;
- BH15, `MaxComponents=10`, cutoff `1e-4`, fresh inward seed `-1`, events
  job12/entry2 and job32/entry31 from the earlier two-percent-loss panel;
- the same two BH15 events with copied inward scale `1`.

Across these seven configuration/event pairs, all 64 BestBranch,
WeightedMean, FullMixtureMode, and final-mixture-component branches were
bit-for-bit equal to the pre-change tuple. The maintained hard-event endpoint
values remained:

| Entry | Truth pT | LCIO pT | BestBranch pT | WeightedMean pT | FullMixtureMode pT |
|---:|---:|---:|---:|---:|---:|
| 11 | 40.7315674 | 40.8954541 | 40.8956875 | 41.3707442 | 40.9034282 |
| 16 | 37.8940163 | 18.2928319 | 18.2872217 | 18.2872149 | 18.2872149 |
| 17 | 18.7969780 | 14.8066689 | 18.6673649 | 18.9729201 | 18.6676807 |

After removing source-3 diagnostics and remapping node identifiers per input
track, every live lineage-node field and every live edge was exactly equal.
The compared live/smoothed node counts were respectively 4,883/14,848,
2,814/3,800, and 4,073/10,852 for hard events 11, 16, and 17, and
8,303/23,347 and 9,154/27,710 for the two BH15 events. Grouping source-3 nodes
by hit made every numerical smoothed-node field exactly equal as well; only
their global serialization order and intended backward-parent edge changed.

Event 16 retains the already-known FullMixtureMode status `-1` fallback and
exact prior fallback value. This gate proves mechanical equivalence of the live
fit and correct timing/parentage of the passive diagnostic. It is not physics
validation and does not resolve the event-16 endpoint-mode failure.
