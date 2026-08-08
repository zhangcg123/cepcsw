# KL merge causality and representative-state counterfactual

Date: 2026-08-08

## Question

The first KL-focused study asked a narrow causal question: when a reduced
radiative component later wins over the protected identity branch, is the
failure caused by the moment-matched child state being an artificial
trajectory, or by the probability mass pooled into that child?

This was not another option scan. The production configuration remained the
five-component `CEPC2GeV85StepConditioned` model with `MaxComponents=12`,
`ReductionTargetComponents=0`, `SymmetricKL`, identity-lineage protection,
`ComponentWeightCutoff=1e-4`, the independent reverse refit, and
`AggregateWeight`/`BestBranch` publication. The already rejected
24-component capacity comparison was not repeated.

## Focused state traces

Five reverse-only events were traced through every influential reverse hit
and reduction. Three are topology-clean false radiative selections and two
are genuine light-eBrem recoveries.

### Clean false selection: seed 16/event 14

The forward seed was all g0. At reverse hit 5, before reduction, identity had
weight 0.224667, while two important radiative clusters had weights 0.139486
and 0.131774. They represented strong g2 losses at different reverse
surfaces, 7 and 8. Their local Gaussian states were close enough that the
decisive consolidation had symmetric-KL cost 0.15628. Reduction pooled the
radiative support to 0.298206, immediately above identity. The next hit
preserved the ordering. The final published pT was 30.8103 GeV.

### Clean false selection: seed 3/event 6

The forward seed was all g0. At reverse hit 4, before reduction, identity had
weight 0.29085. Radiative support was fragmented among weights 0.14133,
0.12883, 0.11702, 0.08687, 0.07564, 0.07463, and smaller branches. Forced
reduction to 12 components eventually merged radiative clusters of weights
0.22136 and 0.09012 at the high symmetric-KL cost 2.2086. Their child weight
0.31147 overtook identity immediately. The next hit strengthened it to
0.36875 versus identity 0.21516. This is a clear example in which mandatory
high-cost pooling creates the winner, although it does not establish a
general solution.

### Clean false selection: seed 17/event 94

After reverse hit 4 reduction, identity still led at 0.3481 over radiative
clusters of 0.2903 and 0.1600. The relevant merge changed one radiative
cluster only from 0.1546 to 0.1600 and its pT from 25.6452 to 25.6466 GeV.
The following hit's exact measurement likelihood then moved that radiative
cluster to 0.2911 versus identity 0.1998. Here reduction neither creates the
winner nor materially changes the state that receives the decisive
likelihood.

### Genuine recovery: seed 14/event 9

The forward seed was all g0. Identity remained the leader through reverse hit
4 at weight 0.429. At hit 3, the exact measurement likelihood reduced it to
0.00034 before reduction, while radiative component 231 already led at
0.228. Reduction subsequently increased radiative support to 0.377. A
decisive radiative merge joined g2 histories at surfaces 5 and 6 with cost
0.25984. The final pT was 16.039 GeV, about -0.298% from truth.

### Genuine recovery: seed 50/event 8

The forward seed was all g0. By reverse hit 5, before reduction, radiative
support already led identity by 0.4836 to 0.0613. Reduction increased it to
0.5335. At hit 4 the pre-reduction ordering was 0.555 to 0.032 and reduction
increased the radiative component to 0.797. This recovery is established by
the measurement sequence, not created by KL reduction.

These examples expose three mechanisms rather than one: low/moderate-cost
mass pooling can create a radiative winner; forced high-cost pooling can
create one; and a subsequent measurement can select a nearly unchanged
radiative parent regardless of reduction.

## Representative-parent state counterfactual

A temporary diagnostic-only edit changed `momentMerge` to retain the heavier
real parent's continuation-surface state and covariance while still summing
the parent weights and preserving the normal ancestry and lineage accounting.
This isolates moment-state interpolation from mass pooling. It was built and
run on the three clean failures and two genuine recoveries above.

| event | normal moment child pT (GeV) | representative-parent pT (GeV) | selected process result |
|---|---:|---:|---|
| clean 16/14 | 30.8103 | 30.8408 | unchanged, g2 at surface 7 |
| clean 17/94 | 25.6824 | 25.6525 | unchanged, g2 at surface 8 |
| clean 3/6 | 40.5900 | 40.6249 | unchanged, g2 at surface 7 |
| light 14/9 | 16.0390 | 16.0504 | genuine radiative recovery survives |
| light 50/8 | 22.8398 | 22.7764 | genuine radiative recovery survives |

All three false selections persisted, and both genuine recoveries survived.
The moment centroid shifts the final pT modestly but is not the causal source
of these selection decisions. The experimental source edit was fully
reverted, the normal target rebuilt and installed, and a fresh seed 16/event
14 smoke run exactly restored pT 30.8103 and final selected weight 0.3323.
There is no remaining C++ diff from this counterfactual.

## Interpretation

The main reduction problem is representation dependence, not an unphysical
centroid alone. Reduction serves two distinct purposes:

1. approximate the filtering density with a bounded number of Gaussians, for
   which merged probability weights must be conserved; and
2. provide candidates to `BestBranch`, which currently interprets the
   largest reduced-component weight as if it were the posterior probability
   of one physical trajectory.

After several histories are merged, those statements are no longer
equivalent. A final component can contain probability from several loss
surfaces and modes. Its aggregate weight is valid as density mass but is not
necessarily the posterior of one causal trajectory. This explains why the
winner changes with capacity even when identity's state and weight barely
move.

The evidence also rejects a universal surface-history merge prohibition.
False seed 16/event 14 pools adjacent surface-7/8 hypotheses, but genuine
seed 14/event 9 also pools adjacent surface-5/6 hypotheses. Likewise, a
global KL-distance ceiling would address the high-cost seed 3/event 6 merge
but not the low-cost seed 16/event 14 merge or the measurement-driven seed
17/event 94 failure. Such a ceiling could only be a narrow numerical safety
device, not the solution to clean/light discrimination.

`DominantLineage` is not the answer: its full light-population test previously
lost eight events from the +-1% core and produced 104 worsenings versus 59
improvements. `SurfaceConsistency`, TopN, Runnalls, and the 24-component
capacity comparison are also already rejected. A new proposal must not
repackage those controls.

## Next causal test

Do not tune the symmetric-KL formula against truth residuals yet. First test
whether final estimation can be made representation-stable while leaving the
filtering density and exact measurement likelihoods unchanged. The candidate
quantity should be a property of the represented posterior density, such as
its state-space mode or a well-defined marginal mode, rather than the raw
weight of whichever Gaussian happens to contain the most pooled histories.
This is conceptually distinct from `DominantLineage`, which selects one
pre-merge constituent weight, and from `WeightedMean`, which publishes a
global moment.

## First q/p-marginal density-mode screen

Temporary verbose-only instrumentation printed every final reverse
component's IP-extrapolated kappa mean and variance. An offline calculation
then maximized the normalized one-dimensional Gaussian-mixture density in
kappa. It used all components and their weights; it did not select by
generator truth or tune a threshold. The instrumentation was diagnostic only
and introduced no property.

The targeted screen covered the three clean reverse-only failures, one
forward-seeded clean failure, one weak reverse-only light case, two
reverse-only genuine recoveries, and four forward-supported genuine
recoveries:

| event/mechanism | BestBranch residual | q/p-mode residual | interpretation |
|---|---:|---:|---|
| clean 3/6, forced high-cost pool | +2.5510% | +0.0353% | clean core restored |
| clean 16/14, low-cost pool | +1.5568% | -0.0249% | clean core restored |
| clean 17/94, next-hit flip | +1.6336% | +0.1716% | clean core restored |
| clean 11/4, forward-seeded false mode | +2.9629% | +2.9784% | not repaired; false density itself is dominant |
| light 45/7, weak reverse-only mode | -0.7344% | -0.9480% | remains inside 1%, but worsens |
| light 14/9, genuine reverse-only | -0.2983% | -0.2674% | recovery retained |
| light 50/8, genuine reverse-only | +0.6012% | +0.6033% | recovery retained |
| light 20/3, genuine forward support | -0.6389% | -0.6310% | recovery retained |
| light 26/6, genuine forward support | -0.1428% | -0.0530% | recovery retained |
| light 37/4, genuine forward support | +0.2437% | +0.2453% | recovery retained |
| light 6/9, genuine forward support | +0.0970% | +0.1062% | recovery retained |

In the three repaired clean cases, the narrow identity-region density peak
beats the broader pooled radiative component even though the latter has the
largest raw weight. In every strong genuine recovery, the radiative density
peak remains dominant. The forward-seeded clean failure remains false,
which is an important scope check: a density-mode estimator can remove
representation-induced publication errors, but it cannot repair a posterior
whose likelihood/prior calculation genuinely concentrates on the wrong
state.

This is promising causal evidence, not a candidate implementation or
validation. A one-dimensional kappa mode alone does not define a consistent
five-parameter track state and covariance. The next step is to formulate and
screen a representation-stable full-state estimator, or a principled
conditional state at the kappa marginal mode, on a larger clean/light causal
cohort. It must reproduce the same separation without truth input, retain all
hard recovery, and avoid the weak-light worsening seen in 45/7 before any new
configurable is added.
