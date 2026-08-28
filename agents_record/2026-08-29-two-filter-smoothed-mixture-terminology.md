# Two-filter smoothed-mixture terminology

Date: 2026-08-29

Status: the **smoothed-mixture name and numeric lineage map remain active**,
but its former CMS-like hit-1 publication contract is superseded by
`2026-08-29-smoothed-diagnostic-only-publication.md`. The remainder below is
preserved as the exact terminology-refactor checkpoint.

## Decision

The same-surface Gaussian mixture formerly described in current code as the
inward side product or CMS combined mixture is now called the **two-filter
smoothed mixture**:

```text
B_smoothed[i] = F_updated[i] x B_predicted[i]
```

This is a terminology-only refactor. The smoothed mixture remains passive: it
is materialized after the complete live inward recursion and is never
propagated into a later backward update. Reverse still publishes the terminal
backward mixture. CMS-like still publishes `B_smoothed[1]` and falls back to
the terminal backward mixture when that smoothed mixture cannot be formed.

## Code terminology

The active C++ names now include:

- `GsfSmoothedSurfaceResult` and `GsfSmoothedSurfaceInput`;
- `buildSmoothedSurfaceMixture`;
- `GsfInwardFilterResult::smoothedSurfaces`;
- `cmsSmoothedEndpoint`, `cmsSmoothedComponents`, and
  `cmsUsingSmoothedMixture`;
- `LineageNodeSource::SmoothedMixture`;
- `LineageNodeOperation::Smoothing` and
  `LineageEdgeOperation::Smoothing`;
- `FinalMixtureComponentSource::CmsLikeSmoothed`;
- the verbose label `SMOOTHED MIXTURE`.

The names `product` and `combined` remain valid mathematical descriptions of
the Gaussian operation, but they are no longer the current project name for
this state.

## Compatibility boundary

No configurable property, allowed value, default, algorithm schedule,
collection, EDM field, flat-tuple branch, or numeric schema value changed.
In particular:

- final-mixture source code `3` still means the CMS-like hit-1 smoothed
  mixture and code `4` still means its terminal-backward fallback;
- lineage source code `3` still means the per-surface smoothed mixture;
- lineage node operation and edge operation code `5` still identify its
  two-parent smoothing construction;
- lineage fate code `7` still identifies an internal retained message that is
  not part of the published endpoint.

Old tuples therefore remain directly interpretable with the new terminology.
Historical records retain their original wording and filenames for
provenance. This record is the current terminology map for those references.

## Required mechanical gate

Because the refactor changes names only, the acceptance gate is exact equality
of the reverse and CMS-like endpoint scalars and complete lineage numeric
vectors on the same focused events before and after the rename, in addition to
the normal build/install and verbose-log-name checks.

## Completed validation

The focused five-component production-baseline gate used events 11, 16, and
17 from `trk_large_20260823/trk-e--2.0-85-1.root`, with
`MaxComponents=12`, `ComponentWeightCutoff=5e-3`,
`InwardSeedCovarianceScale=100`, and the standard forward initializer. Both
reverse and CMS-like completed after a clean package build and install.

Exact pre/post comparison covered every row through event 17 and all selected
BestBranch, WeightedMean, FullMixtureMode, final-component, lineage-node, and
lineage-edge scalar/vector branches. For each method it performed 900 scalar
and 882 vector comparisons, with zero mismatches. A comprehensive verbose
CMS-like event-11 run emitted `SMOOTHED MIXTURE` for all successfully processed
surfaces and reported publication from the smoothed mixture at hit 1. These
are mechanical compatibility checks, not new physics validation.
