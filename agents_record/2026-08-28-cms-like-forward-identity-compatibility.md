# CMS-like forward-identity compatibility diagnostic

Date: 2026-08-28

## Question and correction

The desired local comparison holds the forward hypothesis fixed while testing
two backward hypotheses at the same hit:

```text
chi2(F_updated_identity[i], B_pred_identity[i])
chi2(F_updated_identity[i], B_pred_truth_like[i])
```

The pre-existing
`lineage_node_cms_smooth_all_hit_compatibility_*` branches do not answer this
question. Their forward side is the moment match of every surviving forward
component. Earlier tables calling those values simply "F/B chi2" are
therefore moment-matched-mixture comparisons, not identity-controlled
comparisons. They were not numerically wrong, but the shorter label was
ambiguous and must not be reused for the identity question.

## Passive implementation

For each accepted forward hit `i`, after the normal posterior cutoff and KL
reduction, the CMS-like workflow snapshots the positive-weight
`noRadiationLineage` survivor with the largest normalized weight. It records
the exact full five-dimensional mean and covariance, lineage node ID, and live
post-reduction weight. If no valid identity survivor exists, the new
compatibility remains invalid.

For every accepted interior backward component `k` at the same hit, the
diagnostic evaluates

```text
delta = mu_B(i,k) - mu_F_identity(i)
Csum  = C_B(i,k) + C_F_identity(i)

dchi2      = delta^T Csum^-1 delta
logdet     = log(det(Csum))
log_overlap = -0.5 * [5*log(2*pi) + logdet + dchi2]
```

Both the identity backward child and every radiative backward child use the
same recorded forward identity node. The values do not include the backward
prior. They do not replace a state or weight and do not enter propagation,
measurement update, pruning, KL reduction, CMS product formation, or endpoint
publication.

The exact post-cutoff/reduction identity weight is carried by the new
`forward_identity_weight` field. The older lineage node's generic `weight`
can represent its pre-cutoff normalization when cutoff renormalization did not
also trigger a reduction, so consumers must not substitute that older field
for this snapshot weight.

## Persisted schema

Eight row-aligned EDM collections and flat branches were added:

```text
GSFLineageNodeCmsSmoothIdentityCompatibilityValid
GSFLineageNodeCmsSmoothForwardIdentityNodeId
GSFLineageNodeCmsSmoothForwardIdentityWeight
GSFLineageNodeCmsSmoothForwardIdentityUpdatedKappa
GSFLineageNodeCmsSmoothForwardIdentityUpdatedKappaVariance
GSFLineageNodeCmsSmoothIdentityCompatibilityDChi2
GSFLineageNodeCmsSmoothIdentityCompatibilityLogDet
GSFLineageNodeCmsSmoothIdentityCompatibilityLogOverlap
```

The flat spellings are the corresponding
`lineage_node_cms_smooth_*` names. The node ID is resolved together with the
existing `lineage_node_input_track_index`; node IDs are track-local.

No configurable property was added, and the maintained run card is
unchanged.

## Mechanical and non-interference gates

Both `RecGsfTracking` and `RecGsfFlatTuple` built and installed in the
maintained EL9/LCG 105 tree.

A fresh before/after rerun of seed 12, zero-based entry 2 used CMS-like,
`CEPCRuntimeCategoryAligned15Clear`, `CmsErrorRescaling=1`,
`MaxComponents=10`, and `ComponentWeightCutoff=1e-4`. Across all three flat
rows, all 270 pre-existing branches were exactly unchanged. The selected row
contained 8,509 lineage nodes and 1,863 valid identity-controlled
compatibilities; every new vector had 8,509 entries and every field required
by a valid row was finite.

Comprehensive verbose runs of the canonical file-1 hard events also
completed:

| zero-based event | lineage nodes | valid identity compatibilities | track/hit groups |
|---:|---:|---:|---:|
| 11 | 10,367 | 2,493 | 232 |
| 16 | 8,141 | 1,953 | 229 |
| 17 | 8,354 | 1,549 | 197 |

For every valid row in these events, the referenced forward node exists in
the same input-track graph, has source `ForwardFiltering`, the same hit index,
and `noRadiationLineage=1`; all backward candidates in a track/hit group
reference exactly one common forward identity node. The persisted identity
kappa equals that forward node's filtered kappa exactly.

These gates prove schema mechanics and non-interference only. They do not
show that the identity-controlled compatibility selects the truth-like loss
hypothesis.

## Corrected two-percent comparison

The same-code rerun used the frozen 28-event near-two-percent-loss panel,
CMS-like, `CEPCRuntimeCategoryAligned15Clear`, `CmsErrorRescaling=1`,
`MaxComponents=10`, and `ComponentWeightCutoff=1e-4`. Event 13/57 had no
runtime BH split at its internal-TPC loss interval, so it has no identity/
radiative sibling comparison. Event 63/62 loses energy in interval 0; the
forward loop starts its exact measurement messages at hit 1, so it has a
backward sibling comparison but no exact `F_updated_identity[0]` message.
Twenty-six events therefore have the requested identity-controlled pair.

In only 4/26 events does the truth-like backward child have the lower raw
five-dimensional compatibility chi-square. Adding the covariance
log-determinant leaves the same 4/26 preference. The median
`dchi2_truth - dchi2_identity` is `+17.54`; positive favors identity. For the
same 26 events the older moment-matched forward comparison favored the
truth-like child in 7/26. The exact live backward measurement chi-square
favored it in 9/27 events with a runtime sibling. Fixing the forward side to
identity therefore does not reveal a hidden truth preference; it makes the
identity preference stronger.

| event | hit `i` | truth loss (%) | `chi2(Fid,Bid)` | `chi2(Fid,Btruth)` | truth - identity | preferred |
|---:|---:|---:|---:|---:|---:|---|
| 98/15 | 4 | 1.9523 | 8.4786 | 16.0882 | +7.6097 | identity |
| 59/72 | 3 | 1.9722 | 9.2768 | 20.9912 | +11.7143 | identity |
| 41/72 | 6 | 1.9800 | 2.0768 | 9.0015 | +6.9248 | identity |
| 12/2 | 6 | 2.0193 | 1.6786 | 29.6996 | +28.0209 | identity |
| 32/31 | 8 | 1.9342 | 24.1388 | 7.8682 | -16.2706 | truth-like |
| 9/57 | 226 | 1.9059 | 0.0150 | 103.9070 | +103.8919 | identity |
| 49/25 | 229 | 2.0157 | 0.4160 | 90.4792 | +90.0632 | identity |
| 70/19 | 9 | 1.9970 | 17.3534 | 38.7241 | +21.3707 | identity |
| 74/35 | 231 | 1.9943 | 1.3272 | 79.1786 | +77.8514 | identity |
| 67/46 | 8 | 1.9847 | 9.4830 | 3.6191 | -5.8639 | truth-like |
| 80/17 | 5 | 1.9807 | 9.4760 | 16.0304 | +6.5544 | identity |
| 78/87 | 5 | 1.9802 | 1.6344 | 9.6043 | +7.9699 | identity |
| 28/91 | 4 | 2.0209 | 3.6913 | 5.8485 | +2.1571 | identity |
| 32/89 | 7 | 1.9767 | 13.7419 | 10.1610 | -3.5809 | truth-like |
| 79/1 | 9 | 1.9698 | 10.9026 | 13.1640 | +2.2614 | identity |
| 65/66 | 8 | 1.9662 | 21.7115 | 9.5012 | -12.2103 | truth-like |
| 92/11 | 5 | 2.0383 | 87.5495 | 137.2719 | +49.7225 | identity |
| 83/4 | 232 | 2.0409 | 0.1907 | 81.8104 | +81.6197 | identity |
| 94/34 | 231 | 2.0412 | 0.4277 | 95.4607 | +95.0329 | identity |
| 59/71 | 3 | 1.9523 | 6.3005 | 39.2094 | +32.9089 | identity |
| 89/18 | 231 | 2.0562 | 0.1891 | 76.2191 | +76.0300 | identity |
| 23/84 | 5 | 1.9348 | 5.2639 | 30.7144 | +25.4505 | identity |
| 5/83 | 6 | 2.0692 | 6.9547 | 20.6576 | +13.7029 | identity |
| 58/34 | 232 | 1.9306 | 2.4039 | 107.4746 | +105.0707 | identity |
| 44/73 | 5 | 1.9291 | 2.2965 | 9.1403 | +6.8439 | identity |
| 48/94 | 231 | 1.8979 | 0.2137 | 72.9662 | +72.7525 | identity |

All eight late/outer loss cases at hits 226--232 favor the backward identity,
often by `dchi2` of 73--105. Four of the eighteen auditable early/silicon
cases at hits 3--9 favor the truth-like child.

## Structural interpretation

The fixed forward identity is not an independent inner-hit-only estimate.
`extractSeed()` takes the `CompleteTracks` `AtFirstHit` state (falling back to
`AtIP`), and `makeInitialSite()` copies its kappa mean into the forward seed.
The configured `KappaSeedCov=1e-7` tightly anchors that mean. `CompleteTracks`
was already fit using the complete hit set. The CMS backward pass is then
seeded from the final forward prediction, so the two sides are correlated a
second time.

Consequently, `F_updated_identity` is predisposed toward the same global
no-radiation solution as `B_pred_identity`; it is not an independent forward
message capable of supplying unbiased Bayes evidence for a radiative backward
child. The new table diagnoses this structural correlation. It does not show
that a physical two-percent loss is absent or that the BH component mean is
necessarily wrong.

The next justified diagnostic is to construct a genuinely independent
forward message with a reviewed initial state/covariance and repeat this
same fixed-forward comparison. Changing the BH priors, posterior cutoff, or
endpoint selector before that test would mix the seeding correlation with the
energy-loss hypothesis question.
