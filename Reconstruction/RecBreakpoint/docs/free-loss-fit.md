# Optional free-loss fitting

`FreeLossFit=True` enables the promoted normalized-likelihood prototype inside
RecBreakpoint. `False` retains the established Gaussian-prior fitting exactly.
The compiled default is false, while the maintained standalone/batch card defaults
to true. `BP_FREE_LOSS_FIT=0` explicitly disables it; previously prepared cards
retain their frozen settings. Ordinary results are ALWAYS retained; the optional method
occupies a separate pair of collections and flat branches.

This mode supports ONE selected interval with Persistent6D.
Every trial and final refit uses the same BreakpointFitter. It does
not discover intervals and does not change Truth/Manual selection. An empty
interval list remains the 5D reference. Truth selection now chooses only the
interval with the largest summed absolute G4 eBrem momentum loss, shared by all
remaining pairs (equal losses choose the innermost interval). Other losses are not fitted.
Manual multiple intervals report unsupported status. Unsupported
or failed searches copy the INPUT CompleteTracks KF into both FreeLoss outputs.
The ordinary and truth-prior pairs are unchanged by this fallback. This limitation matches the tested
scalar prototype; a simultaneous multiple-loss optimizer is not implemented.

## Controls and workflow

```python
fit.FreeLossFit = True
fit.SigmaLogLoss = 0.001  # SAME prior width for ordinary, free and truth fits
fit.FreeLossMaxLogLoss = 1.0
fit.FreeLossMaxCallsPerStart = 180
fit.FreeLossTolerance = 0.001
fit.BeamSpotConstraint = False  # initial KF measurement shared with ordinary
fit.BeamSpotX = fit.BeamSpotY = 0.0  # mm
fit.BeamSpotSigmaX = 0.0145  # mm
fit.BeamSpotSigmaY = 3.6e-5  # mm
fit.BackwardSeedScale = 100.0
```

The dedicated card supports BP_FREE_LOSS_FIT, BP_FREE_LOSS_MAX_LOG_LOSS,
BP_FREE_LOSS_MAX_CALLS and BP_FREE_LOSS_TOLERANCE. The
shared beam measurement is controlled by BP_BEAM_SPOT_CONSTRAINT (default 0);
BP_BEAM_SPOT_X/Y and BP_BEAM_SPOT_SIGMA_X/Y control its position and widths.
It also works with FreeLossFit disabled. The beam-guided output family and
its old environment controls are removed. This observation updates the seed
before the first tracker hit; ordinary and free-loss use that same constrained
KF/RTS. No post-RTS beam objective is added.

## One model, not three objective choices

When BeamSpotConstraint is enabled, the shared fitter conditions its original
seed on one transverse beam-origin observation before the first tracker hit:

```text
R_beam = cos(phi0)^2*sigma_x^2 + sin(phi0)^2*sigma_y^2
S_beam = H_beam*P_seed*H_beam^T + R_beam
L(all observations) = L(beam | original seed) * L(hits, ECAL | beam)
```

The beam is an explicit first site at the configured beam pivot, with
H_beam=(1,0,0,0,0). Native KalTest filtering updates its full seed covariance.
Native detector transport then supplies the first tracker prediction and F/Q,
including enabled material effects. RTS returns through this retained edge to
the beam, which supplies the published IP state without a second material pass.
The complete smoothed quadratic includes the original beam seed penalty, beam
measurement residual and beam-to-hit process penalty. The likelihood uses the
transported conditioned seed at hit 0 and includes log(2*pi*S_beam) once.
There is no separate beam-guided search or post-RTS penalty. This transverse
observation does not constrain z; the projected width is frozen at the seed.

For the exact relation to complete smoothed residuals, see
[forward likelihood and smoothed quadratic](smoothed-objective.md).

The native state is `(drho, phi0, kappa, dz, tanl)` at each hit's native pivot.
For one prior-center trial, xp and xf denote the native forward predicted and
updated states. H and V are the native measurement derivative and covariance;
F includes geometric propagation and the loss Jacobian at mu. At the selected
birth edge, Q is native process noise plus `SigmaLogLoss^2 * g * g^T`, where
g is the target-state derivative with respect to b. Other edges retain native Q.
This includes the Gaussian loss prior ONCE, after marginalizing its local loss
coordinate; no separate b penalty or independent log(sigma) term is added.
The frozen model is:

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

S_all = joint covariance of all unwhitened measurements
logdet(S_all) = logdet(C) + sum logdet(V_i)

objective = -2 log L
          = complete smoothed chi2 + logdet(S_all) + m*log(2*pi)

complete smoothed chi2 = measurement chi2 + process chi2 + seed chi2
                      = d^T*C^-1*d  (for the same fixed affine Gaussian model)
```

m counts every measured coordinate at every accepted hit. This integrates
over the Gaussian trajectory variables. It is not just the minimized
quadratic or a product of independent smoothed-hit residual densities.
The determinant term matters when changing mu changes the uncertainty.
Continuous-density -2logL may be negative; it is not a chi-square statistic.

The production objective reads `FitResult::smoothedTotalChi2` directly from
the existing RTS pass. `evaluateSmoothedTrackLikelihood` adds the normalization;
QR of [I,A]^T supplies the joint covariance determinant without forming an
ill-conditioned C. It does NOT compute or substitute the marginal quadratic.
`S_all` is not the RTS-smoothed state covariance. Original V, Q and seed
covariance enter the complete smoothed chi2; no independent smoothed-hit
likelihoods are multiplied together.

The separate `evaluateTrackLikelihood` marginal quadratic remains a numerical
regression reference, not the Minuit objective path. There is no runtime
cross-formulation audit, new iteration, or Forward/Backward/Smoothed selector.
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

## Parallel endpoint families

### Shared loss prior, different inward calculations

Each applied free-loss pair has one Minuit search and one selected mu shared
by its RTS and backward endpoints. Beam, charged and neutral measurements are
optional observations in this same fit, not separate endpoint families.
For each trial the existing fitter uses the SAME positive SigmaLogLoss.
Minuit chooses the minimum normalized full-track likelihood; no truth loss
amount sets the chosen mu. The final fitter call uses that same selected center
and width for both RTS and backward continuation. Their fitted b values and
posterior variances can differ because their inward calculations differ.

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

For positive mu, the forward loss map is linearized at exp(mu), with the
configured loss variance transported by the same shared fitter. RTS uses
the resulting forward transitions; it does not apply a second loss. The native
backward refilter instead propagates inward, applies the inverse map at the
selected upstream surface with that same prior width, and updates its hit.
Same loss-prior settings, opposite direction.
Its seed is the final forward endpoint with covariance scaled by
BackwardSeedScale; this scale does not change the RTS calculation or objective.

With the beam constraint disabled, RTS and backward endpoints use native
MarlinTrk propagation from their innermost state to IP, honoring MSOn/ElossOn.
With it enabled, ordinary/free-loss RTS instead smooths back to the explicit
beam site through its stored native transition; the smoothed beam state supplies
the IP endpoint without a second material traversal. Independent truth/diffuse
fits and optional backward refilters retain native innermost-to-IP propagation;
the backward refilter does not add a second beam update. No extra loss
breakpoint is introduced at this boundary.

### Output contract

The existing names remain:

- BreakpointTracksRTS / BreakpointTracksBackwardFilter: ALWAYS the ordinary pair.
- BreakpointTracksFreeLossRTS / BreakpointTracksFreeLossBackwardFilter:
  optimized-prior results when applied; ordinary copies when off/empty;
  input KF copies on failed or unsupported optimization.
- BreakpointTracksTruthOverrideRTS / BreakpointTracksTruthOverrideBackwardFilter:
  the established truth-centered positive-SigmaLogLoss comparison when enabled.

TruthOverride has NOT been redefined as a fixed-loss oracle. Its prior center
and the configured positive sigma still go through the existing fitter. With
free fitting enabled, the ordinary, free-loss and truth-prior pairs use the SAME configured Gaussian width.
Their prior centers differ: configured MeanLogLoss, optimized mu, or truth b.
Equal prior widths do not imply equal posterior covariances or loss estimates.
When TruthOverride is off, the truth pair ALWAYS copies the ordinary pair,
even when free fitting is on. The PriorCenter contract is unchanged.
Truth interval selection may still need embedded
truth when the override is off; the optimizer interface accepts no truth loss.

Optimized FreeLoss track covariances include the fitted loss uncertainty and
track/loss correlations from the shared Gaussian fitter. They are NOT fixed-b
conditional covariances. They do still condition on the chosen prior center:
the local Minuit error on mu is recorded but not added as a second uncertainty.
SigmaLogLoss is the prior width; saved fitted_log_loss_variance is the posterior
variance; b_error is the Minuit error on the prior center. These are distinct.
Track.chi2 and existing per-hit lists retain their
quadratic definitions; they are not replaced with -2logL. NDF remains the
existing dimension bookkeeping, not a calibrated significance for free fitting.

## Flat tuple contract

Optimizer fields are always present; unused scores are NaN, trials empty and
flags false. `free_loss_enabled` records the request, `free_loss_applied` the
actual optimized-extra-pair method. Disabled runs retain all old values exactly.

| Field(s), prefix free_loss_ | Meaning |
|---|---|
| enabled, max_log_loss, max_calls_per_start, tolerance | Effective steering; max_log_loss bounds the prior center |
| treatment, prior_sigma_log_loss | PriorCenter and the configured SigmaLogLoss; present even when free fitting is off |
| prior_mean_log_loss | Optimized Gaussian prior center mu; NaN if no optimization was applied |
| status | 0 not attempted; 1 no interval/5D; 2 applied; -1 unsupported KF fallback; -2 failed-search KF fallback |
| interval | Selected scalar breakpoint index, or -1 |
| b, b_error, minuit_status, edm | b is a compatibility alias for prior_mean_log_loss; b_error is the Minuit center error, NOT SigmaLogLoss or posterior sigma_b; successful minima have status0; -99 means no accepted minimum |
| nll2, quadratic, logdet | Normalized objective, the complete RTS-smoothed chi2 used by Minuit, and logdet(S_all); m*log(2*pi) is also included in nll2 |
| lower_bound, upper_bound | Selected prior center within1e-6 of a bound |
| covariance_conditional | Legacy exact-fixed-b flag; false for the new Gaussian-prior fit. Center-estimation uncertainty is still not integrated |
| error | Unsupported/failed-search explanation |
| trial_b, trial_nll2, trial_valid, trial_phase, trial_error | Row-aligned prior-center trials; phases0 scan,1 Minuit,2 uncached repeat,3 local scan |

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

The parallel `ordinary_likelihood_*`, `free_loss_likelihood_*`,
and `truth_override_likelihood_*` branches
contain the normalized
`-2 log L = complete RTS chi2 + log det S_all + M log(2*pi)` for each RTS
result. They save `status`, `nll2`, `quadratic`, `logdet`,
`measurement_dimensions`, `latent_dimensions`, and `error`. Status 1 is
valid, 0 absent, -1 unavailable/failed. An optimized free-loss result uses
the accepted Minuit objective; an off/no-interval result copies ordinary;
an input-KF fallback has no breakpoint likelihood. The truth-assisted result
copies ordinary only when its own fit is disabled. These diagnostics do not
change track publication or the existing `Track.chi2` convention.

Always-present branches, all prefixed `free_loss_`:

- `{rts,backward}_pt`, `_ip_parameters`, `_ip_covariance`, `_fitted_log_loss`,
  `_fitted_log_loss_variance`: IP parameters are EDM `(D0,phi,omega,Z0,tanLambda)`;
  covariance has the15 packed EDM entries. Loss vectors use `breakpoint_interval`.
- `forward_chi2`, `backward_chi2`, `smoothed_chi2`, their corresponding
  `*_local_chi2` lists, and `smoothed_chi2_status/error`: same definitions as ordinary.
- `smoothed_measurement_chi2`, `smoothed_process_chi2` and
  `smoothed_native_measurement_chi2` per-hit vectors, plus scalar
  `smoothed_seed_chi2`: always saved under both `free_loss_` and
  `truth_override_`, just as for ordinary RTS. Measurement + process + the
  seed term once at hit 0 reproduces the complete RTS score. Native measurement
  is a separate check, not an additional contribution. Off/copy results copy
  these terms; input-KF fallback has empty vectors and NaN seed, not a fabricated
  RTS decomposition. Gate score use on the corresponding smoothed status.
- `{forward_predicted,forward_filtered,smoothed,backward_predicted,backward_filtered}_parameters`
  and `_covariance`: all accepted-hit 5D states, flattened hit-major;
  parameters are native `(drho,phi0,kappa,dz,tanl)`, covariance row-major25 entries/hit.
  They share the ordinary ordered-hit metadata; these are not packed IP covariances.

Historical tuples produced before this separate-pair change used the ordinary
primary names for the optimized result and, with TruthOverride off, used
CopiedFreeLikelihood for the extra truth copies. Do not relabel those tuples.
The presence of `free_loss_result_status` identifies the separate-pair schema.
Within that schema, `free_loss_treatment="PriorCenter"` identifies the shared
positive-sigma method introduced on 2026-09-30. Older tuples lacking this field
retain their fixed-b interpretation; do not relabel them.

## Code organization

| Component | Responsibility |
|---|---|
| RecBreakpoint | Gaudi steering, fallback choice and common publication for the remaining pairs |
| BreakpointFitter + KalmanAdapter | Existing physical fitting; optional passive native model capture |
| GaussianTrackModel | Data-only affine model, with no detector/KalTest ownership |
| TrackLikelihood | Add joint-measurement normalization to the supplied complete RTS-smoothed chi2; retain independent marginal evaluation for regression |
| FreeLossFitter | Blind bounded prior-center search; calls the existing Gaussian fitter with shared sigma for each trial |
| FreeLossTuple | Serialization only; no fitting or likelihood logic |
| FitPairTuple | Additional result-pair serialization; exact copies use the existing fit object |

Free fitting remains a research option. Historical fixed-b results do not
validate the new positive-sigma workflow. Wider categorized validation and
assessment of prior-center estimation uncertainty remain necessary before
physics use.
