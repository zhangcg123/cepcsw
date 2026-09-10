# Free-loss fitting: three consistent normalized likelihood formulations

## Scope and outcome

The user authorized trying forward, backward and smoothed full-track
likelihoods after the three quadratic-only objectives. Implemented and tested
an ISOLATED prototype in
TrackingPerformanceStudies/breakpoint_free_loss_likelihood_20260910.
No maintained RecBreakpoint, RecGsfTracking, shared KF, installed library,
maintained run card or batch script was changed by this experiment.
Existing generated studies are not overwritten or relabeled.
The experimental C++ sources/headers and durable records are checkpointed
locally under the source-tracking law, without moving them into a maintained
package. Generated ROOT files, libraries, plots, CSVs and experiment scripts
are not staged. No remote operation is performed.

There is ONE marginal data-likelihood model, evaluated three ways, not three
independent sources of evidence. Independent Minuit searches using each
formulation give the same endpoints on seed12:11 and the secondary control
12:17. The latter remains a lower-bound, badly reconstructed event.
Normalization eliminates the extreme overshoot of clean event4:11, but large
errors persist in other selected examples. This is not population validation.

## Conditional loss trial and native fitting

b = log(p_before / p_after); loss fraction = 1-exp(-b).

Each outer trial fixes b with sigma_b=0 and calls the existing LocalMarginal
BreakpointFitter algorithm. The ordinary/oracle outputs remain their original
sigma_b=.001 fits. Native KalTest still provides every propagation and actual
measurement update. Only the following passive capture was added to local
COPIES of BreakpointFitter and KalmanAdapter:

- transport F and process covariance Q for every forward edge, including
  the fixed-b loss Jacobian on the selected edge;
- the native hit derivative H, measurement noise V and predicted residual;
- the existing predicted/filtered means and initial seed covariance.

The copied code is compiled into an isolated libFreeLossProbe.so and never
installed. TrackLikelihood.cpp only scores the frozen model; it does not
replace the native filter or publish a different smoothed trajectory.
The existing native RTS/backward endpoints at the optimized b are reported.

FirstMiddleLast, forward SeedScale1, MS on and deterministic Eloss off remain
unchanged. BackwardSeedScale=100 follows the new user default and affects the
published backward endpoint, NOT the forward-model likelihood. The finite
forward seed and its covariance are identical across b trials.
Truth selects the single interval; its loss amount enters only in a separate
post-search diagnostic trial. No truth amount initializes or bounds Minuit.
Bounds remain b in [0,1], i.e. loss0--63.2121%. Identical coarse grid and three
MIGRAD starts are inherited from the preceding experiment. Boundary/status
flags and failed native propagation trials are retained.

## Exactly what model is scored

x_i is the 5D track state at the receiving hit's native pivot, ordered
(drho, phi0, kappa, dz, tanl). xp_i and xf_i are the native forward predicted
and updated expansion states from the trial. The frozen affine model is:

```text
x_0 ~ Gaussian(xp_0, Pseed)
x_i = xp_i + F_i (x_(i-1) - xf_(i-1)) + w_i
w_i ~ Gaussian(0, Q_i)
y_i = h(xp_i) + H_i (x_i - xp_i) + v_i
v_i ~ Gaussian(0, V_i)
```

The angular state difference uses the maintained wrapped-phi convention.
All measurement dimensions at every accepted hit are included exactly once.
The covariance of w_i may be singular. A diagonally scaled eigensystem keeps
its supported directions; normalized eigenvalues below1e-12 are treated as
null, materially negative eigenvalues fail, and QQ-root closure is checked.
There is no inverse of singular Q and no artificial variance in null modes.

After expressing the seed/process variations using independent standard
Gaussian coordinates u and whitening each hit by the Cholesky factor of V:

```text
d = A u + epsilon
u ~ Gaussian(0,I), epsilon ~ Gaussian(0,I)
C = I + A A^T

-2logL = d^T C^-1 d + logdet(C) + sum_i logdet(V_i) + m log(2*pi)
```

d is the full whitened measurement residual relative to the unconditioned
mean propagated from the seed through the frozen affine transitions, not
the vector of residuals to the final smoothed track. m counts all measured
coordinates. This integrates over the seed/process variables rather than
just profiling their best values. Continuous-density -2logL can be negative;
only differences under the same conventions are meaningful here.

Three implementations:

1. **Forward**: QR of [I,A]^T in ascending measurement order yields a square
   root of C. Triangular solves and its diagonal give the quadratic and
   normalization. This is the Gaussian conditional/innovation factorization.
2. **Backward**: reverse the measurement row order and independently repeat
   the factorization. It uses the SAME joint covariance and initial prior;
   it is NOT the copied-forward-seed backward refilter's update chi2 plus
   log determinants. No data-conditioned outer seed is introduced.
3. **Smoothed**: SVD of A gives the joint posterior mean u*. Evaluate the
   complete quadratic ||d-Au*||^2 + ||u*||^2, separately saving measurement,
   seed and process penalties. Add the independently computed posterior
   normalization logdet(I+A^T A) using its singular values. This is not a
   product of independent smoothed-hit residual densities.

All three integrate the same latent states and must agree. They are separate
numerical checks, not tunable alternative statistical models. Disagreement
above1e-4 in -2logL fails a trial. Common data-seed caveat: FirstMiddleLast
prefit uses the observed hits. We condition on that fixed initialization
convention; this is not a calibrated unconditional detector likelihood.
Also, F/H/Q are generated anew along the native trial trajectory for each b;
this is a local affine approximation, not an exact nonlinear trajectory
integral. Normalization does not remove material-placement/model limitations.

## Validation and focused results

An independent dense-covariance Gaussian reference agrees within1e-10 with
all three evaluators for both Q=0 and rank-one Q. Includes correlated 2D hit
noise and unobserved state directions. Build and test executable passed.
ROOT dictionary-location warnings are inherited environment diagnostics.

Focused data: five clean single-breakpoint events plus secondary control12:17.
Event12:16 is retained as a multiple-interval ordinary regression control and
is not scalar-profiled. Ten minimizations total: smoke2:68, three formulations
on12:11/17, and Forward minimization on4:11,3:36,6:38. EVERY valid trial,
including those Forward-only minimizations, evaluates all three formulations.
All816 valid trials agree within1.32e-6 in -2logL. Nine invalid trial evaluations
are native propagation/update failures at large b in the three12:11 searches;
none is a disagreement between the likelihood formulations.
Repeated minimum evaluations pass; local scans and Minuit status remain saved.

All13 ordinary flat rows, every one of104 fields, and full verbose native
state/covariance dumps exactly match the maintained-code scale100 references.
Thus the passive capture/scoring does not change the original outputs.

Residual below = 100*(pT_reco/pT_truth - 1), percent. Old chi2 column is the
previous COMPLETE smoothed-quadratic minimization's RTS endpoint. Its old
backward scale1 does not affect RTS, so this comparison does not confound
the new scale100. New backward endpoints are separately provided in results.

| Seed:event | Truth interval loss % | Fitted likelihood loss % | KF residual % | Old chi2 RTS residual % | Likelihood RTS residual % |
|---|---:|---:|---:|---:|---:|
| 2:68 | 3.15823 | 4.08392 | -2.51915 | +0.79111 | +0.78298 |
| 12:11 | 0.52495 | 0.35539 | -0.58449 | -0.28058 | -0.28371 |
| 4:11 | 0.00962 | approximately0, lower bound | +0.12700 | +172.06433 | +0.08580 |
| 3:36 | 20.45228 | 34.46953 | -20.44427 | +71.93510 | +21.35037 |
| 6:38 | 27.35234 | 39.68571 | -26.49789 | +20.83218 | +20.71325 |

Secondary control12:17: truth loss42.54744%, fitted loss0 at lower boundary,
RTS residual-42.63631% (old+56.05544%). Do not include it in clean counts.
Its sentinel Minuit status-99 means the coarse lower-bound candidate was
retained, not a certified interior minimum. Other minima have status0;
4:11 is nevertheless explicitly flagged as a boundary fit.

On12:11, independent fitted b values are .003560273471153901 (Forward),
.0035602740532386765 (Backward) and .0035602742583691624 (Smoothed).
All publish the exact same pT: RTS9.1251144747800055 GeV and
backward9.1281023517722524 GeV. Differences are numerical optimization noise.

For4:11, comparing b0 to b1 WITHIN THE SAME FROZEN GAUSSIAN MODEL:

```text
                      b=0                 b=1
quadratic             458.743297888        458.701275990
log covariance det  -1403.190369074      -1401.176233219
-2logL                -87.996358239        -86.024244281
```

The large loss improves the quadratic by only0.04202 but pays2.01414 in
normalization. That explains why this artificial overshoot is rejected by
the normalized objective. Other events still have20% errors. No claim of
optimal true energy recovery, calibrated uncertainty or production readiness.
Published endpoint covariances are conditional on fitted b and still omit
the outer optimizer's loss uncertainty.

## Artifacts and resumption

Study files: prototype/{TrackLikelihood.h,TrackLikelihood.cpp,FreeLossProbe.h},
local passive-capture copies, build_probe.py, test_likelihood.cpp, provenance.json,
run.py and analyze.py. Outputs include results.md/results.csv, verification.json,
per-event likelihood profiles and per-trial *_likelihood.csv. Those retain
three normalized values, three quadratics/determinants and joint penalties.
Legacy raw *_minimum.csv / *_trials.csv column `chi2` now holds -2logL in THIS
study only; `forward_chi2`, `backward_chi2`, `smoothed_chi2` still contain the
old native/quadratic diagnostics. The exported results.csv labels nll2 explicitly.

Run through the configured build-tree environment; no Condor submission or
package installation occurs. Do not rerun a phase into its existing outputs.
Next: review population performance and seed/model sensitivity before any
integration request. Three likelihood orders are equivalent; do not select
one as a new source of physical evidence or add the three together.

Project-status-curator: the complete outgoing AGENTS is preserved in
2026-09-10-agents-before-normalized-likelihood.md. Only the final current-focus
paragraphs are replaced. Global status, all laws/scope and compile instructions
are unchanged. No headings, history files or directories were removed/moved;
directory-migration manifest checks are inapplicable. Two substantive AGENTS
sections remain, and the earlier quadratic study is retained in its dated record.
