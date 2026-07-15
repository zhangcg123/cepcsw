# Positive-LCIO amplification: MaxComponents 12 versus 24

Date: 2026-07-14

## Test

The fixed 30-ID positive-LCIO amplification set was rerun with the installed
code, `AggregateWeight`, comprehensive component dumps, and
`MaxComponents=12`. This is a direct same-code capacity comparison with the
fresh 24-component audit, not a comparison to stored historical tuples.

All 28 seed jobs completed successfully and produced 30 finite reverse IP
outputs. The 12-component run recorded 80,691 accepted and 48 rejected
component updates, with no missing-track or covariance-failure marker. Fewer
component operations are expected at lower capacity and are not a physics
quality result.

Disposable logs and ROOT files are in
`/tmp/gsf-positive-lcio-amplified-30-verbose12`. Durable tables are:

- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/positive_lcio_amplified_30_current12_diagnostics.csv`
- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/positive_lcio_amplified_30_current12_vs_24.csv`

The reproducible eventwise comparator is
`Reconstruction/RecGsfTracking/scripts/compare_positive_lcio_capacities.py`.
It judges improvement using absolute generator-truth pT residual, not merely a
reduction in positive amplification.

## Results

Across all 30 events, 12 versus 24 components gives 10 improvements, 16
worsenings, and four unchanged results at the comparison tolerance. Ten
selected radiative signatures change. The median absolute residual worsens
from 1.5829% to 1.6192% and the mean absolute residual from 1.4813% to 1.5495%;
both capacities retain 14/30 events inside 1%.

The category split is asymmetric:

- no-eBrem: 4 improve, 12 worsen, 4 unchanged; 8 selections change;
- light-eBrem: 6 improve and 4 worsen; 2 selections change.

The decisive safety problem is the low-amplification/control stratum. Among
the 13 events stored below +1 percentage point, only one improves, nine worsen,
and three are unchanged. Five selections change. MaxComponents 12 publishes
radiative output for five events that the fresh 24-component run leaves on
identity, including 71/6, 285/8, 452/1, 207/3, and 84/5. Accordingly, the
number of current events above +0.25-point amplification rises from 21/30 at
24 components to 26/30 at 12 components. Four versus nine events publish
identity at 12 versus 24.

The large stored-amplification stratum is mixed: among 17 events, nine improve,
seven worsen, and one is unchanged. Its median change in absolute residual is
only -0.0020 percentage point. The largest improvement is 228/5, whose
residual changes from +2.7604% to +1.9711% while its selected mode moves from
surface-8 g2 to surface-5 g2. The largest worsening is clean 181/1, from
+2.4995% to +3.0339%, with its surface-8 g2 support lost while surface-6 g3
remains selected. This is further evidence that capacity changes preserve or
remove individual discrete surface/mode alternatives rather than applying a
uniform correction.

## Decision

Reject `MaxComponents=12` for this target. It does not improve the 30-event
population overall and materially worsens the identity-like low-amplification
controls. Keep `MaxComponents=24` as the active default. The result reinforces
the current plan to diagnose a surface/mode discriminator on persistent
amplifications rather than reducing global mixture capacity.

## State-level mechanism established after the comparison

The 12-component KL result is caused by representation-dependent aggregate
weight, not a new likelihood preference. KL reduction never merges identity
and radiative lineages, but it moment-merges nearby radiative components and
adds their weights. The final `AggregateWeight` publication chooses the single
largest resulting component rather than comparing total radiative mass with
identity mass. At capacity 24 the radiative posterior can remain split among
several components, allowing the protected singleton identity to win; at 12,
the same radiative fragments are consolidated into one heavier component.

The clean false flips make this explicit. At the final hit, 71/6 changes from
identity weight 0.360 versus radiative fragments 0.312 and 0.083 at capacity
24 to a merged radiative component of 0.421 versus identity 0.359 at capacity
12. Analogous comparisons are 84/5: identity 0.416 versus fragments 0.317 and
0.074, becoming radiative 0.481 versus identity 0.409; 207/3: identity 0.393
versus leading radiative 0.380, becoming radiative 0.415 versus identity
0.390; 285/8: identity 0.434 versus leading radiative 0.369, becoming
radiative 0.453 versus identity 0.432; and 452/1: identity 0.442 versus leading
radiative 0.355, becoming radiative 0.440 versus identity 0.439. Identity state
and weight are nearly unchanged; radiative packaging changes the winner.

Radiative-to-radiative changes have the same origin. At 127/4, capacity 12
concentrates surface-5 g2 to weight 0.955 and retains it inward, whereas 24
later selects surface-6 g2. At 181/1, 12 collapses onto surface-6 g3 while 24
retains a surface-8 g2 plus surface-6 g3 aggregate that is closer to truth.
At 279/0, 12 concentrates surface-5 g2 to 0.978 while 24 selects surface-7 g2.
The 228/5 exception happens to improve when 12 selects surface-5 g2 instead of
the surface-8 g2 choice at 24. Thus lowering capacity is not a calibrated
physics discriminator; it changes how posterior mass is packaged and makes
highest-component publication capacity-dependent.

