# Forward likelihood and complete smoothed quadratic

This explanation concerns one fixed trial loss b and the SAME fixed affine
Gaussian model. It does not introduce a different objective or iterations.
The native state is (drho, phi0, kappa, dz, tanl), with kappa=q/pT.

## Complete trajectory score

Let x_i be the track state at layer i, y_i its measured coordinates, mu_0
the initial seed mean, P_0 the seed covariance, f_i the captured affine
transport including fixed loss b, and h_i the captured affine measurement
map. V_i is measurement noise and Q_i is propagation noise. The score for
an entire candidate trajectory X=(x_0,...,x_(N-1)) is:

```text
C(X,b) = (x_0-mu_0)^T P_0^-1 (x_0-mu_0)
       + sum_i (y_i-h_i(x_i))^T V_i^-1 (y_i-h_i(x_i))
       + sum_(i=1..N-1) (x_i-f_i(x_(i-1),b))^T Q_i^+
                        (x_i-f_i(x_(i-1),b)).
```

T means transpose and + denotes the inverse on supported noise directions.
With singular Q, null-space transition residuals must be exactly zero;
a pseudoinverse alone must not silently permit unsupported process changes.
The implementation avoids this issue by constructing process residuals on
the supported noise subspace. The seed term is included once. The fixed-b
free trial has no Gaussian loss prior and no stochastic loss variance.

For this fixed Gaussian model, the RTS conditional mean X_s minimizes C.
The sum at X_s is the complete smoothed quadratic. V and Q are original
noise covariances, NOT the smoothed posterior covariance of x_i.

## Why the forward and smoothed quadratic values agree

Represent independent seed fluctuations and supported process kicks by a
unit-Gaussian vector u. Whiten each hit by its measurement noise, giving:

```text
d = A*u + epsilon,    covariance(u)=I, covariance(epsilon)=I
C(u,b) = ||d-A*u||^2 + ||u||^2
u_hat = (I+A^T*A)^-1 A^T*d
C(u_hat,b) = d^T (I+A*A^T)^-1 d.
```

d is the stacked whitened residual relative to the model mean; A maps the
seed/process fluctuations into whitened hit coordinates. I is the identity
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

The current Minuit objective remains the NORMALIZED marginal likelihood:

```text
J(b) = C(X_s,b) + log det S(b) + M log(2*pi).
```

S is the joint covariance of all UNWHITENED measurements and M their total
dimension. The code computes it through a stable whitened QR factorization.
Replacing J by smoothedTotalChi2 alone would drop the determinant term. It
would be a change of objective, not just a different way to evaluate J.
No such change is made by removing reference-trajectory iterations.

The equalities above require the same fixed affine model. In the actual
native tracking code the nonlinear measurement evaluations/reference choices
can differ from the captured affine ones. Do not claim that every published
native forward chi2 equals the captured marginal quadratic bit-for-bit.
scoreSmoothed uses the affine measurement score for its complete total and
also records a distinct native measurement-only score. BackwardSeedScale=100
does not enter the marginal objective, and the backward refilter is not a
second independent contribution to add to it.

LikelihoodTest independently compares a forward Kalman sum, dense
full-trajectory least squares, and the production marginal quadratic with
zero and singular process noise. These checks establish the algebra, not
physics performance or the correctness of every nonlinear approximation.
