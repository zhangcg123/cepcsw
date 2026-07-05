---
name: gsf-mixture-reduction
description: How KL-divergence-based mixture reduction works in GsfMixture
metadata:
  type: reference
---

# GSF Mixture Reduction

## Purpose
When the number of Gaussian components exceeds `MaxComponents` (default 12), the mixture must be pruned to keep the computation tractable. The reduction uses **symmetric KL divergence** between the full 5x5 covariance matrices to decide which components are most similar and should be merged.

## Algorithm: `GsfMixture::reduce(comps, maxN, bz)`

While `comps.size() > maxN`:
1. Compute pairwise symmetric KL distance for all component pairs
2. Find the pair with minimum distance
3. Merge: keep the higher-weight component, accumulate the lower's weight into it
4. Delete the lower-weight component
5. Repeat until at or below maxN

## KL Distance Computation

### Symmetric KL: `klDistance(a, b, bz)`
```
KL_sym = 0.5 * (KL(P||Q) + KL(Q||P))
```
Where P and Q are 5D multivariate Gaussians with mean from `helixAtLastSite(bz)` and covariance from `covAtLastSite()`.

### Directional KL: `KL(P||Q)`
```
KL(P||Q) = 0.5 * [ ln(|ΣQ|/|ΣP|) - d + tr(ΣQ⁻¹ΣP) + (μQ-μP)ᵀΣQ⁻¹(μQ-μP) ]
```
where d=5 (drho, phi0, kappa, dz, tanLambda).

### Numerical Guards
- If determinant < 1e-12 or NaN → return 1e30 (effectively infinite distance)
- If inversion produces NaN → return 1e30
- Negative KL clamped to 0 (numerical guard)

## Normalization: `GsfMixture::normalizeWeights(comps)`
- Sum all weights
- If sum < 1e-30: set all weights to 1/N (uniform)
- Otherwise: divide each weight by sum

## Key Design Decisions
- **Full 5x5 covariance** is used (not just diagonal), capturing correlations between parameters
- The **higher-weight component's track** is kept (not a weighted average), preserving the KF state history
- Mean vectors are extracted from the **last site** (current position), not the initial site
- The reduction is **O(N²)** per iteration — acceptable for N ≤ 12 but would be expensive for very large mixtures

**Why:** Understanding the reduction algorithm is needed when debugging component merging behavior or tuning MaxComponents.
**How to apply:** Reference this when modifying `GsfMixture.cpp`. See [[gsf-algorithm-flow]], [[gsf-bethe-heitler-model]].