# Shared forward framework for reverse and CMS-like

Date: 2026-08-28

## Decision

Reverse and CMS-like now consume one common outward GSF result. There is no
CMS-specific forward filter and no CMS-specific pre-outermost prediction
cache. The transient `SharedForwardFilterResult` retains the final live
filtered component population as the common source for either inward seed.
For CMS-like only, it also captures every positive-weight filtered mixture
after measurement normalization, weight cutoff, and KL reduction and before
the next outgoing material convolution. Reverse does not allocate this unused
history.

Both workflows now:

1. run the same outward filter once;
2. seed the inward pass from the same final post-measurement, post-reduction
   mixture on hit `N-1`;
3. retain the forward posterior component weights;
4. first propagate from hit `N-1` to hit `N-2` and update hit `N-2`;
5. continue through hit 0 with the same BH splitting, measurement update,
   cutoff, and reduction implementation.

Reverse multiplies each copied seed's full covariance by
`ReverseKappaSeedCov`. CMS-like applies `CmsErrorRescaling` to the same seed
population. At equal scales their live inward recursion is therefore the same.
The CMS-specific operation is downstream: at hit 1 it forms the Gaussian
product of the stored post-reduction forward mixture and the backward-
predicted mixture. BestBranch, WeightedMean, and FullMixtureMode are published
from the reduced product mixture. If no product is valid, the existing final-
backward-mixture fallback remains.

This replaces the historical CMS mechanics that cached the forward prediction
before the outermost measurement, seeded a potentially much larger predicted
population, and applied the outermost measurement again during the inward
pass. Historical tuples from that workflow are not same-method references for
the new CMS endpoint.

## Code and schema simplification

The common framework removed the separate CMS final-prediction ownership,
innermost-forward cache, forward-identity cache, outermost-hit special case,
and associated branching from `GsfAlgorithm.cpp`.

The last CMS-only local diagnostic was also removed:

```text
GSFLineageNodeCmsSmoothIdentityCompatibilityDChi2
lineage_node_cms_smooth_identity_compatibility_dchi2
```

It had already shown no hidden truth-like preference and was not part of fit
steering. Its formula, evidence, and former schema remain preserved in
`agents_record/2026-08-28-cms-identity-chi2-schema-simplification.md` and
`agents_record/2026-08-28-cms-like-forward-identity-compatibility.md`; those
records now describe historical interfaces only. The common
`lineage_node_dchi2` remains the live backward-prediction-versus-measurement
chi-square for both reverse and CMS-like.

No Gaudi configurable property was added, removed, renamed, or given a new
default. The maintained-card comments were updated, but its effective
steering remains unchanged.

## Mechanical validation

`RecGsfTracking` and `RecGsfFlatTuple` built and installed in the focused
EL9/LCG-105 tree.

A same-code reverse regression used job 98 entry 15 with
`CEPCRuntimeCategoryAligned15Clear`, `MaxComponents=10`, cutoff `1e-4`, the
standard initializer, and inward covariance scale 100. All 106 common
endpoint, final-mixture, and lineage branches were exactly equal to the
pre-change reverse reference. The only schema change was removal of the
retired CMS diagnostic.

An equal-scale reverse/CMS structural gate on that event established:

- exact source-1 forward statistical node sequences;
- exact source-2 inward statistical node sequences;
- ten shared final-forward seed components;
- backward measurement coverage from hit 231 through hit 0 for 233 hits, with
  no second application of hit 232;
- 1,500 recorded forward-by-backward product candidates at hit 1;
- a finite three-view CMS endpoint whose final component source is the product
  mixture (`source=3`);
- absence of the retired diagnostic branch in every new tuple.

The maintained five-component baseline was then run on canonical hard-loss
events 11, 16, and 17 with `MaxComponents=12`, cutoff `5e-3`, the standard
initializer, and equal inward covariance scales of 100. Source-1 and source-2
node sequences agreed between reverse and CMS-like to floating-point
precision in every event. All endpoints were finite.

| event | truth pT (GeV) | LCIO pT (GeV) | reverse FullMixtureMode (GeV) | CMS FullMixtureMode (GeV) | shared seeds | CMS products | first inward hit |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 11 | 40.731567 | 40.895454 | 40.917293 | 40.917137 | 12 | 300 | 232 |
| 16 | 37.894016 | 18.292832 | 18.318883 | 18.318882 | 17 | 310 | 222 |
| 17 | 18.796978 | 14.806669 | 18.630966 | 18.630592 | 12 | 275 | 230 |

The seed count can exceed `MaxComponents` when the protected-identity
reduction cannot reach its requested target; the important invariant here is
that reverse and CMS-like receive the same complete final population.

These gates establish shared mechanics, reverse non-regression, and schema
simplification. They do not validate CMS-like physics performance or make it
the production candidate. Population performance claims still require fresh
same-code topology-clear no/light/hard-loss comparisons and separate reporting
of secondary tracker activity.
