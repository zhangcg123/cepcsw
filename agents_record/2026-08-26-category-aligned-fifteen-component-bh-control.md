# Category-aligned fifteen-component BH control

## Decision boundary

`CEPCRuntimeCategoryAligned15Clear` is a new parallel, default-off Bethe-Heitler
proposal bank. It does not replace the production
`CEPC2GeV85StepConditioned` model, does not change its table or interpolation,
and is not physics validated. Runtime evaluation still depends only on the
scalar `DD4hepBetweenSurfaces` `pathTX0` supplied to the splitter.

The model retains the eight t/X0 knots and the knot-local total radiative
probability of `CEPCRuntimeCategoryAligned9Clear`. It changes only how each
knot's radiative probability is partitioned among fixed retained-fraction
proposals.

## Proposal geometry

There are 15 total components: one exact identity atom and 14 radiative
components. Loss is `1-z`.

| index | aggregate-loss stratum | center | sigma | role |
|---:|---:|---:|---:|---|
| 0 | no eBrem | 0 | 0.0001% | exact identity (`z=1`, variance `1e-12`) |
| 1 | 0--0.1% | 0.05% | 0.025% | 0--1% bank |
| 2 | 0.1--0.2% | 0.15% | 0.025% | 0--1% bank |
| 3 | 0.2--0.4% | 0.30% | 0.050% | 0--1% bank |
| 4 | 0.4--0.7% | 0.55% | 0.075% | 0--1% bank |
| 5 | 0.7--1.0% | 0.85% | 0.075% | 0--1% bank |
| 6 | 1.0--1.4% | 1.20% | 0.100% | 1--6% bank |
| 7 | 1.4--1.9% | 1.65% | 0.125% | 1--6% bank |
| 8 | 1.9--2.6% | 2.25% | 0.175% | 1--6% bank |
| 9 | 2.6--3.5% | 3.05% | 0.225% | 1--6% bank |
| 10 | 3.5--4.7% | 4.10% | 0.300% | 1--6% bank |
| 11 | 4.7--6.0% | 5.35% | 0.325% | 1--6% bank |
| 12 | 6--10% | 8.0% | 1.0% | merged former 6--8% and 8--10% region |
| 13 | 10--30% | 20% | 5% | first broad high-loss proposal |
| 14 | 30--100% | 65% | 17.5% | second broad high-loss proposal |

The centers are deliberately not equally spaced. Their successive distances
from identity are 0.05, 0.10, 0.15, 0.25, 0.30, 0.35, 0.45, 0.60, 0.80,
1.05, 1.25, 2.65, 12, and 45 percentage points. Each radiative mean is the
midpoint of its loss stratum and each sigma is one quarter of the stratum
width, so neighboring two-sigma bounds meet without gaps while proposal
spacing grows toward the broad tail.

## Fit input and statistics

Weights are direct counts from the exact topology-clear runtime-interval
dataset under `/tmp/gsf_runtime_bh_closure_20260821/`. The deterministic seed
split uses seed modulo five: 329 files and 6,610,482 fitted training rows have
nonzero remainder; 82 files and 1,567,286 fitted held-out rows have remainder
zero. The packaged JSON preserves these counts and the split definition.

Nineteen of the 8 x 15 knot/component training cells contain fewer than 25
entries; no cell is empty. Sparse high-t/X0 cells therefore require caution.
The sum of radiative weights and the identity weight at every knot are exactly
the same as in the nine-component model; this is a proposal-resolution change,
not a refit of the overall eBrem probability.

Authoritative artifacts:

- `Reconstruction/RecGsfTracking/data/CEPCRuntimeCategoryAligned15Clear/cepc_runtime_category_aligned15_clear.json`
- `Reconstruction/RecGsfTracking/data/CEPCRuntimeCategoryAligned15Clear/compiled_table.inc`

## Mechanical validation

- `RecGsfTracking` and `RecGsfFlatTuple` built and installed successfully in
  the EL9/LCG 105 development build.
- The packaged table has shape 8 x 15, finite positive normalized weights,
  physical retained-fraction means and variances, and an exact positive
  identity component (`mean_z=1`, `variance_z=1e-12`) at every knot. The
  runtime initialization log confirms the canonical selector/name round trip;
  the exact identity also satisfies the global-loss prior contract.
- Verbose reverse runs of focused events 11, 16, and 17 completed. All 15
  component indices appeared in persisted lineage records.
- An independent JSON evaluator checked 9,525 recorded BH child nodes. The
  runtime weight, mean, and variance each matched the packaged table with zero
  observed absolute difference.
- The focused full-mixture pT residuals were +0.4121%, -51.7203%, and +0.2008%
  for events 11, 16, and 17 respectively. Event 17 remains a recovery and event
  16 remains a severe underestimate; these three events do not validate the
  new model.
- A direct same-code rerun compared the nine- and fifteen-component banks with
  the same input, selected events, `MaxComponents=30`, `ReverseKappaSeedCov=1`,
  `SymmetricKL`, and 5e-3 weight cutoff. FullMixtureMode gave:

  | event | truth pT | LCIO residual | 9-component residual | 15-component residual |
  |---:|---:|---:|---:|---:|
  | 11 | 40.7316 | +0.4024% | +0.4035% | +0.4121% |
  | 16 | 37.8940 | -51.7263% | -51.7201% | -51.7203% |
  | 17 | 18.7970 | -21.2285% | +0.1700% | +0.2008% |

  Thus this focused FullMixtureMode gate contains no improvement from the
  fifteen-component geometry. Event 17 remains a strong recovery relative to
  LCIO under both models, but the nine-component endpoint is 0.0309 percentage
  points closer to truth. BestBranch moves differently for event 17
  (+0.3049% to +0.2155%), which reinforces that proposal coverage and endpoint
  selection must be diagnosed separately.
- A same-template first-ten-event run with `MaxComponents=30` and verbose
  output disabled completed successfully. Peak RSS was 2,881,488 KiB versus
  2,495,396 KiB for the stored nine-component/max-30 control: +386,092 KiB
  (+15.5%). The maximum transient split bank is 30 x 15 = 450 children rather
  than 30 x 9 = 270. Wall time was 3:38 versus 3:57, but this single timing is
  not treated as a speed improvement.
- In that ten-event smoke sample, eight full-mixture pT values moved by less
  than 0.008% of truth. Two events changed their selected endpoint by about
  1.1% of truth in opposite performance directions. This demonstrates that
  the finer proposal geometry can alter lineage selection; it is not a
  population-performance conclusion.

## Low-cutoff, max-150 scale gate

A paired run used the same first ten input events, reverse seed covariance 1,
`SymmetricKL`, truth override off, `ComponentWeightCutoff=1e-3`, and
`MaxComponents=150`. Residual below means `(pT_reco-pT_truth)/pT_truth` and
uses FullMixtureMode.

| event | LCIO residual | 9-component residual | 15-component residual |
|---:|---:|---:|---:|
| 0 | -0.00085170 | -0.00088062 | -0.00080342 |
| 1 | -0.00027673 | -0.00033763 | -0.00024708 |
| 2 | -0.00860261 | -0.00867451 | -0.00861398 |
| 3 | -0.00106972 | -0.00108082 | -0.00103631 |
| 4 | -0.01086499 | +0.00037918 | -0.01080112 |
| 5 | -0.20794018 | +0.01845087 | +0.00733272 |
| 6 | -0.00148332 | -0.00145459 | -0.00135611 |
| 7 | -0.00262772 | -0.00265735 | -0.00253162 |
| 8 | +0.00007701 | +0.00009211 | +0.00016192 |
| 9 | +0.00033116 | +0.00031524 | +0.00045597 |

For FullMixtureMode, mean absolute residual changed from 0.0034323 (BH9) to
0.0033340 (BH15), the 68% absolute quantile worsened from 0.0015989 to
0.0031078, and the maximum absolute residual improved from 0.0184509 to
0.0108011. These aggregate changes are driven by an opposing pair: BH15
improves event 5 but loses BH9's recovery of event 4. Ten events are not a
population-performance result.

The computational cost is substantial:

| model | wall time | peak RSS | final IP components/event | lineage nodes |
|---|---:|---:|---:|---:|
| BH9 | 3:45.65 | 3,763,540 KiB | 23--76 | 158,514 |
| BH15 | 7:27.09 | 6,294,816 KiB | 59--102 | 451,624 |

BH15 therefore used 2,531,276 KiB more peak memory (+67.3%), nearly doubled
wall time, and recorded 2.85 times as many lineage nodes. Both jobs completed,
and FullMixtureMode optimization status was successful for all ten events.

## Required next gate

Use same-code paired reruns on the established topology-clear no/light/hard
sample, retain the secondary-activity population as a separately reported
control, and compare the nine- and fifteen-component lineage crossover before
considering any default change. In particular, check whether the new sub-1%
proposals survive the 5e-3 posterior cutoff and KL reduction, rather than
inferring benefit from their presence in the splitter output.
