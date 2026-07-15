# KL reduction-aware Gaussian-sum smoother — 2026-07-15

## Decision

The temporary retained-path smoother and the optional weight-rank reducer were
removed. `RecGsfTracking` now has one mixture reducer, KL moment merging, and
one optional `GaussianSumSmoothing` implementation. The independent
`ReverseFiltering` workflow remains available and unchanged.

Historical TopN experiments remain in dated records as rejected evidence, but
there is no TopN function, public header declaration, `ReductionMode`
property, steering assignment, environment control, or live documentation in
`Reconstruction/RecGsfTracking`.

## Forward reduction graph

When smoothing is enabled, every accepted measurement posterior is stored as
a graph node containing its filtered state/covariance, exact MarlinTrk
prediction, composed BH-plus-transport Jacobian, predicted covariance, and
physical parent node. Before KL reduction each component identifies its own
measurement node. Pairwise KL moment merges combine those source identifiers
with the same normalized weights used for state moment matching. Each final
reduced component becomes a new same-surface graph node pointing to all
pre-reduction contributors.

The graph is bounded by the actual expanded posterior mixtures and reduced
component count. It does not copy complete hidden trajectories through every
later split.

## Backward recursion

Terminal KL components initialize the backward mixture with their normalized
all-measurement weights. Processing the directed acyclic graph in reverse:

1. At a KL node, each pre-merge contributor is conditioned on the smoothed
   reduced Gaussian using its filtered covariance times the inverse reduced
   covariance. Its backward weight is the reduced-node weight times its stored
   KL contribution fraction.
2. At a physical inter-surface edge, the exact RTS gain uses the parent
   filtered covariance, accepted composed transition Jacobian, and exact
   predicted covariance.
3. All candidates arriving at a common node are moment-matched, including
   between-candidate covariance.
4. The innermost active smoothed mixture is moment-matched and extrapolated
   geometrically to the IP. The dedicated smoother card publishes
   `GSFOutputMode=WeightedMean`.

The inverse treatment of a KL moment merge is necessarily an approximation:
moment reduction is information-losing. It preserves contributor identities
and conditional fractions rather than pretending the retained representative
history is the whole merged component.

## Removed implementation

- `GsfMixture::reduceTopN` and both public overloads.
- The `ReductionMode` Gaudi property and all forward/reverse branches on it.
- TopN steering and its environment variable.
- `RetainedLineageSmoothing`; the property is now
  `GaussianSumSmoothing` and the environment control is
  `GSF_GAUSSIAN_SUM_SMOOTHING`.
- The obsolete `run_gsf_lineage_smoothing_event11.py` card.
- Live README and algorithm-diagram references to rank pruning.

## Validation

The configured EL9/LCG-105 target builds and installs.

On exact-pair event 11 from `/tmp/gsf-match-tracks.root`, the KL smoother:

- retained 234/234 hits and produced 12 final forward components;
- built 2,966 graph nodes;
- propagated a finite backward solution through 2,772 active nodes, including
  50 active KL reduction nodes;
- published weighted-mixture pT 1.7938 GeV versus LCIO 1.7938 GeV and truth
  2.0004 GeV;
- had no graph, inversion, covariance, or output failure.

Thus the corrected smoother still does not recover momentum on this event.
This is mechanical validation, not physics validation.

The preserved reverse filter was rerun after TopN/property removal. It retained
234/234 hits, ended with 12 components, and published 1.9838 GeV. Events 16 and
17 remain unavailable because the local exact-pair file ends at event 11.

## Five-event focused extension

The installed final interface (`GaussianSumSmoothing`, KL only) was run on
exact-pair events 3, 5, 7, 9, and 11. All five completed every available hit,
kept 12 final components, and had finite graph and backward solutions:

| Event | Hits | Graph/active/KL nodes | Truth pT | LCIO pT | Smoothed pT |
|---:|---:|---:|---:|---:|---:|
| 3 | 233/233 | 3108/3006/55 | 2.0004 | 1.7980 | 1.8725 |
| 5 | 233/233 | 3119/3055/59 | 2.0004 | 1.9237 | 1.9238 |
| 7 | 233/233 | 2897/2759/52 | 2.0004 | 1.9999 | 1.9993 |
| 9 | 232/232 | 3036/2942/46 | 2.0004 | 2.0040 | 2.0034 |
| 11 | 234/234 | 2966/2772/50 | 2.0004 | 1.7938 | 1.7938 |

Event 3 demonstrates that the smoother can transmit a material-loss correction
backward through the KL graph: it recovers about 0.0745 GeV of the roughly
0.2024 GeV LCIO deficit. Event 11 has a similar LCIO deficit but no recovery.
The remaining three are effectively unchanged at this precision. This small,
selected set establishes nontrivial operation and heterogeneous sensitivity;
it does not establish resolution improvement.

## Six-event smoother versus reverse-filter comparison

Six additional locally available exact-pair events were each reconstructed
independently with `GaussianSumSmoothing` and `ReverseFiltering`. All 12 jobs
completed every available hit with finite output.

| Event | Hits | Truth pT | LCIO pT | Smoother pT | Reverse pT |
|---:|---:|---:|---:|---:|---:|
| 0 | 233 | 2.0004 | 2.0007 | 2.0001 | 2.0005 |
| 2 | 233 | 2.0004 | 1.9950 | 1.9944 | 1.9950 |
| 4 | 233 | 2.0004 | 1.9993 | 1.9987 | 1.9992 |
| 6 | 233 | 2.0004 | 1.9749 | 1.9744 | 1.9749 |
| 8 | 233 | 2.0004 | 2.0012 | 2.0005 | 2.0013 |
| 10 | 234 | 2.0004 | 2.0006 | 1.9999 | 1.9707 |

Using the displayed pT precision, the mean absolute truth errors are about
0.00555 GeV for LCIO, 0.00577 GeV for the smoother, and 0.01047 GeV for the
reverse filter. Median absolute errors are approximately 0.00095, 0.00110, and
0.00330 GeV. These are selected-event diagnostics, not population estimates.

The smoother makes only small changes here: it clearly improves already-clean
event 8, marginally changes events 0/10, and slightly worsens events 2/4/6.
The reverse filter is identity-like on events 2 and 6 and close to LCIO on
0/4/8, but creates a false correction on clean event 10, moving 2.0006 to
1.9707 GeV. This directly reinforces the existing minority radiative-selection
problem and the need for clean-track safety validation.
