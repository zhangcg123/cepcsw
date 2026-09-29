# Retired contract: no-prior and fixed primary breakpoint workflows

Archived verbatim from RecBreakpoint/docs/unconstrained-loss.md on 2026-09-29.
The modes and their selector have been removed at the user request.
This is historical documentation, not a supported run configuration.

# Unconstrained breakpoint loss

`LossPriorMode="Unconstrained"` is an opt-in, one-pass joint fit of the track
and one breakpoint loss, with **zero loss prior precision**. It is not a fixed
loss, not a large Gaussian variance, and not the outer Minuit optimization.
The initial track covariance, measurement noise and native material/MS noise
remain present. Only the prior on the breakpoint parameter is removed.

The implementation uses the existing native conditional 5D KF and RTS, plus
scalar diffuse regression. All native propagation and hit updates remain in
`KalmanAdapter`; `UnconstrainedLoss` estimates only the additional scalar.
No shared KF or GSF source is modified. No reference-trajectory iteration is
introduced. The method is exact for the captured affine Gaussian model, not
a claim of an exact global optimum of the nonlinear native trajectory model.

## Controls and independent outputs

| LossPriorMode | Ordinary RTS and backward outputs |
|---|---|
| Gaussian (unchanged compiled/card default) | Existing MeanLogLoss, SigmaLogLoss Gaussian prior |
| Unconstrained | No b prior; MeanLogLoss is an expansion reference only; SigmaLogLoss unused |
| Fixed | Hold MeanLogLoss exactly with zero loss variance; explicit diagnostic control |

Unconstrained requires LocalMarginal and at most one selected interval. Fixed
requires LocalMarginal to avoid a singular live 6D state. No-interval runs use
the unchanged 5D reference. Unconstrained b may be negative: positivity or a
physical upper bound would be an additional constraint and is not imposed.
The existing positive SigmaLogLoss property remains available for the separate
Gaussian truth-prior output pair. It is not silently reused in this fit.

The maintained card explicitly assigns LossPriorMode; set
`BP_LOSS_PRIOR_MODE=Unconstrained BP_FREE_LOSS_FIT=0` for this experiment.
Batch preparation freezes this setting. Existing prepared cards are unchanged.
FreeLossFit continues to control only its separate Minuit pair, whose trial
states remain conditional on b. TruthOverride continues to produce its
separate Gaussian truth-prior pair; it never constrains the ordinary
unconstrained result. When these switches are off, their ordinary-copy
contracts remain unchanged.

For a genuinely fixed-truth diagnostic, independently obtain the selected
interval's exact G4 b, set MeanLogLoss to it and LossPriorMode to Fixed.
That is a truth-assisted CONTROL, never reconstruction steering in the
unconstrained or Gaussian-prior runs.

## Algebra and code mapping

Use the native five coordinates x = (drho, phi0, kappa, dz, tanl), with
kappa = q/pT, and b = log(p_before/p_after). The loss map is
kappa_after = exp(b) kappa_before. At the configured reference b_ref,
linearize once. Define delta_b = b - b_ref.

The native conditional filter at b_ref gives predicted/updated state means
x_pred[i], x_filt[i], covariances P_pred[i], P_filt[i], and transport F[i]
from hit i-1 to hit i. The derivative of the conditional state with respect
to delta_b is a_pred[i] or a_filt[i], a five-entry column. It starts at zero.
The loss derivative g[i] is zero except at the selected transition:
outward it is the propagated curvature-loss derivative; inward it is
(0,0,-kappa_before,0,0) after native inward propagation.

```text
a_pred[i] = F[i] a_filt[i-1] + g[i]
a_filt[i] = P_filt[i] inverse(P_pred[i]) a_pred[i]
```

The second expression differentiates the existing conditional native update;
it is (I-KH) a_pred in a fixed affine Gaussian model. No second hit update is
performed. H[i] is the native measurement derivative, V[i] the measurement
noise, and r[i] the native conditional measurement innovation at b_ref.

```text
S[i] = H[i] P_pred[i] transpose(H[i]) + V[i]
d[i] = H[i] a_pred[i]
r[i; delta_b] = r[i] - d[i] delta_b

I_b = sum transpose(d[i]) inverse(S[i]) d[i]
c_b = sum transpose(d[i]) inverse(S[i]) r[i]
delta_b_fit = c_b / I_b
Var(b) = 1 / I_b
```

I_b is the data information on b; its initial value is exactly zero, with
no `1/SigmaLogLoss^2` term. c_b is the data score. These quantities
use the retained proper track-seed covariance and native noise assumptions;
"no prior" applies to b, not to every track coordinate. The same sums over a prefix
of hits give the forward filtered b and variance at that prefix. The
prefix chi2 is `sum r^T S^-1 r - c_b^2/I_b`; successive differences provide
the diffuse forward local chi2 list. At a first informative hit this differs
from treating a reference-b innovation as an ordinary finite-prior chi2.
No normalized diffuse likelihood is claimed by this list.

RTS differentiates its existing recursion using its usual gain G[i]:

```text
a_smooth[i] = a_filt[i]
           + G[i] (a_smooth[i+1] - a_pred[i+1])

x_smooth_free[i] = x_smooth[i] + a_smooth[i] delta_b_fit
P_smooth_free[i] = P_smooth[i] + a_smooth[i] Var(b) transpose(a_smooth[i])
Cov(x_smooth_free[i], b) = a_smooth[i] Var(b)
```

These expressions retain the uncertainty of b and its correlation with the
pre-loss track. They do NOT rerun a fit treating the fitted variance as a
new prior. Complete smoothed chi2 is evaluated at the fitted b using the same
conditional model, measurement + process + seed terms, with no b-prior term.
Its difference from the minimized diffuse forward chi2 is recorded as closure.

The backward refilter uses the same scalar regression in inward order, with
the existing copied/scaled forward endpoint. It still reuses forward hit
information and is not an independent Bayesian smoother. Its b variance and
correlations are conditional on that chosen seed policy. It is not asserted
to equal the RTS joint posterior.

## Validity and flat diagnostics

There is no finite b variance before the prefix has information about b.
Very weak prefixes can also be numerically unrepresentable as finite full-rank
5D covariances. Such predicted/filtered covariances are NaN and explicitly
flagged invalid; they are not inputs to the underlying conditional native
filter. No covariance jitter or artificial loss prior is introduced. Every
final RTS covariance and both published endpoints must remain valid; a
globally unidentifiable loss fails explicitly under the existing paired-fit
failure contract.

New always-present branches (empty/default for the Gaussian path):

- loss_prior_mode: Gaussian, Unconstrained or Fixed.
- unconstrained_state_loss_covariance: hit-major Cov(RTS state,b), five native
  coordinates per hit; this includes hits BEFORE the breakpoint.
- unconstrained_backward_state_loss_covariance: corresponding backward
  filtered-prefix cross covariance; unavailable before observing b is NaN.
- unconstrained_prediction_valid, unconstrained_filtered_valid and the two
  corresponding unconstrained_backward_*_valid vectors: one flag per hit.
- unconstrained_loss_information, unconstrained_backward_loss_information:
  I_b for each endpoint construction.
- unconstrained_loss_reference: b_ref, NOT a prior center.
- unconstrained_chi2_closure: complete RTS chi2 minus minimized diffuse
  forward chi2 of the captured model.

Existing fitted_log_loss, fitted_log_loss_variance and backward equivalents
contain the fitted b and its DATA-derived variance. prior_log_loss is NaN
for an unconstrained interval, not a misleading zero prior. Local loss fields
are NaN while the local data have not identified b. Existing state and
covariance fields contain the marginal results, not the fixed-b errors.

## Gates and limitations

The independent numerical test constructs one dense joint normal system with
zero b prior precision and compares b, Var(b), every RTS state/covariance and
Cov(state,b), with/without process noise and changed reference coordinates.
It separately rejects zero-information b. It does not use the runtime RTS
implementation to form the dense reference.

Real-event gates must additionally check unchanged Gaussian results, no-loss
copies, strict event identity, early/later-loss comparisons, and fixed-truth
controls. A tight conditional track error at fixed b is not evidence of a
tight unconstrained pre-loss momentum. Selected examples or lower chi2 do
not establish production physics performance.

For general background on diffuse state-space filtering, generalized least
squares and smoothing, see
[de Jong, Stable algorithms for the state space model](https://researchers.mq.edu.au/en/publications/stable-algorithms-for-the-state-space-model/).
The equations above specify this package's implementation and tested scope;
they are not a claim of implementing every algorithm in that paper.
