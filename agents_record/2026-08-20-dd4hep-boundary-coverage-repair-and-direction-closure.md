# DD4hep boundary-coverage repair and direction closure

Date: 2026-08-20

## Problem and approved scope

The direction-paired runtime audit had isolated a geometry-navigation defect:
on exact TPC measurement boundaries, inward
`MaterialManager::materialsBetween` queries systematically omitted one
T2KGas1 half-segment. All 666 internal-TPC intervals in the three-event audit
therefore received approximately half the corresponding outward thickness.
The user approved the narrow repair in both `RecGsfTracking` and the material
tuple producer in `Simulation/DetSimAna`.

This change does not alter a configurable property, the BH model, reduction,
split/cutoff thresholds, final branch publication, or ECAL steering.

## Implementation

Both callers now enforce a geometry-only coverage invariant around
`MaterialManager::materialsBetween`:

1. Sum every positive returned segment length independently of whether its
   material has a usable radiation length.
2. Require that the segments cover the requested point-to-point distance to
   within `max(1 micrometre, 1e-6 * interval length)`.
3. If the initial query is short, move its start into the interval by
   `min(1 micrometre, 1% * interval length)` and query again.
4. Require the retry to cover the shortened interval, then restore the small
   leading cap using `materialAt` at its midpoint. Mark the material audit entry
   with `coverage-cap`.
5. If the retry is still incomplete, mark the path invalid instead of using a
   silent partial sum.

The GSF now evaluates reverse bounded intervals in the same canonical spatial
orientation as forward propagation: matched inner hit to matched outer hit.
This only canonicalizes the scalar DD4hep path query. The BH splitter retains
its explicit reverse flag and the filter continues to propagate inward.

The material recorder keeps genuinely independent forward and reverse
endpoint-order queries, applying the same coverage repair to each. Its
default-off `dd4hep_surface_tuple` adds:

- `coverage_repaired` and `reverse_coverage_repaired`;
- `initial_covered_length_mm` and
  `reverse_initial_covered_length_mm`;
- `covered_length_mm` and `reverse_covered_length_mm`.

These fields make a recovered boundary omission visible while preserving the
forward/reverse query as an independent diagnostic.

## Build and focused runtime closure

`RecGsfTracking` and `DetSimAna` both built and installed successfully in the
EL9/LCG 105 build.

A one-event CMSSW-like smoother smoke also fitted and finalized successfully.
This exercises its special outermost-hit iteration, where no outgoing material
interval exists, before the canonical reverse endpoint is requested.

The production GSF was rerun on the same three seed-107, 2.00764 GeV,
theta-85-degree events with the five-component conditioned BH model,
`MaxComponents=12`, split/cutoff 1e-4, `SymmetricKL`, reverse filtering, and
ECAL off. The outward material CSV remained byte-identical to the frozen
pre-repair audit. The repaired run contained 5,971 valid forward component
paths in 693 groups and 7,127 valid reverse component paths in 696 groups,
with no invalid paths.

Among the 693 shared non-seed intervals:

| Interval class | Groups | Repaired reverse/forward result |
|---|---:|---|
| silicon, hits 1--7 | 21 | equal to numerical precision |
| ITK to first TPC row, hit 8 | 3 | maximum relative difference 3.5e-9 |
| internal TPC rows, hits 9--230 | 666 | all equal; maximum relative difference 1.35e-9 |
| final TPC row to OTK, hit 231 | 3 | maximum relative difference 1.68e-9 |

The three shared-interval reverse totals differ from the forward totals by
-1.78e-8%, -7.10e-8%, and +2.73e-9%, respectively. This closes the former
systematic reverse/forward ratios near 0.5 and the 6--8% event-total deficits.

## Material-recorder closure

A three-event final-code recorder run produced 696 intervals. All forward and
reverse paths were valid, all covered their requested endpoint distance, and
all paired t/X0 values agreed within 1e-9. The largest final coverage deficit
was 0.000526 mm, below the 0.001 mm absolute tolerance.

No forward query required recovery. Thirty-seven reverse queries did, and
every recovery flag corresponded to an initially incomplete segment list.
The old one-event control had exactly eight incomplete reverse queries and
eight unequal t/X0 values, including losses as large as 2.621 mm. This shows
that the new decision is driven by path coverage rather than a detector-name
or direction-specific exception.

## Direct recorder-to-runtime surface-interval closure

A final cross-implementation audit matched the coverage-corrected material
recorder to the GSF runtime on the same three seed-107 events. Provenance was
checked directly in the PODIO files: all three generator-particle PDG codes and
momentum components were bitwise equal, and the VXD, ITK, TPC, and OTK
SimTrackerHit collection sizes agreed event by event.

The recorder supplied 696 primary surface intervals. The forward GSF audit
supplied all 693 non-seed intervals and the reverse runtime dump supplied all
696 intervals including the seed; there were no missing or extra keys. At each
runtime boundary, every live component received exactly the same DD4hep
thickness. All 5,971 forward component rows had zero difference between the
active `path_t_over_x0` and the independently displayed
`geometry_path_t_over_x0`. The 7,127 reverse rows were likewise
component-independent.

The recorder's forward query and the GSF forward runtime used identical
material-name sets in all 693 paired intervals. Their summed thicknesses were:

| Target region | intervals | recorder sum [X0] | runtime sum [X0] | runtime/recorder difference |
|---|---:|---:|---:|---:|
| VXD | 12 | 0.0165784011 | 0.0165780200 | -0.00230% |
| ITK | 9 | 0.0821215576 | 0.0821243742 | +0.00343% |
| TPC | 669 | 0.0598924016 | 0.0603804464 | +0.81487% |
| OTK | 3 | 0.0493000913 | 0.0493162047 | +0.03268% |

The 231 matched non-seed intervals in events 0, 1, and 2 differed in total by
+0.26893%, +0.18985%, and +0.28571%, respectively. Across all 693 intervals,
the absolute t/X0 difference had median `4.54e-7`, 95th percentile `3.26e-6`,
and maximum `2.23e-5`. The relative difference had median 0.983% and 95th
percentile 6.74%; the largest relative values occur on individual TPC gas
intervals of only about `4.5e-5 X0`, not on the material-dominant silicon
intervals.

The cross-file values are not expected to be bitwise equal because their
endpoints are intentionally different. The recorder uses the Geant4 sensitive-
traversal midpoint, whereas runtime GSF uses the digitized reconstructed hit.
Taking the larger of the two endpoint displacements for each interval gave a
median of 0.699 mm, a 95th percentile of 1.512 mm, and a maximum of 2.197 mm.
For all 666 homogeneous internal-TPC intervals, the runtime/recorder t/X0 ratio
followed the reconstructed-endpoint/truth-midpoint path-length ratio: the
median absolute ratio residual was `4.05e-5` and the maximum was `9.82e-5`.
This directly attributes the TPC spread to endpoint placement rather than a
different material calculator or a missing volume.

Using identical reconstructed endpoints removes that distinction. The 693
paired GSF forward/reverse intervals had median relative disagreement
`5.42e-10` and maximum `3.47e-9`. Together with the recorder's own
forward/reverse closure, complete key matching, and identical material-name
sets, this validates that the material tuple producer and GSF runtime implement
the same surface-to-surface DD4hep material definition. It does not mean that
truth-midpoint and digitized-hit intervals are the same geometrical segment;
an exact call-by-call BH comparison must use the runtime endpoint value.

## Focused branch-safety controls

Verbose post-repair reruns of the required source events 11, 16, and 17
completed with all hits fitted and no invalid paths. A direct same-input
comparison against a temporary plugin built from the pre-repair source showed
the same hit count and NDF for every track and no branch-scale discontinuity.
Event 16 was numerically unchanged; the largest relative momentum change among
events 11, 16, and 17 was about 1.3e-6.

A held-out direct A/B used seven primary no-tracker-eBrem events from seed 1:
9, 14, 23, 25, 27, 28, and 38. Every track retained the same hit count and
NDF. The largest relative p or pT change was 1.10e-6, and the largest absolute
chi-square change was 5.19e-4. These tiny changes are expected because the
restored TPC gas still participates in continuous transport even though its
per-interval thickness remains below the production BH split threshold.

The absolute GSF momenta in this current tuple are frequently pathological,
including in the pre-repair results. The A/B is consistent with an unchanged
selected branch in the focused controls; it is not evidence of acceptable
momentum performance.

## Physics boundary and next question

The exact TPC surface-pair material path is now direction-closed in both the
runtime GSF and material recorder. This resolves a material-path defect, but
does not explain the production branch failures: the restored internal-TPC
paths are approximately 4.4e-5 X0, below the production
`BHSplitThreshold=1e-4`, so the repair does not add BH calls there.

The active investigation therefore returns to branch-local material/BH
consistency. Capture every actual BH call on an unbiased sample, locate the
first loss of truth-compatible posterior rank in bad events, and compare its
exact bounded DD4hep composition and BH response with Geant4 truth and matched
good controls. Do not retune material modes, thresholds, capacity, KL, final
selection, or ECAL before that crossover is classified.
