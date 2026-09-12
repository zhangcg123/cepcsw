# Fixed-loss reference-trajectory iterations

This optional experiment implements the user's 2026-09-12 request. It is not
the retired iteration that adjusted a Gaussian loss prior. Ordinary and
truth-prior results are unchanged, and no GSF/shared KF source is modified.

## Controls and scope

`FreeLossReferenceIterations=0` (compiled/card default) preserves the deployed
one-pass free fit. A positive value limits reference passes **inside each fixed
b trial**; 20 is the initial test setting. `FreeLossReferenceTolerance=0.001`
and `FreeLossReferenceObjectiveTolerance=0.0001` require convergence of both
the trajectory/covariance and the absolute likelihood value. Their card/batch
environment equivalents are `BP_FREE_LOSS_REFERENCE_ITERATIONS`,
`BP_FREE_LOSS_REFERENCE_TOLERANCE`, and
`BP_FREE_LOSS_REFERENCE_OBJECTIVE_TOLERANCE`. Frozen existing cards are unchanged.

Only one fixed-loss LocalMarginal interval is supported, as before. Free trials
still use sigma_b=0. The original seed mean/covariance, hits, measurement noise,
selected interval, and trial b never change during the inner iterations.
No truth loss amount enters the optimizer. The default-on truth-prior pair
remains a separate one-pass fit with its configured positive loss sigma.

## State and equations

The native state at hit i's pivot is x_i=(drho,phi0,kappa,dz,tanlambda), with
kappa=q/pT. b=log(p_before/p_after), so the selected upstream loss maps curvature
to exp(b)*kappa. Let a_i be the reference state, f_i the native propagation from
hit i-1 to i including that fixed loss, h_i the measurement prediction, and F_i
and H_i their state derivatives evaluated at the references. Q_i is native
process covariance on the reference propagation; V_i is original hit covariance.

At a reference pass the model is:

```text
x_0 ~ N(original_seed_mean, original_seed_covariance)
x_i = f_i(a_(i-1); b) + F_i*(x_(i-1)-a_(i-1)) + w_i
y_i = h_i(a_i)       + H_i*(x_i-a_i)             + v_i
w_i ~ N(0,Q_i); v_i ~ N(0,V_i)
```

y_i is the measured coordinate vector. Noise and seed variables are initially
independent. State differences wrap the native azimuth. Affine intercepts are
retained: changing just the Jacobians without f(a)/h(a) would change the model.

Start with the existing one-pass fixed-b RTS trajectory. Rebuild F/H/Q around
that trajectory and run the shared forward/RTS code. Set the next reference to
the resulting RTS mean/covariance. Every pass starts from the ORIGINAL prior,
not the preceding posterior. The algorithm does not repeatedly accumulate hits.

Native propagation supplies f/F/Q. `ReferenceMeasurementSite` supplies the
affine h/H to the **existing TKalTrackSite::Filter()** operation; it implements
no separate Kalman gain/update. `fitLocalRTS` shares its prediction bookkeeping,
transition covariance, RTS recursion, loss fields, score and IP publication
between ordinary and reference paths.

## Likelihood and convergence

Integrate the Gaussian state/process variables to obtain all-hit residual r
and full correlated measurement covariance S. Each pass evaluates:

```text
J(b) = r^T S^-1 r + log(det(S)) + M*log(2*pi)
```

M is the total number of measured coordinates, T is transpose, and log is
natural logarithm. The same supported-noise-root/QR likelihood evaluator is
used. When the measurement expansion origin differs from the captured predicted
state, subtract H*(predicted-reference) from its residual to retain the intercept.

The state convergence measure is the maximum over all hits and coordinates of
absolute mean change divided by the new posterior standard deviation. It also
includes every covariance-element change divided by sqrt(P_ii*P_jj) of the new
posterior. This maximum and the absolute J change must BOTH meet their tolerances.
No monotonic objective assumption is imposed on inner reference iterations.
Nonconvergence or an invalid native update/covariance invalidates that trial;
it cannot be published as a converged trial. If no valid converged Minuit
minimum is found, the existing tagged input-KF fallback applies.

The selected b is reevaluated without the cache, then the same converged fixed-b
procedure produces the published free RTS state. Its covariance remains
conditional on b. This is a locally self-consistent Gaussian approximation,
not an exact nonlinear likelihood or a guarantee of a unique/global minimum.

## Backward output and diagnostics

The free backward refilter starts from the final reference pass's forward
endpoint, with the unchanged BackwardSeedScale. It follows the existing native
inward recursion; it does NOT drive reference updates or the likelihood. Both
free endpoints keep the common native IP propagation. Ordinary and truth-prior
endpoints remain untouched.

Flat diagnostics persist configured limits/tolerances, aligned per-trial
`free_loss_trial_reference_iterations`, `_state_change`, `_objective_change`,
and the existing validity/error vectors. Invalid trials have no successful
iteration summary; nonconvergence detail is in their error string. The final
free pair stores `free_loss_reference_passes`, `_state_change`,
`_objective_change`, and `_parameters`/`_covariance` for the actual reference
used to build its final F/H/Q. Off/copy paths have zero passes and empty reference
states; KF fallback does not borrow failed refit diagnostics.

Focused gates and performance results belong in dated agents_record entries;
successful convergence alone is not physics validation.
