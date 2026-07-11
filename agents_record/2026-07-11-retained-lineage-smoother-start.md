# Retained-lineage Gaussian-sum smoother start — 2026-07-11

## Outgoing focus preserved verbatim in substance

Before this experiment, the active concentration was component-local CEPC
material semantics and physics modelling: incidence-path-corrected `t/X0`,
followed by a step-conditioned CEPC Bethe-Heitler mixture. The resolved hit
update, likelihood, forward continuation state, focused navigation audit, and
reverse filtering were not considered active blockers.

The recorded reverse-filter evidence was IP pT 1.9785, 1.9970, and 2.2591 GeV
for events 11, 16, and 17 against truth 2.0004 GeV, versus LCIO 1.7934, 1.8118,
and 1.5790 GeV. Event 17 overshot, and the user had explicitly deferred it.
This was focused workflow evidence, not validation of nominal material, either
BH model, or arbitrary-track navigation.

The ordered roadmap was:

1. Use event 11 for step-level development and events 11, 16, and 17 for
   primary validation.
2. Compute component-local incidence-path-corrected `t/X0`, with explicit
   pre/post-surface ownership and no forward/reverse double counting.
3. Validate two event-11 material steps, then complete verbose 11/16/17 runs.
4. Fit a per-step-`t/X0` CEPC BH mixture from primary-electron tracker-volume
   Geant4 eBrem truth.
5. Retain 4–5 hypotheses with current-surface KL reduction and low-weight
   cutoff, then reassess component age.
6. Extend navigation validation beyond monotonic focused tracks.
7. Only then perform broad GSF-versus-LCIO studies.

Success required a retained hard-loss branch with measurement support and a
finite full-hit IP state closer to generator truth than LCIO across focused
events, without recovery or catastrophic smoothing. Non-goals were delayed
TopN as a final policy, reopening resolved recovery/segmentation without a new
reproduction, restoring removed KF/initialization experiments, immediate
TopN=1 as a recovery benchmark, fitting final BH physics to SimHit momentum,
treating ACTS ATLAS coefficients as CEPC validation, or changing shared
tracking packages for a purely local workaround.

This roadmap is deferred, not cancelled, while the statistically consistent
smoother is audited.

## Authorized interface extension

The audit showed that retained filtered states and innovation matrices are
insufficient for RTS smoothing: the exact inter-surface transport Jacobian is
also required. The user explicitly authorized a narrow shared-interface
extension. `MeasurementUpdate` now exposes the already-computed KalTest
transport Jacobian; both KalTest and DDKalTest MarlinTrk implementations fill
it. All smoothing logic remains in `Reconstruction/RecGsfTracking`.

## First implementation and validation

`RetainedLineageSmoothing` performs an RTS recursion independently on each
retained forward history. It composes the exact MarlinTrk transport Jacobian
with the selected BH component's surface-local curvature Jacobian. It is
deliberately restricted to `ReductionMode=TopN`, `ReverseFiltering=False`, and
`MaterialIPExtrapolation=False`: KL moment merging no longer represents one
real lineage, reverse refitting is a different backward-information workflow,
and material-aware smoothed IP transport is not implemented yet. A failed
component smoother suppresses the track instead of publishing a filtered
fallback.

The configured EL9/LCG 105 target builds and installs. Comprehensive verbose
runs of event 11 and then events 11, 16, and 17 retained 234/234 hits with no
measurement rejection. All final components completed 233 stored smoothing
steps:

| Event | Smoothed components | LCIO pT [GeV] | lineage-smoothed pT [GeV] |
|---:|---:|---:|---:|
| 11 | 7/7 | 1.7934 | 1.7933 |
| 16 | 7/7 | 1.8118 | 1.8118 |
| 17 | 12/12 | 1.5790 | 1.5789 |

The branch-local inner-pT changes were only a few micro-GeV. Curvature
variance changed only slightly (normally from about `1.0004e-7` to
`1.0004e-7`; the largest focused change was about `1.00075e-7` to
`9.9573e-8`). Thus the implementation is mechanically stable but does not
recover hard-loss momentum. It also demonstrates that the earlier reverse
refit improvement was not equivalent to standard smoothing.

The immediate question is why later measurements exert almost no smoothing
leverage on the innermost curvature. Audit the seed/first-filter covariance,
stored transition composition, predicted-covariance consistency, and branch
survival before changing the physics model or claiming a valid smoother.

## Transition, one-component, and delayed-pruning audit

The interface was extended once more within the same authorized diagnostic
scope to expose KalTest's process-noise covariance paired with the transport
Jacobian. Event-11 diagnostics store the BH continuation covariance, exact
transport `F` and `Q`, predicted covariance, RTS curvature-gain norm, and
curvature correction.

MarlinTrk covariance closure is good: the worst element-wise relative closure
reported on the first transitions is about `1.1e-4`, while typical later
values are `1e-6` to `7e-6`. The dominant loss of backward information occurs
before transport, in the global BH component covariance:

- incoming inner curvature variance is about `1.0004e-7`;
- the nominal near-no-loss `g4` component adds about `7.6e-6`;
- the `z=0.975`, sigma `0.1135` `g2` component adds about `4.4e-3`.

Consequently the first backward curvature gain is only about `0.013` for a
near-no-loss first branch and `2.3e-5` for a `g2` first branch. Later
transitions may have order-one gains and visible local curvature corrections,
but the first broad BH transition blocks those corrections from reaching the
pre-loss inner state.

With `ElectronHypothesis=False` and one component, the same RTS implementation
has curvature gains near one. Event 11 moves from inner pT `1.79334015` to
`1.79318793` GeV and publishes `1.7932` GeV, within `0.0002` GeV of LCIO
`1.7934` GeV. This is a successful central-value control, though large-sample
pull validation remains required.

A diagnostic three-hit delayed-TopN run retained 12 rather than 7 final event-
11 histories. The selected result remained `1.7933` GeV and the largest
smoothed inner pT among retained branches was only `1.793415` GeV. Delayed
pruning therefore does not remove the immediate blocker and is still not a
validated final policy.

The next physics task is to fit a step-conditioned mixture with sufficiently
narrow conditional components. Component means must describe distinct loss
hypotheses while component variances describe uncertainty conditional on each
hypothesis; a broad global loss distribution embedded inside every component
destroys the pre/post curvature correlation needed by a smoother.

## ACTS/ATLAS reference model implementation

The authoritative ACTS documentation was reviewed before implementation. Its
default `AtlasBetheHeitlerApprox<6,5>` uses no change below `0.0001 X0`, one
analytic Gaussian below `0.002 X0`, the six-component low polynomial set below
`0.1 X0`, and the six-component high set up to a `0.2 X0` cap. The existing
`Current` model already contained the ACTS/ATLAS low/high coefficient tables,
but its CEPC toy two-component branch intercepted every step below `0.1 X0`.

A separate `BHModel="ActsAtlas"` option now preserves the existing models while
implementing the ACTS regime selection. Its single-Gaussian mean and variance
use the exact Bethe-Heitler moments `E[z]=exp(-x)` and
`E[z^2]=exp(-x*log(3)/log(2))`. This is an ACTS-compatible ATLAS reference,
not a CEPC-validated model.

The target builds and installs. Complete verbose runs of focused events 11,
16, and 17 retain 234/234 hits and finish their one retained lineage. Every
audited material transition has nominal `t/X0 < 0.002`, so ACTS correctly uses
only its single-Gaussian regime: nine process applications, peak/final
component count one, and no reduction. Results are:

| Event | truth pT | LCIO pT | ActsAtlas lineage pT |
|---:|---:|---:|---:|
| 11 | 2.0004 | 1.7934 | 1.7934 |
| 16 | 2.0004 | 1.8118 | 1.8118 |
| 17 | 2.0004 | 1.5790 | 1.5790 |

For event 11 the first `x=5.72e-4` transition adds about `7.39e-5` curvature
variance and gives an RTS curvature-gain norm of `1.35e-3`. Thus the faithful
ACTS reference does not recover these events under the current nominal CEPC
material estimate. Its six-component low/high regimes were not reached by this
focused sample and still require a direct component-level test or a thicker
transition before being declared runtime-validated.

## Component-local incidence-corrected material

The prior material estimate was
`innerThickness/X0_inner + outerThickness/X0_outer`, identical for every
component and implicitly normal-incidence. It also accumulated summary
material before knowing whether a final outgoing transition existed.

The forward and reverse process-convolution paths now use the filtered
component's local helix tangent `(-sin(phi0), cos(phi0), tanLambda)` and the
DD4hep surface normal evaluated at that component's pivot. The owned outgoing
path is `pathTX0 = normalTX0 / abs(unitTangent dot unitNormal)`.

Invalid or near-tangent projections (`absCos <= 1e-6`) are rejected rather
than generating an infinite transition. Each measurement surface owns exactly
one outgoing transition; the final forward measurement owns none. Reverse
filtering applies the same current-surface rule in the inward direction.
Summary maximum and total material now refer to component-local outgoing path
values (weighted across a mixture for the total), rather than nominal normal
thickness including the final surface.

The first two event-11 transitions changed from `0.000571561` to
`0.000573798 X0` (`absCos=0.996102`) and from `0.000563679` to
`0.000566019 X0` (`absCos=0.995866`). The largest event-11 correction was from
`0.000427073` to `0.000484033 X0` (`absCos=0.882322`). Maximum corrected path
values for events 11, 16, and 17 were `0.00163546`, `0.00163468`, and
`0.00164820 X0`. All remain below the ACTS `0.002 X0` mixture threshold, so
the focused ActsAtlas results remain 1.7934, 1.8118, and 1.5790 GeV.
