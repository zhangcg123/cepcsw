# Material-matched exact g2 crossing diagnostic

Date: 2026-07-14

## Scope

This diagnostic follows the discrete approximately 2.05 GeV attractor in
three topology-clean negligible-loss pathologies and two material-matched
controls. It supports the existing light-tail focus and does not replace or
redirect `AGENTS.md`.

All runs use the installed posterior-order flow, `MaxComponents=24`, aggregate
weight selection, reverse BestBranch, and comprehensive dumps. The package was
rebuilt and fully installed after adding diagnostic-only reverse logging of
the actual pre-update child weight and exact predicted measurement, residual,
measurement covariance, and innovation covariance. Filtering and selection
logic are unchanged.

## Parser compatibility correction

The existing `analyze_positive_lcio_amplification.py` assumes that
`reverse-pre-reduce` contains a pre-measurement mixture. Under posterior-order
reduction it contains the post-measurement posterior, so using that stage to
reconstruct prior odds would apply the likelihood twice. No new-flow claim
should use those reconstructed odds. The new verbose line logs
`priorWeight` directly before the measurement likelihood. The reusable exact
parsers are:

- `Reconstruction/RecGsfTracking/scripts/compare_reverse_exact_hit.py`
- `Reconstruction/RecGsfTracking/scripts/scan_reverse_posterior_crossing.py`

## Material-matched hit-4 comparison

Pathology 164/3 and controls 340/9 and 486/3 have essentially the same true
Geant4 transition-4 thickness: respectively 0.009653204, 0.009653060, and
0.009652459 X0. Their truth loss is negligible: 0.0933%, zero, and 0.0386%.
The controls were selected before examining their verbose likelihood terms;
both have truth-like LCIO and stored GSF results.

| event | Truth/LCIO/GSF pT [GeV] | g2/identity prior odds | likelihood ratio | posterior odds | result |
|---|---|---:|---:|---:|---|
| 164/3 | 2.0004/1.9996/2.0444 | 0.02580 | 42.779 | 1.1038 | g2 crosses |
| 340/9 | 2.0004/1.9993/1.9993 | 0.02580 | 3.304 | 0.0853 | identity stays |
| 486/3 | 2.0004/1.9990/1.9990 | 0.02470 | 0.663 | 0.0164 | identity stays |

For 164/3, identity versus g2 has residual vectors
`[0.26914, -0.01318]` and `[0.05459, -0.01746]`, delta-chi2 8.393 and
0.193, and log(det S) -8.937 and -8.249. The first-coordinate innovation
variances are 0.008658 and 0.017532. Thus g2 is correctly penalized for its
broader innovation, but its shifted prediction removes the large identity
residual and gives a 42.8 likelihood ratio, just exceeding the approximately
39:1 prior penalty.

For 340/9 the corresponding first residuals are 0.16687 and -0.04756,
with nearly identical S matrices to 164/3. The improvement gives only a 3.30
likelihood ratio, far below the same prior penalty. For 486/3 the first
residual changes from 0.07887 to -0.13316 and g2 is disfavored. Therefore the
special property of 164/3 is not transition material, BH prior, or a visibly
different innovation covariance; it is the local residual alignment with the
discrete g2 mean shift.

## Two additional pathologies

In 301/6 and 248/4 the selected surface-6 g2 child does not immediately defeat
identity at hit 6. Its hit-6 posterior odds are only 0.0558 and 0.0506. The
lineage survives reduction and crosses at the next inward measurement, hit 5:

| event | Truth/LCIO/GSF pT [GeV] | crossing | prior odds | likelihood ratio | posterior odds |
|---|---|---:|---:|---:|---:|
| 301/6 | 2.0004/2.0010/2.0493 | hit 5 | 0.06650 | 61.063 | 4.0608 |
| 248/4 | 2.0004/2.0045/2.0718 | hit 5 | 0.06685 | 1182.93 | 79.079 |

For 301/6, identity versus radiative first-coordinate residual is 0.31438
versus -0.07720; delta-chi2 is 12.441 versus 3.164. For 248/4 it is 0.52930
versus 0.19537; delta-chi2 is 16.573 versus 1.777. The second residual
coordinate changes much less in both comparisons. The g2-lineage innovation
is again broader and pays the log-determinant penalty, but the first-coordinate
residual reduction dominates.

The material-matched controls have no reverse posterior stage at which any
radiative component exceeds the exact identity component.

## Mechanism conclusion

The confirmed approximately 2.05 GeV family is a residual-triggered discrete
mode selection. A g2 child with a few-percent prior survives at an informative
surface. A later inward measurement has a roughly 3--4 sigma identity residual
in its first measurement coordinate. The g2 mean shift and broader innovation
explain that residual well enough to overcome both the BH prior and the full
`det(S)^(-1/2)` penalty. BestBranch then publishes the radiative mode.

This is not evidence that likelihood normalization, KL deletion, material
thickness, or pre-measurement reduction is wrong. It localizes the remaining
question: why topology-clean negligible-eBrem tracks contain this tail of
first-coordinate reverse residuals, and whether the identity process model
under-represents non-radiative scattering/straggling or correlations. A
proper next population diagnostic should measure the first-coordinate
identity pull distribution at surfaces 4--8 using the exact S, with
material-matched controls, before changing covariance or selection.

## Validation boundary and outputs

Three pathologies and two controls establish a repeatable mechanism but not a
calibrated solution. Do not introduce an evidence threshold or global
covariance change from this result. Comprehensive logs are disposable outputs:

```text
/tmp/gsf-exactdiag-164-3.log
/tmp/gsf-exactdiag-301-6.log
/tmp/gsf-exactdiag-248-4.log
/tmp/gsf-exactdiag-340-9.log
/tmp/gsf-exactdiag-486-3.log
```
