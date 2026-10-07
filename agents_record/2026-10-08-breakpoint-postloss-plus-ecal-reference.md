# Absolute ECAL loss using diffuse post-break momentum as reference

## Outgoing focus, preserved verbatim

Active work on local `breakpoint` is the default-off absolute-neutral-loss
RTS prototype and its loss-map reference sensitivity. The new default-off
`AbsoluteNeutralDiffuseReference` evaluates only the absolute-loss birth map
and Jacobian at the diffuse upstream smoothed mean. It retains the original
seed/live covariance and independent ECAL loss prior/error; diffuse covariance
is not reused. It is one reference-linearized loss map, not a whole-trajectory
iteration. The switch is built/tested via `build.../run`, not installed yet.

On twelve selected events with 4% ECAL errors, median absolute pT residual
improves 13.09% -> 6.57%, but a new +839% tail in event 2:10 makes it unsafe.
The old log-loss ECAL-prior diagnostic remains better (1.64% median). Exact
historical steering reproduces all old flat fields, and same-code on/off
leaves all non-absolute outputs unchanged. No physics validation or promotion.

Next: review joint-trajectory/loss-map consistency in 2:10 and the remaining
weak early-loss events before designing full reference-trajectory treatment.
Do not replace the prior using the fitted covariance or tune truth-based
fallbacks. Success still requires clean/light-loss safety, tail control and
held-out pT recovery. Current evidence and the complete outgoing focus are in
`agents_record/2026-10-08-breakpoint-absolute-diffuse-reference.md`; the initial
prototype and earlier population evidence remain in dated records.

## Implementation

`AbsoluteNeutralReferenceSource` selects `UpstreamSmoothed` (previous
behavior, compiled/card default) or `PostLossPlusECAL`, effective when
`AbsoluteNeutralDiffuseReference=true`. Both are default-off experiments
overall. The second choice reads the diffuse smoothed six-dimensional state
immediately after the truth-selected interval, computes
`p_after_diffuse=sqrt(1+tanLambda_after^2)/abs(kappa_after)`, then uses
`p_before_reference=p_after_diffuse+E_neutral`. Its reference curvature is
`sign(kappa_after)*sqrt(1+tanLambda_upstream^2)/p_before_reference` at the
upstream diffuse smoothed pivot/direction. The selected ECAL cluster energy
and its error still initialize the independent absolute loss coordinate
`L` and its covariance. The original track seed, live covariance, native
measurement update, and RTS recursion are unchanged. Diffuse covariance is
discarded. No extra prior/likelihood factor or truth loss value is inserted.
The same one-pass affine loss map and Jacobian are used as in the previous
reference test; no full trajectory iteration was added.

The flat tuple now records `absolute_neutral_reference_source` and
`absolute_neutral_reference_p_after` alongside the existing before/reference
and ECAL fields. The validated code checks the selected interval and finite
post-break state before publishing the diffuse output; an invalid reference
follows the existing explicit fallback/status convention. The maintained
`options/run_breakpoint.py` exposes
`BP_ABSOLUTE_NEUTRAL_REFERENCE_SOURCE`; its default remains
`UpstreamSmoothed`. The property is inert when
`AbsoluteNeutralDiffuseReference=false`.

## Same-code local A/B

The twelve selected `(seed,event)` track-0 identities, source rec tuples,
cluster selector, 4% per-cluster energy error, seed, and fit code are the
same on both sides. The old log-loss results are the independently rerun
same-event control from the preceding study. All runs were local; no Condor
jobs or simulations were submitted. Analysis driver, CSV, JSON, ROOT tuples,
and logs are under
`TrackingPerformanceStudies/breakpoint_ecal_refit_repro_20261007/`
and `/tmp/bp-absolute-postplus-KmOfVA/`. Four extra seed-1 controls
(events 0,11,16,17) have unchanged ordinary copies. All non-absolute flat
fields are exactly identical between reference modes, and the newly rerun
`UpstreamSmoothed` absolute pT/status reproduces the previous twelve rows
exactly. Both modes choose the same clusters and ECAL prior energy/error in
every event. Each usable post-plus-ECAL reference satisfies
`p_before_reference = p_after_diffuse + E_neutral` within 1e-10 GeV.

| Seed:event | Truth pT | Upstream reference pT | Post+ECAL reference pT | Old log-prior pT | New status |
|---|---:|---:|---:|---:|---:|
| 1:5 | 13.83536 | 13.15307 | 13.75812 | 13.75904 | 2 |
| 1:25 | 31.82990 | 24.22804 | 18.34864 | 30.85045 | -1 (ordinary copy) |
| 1:31 | 15.03545 | 15.07631 | 15.08094 | 15.08679 | 2 |
| 1:64 | 9.84527 | 10.18373 | 10.21732 | 10.21795 | 2 |
| 2:1 | 32.52551 | 32.13673 | 32.26950 | 32.24724 | 2 |
| 2:3 | 16.99266 | 16.97559 | 16.99193 | 16.99461 | 2 |
| 2:10 | 16.58967 | 155.78470 | 16.78371 | 16.89596 | 2 |
| 2:11 | 19.59764 | 15.85690 | 19.73422 | 19.87927 | 2 |
| 3:1 | 29.68323 | 25.35255 | 25.44900 | 25.45962 | 2 |
| 3:52 | 45.19482 | 43.00041 | 44.94171 | 44.98650 | 2 |
| 4:46 | 14.29305 | 13.11997 | 13.11932 | 13.11810 | 2 |
| 4:92 | 10.50399 | 9.63378 | 9.63378 | 9.63363 | 2 |

On all twelve, median absolute pT residual is 6.5694% for upstream,
0.9784% for post-plus-ECAL, and 1.6417% for the old log prior. Seven of
twelve post-plus-ECAL results improve over upstream. On the eleven successful
post-plus-ECAL fits, its median is 0.7871% against old log-prior 1.4371%,
and the maximum absolute pT difference from the old log-prior result is
0.1451 GeV. These are selected-event comparisons, not population resolution
or physics validation. The new method's worst residual is 42.35% in 1:25;
the old log-prior control's worst is 14.23%. Its apparently attractive
all-event median therefore hides an unacceptable fallback/tail.

The focused verbose checks were 1:25 and 2:10. In 2:10, the original
upstream diffuse reference p_before was 16.3050 GeV. The post-plus-ECAL
reference uses p_after=16.9154 and E=5.5583 GeV, giving p_before=22.4737.
The previous 155.785 GeV tail becomes 16.784 GeV (truth 16.590).

For 1:25, the live forward p_before is about 95.255 GeV, diffuse upstream
reference 88.957 GeV, and the new post-plus-ECAL reference only 32.716 GeV.
The affine prediction made from this distant reference reverses the live
curvature sign, so `AbsoluteLossMapping` rejects it and the output is an
ordinary RTS copy. It is a visible failure, not a silent clipping or
truth-based veto. A full reference-trajectory fit, including the original
seed/hit objective, is the next technical question. Simply replacing one
loss-map expansion point cannot guarantee that a far-away live forward
state remains in its locally valid region.

RecBreakpoint built and the three standalone RecBreakpoint test targets
passed (Transport, Likelihood, Diffuse). The new source/card option was
tested through the build-tree `run`; the shared InstallArea was not changed.
No GSF or shared KF source was edited, and no remote ref was changed.
