# q/p-marginal density-mode 500-event screen

Date: 2026-08-08

## Purpose and fixed baseline

The initial eleven-event counterfactual suggested that maximizing the final
one-dimensional Gaussian-mixture density in IP kappa (the q/p-like helix
curvature parameter) could avoid some representation-dependent
`BestBranch` decisions. This follow-up tested that observation on substantially
more events before considering any implementation.

The filtering and smoothing calculation was unchanged. The run used the
five-component `CEPC2GeV85StepConditioned` model, `MaxComponents=12`,
`ReductionTargetComponents=0`, `SymmetricKL`, identity-lineage protection,
`ComponentWeightCutoff=1e-4`, forward-posterior reverse weights, reverse full
covariance scale 100, the independent reverse refit, and
`AggregateWeight`/`BestBranch`. The rejected 24-component study was not
repeated.

Temporary diagnostic output recorded every final reverse component's
IP-extrapolated kappa mean, kappa variance, normalized weight, and identity
lineage flag. Offline code maximized the normalized one-dimensional Gaussian
mixture directly, using interval scans and local numerical refinement. No
truth quantity entered the estimator and no residual threshold was tuned.
All temporary C++ instrumentation was removed after the run.

## Cohort

The input catalogue was reconstructed from the fresh 4,800-event broad
electron sample. Its counts exactly reproduced the established survey:
1,537 no-eBrem, 2,325 light-eBrem, and 938 hard-eBrem before topology removal;
and 1,534, 2,182, and 539 after excluding secondary tracker activity. Seeds
32 and 36 were excluded in full.

The deterministic, deliberately outcome-enriched 500-event cohort contained:

- 250 topology-clean no-eBrem events: all 37 known positive-tail cases plus
  213 clean preserved controls;
- 250 topology-clean light-eBrem events: 56 truthlike preserved, 50 good
  recoveries, 41 misses, 40 truthlike degradations, 31 partial recoveries,
  25 overshoots, and 7 near-core cases.

This cohort is a causal stress test, not a population-weighted performance
sample. It spans the available energy and angle bins. Same-code rerun
integrity was exact at the relevant precision: none of the 500 current
aggregate residuals differed from the stored catalogue by more than 0.01
percentage point.

## Results

Residuals below are relative pT residuals in percent. `width68` is the central
68% interval width used by the analysis. Improvements/worsenings count an
absolute-residual change greater than 0.1 percentage point.

| cohort | estimator | median | width68 | within +-1% | >+1% | >+5% | abs >10% |
|---|---|---:|---:|---:|---:|---:|---:|
| all 500 | aggregate branch | -0.0003 | 1.2436 | 319 | 101 | 16 | 10 |
| all 500 | kappa density mode | -0.0373 | 0.7696 | 353 | 57 | 7 | 2 |
| no-eBrem 250 | aggregate branch | +0.0629 | 0.3505 | 213 | 37 | 7 | 5 |
| no-eBrem 250 | kappa density mode | +0.0420 | 0.2362 | 232 | 18 | 2 | 1 |
| light-eBrem 250 | aggregate branch | -0.1047 | 1.7999 | 106 | 64 | 9 | 5 |
| light-eBrem 250 | kappa density mode | -0.3057 | 1.4857 | 121 | 39 | 5 | 1 |

Across the enriched sample the mode recovered 41 events into the +-1% core
and lost seven, with 54 improvements and 23 worsenings. The aggregate gains
therefore conceal an important asymmetric failure.

### Clean-track behavior

Among the 37 selected no-eBrem positive-tail cases, the mode recovered 19
into +-1%, reduced >+5% cases from seven to two, and reduced abs >10% cases
from five to one. It worsened none by more than 0.1 percentage point. All 213
already preserved clean controls remained inside +-1%; their width68 changed
from 0.2209% to 0.2115%.

The nearest component to the final scalar mode was identity-lineage in only
117/250 no-eBrem cases. The repair is therefore not equivalent to always
selecting the protected identity branch.

### Light-eBrem behavior and safety failure

The mode produced large gains for the deliberately selected 40
truthlike-before-GSF but GSF-degraded cases: 20 returned to +-1%, the median
moved from +1.8398% to -0.0304%, and >+1% cases fell from 34 to 14. It also
reduced the overshoot subset's >+1% count from 25 to 20.

However, it failed the genuine-recovery gate:

- the 50 aggregate good recoveries fell from 50/50 to 43/50 within +-1%; ten
  worsened and only two improved by more than 0.1 percentage point;
- partial recoveries worsened in six cases and improved in one, with their
  median shifting from -2.4355% to -2.7543%;
- the 41 missed recoveries were essentially unchanged.

In each of the seven lost good recoveries, the scalar mode moved back close
to the original LCIO residual while the aggregate state had correctly
recovered the momentum. Their mode-nearest components comprised four
identity-lineage and three radiative-lineage components. Across all 250 light
events the split was 80 identity-lineage versus 170 radiative-lineage. The
failure is therefore not a hidden global identity selection.

## Interpretation and decision

The larger test confirms the representation-dependence mechanism: a narrow
peak can be a better description of the one-dimensional marginal density
than the largest reduced component's pooled weight, and this removes many
false positive tails without disturbing ordinary clean controls.

But one-dimensional marginalization discards correlations between kappa and
the other four helix parameters. In some genuine light-eBrem recoveries the
correct radiative solution can be broader in kappa yet better supported in
the joint state. The scalar mode then prefers a narrow LCIO-like marginal
peak and loses the recovery. The seven observed losses are direct evidence
of this failure mode, not merely a theoretical concern.

Consequently, the scalar kappa density mode is rejected as a publication
candidate. It must not be implemented by splicing its kappa value into an
unrelated component state. Its clean-tail improvement remains useful causal
evidence for investigating a representation-stable full five-dimensional
posterior estimator. That investigation must explicitly contrast the seven
lost genuine recoveries with the 19 repaired clean tails and preserve the
joint state/covariance geometry. No configurable property was added by this
study.

