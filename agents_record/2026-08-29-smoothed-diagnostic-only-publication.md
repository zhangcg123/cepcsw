# Smoothed mixtures become diagnostic-only

Status note (2026-08-29): the diagnostic-only decision remains active, but the
exactly equivalent CMS-like steering alias was retired. The later inline
construction change also supersedes the deferred post-recursion mechanics
described under "Active mechanics" below. See
`2026-08-29-cms-like-workflow-retirement.md` and
`2026-08-29-inline-smoothed-surface-construction.md`. The evidence below is
preserved as the pre-retirement and pre-inline-construction gate.

Date: 2026-08-29

## Decision

Reverse and CMS-like use the same shared inward recursion and now publish the
same terminal posterior mixture:

```text
B_updated[0] = measurement[0] x B_predicted[0]
```

The same-surface two-filter mixtures

```text
B_smoothed[i] = F_updated[i] x B_predicted[i]
```

remain materialized at every successfully processed inward surface, but are
strictly passive diagnostics. They never feed the inward recursion and never
supply BestBranch, WeightedMean, FullMixtureMode, or final-mixture-component
publication.

This supersedes the publication part of
`2026-08-29-common-inward-filter-side-products.md` and
`2026-08-29-two-filter-smoothed-mixture-terminology.md`. Those records remain
historical evidence for the former hit-1 CMS endpoint and the terminology
transition.

## Active mechanics

One `runGsfInwardFilter` invocation supplies both flags:

```text
B_updated[i+1]
  -> material/BH propagation
  -> B_predicted[i]
  -> measurement[i]
  -> B_updated[i]
  -> posterior cutoff and KL reduction
```

After that live recursion completes, the code independently forms all valid
`B_smoothed[i]` mixtures from saved `B_predicted[i]` snapshots and immutable
`F_updated[i]` snapshots. Their source-3/operation-5 nodes and edges remain in
the default-on lineage EDM and flat vectors. Retained smoothed survivors are
marked fate 7 and cannot be final-mixture members.

At equal configuration, reverse and CMS-like therefore have the same forward
states, inward seed, live backward states, terminal components, selection,
and all three published endpoints. `CmsGsfSmoothing` remains only a compatible
workflow label for producing the common inward result and its passive
diagnostics; it no longer names a distinct refit or endpoint.

## Schema compatibility

No collection, EDM field, flat branch, property name, allowed value, or
compiled default changes. New reverse and CMS-like final-component records use
source code 2 for the common terminal inward mixture. Historical source code 3
continues to mean a CMS-like hit-1 smoothed endpoint, and historical source
code 4 continues to mean its terminal-backward fallback. Lineage source 3,
operation 5, edge operation 5, and fate 7 retain their existing diagnostic
meanings.

The corresponding unused C++ enum labels are explicitly prefixed
`Historical`; no active publication path refers to either value.

## Required gate

The focused acceptance gate must establish all of the following with one
installed binary and equal steering:

1. reverse and CMS-like BestBranch, WeightedMean, FullMixtureMode, and final
   component vectors are exactly equal on events 11, 16, and 17;
2. both methods still persist source-3/operation-5 smoothing nodes and their
   two-parent edges;
3. no smoothing node is marked BestBranch or final-mixture membership;
4. retained smoothing survivors have fate 7;
5. verbose CMS-like output identifies terminal `B_updated[0]` as the endpoint
   source.

These are mechanical compatibility checks, not physics validation.

## Completed validation

The focused EL9/LCG-105 package build and install completed successfully. One
installed binary then ran reverse and CMS-like with the maintained
five-component baseline on events 11, 16, and 17 from
`trk_large_20260823/trk-e--2.0-85-1.root`, with `MaxComponents=12`, cutoff
`5e-3`, the standard forward initializer, and
`InwardSeedCovarianceScale=100`.

An exact comparison of all selected endpoint, final-component, lineage-node,
and lineage-edge branches covered all rows through event 17. It performed 900
scalar and 882 vector comparisons with zero mismatches. Each method recorded
30,285 source-3 smoothed nodes, including 29,088 operation-5 candidates, 5,657
fate-7 retained diagnostic survivors, and 58,176 operation-5 edges. No
source-3 node was marked BestBranch or final-mixture membership, and every
published final component used source code 2.

A comprehensive verbose CMS-like event-11 run retained hit-1 and hit-0
`SMOOTHED MIXTURE` diagnostics and explicitly reported:

```text
CMS-GSF all endpoints source: terminal B_updated[0] mixture
```

The gate therefore establishes the intended mechanical boundary: reverse and
CMS-like are exact endpoint aliases, while the full smoothed record remains
available and non-publishing. It does not establish physics performance.
