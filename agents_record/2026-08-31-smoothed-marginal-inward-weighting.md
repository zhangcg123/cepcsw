# Smoothed-marginal live inward weighting

Date: 2026-08-31

## Superseded checkpoint

The outgoing active focus was the two-mode reverse inward initialization.
Positive `InwardSeedCovarianceScale` copies and scales the final forward
mixture; a finite nonpositive value builds one fresh standard-KF-style inward
seed, updates hit `N-1`, and first revisits `N-2`. Focused events 11, 16, and
17 established exact copied-path regression and equivalence of fresh values 0
and -1. That implementation remains active and is documented in
`2026-08-29-fresh-inward-standard-kf-initialization.md`; the maintained card
continues to select -1. The unfinished topology-clear scale-100 versus fresh
population comparison remains historical follow-up, but it is no longer the
immediate checkpoint.

## Decision

Add the two-value `InwardWeightMode` property:

- `LocalMeasurement` is the compiled and active reverse-template default. It
  preserves the established live reverse weight
  `prior(B_predicted) x likelihood(hit | B_predicted)`.
- `SmoothedMarginal` is an experimental maintained-card mode. At every
  interior surface `0 < i < N-1`, it forms all valid direct products
  `F_updated[i] x B_predicted[i]`, normalizes their exact overlap weights
  before product cutoff or KL reduction, and sums them over forward partners
  for each backward component.

The live state is never replaced by a product state. The measurement-updated
mean and covariance `B_updated[i]` are committed through the baseline
MarlinTrk update exactly as before; only the selected live weight changes.
That weighted `B_updated[i]` population then undergoes the ordinary live
cutoff, KL reduction, material splitting, and next inward propagation. Hit 0
retains the local-measurement weight because no explicit interior product is
defined there. A candidate without a finite positive marginal is rejected;
there is no silent local-weight fallback.

For one direct pair `(f,b)`, the stabilized score is proportional to

```text
q(f,b) = w_F(f) * w_B(b)
         * GaussianOverlap(F_updated[f], B_predicted[b]).
```

The live backward marginal is

```text
q(b) = sum_f q(f,b).
```

All valid direct pairs participate in this marginal. The independent cutoff
and KL reduction used to retain a compact source-3 product mixture happen
afterward and therefore cannot change `q(b)`.

## Interpretation boundary

`SmoothedMarginal` is a direct algorithm experiment requested to determine
whether two-filter evidence can overcome an excessively strong identity
lineage. It deliberately reuses overlapping forward evidence at successive
surfaces, while the propagated state contains only the backward measurement
update. Therefore its live weights are algorithmic Gaussian-overlap scores,
not calibrated Bayesian posterior probabilities. Successful execution or a
focused improvement is not production validation.

Source-3 product states remain non-propagated and non-published. Their direct
pair data remains in the existing lineage schema. For a source-2 measurement
node, `log_unnormalized_posterior` continues to preserve the ordinary local
measurement score for diagnosis; `normalized_posterior` records the actual
selected pre-pruning live weight, which is the smoothed marginal at interior
surfaces when the new mode is selected.

## Configuration contract

The compiled property inventory increases from 40 to 41. The maintained
`DumpGsfTrks/gsf.py.bk` explicitly steers 40 and continues to inherit only
`RecordTruthMaterialIntervals=true`. Its reverse branch deliberately selects:

```python
gsf.InwardWeightMode = "SmoothedMarginal"
```

The neutral initialization outside method branches remains
`LocalMeasurement`; smoother and global-loss behavior is unchanged. No EDM
collection or flat-tuple field was added or renamed.

## Focused validation

The configured EL9/LCG-105 `RecGsfTracking` and `RecGsfFlatTuple` targets
built and installed successfully. Initialization rejects an unknown
`InwardWeightMode` instead of silently selecting a behavior. The maintained
card has valid Python syntax and explicitly steers 40/41 properties; its only
intentional inherited property remains `RecordTruthMaterialIntervals`.

One installed binary reran events 11, 16, and 17 from
`trk_large_20260823/trk-e--2.0-85-1.root` with the five-component production
BH model, `MaxComponents=10`, cutoff `1e-4`, and fresh inward seed -1. The
explicit `LocalMeasurement` control reproduced 192/192 BestBranch,
WeightedMean, FullMixtureMode, and final-component scalars/vectors exactly
against the stored pre-change focused tuple. Its endpoints therefore remain:

| Entry | Truth pT | LCIO pT | BestBranch pT | WeightedMean pT | FullMixtureMode pT |
|---:|---:|---:|---:|---:|---:|
| 11 | 40.7315674 | 40.8954541 | 40.8956875 | 41.3707442 | 40.9034282 |
| 16 | 37.8940163 | 18.2928319 | 18.2872217 | 18.2872149 | 18.2872149 |
| 17 | 18.7969780 | 14.8066689 | 18.6673649 | 18.9729201 | 18.6676807 |

The corresponding `SmoothedMarginal` focused endpoints are:

| Entry | Truth pT | LCIO pT | BestBranch pT | WeightedMean pT | FullMixtureMode pT |
|---:|---:|---:|---:|---:|---:|
| 11 | 40.7315674 | 40.8954541 | 40.9121637 | 41.5489703 | 40.9111651 |
| 16 | 37.8940163 | 18.2928319 | 18.2872217 | 18.2871784 | 18.2871784 |
| 17 | 18.7969780 | 14.8066689 | 18.9383169 | 19.2240705 | 18.6327231 |

The event-11 verbose gate reports every inspected `SMOOTHED MIXTURE hit=i`
before the corresponding `REVERSE UPDATE` records. Interior records explicitly
name `weightMode=SmoothedMarginal`; hit 0 explicitly returns to
`weightMode=LocalMeasurement`. It completes with 10 final components, 1,590
accepted and zero rejected measurement updates, 11 splits, and 10 reductions.

The flat lineage record was then used to reconstruct each backward marginal
without consulting transient C++ values. For every source-3 operation-5 node,
its normalized direct-pair weight was assigned through the source-2 smoothing
parent edge, summed by backward parent, restricted to accepted live
candidates, and renormalized exactly as the live update does.

| Entry / input track | Direct pairs | Interior surfaces | Live nodes compared | Maximum absolute weight difference | Hit-0 local difference | Source-3 publication flags |
|---|---:|---:|---:|---:|---:|---:|
| 11 / 0 | 12,310 | 232 | 1,540 | `8.88e-16` | `0` | 0 |
| 16 / 0 | 1,110 | 222 | 1,110 | `1.11e-16` | `0` | 0 |
| 16 / 1 | 2,494 | 7 | 280 | `6.66e-16` | `0` | 0 |
| 17 / 0 | 9,505 | 230 | 1,437 | `2.78e-16` | `0` | 0 |

Every tested interior surface differs numerically from the normalized local-
measurement weighting, proving that the selected mode is active rather than
an endpoint-only label. No source-3 node is BestBranch or a final-mixture
member. These checks establish mechanical implementation and persistence, not
physics improvement. The required next evidence is a same-code topology-clear
population A/B against `LocalMeasurement`, including clean-track tails and the
separately reported 133-event secondary-activity control population.
