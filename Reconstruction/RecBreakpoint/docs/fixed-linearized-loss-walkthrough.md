# Why Persistent6D and LocalMarginal can give the same result

This is a self-contained, terminal-readable explanation of the implementation,
including the state conventions, every deviation/reference, and the derivation
of the loss Jacobian. It describes **one breakpoint between hits 5 and 6, one
forward pass, and RTS smoothing**, without relinearization iterations.

The equations were checked against the source at local commit `fdbe0fa` on
2026-09-09. This is a mathematical explanation of the implemented model, not
a claim of physics validation. The separately published backward hit refilter
is not the RTS recursion explained here.

## 1. Track parameters and notation

The internal five-dimensional state is:

```text
x = (drho, phi0, kappa, dz, tanLambda)
```

| Index | Parameter | Meaning | Units |
|---|---|---|---|
| 0 | drho | Signed transverse impact parameter relative to the current pivot | mm |
| 1 | phi0 | KalTest helix azimuth convention | radians |
| 2 | kappa | Signed inverse transverse momentum | GeV^-1 |
| 3 | dz | Longitudinal offset at transverse closest approach to the pivot | mm |
| 4 | tanLambda | Longitudinal slope, pz/pT | dimensionless |

The pivot's three-dimensional position is stored separately; it is not part
of these five fitted parameters. All state comparisons use compatible pivots.

Internal coordinates differ from the published EDM coordinates:

```text
drho  = -D0
phi0  = wrap(phi - pi/2)
kappa = omega / (2.99792458e-4 * Bz)
dz    = Z0
```

Here Bz is the magnetic field in tesla and omega is the published geometric
curvature in mm^-1. Internal kappa is not omega or q/p. Internal phi0 must not
be confused with the published phi. Angular differences are wrapped to avoid
an artificial jump across the 2*pi boundary.

General notation:

```text
P       = covariance matrix of the five track parameters
M^T     = transpose of matrix M
inv(M)  = inverse of matrix M
Cov(a,d)= cross-covariance between quantities a and d

_pred   = prediction before using the measurement at that hit
_upd    = state after using that measurement
_smooth = state after incorporating downstream information through RTS
```

Track means and covariances are stored by the code. The possible states and
deviations below explain their uncertainty; the code does not randomly sample
these deviations.

## 2. State and loss immediately before the breakpoint

Both methods have already fitted hits 0 through 5. At hit 5:

```text
x5_ref = fitted mean state after the hit-5 measurement update
P5     = covariance of that state
```

A possible state around this mean is called x5. Its deviation is:

```text
delta_x5 = x5 - x5_ref

delta_x5 =
  (drho5 - drho5_ref,
   wrapped(phi05 - phi05_ref),
   kappa5 - kappa5_ref,
   dz5 - dz5_ref,
   tanLambda5 - tanLambda5_ref)
```

**This is uncertainty around the already-updated state at hit 5.** It is not
the difference between hits 5 and 6, and not the correction produced by the
hit-5 measurement update.

The loss parameter and its prior are:

```text
b       = log(p_before / p_after)
b0      = assumed mean of b
B       = assumed variance of b
delta_b = b - b0
```

Here p means momentum magnitude. B is a scalar loss variance, not magnetic
field Bz. The implementation introduces the loss prior independently of the
incoming track:

```text
Cov(delta_x5, delta_b) = 0
```

## 3. Derive the instantaneous loss mapping, A, and g

The loss is applied at the upstream hit-5 pivot, before propagation to hit 6.
The model changes the momentum magnitude without introducing a separate
directional kick in this operation:

```text
p_after = p_before * exp(-b)
```

The implementation keeps tanLambda unchanged during this operation. Therefore
pT has the same fractional change as p. Since kappa is signed inverse pT and
charge does not change:

```text
kappa_after = kappa_before * exp(b)
```

The full instantaneous map is:

```text
drho_after      = drho_before
phi0_after      = phi0_before
kappa_after     = exp(b) * kappa_before
dz_after        = dz_before
tanLambda_after = tanLambda_before
```

This is the implemented simplified loss map, not a claim that real radiation
never changes direction. Native propagation/scattering are separate. This
operation also does not fit the exact emission position inside the interval.

### Derive A: vary the incoming track, hold the loss fixed

A is the 5x5 matrix of derivatives of outgoing track parameters with respect
to incoming track parameters, evaluated at x5_ref and b0.

For example:

```text
drho_after = drho_before

derivative of drho_after with respect to drho_before = 1
derivative of drho_after with respect to every other incoming parameter = 0
```

The same reasoning gives a diagonal entry of 1 for phi0, dz and tanLambda.
For curvature, holding b fixed:

```text
kappa_after = exp(b) * kappa_before

derivative of kappa_after with respect to kappa_before = exp(b)
```

Evaluating at b0 gives:

```text
A =
  [ 1  0     0       0  0 ]
  [ 0  1     0       0  0 ]
  [ 0  0  exp(b0)    0  0 ]
  [ 0  0     0       1  0 ]
  [ 0  0     0       0  1 ]
```

These entries follow from differentiating the loss map. A is not a freely
chosen matrix and is not the subsequent geometry-propagation Jacobian.

### Derive g: vary the loss, hold the incoming track fixed

g is the five-element column of derivatives of outgoing track parameters
with respect to b. Four parameters do not depend on b in this loss map, so
their derivatives are zero. For curvature:

```text
kappa_after = kappa_before * exp(b)

derivative of exp(b) with respect to b = exp(b)

derivative of kappa_after with respect to b = kappa_before * exp(b)
```

Evaluating at kappa5_ref and b0 gives:

```text
g =
  [ 0                       ]
  [ 0                       ]
  [ kappa5_ref * exp(b0)     ]
  [ 0                       ]
  [ 0                       ]
```

The sign of the nonzero derivative follows the signed kappa. Nothing here
replaces kappa by its absolute value.

### Assemble the first-order expansion

The reference state immediately after the loss, still at the same pivot, is:

```text
x_after_ref =
  (drho5_ref,
   phi05_ref,
   exp(b0)*kappa5_ref,
   dz5_ref,
   tanLambda5_ref)

x_after approximately equals x_after_ref + A*delta_x5 + g*delta_b
```

Defining delta_x_after explicitly:

```text
delta_x_after = x_after - x_after_ref

delta_x_after approximately equals A*delta_x5 + g*delta_b
```

This is the fixed first-order model used for uncertainty transport. It is not
the incorrect reference-free equation x_after = A*x_before + g*b, which would
generally omit an affine offset or count the reference loss twice.

## 4. Transfer the track and loss uncertainty

Within that linearized model:

```text
P_after = A*P5*A^T + g*B*g^T
```

The first term transfers incoming track uncertainty. The second transfers
uncertainty of the assumed loss into the outgoing track covariance. Cross
terms vanish because the newly introduced loss prior and incoming track are
independent. Existing correlations within P5 are not discarded.

Even for b0=0:

```text
A = identity
g = (0, 0, kappa5_ref, 0, 0)^T

variance(kappa_after)
    = variance(kappa_before) + kappa5_ref^2 * B
```

**Zero prior mean loss does not mean zero loss uncertainty.**

## 5. Propagate to hit 6 and compare the stored information

Define:

```text
F = 5x5 Jacobian of ordinary propagation from the loss pivot to hit 6
Q = process-noise covariance added during that propagation
```

The predicted mean x6_pred comes from propagating x_after_ref. Its covariance
and cross-covariance with the loss are:

```text
P6_pred = F*P_after*F^T + Q

c6_pred = Cov(b, x6_pred)
        = B*g^T*F^T
```

c6_pred is a 1x5 row. Process noise is independent of the new loss prior in
this fixed model.

Persistent6D stores the entire joint prediction:

```text
mean = (x6_pred, b0)

covariance =
  [ P6_pred       c6_pred^T ]
  [ c6_pred       B         ]
```

LocalMarginal carries the five-dimensional x6_pred and P6_pred. It saves the
original breakpoint prediction, loss prior, c6_pred and transition information
for loss recovery during RTS.

Taking the five-dimensional track block does **not** remove the loss-variance
contribution already inside P6_pred.

## 6. Use the measurement at hit 6

Define:

```text
y6 = measured hit coordinates
h(x6_pred) = predicted hit coordinates
H6 = derivative of the measurement prediction h with respect to x
V6 = measurement-error covariance

r6 = y6 - h(x6_pred)                 measurement residual
S6 = H6*P6_pred*H6^T + V6           residual covariance
```

The track Kalman gain and update are:

```text
Kx = P6_pred*H6^T*inv(S6)

x6_upd = x6_pred + Kx*r6
P6_upd = P6_pred - Kx*S6*Kx^T
```

Kx converts a measurement residual into a track correction. Native KalTest
uses the algebraically equivalent inverse-covariance form, not a separate
hand-coded update in this package.

Both representations have the same track prediction, covariance and
measurement. Therefore the five track parameters receive the same update.

Persistent6D also updates its explicit loss coordinate and cross-covariance:

```text
Kb = c6_pred*H6^T*inv(S6)

b6_upd = b0 + Kb*r6
B6_upd = B - Kb*S6*Kb^T
c6_upd = c6_pred - Kb*S6*Kx^T
```

Kb converts the residual into a loss correction. The measurement derivative
with respect to b is zero: the native measurement projection sees only the
five helix parameters. Nevertheless Kb need not be zero, because the loss
and track are correlated.

## 7. Continue to later hits without reapplying the same loss

For every subsequent transition:

```text
x_next_pred = ordinary propagation of x_current_upd

P_next_pred = F_next*P_current_upd*F_next^T + Q_next
```

F_next and Q_next are that transition's propagation Jacobian and process
noise. Persistent6D additionally carries:

```text
b_next_pred = b_current_upd
B_next_pred = B_current_upd
c_next_pred = c_current_upd*F_next^T
```

The same selected loss is not applied again. The ordinary downstream joint
Jacobian has a track block F_next, a unit b-to-b entry, and zero direct
b-to-track entries.

Consequently, the next track prediction does not require another explicit
b term. Its effect is already contained in the current track mean and
covariance. Both methods continue presenting the same track distribution to
subsequent hits. Persistent6D updates b explicitly along the way;
LocalMarginal leaves the final loss recovery until smoothing.

## 8. Bring downstream information back with RTS

RTS starts with the last smoothed state equal to its last updated state, then
works inward. For a transition from hit i to hit i+1, define:

```text
T_i = derivative mapping the updated track at i
      to the predicted track at i+1
```

For LocalMarginal, T_5=F*A at the selected breakpoint; elsewhere T_i is the
ordinary propagation Jacobian. The smoothing gain is:

```text
G_i = P_i_upd*T_i^T*inv(P_(i+1)_pred)
```

The earlier track is corrected by how the downstream smoothed state differs
from its original prediction:

```text
x_i_smooth =
    x_i_upd
  + G_i*(x_(i+1)_smooth - x_(i+1)_pred)

P_i_smooth =
    P_i_upd
  + G_i*(P_(i+1)_smooth - P_(i+1)_pred)*G_i^T
```

The implementation uses an equivalent conditional-covariance/Joseph form
for numerical stability. LocalMarginal performs this recursion in 5D.
Persistent6D performs the corresponding joint recursion in 6D downstream
of the loss birth and crosses the rectangular boundary back to 5D upstream.

## 9. Recover the final loss in LocalMarginal

RTS produces x6_smooth and P6_smooth, incorporating later measurements.
LocalMarginal uses its saved ORIGINAL breakpoint prediction and correlation:

```text
L = c6_pred*inv(P6_pred)

b_final = b0 + L*(x6_smooth - x6_pred)

B_final = B + L*(P6_smooth - P6_pred)*L^T
```

L translates the smoothed track correction into a correction of the original
loss estimate. b0 and B are still the original loss prior, not a previously
updated loss posterior used a second time.

These are the mean and variance operations in inferLoss(). The fitter calls
it with the smoothed downstream state for the all-hits loss estimate; a
separate call with the first updated downstream state provides the local-only
diagnostic. LocalMarginal does not infer the final loss using hit 6 alone.

Persistent6D already has a loss posterior at the final outward hit, after all
downstream measurements, and retains the joint smoothed states as well.

## 10. Why the final results match, and the limits of that statement

```text
Same incoming track and loss prior
    -> same outgoing track mean and covariance
    -> same downstream track predictions
    -> same measurement corrections
    -> equivalent final track and loss estimates
```

Persistent6D keeps the loss estimate explicit throughout the downstream
filter. LocalMarginal preserves its effect in the track covariance and
reconstructs the final estimate from saved correlations and RTS. The sixth
coordinate changes how the information is represented and updated, not how
much measurement information is available.

This equivalence requires the same fixed linearized model, priors, process
noise, measurements and retained correlations. It is not a statement about
arbitrary nonlinear refits, discarded correlations, different priors, or
future transport that depends explicitly on the historical b beyond its
already-applied effect on the track.

The current implementation's iteration capabilities differ: Persistent6D
can iterate RTS and backward separately; LocalMarginal retains one-pass RTS
and iterates backward only. Therefore enabling iterations does not constitute
an equivalent RTS comparison. Tiny floating-point differences can also occur;
matching saved pT does not imply bit-identical full covariance or chi2.

## Source verification map

These references support the full explanation above; reading them is not
required to understand the notation:

| Part | Implementation |
|---|---|
| Coordinates, units, separate pivot | [TrackState.h](../src/TrackState.h), [TrackState.cpp](../src/TrackState.cpp): fromEDM, toEDM, stateDifference |
| Native coordinate order | [KalTest TVTrack.h](../../../Utilities/KalTest/src/geomlib/TVTrack.h): PutInto/SetTo |
| Independent loss birth | [LossTrackState.h](../src/LossTrackState.h): introduce |
| Loss map and A/g derivatives | [BreakpointFitter.cpp](../src/BreakpointFitter.cpp): applyBreakpoint; [KalmanAdapter.cpp](../src/KalmanAdapter.cpp): advancePersistent |
| Joint covariance transformation | [AugmentedTransport.cpp](../src/AugmentedTransport.cpp): jacobian, covariance |
| Saved loss/track and source/target correlations | [BreakpointFitter.cpp](../src/BreakpointFitter.cpp): fitLocalRTS |
| Native measurement gain/update | [KalTest TVKalSite.cxx](../../../Utilities/KalTest/src/kallib/TVKalSite.cxx): Filter |
| Zero direct measurement derivative for b | [KalmanAdapter.cpp](../src/KalmanAdapter.cpp): LossMeasurementSite |
| Loss applied only at birth, persistent recursion | [BreakpointFitter.cpp](../src/BreakpointFitter.cpp): fitPersistent; [KalmanAdapter.cpp](../src/KalmanAdapter.cpp): advancePersistent |
| Smoothed loss recovery | [BreakpointFitter.cpp](../src/BreakpointFitter.cpp): inferLoss and its allHits call |

The paired-run numerical checks and endpoint/chi2 distinctions are documented
in the [parallel implementation record](../../../agents_record/2026-09-09-recbreakpoint-parallel-endpoints-chi2.md).
