# Exact MarlinTrk innovation likelihood completion

Date: 2026-07-10

This record preserves completion of the first two actions from the ACTS GSF
review and the evidence that moves the active focus to explicit material
transition semantics.

## Implemented capability

With explicit user authorization for the shared-interface work, MarlinTrk now
offers an `addAndFit` diagnostic overload returning the quantities used by the
accepted Kalman measurement update.  Both the KalTest and DDKalTest backends
provide the predicted state and covariance, predicted measurement, calibrated
residual, measurement projector `H`, measurement covariance `R`, innovation
covariance `S`, and `log(det(S))`.  A small non-virtual, read-only KalTest site
accessor exposes the stored measurement derivative without changing KalTest's
virtual interface.

`RecGsfTracking` consumes this overload and evaluates each accepted component
with the complete Gaussian innovation likelihood

```text
log(w_post) = log(w_prior) - 0.5 * (deltaChi2 + log(det(S)))
```

Weights are normalized using a common maximum-log shift.  The focused verbose
dump prints all input matrices and the old and new log weights, so the weight
calculation is directly auditable without a second hand-coded Kalman update.

The implementation changes are in:

- `Reconstruction/RecGsfTracking/src/GsfAlgorithm.cpp`
- `Service/TrackSystemSvc/include/TrackSystemSvc/IMarlinTrack.h`
- `Service/TrackSystemSvc/src/IMarlinTrack.cc`
- `Service/TrackSystemSvc/src/MarlinKalTestTrack.{h,cc}`
- `Service/TrackSystemSvc/src/MarlinDDKalTestTrack.{h,cc}`
- `Utilities/KalTest/include/kaltest/TVKalSite.h`
- `Utilities/KalTest/src/kallib/TVKalSite.h`

The new virtual MarlinTrk overload was appended to the interface to preserve
the order of existing vtable slots.  The KalTest accessor remains non-virtual;
making it virtual caused an ABI dispatch failure during development.

## Focused evidence

The relevant targets build, and focused runs for events 11, 16, and 17 each
completed with one fitted track.  Event 11 produced full exact component dumps.
At the first two measurement updates the important components had nearly equal
innovation determinants and very small chi-square differences, so the complete
likelihood caused little relative reweighting at those surfaces.  This is a
measured property of those updates, not a failure to apply the determinant.

The final event-11 comparison was:

| quantity | truth | LCIO | GSF |
|---|---:|---:|---:|
| pT [GeV] | 2.0004 | 1.7934 | 1.2167 |
| p [GeV] | 2.008 | 1.800 | 1.221 |

The selected GSF branch had the history `g4 -> g1` and retained all 234 hits.
Thus the statistical measurement likelihood is now more correct, but hard-loss
momentum recovery is not demonstrated and physics validation remains open.

## Material-transition clarification

The current algorithm performs BH splitting before the target hit's measurement
update.  Its operational sequence is approximately

```text
previous stored site --rewrite its states with BH loss-->
MarlinTrk initialise from rewritten state --> propagate/addAndFit target hit
```

This gives the target hit a loss-adjusted hypothesis, so the ordering is not
the missing issue.  The semantic problem is that `BetheHeitlerSplitter::split`
changes kappa and covariance in every state of the preceding stored
`TKalTrackSite`.  It creates neither a material-surface state nor distinct
pre-material and post-material states.  It also uses one nominal target-layer
`t/X0` for every component, without a component-local crossing or incidence
path correction.

This ordering was subsequently checked directly against the local ACTS checkout
at commit `d33613d3f95a26779deba20d766ac748d965b3a4`. ACTS is surface-centric:
it propagates and binds every component to the current surface, performs the
Kalman measurement update, computes posterior weights, and then convolves the
filtered components with that surface's full material slab. The slab is
evaluated at each component's local position and propagation direction and is
scaled by `surface.pathCorrection(...)`. Components are then reduced, subjected
to a low-weight cutoff, and installed into the multi-stepper for propagation to
the next surface. Forward and backward passes change the momentum transformation
according to propagation direction.

Therefore, preserving split-before-measurement is not itself an ACTS-derived
requirement. The next implementation must first establish how CEPC associates
pre- and post-measurement material with each sensitive surface. Where that
mapping matches ACTS, it should preserve the filtered measurement state as the
pre-material state, perform direction-aware surface-local BH convolution, and
continue with distinct post-material components. The previous-site rewrite and
single nominal `t/X0` remain defects regardless of the chosen surface convention.

## GSF workflow implementation roadmap

The next phase compares the tracking workflow itself with ACTS, independently
of whether the current BH/material physics model is ultimately valid.

Capabilities already aligned sufficiently for this phase are multi-component
branching, the baseline MarlinTrk measurement update, exact innovation
diagnostics, the complete Gaussian posterior likelihood, stable normalization,
and branch ancestry diagnostics.

The remaining workflow work and expected difficulty are:

1. **Surface-local component snapshots (moderate):** represent a component by
   its surface identity, parameters, covariance, weight, accumulated
   likelihood, and ancestry. A cloned full Kalman history must not be the
   object on which current-surface mixture mathematics depends.
2. **Continuous propagation/continuation (high):** determine whether MarlinTrk
   can continue cleanly from an externally supplied filtered or post-process
   surface state while preserving transport covariance and Jacobians. If the
   capability exists only internally, expose it through a narrow interface
   rather than implementing another propagator or Kalman update.
3. **Surface workflow and boundary handling (moderate):** establish CEPC's
   pre/post-measurement material convention, handle the initial and final
   surfaces without double counting, and replace previous-site mutation with
   a surface-bound transition.
4. **Current-surface KL reduction (moderate/high):** compare and merge only
   states expressed at one surface. Handle periodic phi, charge consistency,
   positive-definite covariance, and reconstruction of a valid continuation
   state after merging.
5. **Component cutoff/cap (low):** after posterior evaluation, apply a
   low-weight cutoff, current-surface reduction, and normalization in a defined
   order. Delayed or immediate TopN remains experimental rather than the
   reference policy.
6. **Path-based navigation (high):** radius sorting is not generally a track
   path. Initially verify monotonic path ordering for the focused 2 GeV,
   85-degree events; generalize navigation only after the surface-state
   workflow is stable.
7. **Reverse multi-component filtering (high):** initialize from the final
   forward mixture, traverse surfaces and measurements in reverse, use
   direction-aware transition semantics, avoid material double counting, and
   produce the mixture at the IP reference surface.
8. **Output semantics (low/moderate):** retain best-branch output until a
   coherent moment-matched covariance and fit-quality definition exists. Do
   not combine mixture parameters with best-branch chi-square/NDF metadata.
9. **Focused tests:** add deterministic checks for posterior weights,
   current-surface merging, low-weight removal, navigation ordering,
   forward/reverse consistency, and output covariance/metadata.

The implementation gates are:

```text
A. introduce surface-local component snapshots
B. implement current-surface update/transition/continuation ordering
C. validate two event-11 steps without reduction
D. add cutoff and current-surface KL reduction
E. validate complete forward runs for events 11, 16, and 17
F. remove radius-order assumptions where the focused evidence requires it
G. implement reverse multi-component filtering
H. validate interaction-point momentum recovery
```

The first architectural investigation is MarlinTrk continuation from a supplied
surface state. This determines whether the forward workflow can remain local to
`RecGsfTracking` or needs another narrowly scoped shared-interface extension.
