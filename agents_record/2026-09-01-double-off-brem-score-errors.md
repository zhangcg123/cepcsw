# Double-off directional defaults and brem-score error study

Date: 2026-09-01

## Decision and option contract

At the user's request, the compiled defaults of both directional BH-child
creation gates changed to

```text
ForwardBHSplitting = false
InwardBHSplitting = false
```

An unsteered forward/reverse run therefore creates no BH children. This does
not disable propagation, measurement updates, multiple scattering,
deterministic energy loss, material-path evaluation, passive truth-interval
recording, or the passive `F_updated x B_predicted` products.

The dedicated option-surface audit synchronized
`Reconstruction/RecGsfTracking/README.md` and the explicit maintained card.
The maintained reverse campaign remains a deliberate exception: its common
steering is false/false and its reverse branch explicitly sets
`InwardBHSplitting=true`, giving forward-off/inward-on behavior. Unsteered
active templates inherit the compiled false/false pair. Dated evidence for
the former true/true defaults remains historical and was not rewritten.

## Validation and sample

The focused EL9/LCG-105 build and install completed successfully. A verbose
event-11 run and focused events 11, 16, and 17 inherited the compiled defaults
without explicit directional assignments. Their lineage graphs contained
zero operation-2 BH-child nodes in either direction and retained one finite
identity-pair F/B probability for each interior surface.

The detector study used ten input seeds from
`trk_large_20260823/trk-e--2.0-85-{1..10}.root`. It selected 199
truth-material-valid, topology-clear events: 100 no-brem controls and 99
truth-brem events. The brem sample was deliberately stratified over losses
below 0.2%, 0.2--1%, 1--2%, and at least 2%. The 99 events contained 155
positive Geant4 eBrem intervals; 96 intervals lost at least 0.2% and 54 lost
at least 1%. The run created zero BH children across the complete selected
sample. Secondary-tracker-activity events were excluded and were not sampled
for a separate control error rate in this focused gate.

Two decision scopes were evaluated:

1. The operational event scan flags brem when the maximum identity-pair
   `P_brem` over every interior surface exceeds the threshold.
2. The interval diagnostic compares each identity-pair score with the exact
   Geant4 eBrem label for the corresponding saved surface interval. The
   `hit_from` mapping reproduced the independently recorded dominant truth
   transition in 98/98 mappable brem events.

Type I is a truth-no-brem event or interval incorrectly flagged. Type II is a
truth-brem event or interval missed. The score never steered the fit.

## Event-scan errors

| Threshold | Type I, no-brem | Type II, any brem | Type II, loss >=0.2% | Type II, loss >=1% |
|---:|---:|---:|---:|---:|
| 0.95 | 93/100 = 93.0% | 2/99 = 2.0% | 2/79 = 2.5% | 1/49 = 2.0% |
| 0.977249868 (one-sided 2 sigma) | 91/100 = 91.0% | 6/99 = 6.1% | 6/79 = 7.6% | 3/49 = 6.1% |
| 0.998650102 (one-sided 3 sigma) | 82/100 = 82.0% | 12/99 = 12.1% | 11/79 = 13.9% | 7/49 = 14.3% |

The event-level ROC AUC was 0.484, consistent with no useful discrimination.
The largest score occurred at hit 220 or later in 88/100 no-brem events and
87/99 brem events. It saturated exactly at one in 56/100 no-brem and 51/99
brem events. Consequently, an odd-seed null calibration chose a threshold of
exactly one; ties still produced a 29/50 = 58% held-out Type-I error. A naive
maximum over about 230 correlated and miscalibrated surface scores is not a
usable event decision.

## Interval-local errors

| Threshold | Type I, 45,673 no-brem intervals | Type II, 155 brem intervals | Type II, loss >=0.2% | Type II, loss >=1% |
|---:|---:|---:|---:|---:|
| 0.95 | 5,343/45,673 = 11.70% | 133/155 = 85.81% | 79/96 = 82.29% | 39/54 = 72.22% |
| 0.977249868 (one-sided 2 sigma) | 3,823/45,673 = 8.37% | 139/155 = 89.68% | 83/96 = 86.46% | 42/54 = 77.78% |
| 0.998650102 (one-sided 3 sigma) | 2,005/45,673 = 4.39% | 145/155 = 93.55% | 87/96 = 90.62% | 45/54 = 83.33% |

The interval-level ROC AUC was 0.539. The median score was 0.549 for no-brem
intervals and 0.551 for brem intervals. Thus the low event-level Type-II rate
comes from scanning many surfaces, not from locating the true radiative
interval. At the correct interval the identity-pair score misses most losses,
including most losses above the nominal tracker-resolution scale.

## Conclusion

The current one-sided Gaussian score cannot by itself judge whether or where
brem occurred. The event maximum gives high apparent efficiency only by
accepting catastrophic false-positive rates, while a local threshold with
moderate false-positive rate misses most truth-brem intervals. The observed
outer-surface saturation and near-random AUC confirm that the zero F/B
cross-covariance approximation and present backward-message construction are
not calibrated for this decision.

Do not use this score to steer the reverse filter or the proposed hit-0 bank.
A subsequent detector needs a well-defined outer-hit likelihood message or a
joint covariance treatment, explicit boundary handling, and a coherent
change-point/path statistic calibrated on held-out no-brem and truth-brem
events rather than a maximum of per-surface Gaussian CDF values.

Generated event, interval, and rate tables are under
`/tmp/gsf-doubleoff-brem-detector/` and are intentionally not tracked.
