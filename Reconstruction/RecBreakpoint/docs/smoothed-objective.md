# Forward likelihood and complete smoothed quadratic

This explanation concerns one trial Gaussian loss-prior center mu and the
SAME fixed affine Gaussian model. The configured sigma is SigmaLogLoss and
the loss parameter has prior b ~ N(mu, sigma^2). Minuit varies mu, not sigma.
It does not introduce a different filter or reference-trajectory iterations.
The native state is (drho, phi0, kappa, dz, tanl), with kappa=q/pT.

## Complete trajectory score

Let x_i be the track state at layer i, y_i its measured coordinates, mu_0
the initial seed mean, P_0 the seed covariance, f_i the captured affine
transport linearized at mu, and h_i the captured affine measurement map.
V_i is measurement noise. Q_i is native propagation noise plus the Gaussian
loss-prior contribution at the selected birth edge only. The score for
an entire candidate trajectory X=(x_0,...,x_(N-1)) is:

```text
C(X,mu) = (x_0-mu_0)^T P_0^-1 (x_0-mu_0)
       + sum_i (y_i-h_i(x_i))^T V_i^-1 (y_i-h_i(x_i))
       + sum_(i=1..N-1) (x_i-f_i(x_(i-1),mu))^T Q_i^+
                        (x_i-f_i(x_(i-1),mu)).
```

T means transpose and + denotes the inverse on supported noise directions.
With singular Q, null-space transition residuals must be exactly zero;
a pseudoinverse alone must not silently permit unsupported process changes.
The implementation avoids this issue by constructing process residuals on
the supported noise subspace. The seed term is included once. At the loss
birth, Q_i includes sigma^2*g_i*g_i^T, where g_i is the target-state derivative
with respect to b. This marginalizes the local Gaussian loss coordinate and
includes its prior ONCE, exactly as in ordinary LocalMarginal fitting.
No extra independent b penalty or log(sigma) normalization is added.
Here mu_0 is the track seed mean; mu without a subscript is the loss prior center.

For this fixed Gaussian model, the RTS conditional mean X_s minimizes C.
The sum at X_s is the complete smoothed quadratic. V and Q are original
noise covariances, NOT the smoothed posterior covariance of x_i.

## Why the forward and smoothed quadratic values agree

Represent independent seed fluctuations and supported process kicks by a
unit-Gaussian vector u. Whiten each hit by its measurement noise, giving:

```text
d = A*u + epsilon,    covariance(u)=I, covariance(epsilon)=I
C(u,mu) = ||d-A*u||^2 + ||u||^2
u_hat = (I+A^T*A)^-1 A^T*d
C(u_hat,mu) = d^T (I+A*A^T)^-1 d.
```

d is the stacked whitened residual relative to the model mean; A maps the
seed/process fluctuations (including loss-prior uncertainty) into whitened
hit coordinates. I is the identity
matrix and ||v||^2=v^T*v. u_hat describes the smoothed best-fit trajectory
and fitted process kicks. Substitution and the Woodbury identity give the
last equality: the marginal-likelihood quadratic is exactly the optimized
complete smoothed quadratic.

The forward Kalman recursion is another factorization of this SAME Gaussian
model. Its sum of innovation quadratics is the same scalar. The individual
per-hit terms differ: forward terms use upstream conditional predictions,
whereas smoothed terms distribute the score over final hit, process and seed
residuals. No downstream hit information disappears from the total forward
factorization. Smoothing recovers the conditional trajectory; it does not
add new observations to the likelihood.

## Which objective is currently minimized

The current Minuit objective evaluates the NORMALIZED marginal likelihood
using the complete RTS-smoothed chi2 directly:

```text
J(mu; sigma) = C(X_s,mu) + log det S(mu; sigma) + M log(2*pi).
```

S is the joint covariance of all UNWHITENED measurements and M their total
dimension. QR of the captured model supplies its log determinant. The existing
RTS pass supplies C(X_s,mu) through `FitResult::smoothedTotalChi2`.
Replacing J by smoothedTotalChi2 alone would drop the determinant term. It
would be a change of objective, not just a different way to evaluate J.
Sharing SigmaLogLoss does not remove either normalization term: the new loss
variance is present in BOTH the RTS score and the captured joint covariance.

On 2026-09-13 the user explicitly required retaining BOTH normalization terms
in the proposed switch to direct complete RTS-smoothed chi2 evaluation:

```text
J(mu; sigma) = smoothedTotalChi2(mu; sigma)
            + log det S_all(mu; sigma) + M log(2*pi).
```

S_all is the same joint measurement covariance denoted S above, NOT the
RTS-smoothed state covariance. M is the same total measurement dimension.
The production `FreeLossFitter` passes this trial's complete RTS-smoothed chi2
and captured model to `evaluateSmoothedTrackLikelihood`. That helper uses
the supplied chi2 without replacing it by a marginal quadratic, and adds
exactly these two normalization terms. Invalid smoothed scores invalidate
the trial; there is no silent fallback to another objective. The independent
`evaluateTrackLikelihood` is retained for numerical regression only. No
reference-trajectory iteration, new filter, or new run-card control is added.

The equalities above require the same fixed affine model. In the actual
native tracking code the nonlinear measurement evaluations/reference choices
can differ from the captured affine ones. Do not claim that every published
native forward chi2 equals the captured marginal quadratic bit-for-bit.
scoreSmoothed uses the affine measurement score for its complete total and
also records a distinct native measurement-only score. BackwardSeedScale=100
does not enter the marginal objective, and the backward refilter is not a
second independent contribution to add to it.

LikelihoodTest independently compares a forward Kalman sum, dense
full-trajectory least squares, and the marginal quadratic with zero and
singular process noise. It also checks the direct smoothed objective against
the normalized dense likelihood, verifies that the supplied chi2 is used,
and rejects invalid smoothed scores. These checks establish the algebra, not
physics performance or the correctness of every nonlinear approximation.

The shared-width test additionally checks one Gaussian loss kick feeding all
downstream hits, with sigma=0,0.001,0.01,0.05 and several prior centers, against
independent dense measurement covariance and full latent least squares. Sigma=0
is only an algebraic test limit; live free-loss fitting requires positive sigma.
