# Optional free-loss fitting

`FreeLossFit=True` enables the promoted normalized-likelihood prototype inside
RecBreakpoint. `False` retains the established Gaussian-prior fitting exactly.
The compiled default is false, while the maintained standalone/batch card defaults
to true. `BP_FREE_LOSS_FIT=0` explicitly disables it; previously prepared cards
retain their frozen settings. Ordinary results are ALWAYS retained; the optional method
occupies a separate pair of collections and flat branches.

`FreeLossReferenceIterations>0` enables the optional fixed-b reference-trajectory
experiment described in [reference-trajectory.md](reference-trajectory.md).
The one-pass likelihood construction below remains the default when it is zero.
The ordinary/truth-prior workflow and original Minuit controls are unchanged.

This mode currently supports ONE selected LocalMarginal interval. It does
not discover intervals and does not change Truth/Manual selection. An empty
interval list remains the 5D reference. Truth selection now chooses only the
interval with the largest summed absolute G4 eBrem momentum loss, shared by all
three pairs (equal losses choose the innermost interval). Other losses are not fitted.
Manual multiple intervals or Persistent6D report unsupported status. Unsupported
or failed searches copy the INPUT CompleteTracks KF into both FreeLoss outputs.
The ordinary and truth-prior pairs are unchanged by this fallback. This limitation matches the tested
scalar prototype; a simultaneous multiple-loss optimizer is not implemented.

## Controls and workflow

```python
fit.FreeLossFit = True
fit.FreeLossMaxLogLoss = 1.0
fit.FreeLossMaxCallsPerStart = 180
fit.FreeLossTolerance = 0.001
fit.BackwardSeedScale = 100.0
```

The dedicated card supports BP_FREE_LOSS_FIT, BP_FREE_LOSS_MAX_LOG_LOSS,
BP_FREE_LOSS_MAX_CALLS and BP_FREE_LOSS_TOLERANCE. The
dedicated batch helper freezes these controls and the card. Example:

```bash
BP_FREE_LOSS_FIT=1 ./subbreakpointjobs.sh
```

That uses the existing input/output/stage controls and does not change the
GSF worker/cards. The script's selected loss sigma is still relevant to
the ordinary and truth-prior comparison pairs, not to free-b trials or KF copies.

For one selected interval:

1. Fit the ordinary pair once, preserving a fallback if optimization fails.
2. Scan and optimize `b = log(p_before/p_after)`, with fractional loss
   `1-exp(-b)`. For each b, invoke the SAME BreakpointFitter with b fixed and
   sigma_b=0. No b Gaussian prior or truth amount enters this optimization.
3. Evaluate the normalized marginal likelihood of the captured affine model.
4. Repeat the chosen trial without its scalar cache, then make the native
   conditional fit at that b the ADDITIONAL FreeLoss RTS/backward output pair.

The bounds are [0,FreeLossMaxLogLoss], default[0,1]. The tested14-point scan
is scaled with the upper bound. Three MIGRAD starts are the best scan point,
min(.005,upper), and min(.05,upper). The normal initial step is .002, capped
at2% of the upper bound. Minuit ErrorDef=1, precision1e-8. A local diagnostic
scan uses b_best + j*.001 for j=-10..10 within bounds; it does not silently
replace the selected minimum. Failed native trials are tagged and receive
a large invalid-objective penalty, not a fabricated valid likelihood.

The result may be at a boundary but must come from a valid converged MIGRAD
minimum (Minimize true, status0, finite EDM). A finite scan or non-converged
candidate alone is no longer accepted. If all starts fail this gate, use the
input KF fallback. Always inspect the separately saved Minuit
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
ill-conditioned C. There is one objective calculation and no runtime
cross-formulation audit or Forward/Backward/Smoothed objective selector.
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

## Six collections: ordinary, free-loss and truth-assisted pairs

### Shared loss, different inward calculations

There is ONE Minuit search and one selected b for both free-loss endpoints.
For each trial the existing fitter runs with that b fixed (sigma_b=0).
Minuit chooses the minimum normalized full-track likelihood; no truth loss
amount sets the chosen b. The final fitter call uses that same selected value
for both RTS and backward continuation.

RTS uses transitions buffered DURING THIS trial's forward filter, not from a
previous event or an old fit. Each transition retains predicted state/covariance,
full transport F (including the selected loss Jacobian), process noise Q and
adjacent-state cross covariance C=P_filtered*F^T. With G=C*inverse(P_predicted),
its inward mean update is:

```text
x_smoothed[i] = x_filtered[i]
              + G[i] * (x_smoothed[i+1] - x_predicted[i+1])
```

Here x is the native5D state (drho,phi0,kappa,dz,tanl); phi differences are wrapped.
The stored cross covariance is what makes downstream measurements inform the
upstream state. This is the RTS procedure for the current linearized model,
not a fresh backward hit refit. The implementation also transports the full
smoothed covariance in conditional/Joseph form.

For positive b, the forward loss map multiplies curvature by exp(b). RTS uses
the resulting forward transitions; it does not apply a second loss. The native
backward refilter instead propagates inward, applies exp(-b) at the selected
upstream surface and updates its hit. Same physical loss, opposite direction.
Its seed is the final forward endpoint with covariance scaled by
BackwardSeedScale; this scale does not change the RTS calculation or objective.

All ordinary/free/truth RTS and backward endpoints now share native MarlinTrk
propagation from their innermost state to the IP, honoring MSOn/ElossOn. The
removed RTS-only geometric extrapolation is not an alternative live path.
This synchronizes the final transport, not the distinct endpoint estimates;
it adds neither a beam-spot measurement nor an extra loss breakpoint.

### Output contract

The existing names remain:

- BreakpointTracksRTS / BreakpointTracksBackwardFilter: ALWAYS the ordinary pair.
- BreakpointTracksFreeLossRTS / BreakpointTracksFreeLossBackwardFilter:
  optimized results when applied; otherwise exact ordinary copies without a rerun.
- BreakpointTracksTruthOverrideRTS / BreakpointTracksTruthOverrideBackwardFilter:
  the established truth-centered positive-SigmaLogLoss comparison when enabled.

TruthOverride has NOT been redefined as a fixed-loss oracle. Its prior center
and the configured positive sigma still go through the existing fitter. With
free fitting enabled, the FreeLoss pair has no b Gaussian prior, while the
ordinary/truth pairs retain their priors: these are not identical-uncertainty fits.
When TruthOverride is off, the truth pair ALWAYS copies the ordinary pair,
even when free fitting is on. The PriorCenter contract is unchanged.
Truth interval selection may still need embedded
truth when the override is off; the optimizer interface accepts no truth loss.

Optimized FreeLoss track covariances are CONDITIONAL on the fitted b. The local
Minuit error is recorded but is not added to those covariances. Zero fitted
loss variance in the native tuple therefore does not mean the optimized loss
has zero uncertainty. Track.chi2 and existing per-hit lists retain their
quadratic definitions; they are not replaced with -2logL. NDF remains the
existing dimension bookkeeping, not a calibrated significance for free fitting.

## Flat tuple contract

Optimizer fields are always present; unused scores are NaN, trials empty and
flags false. `free_loss_enabled` records the request, `free_loss_applied` the
actual optimized-extra-pair method. Disabled runs retain all old values exactly.

| Field(s), prefix free_loss_ | Meaning |
|---|---|
| enabled, max_log_loss, max_calls_per_start, tolerance | Effective steering |
| status | 0 not attempted; 1 no interval/5D; 2 applied; -1 unsupported KF fallback; -2 failed-search KF fallback |
| interval | Selected scalar breakpoint index, or -1 |
| b, b_error, minuit_status, edm | Selected loss and local optimization diagnostics; successful minima have status0; -99 means no accepted minimum |
| nll2, quadratic, logdet | Normalized objective, its quadratic and total covariance-logdet contribution; m*log(2*pi) is also included in nll2 |
| lower_bound, upper_bound | Selected b within1e-6 of a bound |
| covariance_conditional | True when the extra FreeLoss covariance conditions on optimized b |
| error | Unsupported/failed-search explanation |
| trial_b, trial_nll2, trial_valid, trial_phase, trial_error | Row-aligned evaluated trials; phases0 scan,1 Minuit,2 uncached repeat,3 local scan |

No side CSV writer, temporary source loader or extra ROOT helper input is
part of the maintained workflow. Trial arrays replace the prototype CSV
logging; failed trials retain explanatory strings. Cached trajectories are
not retained across trials or tracks.

### Always-present result branches and EDM row maps

The additional endpoint branches are NOT empty when the optimizer is disabled.
They copy the ordinary pair exactly, including covariances, fitted losses and
per-hit chi2 lists. Empty intervals also copy it. Unsupported/failed searches
instead copy the original input KF track, including every stored track state,
covariance, chi2, NDF and relation. The flat free-loss endpoint parameters and
covariances then describe the KF IP. `free_loss_kf_chi2` stores its published
score; breakpoint loss/per-hit vectors remain empty and all three unavailable
refit scores remain NaN, rather than borrowing another fit's diagnostics.
If the ordinary fit fails or a track is excluded, no successful pair is invented:
EDM indices are -1, result status0, endpoint pT is NaN and vectors are empty.
Flat rows continue to exist only for attempted tracks.

`BreakpointFreeLossStatus`, `BreakpointFreeLossRTSIndex` and
`BreakpointFreeLossBackwardIndex` are input-track-row-aligned EDM collections.
The same values appear as `free_loss_result_status`, `free_loss_rts_index` and
`free_loss_backward_index`. Result status0 means absent,1 ordinary copy,2
optimized pair,3 input KF fallback. This is separate from `free_loss_status`, which describes the
optimizer request/outcome (off0, empty1, applied2, unsupported-1, failed-2).

Always-present branches, all prefixed `free_loss_`:

- `{rts,backward}_pt`, `_ip_parameters`, `_ip_covariance`, `_fitted_log_loss`,
  `_fitted_log_loss_variance`: IP parameters are EDM `(D0,phi,omega,Z0,tanLambda)`;
  covariance has the15 packed EDM entries. Loss vectors use `breakpoint_interval`.
- `forward_chi2`, `backward_chi2`, `smoothed_chi2`, their corresponding
  `*_local_chi2` lists, and `smoothed_chi2_status/error`: same definitions as ordinary.
- `{forward_predicted,forward_filtered,smoothed,backward_predicted,backward_filtered}_parameters`
  and `_covariance`: all accepted-hit 5D states, flattened hit-major;
  parameters are native `(drho,phi0,kappa,dz,tanl)`, covariance row-major25 entries/hit.
  They share the ordinary ordered-hit metadata; these are not packed IP covariances.

Historical tuples produced before this separate-pair change used the ordinary
primary names for the optimized result and, with TruthOverride off, used
CopiedFreeLikelihood for the extra truth copies. Do not relabel those tuples.
The presence of `free_loss_result_status` identifies the new separate-pair schema.

## Code organization

| Component | Responsibility |
|---|---|
| RecBreakpoint | Gaudi steering, fallback choice and common publication for all three pairs |
| BreakpointFitter + KalmanAdapter | Existing physical fitting; optional passive native model capture |
| GaussianTrackModel | Data-only affine model, with no detector/KalTest ownership |
| TrackLikelihood | Read-only normalized Gaussian marginal likelihood |
| FreeLossFitter | Blind bounded scalar search; calls the existing fitter for each trial |
| FreeLossTuple | Serialization only; no fitting or likelihood logic |
| FitPairTuple | Additional result-pair serialization; exact copies use the existing fit object |

Free fitting remains a research option. The focused prototype eliminated one
extreme overshoot but retained20% errors in other examples. Wider categorized
validation and a treatment of outer-loss uncertainty remain necessary before
physics use.
