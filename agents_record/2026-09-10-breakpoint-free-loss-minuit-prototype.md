# Isolated free-loss Minuit prototype — 2026-09-10

## Scope and result

User authorized trying the proposed free-b fitting method after removing the
old relinearization iteration. This study does not restore those iterations.
No maintained RecBreakpoint, GSF, shared KF source, production card, or batch
script was changed. No remote operation was performed.

On seven topology-clear single-selected-interval tracks, Minuit2 converges,
and five have smaller absolute RTS pT residual than the ordinary zero-centered
sigma_b=0.001 fit; two worsen. The hard-loss counterexample 1:47 becomes an
overshoot despite a substantially lower objective. This is a working scalar
minimization experiment, NOT an established tracking improvement or globally
optimal physical fit.

## Implementation and fixed choices

Study directory:
`TrackingPerformanceStudies/breakpoint_free_loss_minuit_20260910/`

- `prototype/FreeLossProbe.h`: human-readable scalar scan/minimization helper.
- `prototype/RecBreakpoint.cpp/.h`: isolated algorithm wrapper snapshot.
- `prototype/build_probe.py`: compiles a distinct
  `RecBreakpointFreeLossProbe` algorithm in `libFreeLossProbe.so`, reusing the
  already built, unmodified ordinary helper objects and truth reader.
- `prototype/run_probe.py`: snapshot card importing that distinct algorithm.
- `run_trials.py`: direct local job driver, not Condor.
- `analyze.py`: event-keyed comparisons, exact regressions, profile plots.
- `runs/`: ordinary flat tuples, verbose states, trial/minimum CSVs, logs.
- `results.csv`, `analysis.json`, `profile_seed*_event*.png`: derived results.
- `prototype/provenance.json`: source/library SHA-256 provenance.

Only one selected interval is supported by this prototype. The interval is
selected by the maintained Truth interval selector; this is an oracle-location
test, not reconstruction-based discovery. Amount fitting does not use truth:

```text
b = log(p_before / p_after)
fractional loss = 1 - exp(-b)

each trial b:
  start again from the same original seed and covariance
  apply deterministic exp(b) curvature mapping at the selected upstream hit
  run native KF measurements + RTS + paired backward continuation
  return complete RTS smoothed quadratic score

outer optimization:
  scan, then Minuit2/MIGRAD with multiple truth-independent starts
  truth b evaluated only after blind minimization is complete
```

Internal trial settings are LocalMarginal, sigmaLogLoss=0, trial meanLogLoss=b,
and no per-interval center map. This deliberately fixes b conditionally; it
does not assign a Gaussian fitting prior to b. Public positive-sigma validation
and the ordinary sigma_b=0.001 workflow are unchanged. Each successful trial
checks that the inferred loss is exactly b with zero variance. Native MS is
on, deterministic Eloss off. SeedScale=BackwardSeedScale=1, FirstMiddleLast
prefit. Every trial uses the existing fitter; no alternate measurement algebra.

The minimized scalar includes original-V hit penalties, native process terms,
and the original seed penalty once. It has no b-prior penalty. The paired
backward result uses the RTS-selected b; backward chi2 is NOT independently
minimized or added to the RTS score.

Search bounds b=[0,1], corresponding to losses [0,63.2121%], identical for all
tracks. Scan b values: 0,.0005,.001,.002,.005,.01,.02,.04,.08,.15,.3,.5,.75,1.
MIGRAD starts from the best scan point, .005 and .05; step=.002, tolerance=.001,
precision=1e-8, ErrorDef=1, max calls=180 per start. The numerical step is not
a physical sigma_b. Fine scan uses 21 offsets of .001 around the minimum,
clipped to the same bounds. Repeated uncached minimum evaluations check
determinism. No uncertainty interval or full free-fit endpoint covariance is
published: fixed-b KF covariance is conditional and omits fitted-b uncertainty.

The inner KF/RTS remains a one-pass linearized trajectory calculation with
trial-dependent transport/process covariance. Its minimum is not certified as
an exact nonlinear global least-squares or normalized likelihood optimum.
Delta-chi2=1 lines on plots are diagnostic only, not calibrated confidence
limits. No claim that these seven hand-selected tracks represent a population.

## Results

Residuals below are 100*(pT_reco-pT_truth)/pT_truth, in percent.
Fixed-truth means b fixed at truth, sigma_b=0 inside the same trial workflow;
it is NOT the maintained truth-prior endpoint with sigma_b=0.001.

| Seed:event | Interval i -> i+1 | Truth loss % | Free loss % | KF residual % | Ordinary RTS % | Free-b RTS % | Fixed-truth RTS % |
|---|---:|---:|---:|---:|---:|---:|---:|
| 2:68 | 7 | 3.1582 | 4.0933 | -2.5192 | -2.5863 | +0.7911 | -0.0145 |
| 1:1 | 153 | 1.1514 | 1.4478 | -0.0277 | -0.1006 | +0.1459 | +0.0944 |
| 1:2 | 6 | 1.0924 | 1.5475 | -0.8603 | -0.8866 | +0.6257 | +0.1730 |
| 1:40 | 7 | 1.7280 | 0.7288 | -1.8328 | -1.8484 | -1.1685 | -0.2088 |
| 1:47 | 7 | 22.1634 | 38.5169 | -19.6234 | -19.8449 | +27.7236 | +1.6410 |
| 1:82 | 6 | 3.1213 | 1.4477 | -2.9977 | -3.0022 | -1.6022 | +0.0764 |
| 12:11 | 10 | 0.5249 | 0.3588 | -0.5845 | -0.6017 | -0.2806 | -0.1254 |

All seven have Minuit status 0. The previous sigma_b=0.001 comparison
classified 1:40 and 1:82 as ordinary-chi2-better; the other five as
truth-prior-chi2-better. Both classes were intentionally tested.

There are three invalid extreme trial points for seed12:11 (b=.75, 1 and
approximately .951987), rejected by the native propagation/hit update and
recorded with a penalty score and validity=false. They are not accepted
minima. No trial failed in the seed1 or seed2:68 profiles. Finite penalization
does not establish smoothness across an invalid region or global convergence.

Seed12:17 is a separately reported secondary-activity control, not included
above. Its scan winner is b=1 (upper bound), loss=63.2121% against truth
42.5474%, free RTS residual +56.0554%, ordinary -42.6373%, fixed-truth -0.1317%.
The best point is the scan boundary (status -99 sentinel), not a certified
interior Minuit minimum. Seed12:16 has multiple selected intervals: the
ordinary regression runs, but the scalar profiler explicitly skips it.

## Counterexample: a lower score does not guarantee a better loss

Seed1:47, truth pT=39.207481384277344 GeV:

| Quantity | Fixed-truth b | Free minimum b |
|---|---:|---:|
| b | 0.25055888026490408 | 0.48640734275095571 |
| Complete RTS chi2 | 667.7405075832288 | 524.2741172769798 |
| Forward increment sum | 667.9935428188124 | 524.5517807665972 |
| Backward increment sum | 709.3317981379854 | 557.9819757684845 |
| Native smoothed hit-only sum (diagnostic) | 521.0427179611844 | 487.0831052339020 |
| RTS pT GeV | 39.85088500104272 | 50.07720050184992 |

Thus the wrong loss is preferred not only by the affine RTS score but also
by the independently recorded forward score and native hit-only diagnostic.
This does not identify the physical cause: within-interval loss placement,
the trajectory/model approximation and measurement fluctuations remain to be
separated. Do not state any one of them as proven by this experiment.

## Regression, provenance and operational details

The maintained installed libRecBreakpoint.so SHA256 remains:
`06eec6427c38e8789488dc69ebfd1f8d60f76da082b3d5d1776ff82a370b8332`.
All recorded source and library hashes remain unchanged after the study.

All 104 ordinary flat fields reproduce exactly (equal NaNs) against the
iteration-removal after-gate for seed2:68 and seed12:11,16,17. Seven clean
events also reproduce the stored batch ordinary RTS and truth-prior RTS pT
exactly, with generator pT/event/track identities checked.

An initial seed1 job requested event170, but the preserved tracker input has
100 events and stopped at EOF after producing the five available selected
events. This was a selection/input-length issue, not a fit failure. A second
job (`seed1_confirm`) requests only 1,2,40,47,82 and exits successfully. Its
104-field ordinary rows, all scalar trials, minima and full fitted-minimum
state/covariance dumps reproduce the first run exactly. Never count event170
as tested. No input data or generated outputs were deleted.

Next useful investigation: explain the robust wrong minimum for 1:47 using
per-surface measurement/process contributions and the loss mapping geometry,
before exposing this free-loss optimizer in the maintained production card.
Keep fixed-truth, ordinary-prior and free-loss hypotheses explicitly distinct.
