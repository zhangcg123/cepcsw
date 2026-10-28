# Neutral-update diagnosis paused; charged electron+photon constraint next

## User decision and scope

On 2026-10-29 the user asked to remember the neutral-update issues, stop this
line of work for now, and discuss charged-cluster electron+photon constraints.
Do NOT reinterpret this as declaring the early neutral approximation harmless.
Further fit experiments must remain under /tmp until a solid improvement is
established and production changes are explicitly authorized. No default,
installation, fitter, GSF, shared KF or PFA change was made in these tests.

Current branch: `breakpoint-delayed-neutral`, source head before this memory
checkpoint `87cec04`. Original `breakpoint` remains preserved. No new-branch
remote publication was requested. Installed RecBreakpoint SHA256 remains
`911962a6b7e9e91d533018fc699c522a7e3c3d68e50dd58dbbdea774872ec986`.

## Three issues that must remain distinct

1. **Outer optimizer bound:** Minuit optimizes a Gaussian prior center b0 in
   [0,FreeLossMaxLogLoss], currently default 1. The augmented fit retains
   SigmaLogLoss=0.001. The final fitted b is not clipped to this interval.
   b=log(p_before/p_after), so b0=1 corresponds to a 63.2% loss at the prior
   center, not 100%. Physically b can range from zero to arbitrarily large
   positive values as p_after approaches zero. Removing the artificial upper
   bound was discussed but NOT implemented, even experimentally.
2. **Neutral-measurement approximation:** h=p_after*(exp(b)-1) is linearized
   at a reference state. Early update can yield a final state whose exact h
   disagrees badly with the affine h used in the measurement likelihood.
   Delayed update evaluates this measurement at the neutral-free smoothed
   state and conditions the correlated trajectory. This addresses the tested
   early failure mechanism, not every remaining performance problem.
3. **Loss-transition approximation:** kappa_after=kappa_before*exp(b) is
   evaluated during forward filtering; its stored Jacobian is retained during
   smoothing/conditioning within a trial. Its final exact-versus-affine closure
   is a DIFFERENT diagnostic from item 2. Tests do not establish it as the main
   cause of the remaining delayed-fit errors. Leave that mapping unchanged;
   small closure alone does not prove all Jacobians/covariances optimal.

## Same-code temporary four-way comparison

Artifacts: `/tmp/bp-resolved-map-j9iBQ8/fourway/` (volatile, not permanent).
Includes README, manifest, comparison.csv, diagnostics.csv, trial_audit.json,
summary.json, upstream_comparison.csv and per-job cards/logs/ROOT files.
Parent directory holds logging-only RecBreakpoint.cpp/library, build/run scripts,
selection and preceding delayed tests. No changed fit algebra. Loaded-library
paths and protected source/card/install hashes were verified.

Twelve topology-clear single-eBrem tracks: two bad and two good delayed tracks
per VXD/ITK/TPC. Bad selection: |delayed residual|>5%, |simple addition|<5%; good:
|delayed residual|<1%. First two in seed/event order per category. This is NOT
a random sample or population validation. Event indices below are zero-based.

Cross early/delayed with max b0=1/2 at unchanged sigma=0.001 and measurement
inputs. Forty seed/config jobs yield 48 track/config results; fourteen results
reuse the immediately preceding identical-library temporary runs, 34 are new.
At each bound, the 000/beam/charged free-loss pT values match EXACTLY between
early and delayed runs. Initial delayed pT reproduces stored campaign values
for all 12; do not claim all branches exact (8 rows differ in other diagnostics).

Residuals = 100*(pT_reco/pT_truth-1), percent. F denotes failed neutral fit and
publication of the preceding constraint-stage fallback, not an early neutral fit.

| seed:event | owner | early max1 | early max2 | delayed max1 | delayed max2 | simple addition |
|---|---|---:|---:|---:|---:|---:|
| 10:60 | VXD | -69.99754 F | -69.99754 F | -16.32194 | -2.48504 | -2.51345 |
| 13:169 | VXD | -74.68319 F | -74.68319 F | -29.10473 | -1.32856 | -1.30996 |
| 1:5 | VXD | +1.02629 | +1.02651 | -0.46714 | -0.46717 | -0.50460 |
| 4:11 | VXD | +5.32049 | +5.32045 | +0.40232 | +0.40245 | +0.45658 |
| 467:96 | ITK | -5.78720 | -5.78717 | -5.82969 | -5.82970 | -4.91251 |
| 492:7 | ITK | -5.37576 | -5.37583 | -5.45259 | -5.45260 | -4.87805 |
| 1:194 | ITK | +0.03291 | +0.03291 | +0.03268 | +0.03275 | +0.66893 |
| 3:59 | ITK | -0.46251 | -0.46245 | -0.46271 | -0.46266 | -0.34741 |
| 55:186 | TPC | -6.57186 | -6.57186 | -6.60476 | -6.60483 | -2.75318 |
| 79:3 | TPC | -5.87205 | -5.87203 | -5.89660 | -5.89657 | -3.23616 |
| 3:173 | TPC | -0.24057 | -0.24052 | -0.24063 | -0.24056 | +0.60944 |
| 5:131 | TPC | +0.25303 | +0.25305 | +0.25233 | +0.25234 | +0.82282 |

Both failed early VXD fits have 33 evaluated trials at either bound: only b0=0
valid; 32 rejected with `Scalar update crosses zero curvature`, ending with
`No converged valid Minuit minimum`. Widening does not rescue them.
For early 4:11, b0=0.44823 (not bound-limited): affine neutral residual is
approximately zero but exact neutral energy differs by 17.77 measurement sigma.
Its loss-transition closure defect is only about 4e-6 percent. Early 1:5 has a
7.35-sigma exact-versus-affine neutral discrepancy at b0=0.22615. Delayed resolves
those measurement discrepancies on these two tracks.

The two delayed large-loss VXD fits improve when the bound is widened without
changing mapping/Jacobian code. Their truth b is approximately 1.20425/1.37454.
Optimized centers at max2 are 1.1787698162/1.3603423883, neither at a bound.
Minuit status is 0, EDM 5.99e-9/2.13e-8. Published objectives are
56.3287199100/-25.8780562440: best among 107/108 valid evaluated trials, with
local scan rising on either side. Selected objective repeats reproducibly.
This supports numerical minima of the implemented model, NOT a proof of an
exact nonlinear global optimum. No dense global scan or b0>2 check was made.
Increasing the bound also rescales scan starts; do not omit that qualification.

Four bad ITK/TPC delayed tracks still have 5.45--6.60% pT errors with loss-map
closure defects below 0.001%. Their causes remain unresolved. None of this
establishes a population-wide benefit from an unbounded optimizer.

## Next discussion: unresolved charged-cluster electron+photon energy

Code rechecked at handoff: `ChargedEnergyMeasurement.h`,
`BreakpointFitter.cpp::constrainChargedEnergy`, and conditional routing in
`RecBreakpoint.cpp`. With a selected breakpoint and no collected separate
neutral, requested ElectronPhoton uses the ECAL-face state to predict
h_sum=p_ECAL*exp(b), under the existing massless one-loss approximation.
The charged measurement is a terminal KF update and RTS carries its information
inward. With no breakpoint or a collected separate neutral, routing uses
ElectronOnly instead. Do not assume delayed neutral changes this charged step.

Discuss whether this measurement and its linearization correctly constrain
unresolved electron+photon events; distinguish charged measurement approximation,
loss transition, optimizer bounds, and cluster energy completeness/calibration.
Do not modify PFA or promote a new fitter/default before user agreement.
Earlier unresolved-map tests remain in
`Reconstruction/RecBreakpoint/docs/unresolved-loss-map-diagnostic.md`.

## Memory preservation / migration map

Complete outgoing AGENTS preserved verbatim as
`agents_record/2026-10-29-agents-before-charged-sum-focus.md`.
Section 1/global status, laws and compile/run instructions retained unchanged.
Section 2/outgoing focus retained in that snapshot; replaced with the current
charged-sum focus plus links here. No history directory moved or deleted.
The older independent acceptance audit is NOT closed by this small test;
its ignored-better-candidate issue remains separate. See
`agents_record/2026-10-26-breakpoint-independent-audit-handoff.md`.
