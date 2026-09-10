# Optional free-loss fitting

`FreeLossFit=True` enables the promoted normalized-likelihood prototype inside
RecBreakpoint. `False` retains the established Gaussian-prior fitting exactly.
The default is false so existing campaigns do not silently change method.

This mode currently supports ONE selected LocalMarginal interval. It does
not discover intervals and does not change Truth/Manual selection. An empty
interval list remains the 5D reference. Multiple intervals or Persistent6D
retain the ordinary pair and report an explicit unsupported status. They are
never silently reduced to one interval. A failed search also retains the
ordinary pair and reports its error. This limitation matches the tested
scalar prototype; a simultaneous multiple-loss optimizer is not implemented.

## Controls and workflow

```python
fit.FreeLossFit = True
fit.FreeLossMaxLogLoss = 1.0
fit.FreeLossMaxCallsPerStart = 180
fit.FreeLossTolerance = 0.001
fit.FreeLossCheckLikelihoods = False
fit.BackwardSeedScale = 100.0
```

The dedicated card supports BP_FREE_LOSS_FIT, BP_FREE_LOSS_MAX_LOG_LOSS,
BP_FREE_LOSS_MAX_CALLS, BP_FREE_LOSS_TOLERANCE and BP_FREE_LOSS_CHECK. The
dedicated batch helper freezes these controls and the card. Example:

```bash
BP_FREE_LOSS_FIT=1 ./subbreakpointjobs.sh
```

That uses the existing input/output/stage controls and does not change the
GSF worker/cards. The script's selected loss sigma is still relevant to
ordinary fallback and the truth-prior comparison pair, not to free-b trials.

For one selected interval:

1. Fit the ordinary pair once, preserving a fallback if optimization fails.
2. Scan and optimize `b = log(p_before/p_after)`, with fractional loss
   `1-exp(-b)`. For each b, invoke the SAME BreakpointFitter with b fixed and
   sigma_b=0. No b Gaussian prior or truth amount enters this optimization.
3. Evaluate the normalized marginal likelihood of the captured affine model.
4. Repeat the chosen trial without its scalar cache, then make the native
   conditional fit at that b the primary RTS/backward output pair.

The bounds are [0,FreeLossMaxLogLoss], default[0,1]. The tested14-point scan
is scaled with the upper bound. Three MIGRAD starts are the best scan point,
min(.005,upper), and min(.05,upper). The normal initial step is .002, capped
at2% of the upper bound. Minuit ErrorDef=1, precision1e-8. A local diagnostic
scan uses b_best + j*.001 for j=-10..10 within bounds; it does not silently
replace the selected minimum. Failed native trials are tagged and receive
a large invalid-objective penalty, not a fabricated valid likelihood.

The result may be at a boundary or retain a finite non-converged/scan
candidate, as in the prototype. Always inspect the separately saved Minuit
status, boundary flags and trial records; `status=Applied` means an endpoint
was produced, not that its loss is accurately measured.

## One model, not three objective choices

The native state is `(drho, phi0, kappa, dz, tanl)` at each hit's native pivot.
For one fixed-b trial, xp and xf denote the native forward predicted and
updated states. H and V are the native measurement derivative and covariance;
F includes geometric propagation and the selected fixed-loss Jacobian; Q is
the native process noise. The frozen model is:

```text
x_0 ~ N(xp_0, Pseed)
x_i = xp_i + F_i * (x_(i-1) - xf_(i-1)) + w_i,  w_i ~ N(0,Q_i)
y_i = h(xp_i) + H_i * (x_i-xp_i) + v_i,         v_i ~ N(0,V_i)
```

Wrapped phi differences use the maintained stateDifference helper. Whitened
independent seed/process variables u give the full measurement model:

```text
d = A*u + epsilon,  u~N(0,I), epsilon~N(0,I)
C = I + A*A^T

objective = -2 log L
          = d^T*C^-1*d + logdet(C) + sum logdet(V_i) + m*log(2*pi)
```

m counts every measured coordinate at every accepted hit. This integrates
over the Gaussian trajectory variables. It is not just the minimized
quadratic or a product of independent smoothed-hit residual densities.
The determinant term matters when changing b changes the uncertainty.
Continuous-density -2logL may be negative; it is not a chi-square statistic.

Forward-order QR of [I,A]^T computes the objective without forming an
ill-conditioned C. Optional reverse-order QR and joint-state SVD evaluate the
SAME likelihood independently and must agree within1e-4. There is deliberately
no Forward/Backward/Smoothed objective selector. In particular, the reverse
calculation does NOT use the current backward refilter's copied forward seed.
BackwardSeedScale changes that refilter's endpoint, not the likelihood.

Singular Q is represented by its supported square-root directions; no inverse
of Q and no artificial process variance is needed. Decomposition is performed
after diagonal scaling, with normalized eigenvalues <=1e-12 treated as null,
materially negative modes rejected and square-root closure checked. Thus
deterministic coordinates and multiple-scattering null directions are kept.

The forward prefit uses observed hits. This calculation conditions on its
fixed seed convention; it is not an unconditional calibrated detector
likelihood. F/H/Q also change with the trial trajectory, so the model is a
local affine approximation to nonlinear tracking. Neither a low objective
nor a successful Minuit call establishes correct physical loss recovery.

## Publication and truth-assisted pair

The existing names remain:

- BreakpointTracksRTS / BreakpointTracksBackwardFilter: free fit when applied;
  otherwise the ordinary pair.
- BreakpointTracksTruthOverrideRTS / BreakpointTracksTruthOverrideBackwardFilter:
  the established truth-centered positive-SigmaLogLoss comparison when enabled.

TruthOverride has NOT been redefined as a fixed-loss oracle. Its prior center
and the configured positive sigma still go through the existing fitter. With
free fitting enabled, the primary pair has no b Gaussian prior, while the
extra truth pair retains its prior: these are not identical-uncertainty fits.
When TruthOverride is off and free fitting was applied, the extra pair is
copied exactly and tagged `truth_override_loss_treatment=CopiedFreeLikelihood`
with `truth_override_prior_sigma_log_loss=0`. Otherwise the old PriorCenter
contract is unchanged. Truth interval selection may still need embedded
truth when the override is off; the optimizer interface accepts no truth loss.

Primary native track covariances are CONDITIONAL on the fitted b. The local
Minuit error is recorded but is not added to those covariances. Zero fitted
loss variance in the native tuple therefore does not mean the optimized loss
has zero uncertainty. Track.chi2 and existing per-hit lists retain their
quadratic definitions; they are not replaced with -2logL. NDF remains the
existing dimension bookkeeping, not a calibrated significance for free fitting.

## Flat tuple contract

These fields are always present; unused scalars are NaN, trials empty and
flags false. `free_loss_enabled` records the request, `free_loss_applied` the
actual selected primary method. Disabled runs retain all old values exactly.

| Field(s), prefix free_loss_ | Meaning |
|---|---|
| enabled, max_log_loss, max_calls_per_start, tolerance, check_likelihoods | Effective steering |
| status | 0 not attempted; 1 no interval/5D; 2 applied; -1 unsupported ordinary fallback; -2 failed-search ordinary fallback |
| interval | Selected scalar breakpoint index, or -1 |
| b, b_error, minuit_status, edm | Selected loss and local optimization diagnostics; -99 status identifies a retained scan winner |
| nll2, quadratic, logdet | Normalized objective, its quadratic and total covariance-logdet contribution; m*log(2*pi) is also included in nll2 |
| reverse_order_nll2, joint_smoothed_nll2 | Optional equivalence checks, NaN when disabled |
| lower_bound, upper_bound | Selected b within1e-6 of a bound |
| covariance_conditional | True when the primary covariance conditions on optimized b |
| error | Unsupported/failed-search explanation |
| trial_b, trial_nll2, trial_valid, trial_phase, trial_error | Row-aligned evaluated trials; phases0 scan,1 Minuit,2 uncached repeat,3 local scan |
| trial_reverse_order_nll2, trial_joint_smoothed_nll2 | Optional per-trial likelihood checks |

No side CSV writer, temporary source loader or extra ROOT helper input is
part of the maintained workflow. Trial arrays replace the prototype CSV
logging; failed trials retain explanatory strings. Cached trajectories are
not retained across trials or tracks.

## Code organization

| Component | Responsibility |
|---|---|
| RecBreakpoint | Gaudi steering, fallback choice and existing endpoint publication |
| BreakpointFitter + KalmanAdapter | Existing physical fitting; optional passive native model capture |
| GaussianTrackModel | Data-only affine model, with no detector/KalTest ownership |
| TrackLikelihood | Read-only Gaussian marginal likelihood and optional consistency checks |
| FreeLossFitter | Blind bounded scalar search; calls the existing fitter for each trial |
| FreeLossTuple | Serialization only; no fitting or likelihood logic |

Free fitting remains a research option. The focused prototype eliminated one
extreme overshoot but retained20% errors in other examples. Wider categorized
validation and a treatment of outer-loss uncertainty remain necessary before
physics use.
