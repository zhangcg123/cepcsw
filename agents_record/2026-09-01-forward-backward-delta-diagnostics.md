# Forward/backward delta diagnostics

Date: 2026-09-01

## Purpose

Each explicit interior two-filter candidate already combines
`F_updated[i] x B_predicted[i]` and records the backward prediction, product
state, overlap evidence, and weights.  This change adds the direct curvature
and transverse-momentum disagreement between those two input messages.  The
quantities are passive diagnostics only: they do not change the product,
component weight, cutoff, KL reduction, inward recursion, or published track.

## Sign and variance contract

For a direct source-3, operation-5 pair at an interior surface, define

```text
kappa_F = kappa(F_updated[i])
kappa_B = kappa(B_predicted[i])
delta_kappa = kappa_B - kappa_F
Var(delta_kappa) = C_F(kappa,kappa) + C_B(kappa,kappa)

pT_F = 1 / abs(kappa_F)
pT_B = 1 / abs(kappa_B)
delta_pT = pT_B - pT_F
Var(delta_pT) = C_F(kappa,kappa) / kappa_F^4
              + C_B(kappa,kappa) / kappa_B^4
```

The pT variance is first-order propagation.  Both variances assume zero
forward/backward cross-covariance because that cross-covariance is not
available in the current message representation.  Therefore they are
algorithmic compatibility uncertainties, not calibrated physical errors.
Invalid or singular inputs produce NaN.  Non-direct nodes, source-3 KL merge
outputs, and the hit-0/outermost boundary mixtures also receive NaN.

## Persisted schema

The GSF EDM contains four row-aligned `podio::UserDataCollection<double>`
collections:

```text
GSFLineageNodeFBDeltaKappa
GSFLineageNodeFBDeltaKappaVariance
GSFLineageNodeFBDeltaPT
GSFLineageNodeFBDeltaPTVariance
```

`RecGsfFlatTuple` writes them as:

```text
lineage_node_fb_delta_kappa
lineage_node_fb_delta_kappa_variance
lineage_node_fb_delta_pT
lineage_node_fb_delta_pT_variance
```

The branches exist automatically with the component-lineage schema.  When an
older EDM input lacks the new collections, the flat tuple preserves the old
lineage graph and fills the four new vectors with row-aligned NaNs.

## Validation

The focused `RecGsfTracking` and `RecGsfFlatTuple` build and installation
completed successfully.  The maintained reverse template then processed
events 11, 16, and 17 from
`trk_large_20260823/trk-e--2.0-85-1.root`, fitting 1/1, 2/2, and 1/1 tracks.

The flat-tuple audit followed every direct product's operation-5 edge to its
source-1 forward parent.  Across 35,701 direct pairs, all four fields were
finite and row-aligned; no field was finite outside a direct source-3,
operation-5 node.  The kappa difference, kappa variance, and pT difference
matched their reconstructed formulas exactly.  The maximum checked relative
error for the pT variance was `5.45e-16`.

An exact branch-by-branch comparison with the immediately preceding
diagnostic-only run found zero mismatches in all 243 pre-existing flat-tuple
branches.  The only additions were the four fields listed above.  A verbose
event-11 gate also completed with 10 final components, 2,189 accepted and zero
rejected reverse measurements, 11 BH splits, and 11 reductions.
