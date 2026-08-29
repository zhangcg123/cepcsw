# Symmetric surface-local evidence for smoothed products

Date: 2026-08-29

## Decision

Every direct interior two-filter product candidate now persists the same
surface-local evidence categories as a live measurement candidate. The state
name differs by meaning: a measurement result remains `filtered_*`, while the
two-filter product is exposed as `smoothed_*`.

For one pair at surface `i`, the persisted contract is:

```text
prior_weight = weight(F_updated[i]) * weight(B_predicted[i])

compatibility_dchi2
  = delta^T * inverse(C_F + C_B) * delta

logdet_innovation
  = log(det(C_F + C_B))

log_overlap
  = -0.5 * [5*log(2*pi) + logdet_innovation
                          + compatibility_dchi2]

log_unnormalized_posterior
  = log(weight(F_updated[i]))
    + log(weight(B_predicted[i]))
    + log_overlap
```

The direct candidates are normalized together at the surface before cutoff and
KL reduction; that value is persisted as `normalized_posterior`. The existing
`weight` continues to describe the node's current/final weight, so it may be
renormalized for a retained candidate while a removed candidate preserves its
pre-pruning value.

## Field mapping

The existing EDM and generic flat fields are populated for source-3,
operation-5 candidates:

- `prior_weight`: product of the forward and backward pair priors;
- `dchi2`: the five-dimensional forward/backward compatibility quadratic;
- `logdet_innovation`: `log(det(C_F+C_B))`;
- `log_unnormalized_posterior`: the exact pair log weight used for selection;
- `normalized_posterior`: pair posterior before cutoff and KL reduction;
- `predicted_kappa`, `predicted_kappa_variance`, and derived `predicted_pT`:
  the exact transported `B_predicted[i]` input;
- `weight`, identity/fate fields, and merge edges: the established passive
  product lineage record.

The flat tuple adds three explicit aliases:

```text
lineage_node_smoothed_kappa
lineage_node_smoothed_kappa_variance
lineage_node_smoothed_pT
```

They contain the product state for every source-3 direct candidate and KL
output and are NaN for sources 1 and 2. The generic `filtered_*` vectors remain
unchanged and equal the aliases for source 3, preserving existing analysis
compatibility. No new EDM collection is required because the aliases are
derived losslessly from the persisted source and filtered-state collections.

The source-3 `measurement_status` remains `-1`: the overlap is a Gaussian
forward/backward compatibility evaluation, not a detector measurement. Seed,
split, and KL-output nodes retain NaN in candidate-only evidence fields, just
as their live forward/reverse counterparts do.

## Mechanical gate

The EL9/LCG-105 focused build and install completed successfully. The installed
binary reran maintained fresh-inward event 11 with comprehensive verbose dumps
and hard-loss events 11, 16, and 17 together. The verbose record retained the
required ordering: `SMOOTHED MIXTURE hit=232` preceded the first
`REVERSE UPDATE accept hit=232`, and the application finalized normally.

Against the immediately preceding same-code tuples:

- all 64 BestBranch, WeightedMean, FullMixtureMode, and final-component
  branches were bit-for-bit equal;
- every pre-existing lineage field outside the intentionally populated
  source-3 evidence fields was exactly equal, including the complete live
  source-1/source-2 graph and passive weights/KL outcomes;
- all 12,310, 3,581, and 9,385 direct smoothed candidates in events 11, 16,
  and 17 respectively had finite prior, overlap chi-square, overlap
  log-determinant, log weight, normalized posterior, and backward-predicted
  kappa/variance;
- the normalized direct-candidate posteriors summed to one independently for
  every input-track/surface group to within `1.6e-15`;
- the stored log-weight identity agreed within floating-point reconstruction
  error (`2.3e-13` maximum), and all explicit smoothed aliases exactly matched
  the corresponding source-3 generic state while remaining NaN for other
  sources.

The known event-16 FullMixtureMode fallback remains unchanged. These checks
establish schema completeness and non-interference with the live fit; they are
not a physics-performance validation of the smoothed product.
