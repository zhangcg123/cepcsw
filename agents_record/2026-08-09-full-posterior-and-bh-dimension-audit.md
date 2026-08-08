# Full-posterior estimator and BH-dimension audit

Date: 2026-08-09

## Purpose and fixed baseline

The preceding 500-event screen found that the scalar IP-kappa marginal-density
mode repaired 19 clean positive tails but lost seven genuine light-eBrem
recoveries. This study tested the recorded explanation that correlations
between kappa and the other four helix parameters might let a full
five-dimensional posterior mode retain those seven radiative solutions. It
then used the new broad Geant4 sample to test whether momentum, angle, or
tracker transition is a justified additional dimension of the BH prior.

Filtering, reduction, and publication remained fixed at the production
baseline: `CEPC2GeV85StepConditioned`, `MaxComponents=12`, posterior cutoff and
`SymmetricKL`, identity-lineage protection, forward-posterior reverse weights,
reverse full-covariance scale 100, `AggregateWeight`, and reverse `BestBranch`.
No runtime property or production estimator was added.

## Full final-posterior contrast

Temporary bounded instrumentation captured each final reverse component's
five-parameter IP mean, packed 5x5 covariance, normalized weight, process
history, and lineage. It was run on the exact 26-event contrast consisting of
the 19 clean positive tails repaired by the scalar mode and the seven genuine
light recoveries lost by it. All 26 selected events completed.

The offline estimators were defined before inspecting their truth residuals:
the scalar kappa marginal-density mode; the full five-dimensional
Gaussian-mixture density mode after numerical whitening; the same full mode
after removing covariance correlations; the component centre having the
greatest exact mixture density; and the kappa marginal mean and median.

| cohort and estimator | inside +-1% | median absolute residual | maximum absolute residual |
|---|---:|---:|---:|
| 19 clean: aggregate branch | 0/19 | 2.662% | 288.136% |
| 19 clean: scalar mode | 19/19 | 0.165% | 0.411% |
| 19 clean: full 5D mode | 19/19 | 0.165% | 0.411% |
| 19 clean: diagonal-covariance 5D mode | 19/19 | 0.166% | 0.403% |
| 19 clean: maximum-density component centre | 18/19 | 0.153% | 2.662% |
| 19 clean: marginal mean | 1/19 | 4.464% | 336.865% |
| 19 clean: marginal median | 8/19 | 1.244% | 370.981% |
| 7 light: aggregate branch | 7/7 | 0.765% | 0.880% |
| 7 light: scalar mode | 0/7 | 1.333% | 1.819% |
| 7 light: full 5D mode | 0/7 | 1.334% | 1.819% |
| 7 light: diagonal-covariance 5D mode | 1/7 | 1.340% | 1.851% |
| 7 light: maximum-density component centre | 1/7 | 1.384% | 1.819% |
| 7 light: marginal mean | 6/7 | 0.643% | 1.427% |
| 7 light: marginal median | 5/7 | 0.865% | 1.235% |

The full 5D and scalar modes are effectively identical, and suppressing all
off-diagonal correlations changes almost nothing. The hypothesis in
`2026-08-08-qoverp-density-mode-500-event-screen.md` that the seven failures
are caused by discarded helix-parameter correlations is therefore disproved.
That older record remains an accurate record of the scalar experiment, but
its proposed explanation is superseded here.

The extra dimensions do not supply a hidden discriminator. The ratio of
radiative aggregate weight to identity weight has median 2.085 in the clean
contrast and 1.950 in the light contrast. The corresponding kappa-sigma ratios
are 5.375 and 3.956, while full covariance-volume ratios are 5.386 and 3.888.
Thus nearly all differential density-volume penalty is already in kappa. The
non-kappa Mahalanobis-distance distributions overlap: clean median 3.677
(range 1.396--20.516) and light median 2.676 (0.081--16.678).

A continuous hybrid between probability mass and density height would require
a covariance-penalty exponent. The eventwise crossover exponent has clean
range 0.065--1.252, median 0.465, and light range 0.231--0.659, median 0.557.
The overlap leaves no truth-free tuning interval. Process signatures overlap
as well: clean false branches and genuine light branches both concentrate at
surfaces 5--8 and predominantly use g2. Mean and median retain more genuine
recoveries but catastrophically fail clean-track safety. No natural final
posterior summary passes both gates.

## Broad Geant4 BH audit

The new 50-file simulation sample yielded 1,166,680 authoritative primary-
electron Geant4 transitions from 5,000 events. Using eBrem-attributed loss and
the same five loss strata, 1,166,602 transitions were inside the model's t/X0
range and 4,685 contained eBrem.

Against the current model-training source, broad-sample radiative-tail
fractions differ modestly by t/X0 bin:

| t/X0 centre | training tail | broad tail | absolute change | significance |
|---:|---:|---:|---:|---:|
| 0.000050 | 0.0784% | 0.0858% | +0.0074 point | +2.25 sigma |
| 0.000224 | 0.3020% | 0.2658% | -0.0362 point | -0.30 sigma |
| 0.001000 | 0.7440% | 1.0166% | +0.2725 point | +3.23 sigma |
| 0.003162 | 4.4830% | 5.1026% | +0.6196 point | +1.53 sigma |
| 0.007071 | 8.9449% | 9.2072% | +0.2623 point | +0.85 sigma |
| 0.012247 | 14.7385% | 13.6768% | -1.0616 points | -2.50 sigma |
| 0.017321 | 18.7102% | 18.8964% | +0.1862 point | +0.19 sigma |
| 0.024495 | 25.3033% | 26.0753% | +0.7720 point | +0.32 sigma |

These are localized sub-percentage-point to one-percentage-point shifts, not a
coherent global failure of t/X0 conditioning.

For a stronger dimensional test, source files were split by seed parity into
independent 2,500-event halves. Logistic occurrence models used t/X0-bin
dummies as the baseline and separately added incident-momentum bins
(<20, 20--30, 30--40, >=40 GeV), folded polar-angle bins, or transition groups
(0--4, 5--6, 7--11, >=12). Neither half found a significant added occurrence
effect: p-values for momentum, angle, and transition were respectively
0.459/0.483/0.788 in one half and 0.865/0.307/0.696 in the other. A model with
all dimensions improved held-out log likelihood by only 1.66 and 0.97 over
approximately 0.58 million transitions per half.

Conditional eBrem severity also fails the reproducibility gate. A transition
effect on the probability of at least 5% loss was nominally significant in
one half (p=0.025) but worsened the other half's log likelihood by 4.26; the
reverse training direction was non-significant and improved held-out log
likelihood by only 2.32. Continuous fractional-loss fits show a small,
repeatable tendency for the outer `>=12` group to have lower loss, but reduce
held-out squared error by only 0.23/73.11 and 0.05/70.48. The effects for the
active surfaces 5--11 change magnitude or sign between halves. Momentum and
angle do not improve held-out fractional-loss prediction.

## Decision and next direction

Reject the full 5D posterior mode and the entire tested family of final
mean/median/mode/component-centre publication heuristics. KL reduction can
amplify or pool an already-supported mode, but it is not the principal source
of this ambiguity, and final-state geometry cannot recover information that
is absent from the reduced likelihood competition.

Also reject an immediate multidimensional BH model keyed by energy, angle, or
literal layer. The broad truth sample gives no reproducible energy/angle
dependence after t/X0 conditioning, and no stable surface-5--11 loss-shape
dependence. The small outer-tracker effect is irrelevant to the active
surface-5--8 failures and is too weak to justify a new model. A literal layer
table would encode sample-specific fluctuations and geometry labels rather
than a demonstrated physical correction.

The evidence now points upstream of final publication: audit calibration of
the reverse measurement likelihood at decisive surfaces 5--8. On independent
clean and light cohorts, compare whitened innovation pulls and covariance
coverage for identity and radiative hypotheses, and test whether repeated use
of measurements or direction-dependent state/covariance transport makes the
reported likelihood ratios overconfident. Only a reproducible likelihood-
calibration defect, or a separately held-out material effect expressed in
physical variables, can justify an implementation candidate.

All final-component instrumentation and offline outputs were temporary and
were removed. `GsfAlgorithm.cpp` exactly matches the committed source. A
normal rebuild/install followed by seed 16/event 14 completed successfully
with the baseline configuration and emitted no experimental diagnostic.
