# CEPC conditioned BH model: tested transferability universe

Date: 2026-07-13

## Question and model variable

`CEPC2GeV85StepConditioned` was extracted at generated 2 GeV pT and theta 85
degrees. Its stochastic variable is retained momentum fraction, equivalently
the eBrem-attributed fractional loss

```text
f_i = ebrem_step_loss_sum_GeV_i / p_before_GeV_i,
z_i = 1 - f_i,
```

conditioned on the outgoing-current Geant4 surface interval's `t/X0`. It does
not model absolute loss in GeV. Non-eBrem transitions form the exact `z=1`
atom because ordinary deterministic loss is already enabled through
`ElossOn=True`.

The tested transferability universe now contains:

| generated pT | theta | approximate initial total p | events | accepted transitions with `t/X0<0.03` |
|---:|---:|---:|---:|---:|
| 2 GeV | 85 deg | 2.01 GeV | 10,000 | 2,573,891 |
| 10 GeV | 85 deg | 10.04 GeV | 1,000 | 235,186 |
| 10 GeV | 20 deg | 29.32 GeV | 1,000 | 82,530 |

The filename momentum is pT. At 20 degrees, `p = pT/sin(theta)`; the 10 GeV
pT sample is therefore not a 10 GeV total-momentum or beam-energy sample.

## Three different loss-per-X0 quantities

For transition `i`, the top-right comparison panel plots only positive-eBrem
owned intervals:

```text
r_i = (DeltaE_i^eBrem / p_before_i) / (t/X0)_i.
```

One transition normally aggregates several individual Geant4 steps between
consecutive sensitive-surface anchors; it is not a single G4 step.

For a positive-eBrem event, the bottom-right panel plots:

```text
r_event = [sum_i (DeltaE_i^eBrem / p_before_i)] / [sum_i (t/X0)_i].
```

The sums run over all accepted outgoing-current intervals of that primary
electron with `0 < t/X0 < 0.03`. Non-radiative intervals contribute material
to the denominator and zero to the numerator. Only events with positive eBrem
were drawn in the displayed event spectrum.

The most stable sample-wide rate used for validation is instead one ratio of
global sums:

```text
R_global = [sum_events sum_i (DeltaE_i^eBrem / p_before_i)]
           / [sum_events sum_i (t/X0)_i].
```

This is not the arithmetic mean of `r_i`; an arithmetic transition mean would
overweight extremely thin intervals.

## Cross-energy evidence at fixed angle

At theta 85 degrees, the global eBrem-transition fractions are 0.3702% at 2
GeV pT and 0.3682% at 10 GeV pT. In the well-populated `t/X0` ranges the
occurrence probabilities agree statistically. The radiative fractional-loss
class populations also agree: 58.20% versus 58.66% below 1%, 16.47% versus
15.94% at 1--5%, 12.90% versus 13.97% at 5--20%, and 12.44% versus 11.43%
above 20%. The full positive-eBrem fractional-loss distributions have KS
distance 0.0218 and `p=0.84`.

## Cross-angle evidence at fixed pT

The 20-degree trajectory has a fundamentally different material population:
its `t/X0` median, 90th, 95th, and 99th percentiles are approximately
`1.24e-4`, `1.72e-3`, `8.48e-3`, and `2.25e-2`, versus `4.27e-5`, `4.28e-5`,
`5.39e-5`, and `9.71e-3` at 85 degrees. Raw transition fractions must
therefore not be compared without `t/X0` conditioning.

For the two 10 GeV-pT samples, the complete positive-eBrem fractional-loss
spectra are compatible: KS distance 0.0432 with `p=0.31`. Individual broad
`t/X0` bins show some occurrence differences where the within-bin material
distribution differs, but the rate per accumulated radiation length is
consistent.

The sample-wide results are:

| sample | eBrem transitions / accumulated X0 | `R_global` |
|---|---:|---:|
| 2 GeV pT, 85 deg | 12.56 | 0.9580 |
| 10 GeV pT, 85 deg | 13.11 | 0.9730 |
| 10 GeV pT, 20 deg | 12.16 | 0.9872 |

Event bootstrap 95% intervals for `R_global` are respectively
`[0.912,1.003]`, `[0.821,1.136]`, and `[0.852,1.128]`. They overlap strongly;
the central values agree within about 3%.

Absolute `DeltaE/(t/X0)` does not coincide across these samples and should not
be expected to: its scale follows total electron momentum. In particular the
20-degree sample has about 29.32 GeV total momentum, explaining its much
larger absolute loss rate. Momentum normalization restores the relevant BH
comparison.

## Conclusion boundary

Geant4 truth supports `CEPC2GeV85StepConditioned` as a provisional
fractional-loss process model across the tested 2--10 GeV pT range and from 85
to 20 degrees in the same detector geometry, ownership convention, particle
hypothesis, and `t/X0<0.03` execution range. The evidence disfavors a gross
energy- or angle-scaling failure of the fractional BH spectrum.

This is not universal validation. Untested or insufficiently tested regions
include other angles and detector regions, low momenta where curvature and
ionization dominate, `t/X0>=0.03`, displaced/curling/repeated-crossing tracks,
different geometry/material configurations, and independent held-out
production phases. It also does not validate the reverse-GSF selection
workflow: the 20-degree GSF jobs exceeded memory, while 10 GeV/85-degree
tracking still has false-radiative outliers.

Durable products are under
`TrackingPerformanceStudies/material_loss_10p0_theta20/`: the derived
transition CSV and audit, the three-sample energy-loss-per-X0 PNG/PDF and
summary. Reproduction uses
`Reconstruction/RecGsfTracking/scripts/build_g4_transition_dataset.py` and
`compare_energy_loss_per_x0.py`.

This enhanced transferability understanding is evidence, not a change to the
active light-eBrem optimization TODO order.
