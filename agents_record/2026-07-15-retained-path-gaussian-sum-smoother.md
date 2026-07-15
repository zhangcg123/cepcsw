# Retained-path Gaussian-sum smoother upgrade — 2026-07-15

## Request and scope

The optional retained-lineage smoother was upgraded into an explicit
retained-path Gaussian-sum smoother.  The independent reverse
multi-component filter was preserved unchanged.  All implementation changes
are confined to `Reconstruction/RecGsfTracking`.

## Implemented semantics

The forward filter records an ancestry node identifier at every accepted
measurement update.  Descendants cloned by a later BH split retain all prefix
node identifiers.  With `ReductionMode=TopN`, every terminal component is one
real, unmerged trajectory, so its normalized terminal filter weight is

```text
P(retained complete trajectory | all measurements).
```

The smoother now has both parts of a retained-path Gaussian-sum posterior:

1. The existing exact RTS recursion uses the accepted MarlinTrk transport
   Jacobian, process Jacobian, predicted covariance, and filtered covariance
   to smooth the state and covariance on every retained trajectory.
2. A discrete backward pass assigns each earlier forward node the sum of the
   posterior probabilities of all retained terminal descendants.  This is an
   exact association over the retained TopN ancestry, not a Gaussian-distance
   match.  Per-path and per-node smoothed probabilities are stored separately
   and comprehensive dumps expose the first, last, and boundary levels.

`TopN` remains mandatory for this option.  KL merging moment-matches different
physical ancestries and therefore cannot provide the exact parent/descendant
mapping required by this implementation.  TopN pruning still makes the result
a truncated approximation to the unbounded Gaussian sum; deleted paths are
not revived by smoothing.

The smoothed mixture is extrapolated geometrically from the first accepted
measurement to the IP.  `GSFOutputMode=WeightedMean` publishes the complete
retained smoothed mixture including between-path covariance.  `BestBranch`
remains available as a diagnostic estimator.  The dedicated smoother option
now selects `WeightedMean` explicitly.

The separate `ReverseFiltering` implementation and option remain present and
unchanged.  As before, it is mutually exclusive with the smoother because it
is an independent reverse refit, not the backward pass of this smoother.

## Files

- `src/GsfComponent.h` and `.cpp`: smoothing-node identity, smoothed node
  probability, smoothed path probability, and clone preservation.
- `src/GsfAlgorithm.cpp`: node capture, retained-path backward probability
  aggregation, normalization checks, and comprehensive diagnostics.
- `options/run_gsf_cepc2gev85_step_conditioned_smoother.py`: publish the
  weighted smoothed mixture.
- `README.md`: updated option semantics and reverse-filter preservation.

## Validation

The configured EL9/LCG-105 `RecGsfTracking` target built and installed.

Focused exact-pair event 11 from `/tmp/gsf-match-tracks.root` completed with:

- 234/234 accepted hits;
- 12/12 RTS-smoothed retained trajectories;
- 233 smoothing measurement states per trajectory;
- one common ancestry node at the inner three measurements;
- eight ancestry nodes at the penultimate measurement and 12 terminal nodes;
- posterior probability sums equal to one at every measurement;
- no smoothing-weight failure or covariance failure;
- weighted-mixture IP pT 1.7938 GeV versus truth 2.0004 GeV and LCIO
  1.7938 GeV.

Thus the implementation is mechanically healthy but does not recover momentum
on event 11.  The result is not a physics validation.

The unchanged reverse-filter card was rerun on the same event after the
upgrade.  It retained 234/234 hits, produced 12 final components, and published
1.9838 GeV pT versus 2.0004 GeV truth.  This is a focused regression check that
the reverse workflow remains operational, not new broad validation.

The locally available exact-pair file ends after event 11.  Events 16 and 17
could not be rerun in this session; that required validation remains pending
when an input file containing those entries is available.

Generated validation outputs are under `/tmp` and are not status records.

