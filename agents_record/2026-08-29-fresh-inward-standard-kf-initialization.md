# Fresh inward standard-KF initialization

Date: 2026-08-29

## Decision and scope

`InwardSeedCovarianceScale` now selects two common inward-seed modes for both
reverse and CMS-like:

- A finite positive value preserves the established path: copy the complete
  final forward mixture, retain its means and lineage metadata, and multiply
  every copied covariance element by the configured scale.
- A finite value at or below zero constructs one fresh standard-KF-style
  backward seed. It uses the dedicated `GsfTrackInitializer`, the
  first/middle/last two-dimensional-hit helix prefit, the loose
  `FullLDCTracking` covariance, and one explicit MarlinTrk update at the
  outermost accepted hit.

The compiled and active reverse-template default remains positive 100. The
maintained `DumpGsfTrks/gsf.py.bk` campaign value remains positive 1. This
change adds a diagnostic mode; it does not promote a new production default or
claim a physics improvement. No EDM or flat-tuple schema changed.

## Exact mechanics

`GsfTrackInitializer` now accepts an outward or inward direction and a seed
measurement surface. The outward path remains the existing first-hit start.
The inward path initializes MarlinTrk in the backward direction, evaluates the
outermost accepted hit `N-1` once, and returns that filtered state and its exact
measurement `dchi2` and dimension.

The fresh inward seed has:

- one component with normalized weight 1;
- source 2, operation 1 lineage at hit `N-1`;
- no incoming lineage edge and no forward-to-backward seed edge;
- no copied forward component weight, mean, process signature, radiative
  ancestry, or covariance;
- the configured `KappaSeedCov` convention, where a finite value at or below
  zero gives exact `Var(omega)=1e-4` and a positive value remains a
  curvature-only diagnostic override.

The live inward loop starts at `N-2` because `N-1` was already consumed by the
initializer. Therefore the outermost measurement is neither omitted nor
double-counted. `ReverseInitialWeightMode` is ignored in fresh mode because
there is only one unit-weight root. `SurfaceConsistency` has no inherited
forward process metadata in this mode and consequently supplies its common
uninformative floor to every candidate, reducing its ranking to aggregate
weight.

The fresh seed is independent of the final forward posterior but is not an
independent measurement sample. Its helix prefit uses the first, middle, and
last two-dimensional-hit positions, and those measurements are subsequently
consumed by the loose-covariance fit.

## Focused validation

The configured EL9/LCG 105 targets `RecGsfTracking` and `RecGsfFlatTuple`
built and installed successfully. One-event verbose component execution also
completed. Same-code five-component runs used the 2 GeV, 85 degree sample and
selected hard-loss events 11, 16, and 17.

Compatibility gates:

- New positive-scale 100 reverse output exactly matches the pre-change
  reference endpoints for events 11, 16, and 17.
- New positive-scale 100 CMS-like output exactly matches the pre-change
  reference endpoints for the same events.
- Scale 0 and scale -1 are exactly equal endpoint by endpoint for reverse and
  CMS-like on all three events.
- Each fresh output has exactly one source-2 seed at the outermost hit, no
  incoming seed edge, no operation-4 copied-seed edge, and its first inward
  target is `N-2`.

The fresh-scale-0 focused endpoint table is:

| Method | Event | Truth pT [GeV] | LCIO pT [GeV] | BestBranch [GeV] | WeightedMean [GeV] | FullMixtureMode [GeV] | NDF | Hits |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| reverse | 11 | 40.731567 | 40.895454 | 40.895687 | 40.952787 | 40.899209 | 462 | 234 |
| reverse | 16 | 37.894016 | 18.292832 | 18.287222 | 18.287215 | 18.287215 | 442 | 224 |
| reverse | 17 | 18.796978 | 14.806669 | 18.612261 | 18.852110 | 18.616764 | 458 | 232 |
| CMS-like | 11 | 40.731567 | 40.895454 | 40.895637 | 40.952736 | 40.899155 | 462 | 234 |
| CMS-like | 16 | 37.894016 | 18.292832 | 18.287220 | 18.287214 | 18.287214 | 442 | 224 |
| CMS-like | 17 | 18.796978 | 14.806669 | 18.612252 | 18.852653 | 18.616726 | 458 | 232 |

Event 16 emits the existing non-positive-definite FullMixtureMode warning and
uses its established fallback. It also has secondary tracker activity. The
finite endpoint is not evidence that fresh initialization solves that event.

## Interpretation and next gate

This checkpoint proves a mechanically fresh inward start without changing the
positive production path. It does not validate the new mode. The next study is
a same-code topology-clear population comparison of scale 100 and scale 0 for
reverse and CMS-like, categorized by no/light/hard loss and early transition,
with the 133-event secondary-activity population reported separately. It must
check clean-track preservation, tails, output-mode failures, and the first
truth-compatible lineage rank crossover before any default discussion.

The superseded common-inward implementation focus and evidence remain in
`2026-08-29-common-inward-filter-side-products.md` and
`2026-08-29-inward-seed-covariance-property-unification.md`.
