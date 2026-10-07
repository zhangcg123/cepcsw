# Absolute ECAL loss with a diffuse loss-map reference

## Outgoing focus, preserved verbatim

Active work on local `breakpoint` is the default-off absolute-neutral-loss
RTS prototype. It selects reconstructed hit-supported neutral ECAL clusters
near the track and initializes a persistent sixth coordinate with their
absolute energy and error. It runs beside the existing log-loss results;
the installed maintained card leaves it off unless explicitly enabled.
In 12 selected local events it completed 11 fits and fell back once. The
median absolute pT residual improved from 18.39% to 13.44% versus ordinary
RTS, but one severe overshoot and weak early-loss responses make it worse
than the prior log-loss ECAL diagnostic. It is not physics-validated.

Next: diagnose the predicted and smoothed states at the breakpoint in the
weak/overshooting events, test a genuine relinearized absolute-loss fit,
then perform same-code A/B comparisons with the log-loss ECAL prior.
Success requires better early-loss pT recovery without clean/light-loss
degradation or new tails on a held-out population. Do not promote the
new method or broaden its use from the 12-event mechanical gate. Full
implementation/results and the superseded ECAL focus are in
`agents_record/2026-10-07-breakpoint-absolute-neutral-loss-prototype.md`;
the prior population records remain under `agents_record/`.

## What was implemented

User authorized trying the absolute ECAL-loss method with a diffuse reference
for its mapping/Jacobian. The first implementation deliberately changes ONLY
the loss birth mapping, not every geometric/measurement linearization and
not an iteration loop. `AbsoluteNeutralDiffuseReference` is default false,
explicit in `options/run_breakpoint.py` through
`BP_ABSOLUTE_NEUTRAL_DIFFUSE_REFERENCE`. When true with
`AbsoluteNeutralLossRTS`, `DiffuseAugmentedRTS` must be true.

At selected interval i -> i+1, the upstream diffuse smoothed mean supplies
the loss-map expansion point. Its covariance is explicitly zeroed/discarded.
The new independent fit restarts with its ORIGINAL seed and hits. The loss
coordinate L and its variance are initialized from the selected neutral ECAL
energy/error, exactly as in the existing absolute method. No diffuse loss
prior and no second ECAL measurement are added.

Internal coordinates are `(drho,phi0,kappa,dz,tanLambda,L)`, kappa=1/pT with
charge sign. At the reference, p=sqrt(1+tanLambda^2)/abs(kappa), a=p/(p-L).
The loss mapping has kappa_out=kappa*a, with derivatives
`d(kappa_out)/d(kappa)=a^2`,
`d(kappa_out)/d(tanLambda)=-kappa*L*p*tanLambda/((p-L)^2*(1+tanLambda^2))`,
and `d(kappa_out)/dL=kappa*p/(p-L)^2`. Other coordinates have the identity map.

The live mean is transported as `f(reference)+J(reference)*(live-reference)`.
The live covariance is transported as `J*P_live*J^T`, not with the diffuse
covariance. Native material/geometric propagation and the native 6D hit
update follow. The stored transition is used by the existing RTS recursion.
The pure helper `include/RecBreakpoint/AbsoluteLossMapping.h` is shared by the
adapter and numerical tests. Pivots must match. Invalid/missing references
use the existing status -1 ordinary-RTS fallback, with an explicit error.

New passive flat branches record reference requested/used, reference track
parameters, and forward/reference p_before. A successful reference fit has
`absolute_neutral_diffuse_reference_used=1`. Fit status 2 still means only
mechanical success, not acceptable physical recovery. Verbose dumps include
all downstream 6D predicted/filtered/smoothed means/covariances, transports,
and process noise for the absolute fit.

## Verification and exact comparisons

Built RecBreakpoint and ran RecBreakpointTransport, RecBreakpointLikelihood,
and RecBreakpointDiffuse: 3/3 passed. New transport tests exercise both charge
signs, both slope signs, zero/light/large losses, finite-difference Jacobians,
same-reference equivalence, affine-offset arithmetic, and invalid loss.

All event identities are zero-based `(rec file seed, event_index, track 0)`.
Inputs are `sim_large_barrel_20261001/rec-barrel-{1,2,3,4}.root`. No Condor
jobs or simulation jobs were submitted. Local outputs/logs/CSV/JSON are under
`/tmp/bp-absolute-diffuse-reference-EnCks6/`; driver is
`TrackingPerformanceStudies/breakpoint_ecal_refit_repro_20261007/reference_compare.py`.

- Same-code reference off/on: all non-absolute flat branches are exactly
  unchanged, including ordinary RTS/backward, diffuse, and disabled copies.
- Exact historical steering (`LocalMarginal`, diffuse off) reproduces ALL
  pre-existing branches of the previous twelve absolute-prototype rows.
  The main comparison instead keeps diffuse on and uses `Persistent6D` for
  the ordinary log-loss controls. Comparing those different steering modes
  directly to the archive would produce expected diagnostic differences.
- The old twelve log-loss ECAL-prior runs were also rerun using the new code;
  RTS pT matches the recent reproduction archive exactly (maximum difference
  zero). They use `MeanLogLoss=log(1+E/p_after_diffuse)` and
  `SigmaLogLoss=.04*E/(p_after_diffuse+E)` with the archived PFO energy and
  diffuse p_after. This is NOT the new reference-only method.
- Added seed-1 events 0,11,16,17 have status 1 ordinary copies and are unchanged
  by the reference switch. These are current-input control events, not claims
  to have rerun the original historical hard-event set with those indices.
- Focused verbose A/B: event 1:5. Additional verbose bad case: 2:10.

The direct absolute A/B uses IDENTICAL reconstructed selected ECAL energies
and errors on each side. Two error assumptions were checked: maintained
`0.011 sqrt(E)+0.004 E`, and 4% of E (stochastic coefficient zero, constant
coefficient .04). Old log-prior runs used neutral PFO energy, which can differ
slightly from the ECAL-cluster-only energy; their comparison is not a pure
parameterization-only equivalence test.

Residual below is `100*(pT_reco-pT_truth)/pT_truth`.

| Seed:event | Absolute ECAL, 4% | + diffuse reference | Old log-prior, 4% |
|---|---:|---:|---:|
| 1:5 | -19.44% | -4.93% | -0.55% |
| 1:25 | -25.14% | -23.88% | -3.08% |
| 1:31 | +0.29% | +0.27% | +0.34% |
| 1:64 | +3.78% | +3.44% | +3.79% |
| 2:1 | -11.60% | -1.20% | -0.86% |
| 2:3 | -4.98% | -0.10% | +0.01% |
| 2:10 | -23.83% (fallback) | +839.05% | +1.85% |
| 2:11 | -34.92% | -19.09% | +1.44% |
| 3:1 | -14.59% | -14.59% | -14.23% |
| 3:52 | +30.75% | -4.86% | -0.46% |
| 4:46 | -8.21% | -8.21% | -8.22% |
| 4:92 | -8.28% | -8.28% | -8.29% |

Across all twelve (including the old fallback), median absolute residual is
13.0931% -> 6.5694% with 4% ECAL errors; old log-prior gives 1.6417%.
With the maintained ECAL error formula it is 13.4378% -> 6.5539%.
Eight of twelve move closer to truth under each error assumption, but the
new extreme tail forbids interpreting the median improvement as a safe gain.
Off completes eleven fits/one fallback; on returns twelve mechanical fits.

## Interpretation and next question

The loss-map expansion point materially affects the answer: e.g. 1:5 moves
from 11.14558 to 13.15307 GeV (truth 13.83536), and 3:52 from 59.09362 to
43.00041 GeV (truth 45.19482). This does NOT establish that a diffuse mean is
generally a good reference or that this one change solves the model.

In 2:10, ECAL supplies 5.5583 GeV, the upstream forward momentum is only
3.4765 GeV, and the diffuse reference is 16.3050 GeV. The old exact loss map
cannot subtract that loss from the forward mean and falls back. The new
affine map can be evaluated, but extrapolates far from its reference and
returns pT=155.7847 GeV instead of truth 16.5897. The retained loss itself is
5.5571 GeV: the failure is not a large fitted change of the ECAL loss prior.
This flags the joint trajectory/mapping consistency for investigation; it is
not evidence to tune a truth-based fallback or recycle a fitted covariance.
The verbose trace gives a more specific warning: upstream diffuse reference
kappa is +0.08208774, the first downstream prediction is +0.82154770, its
first filtered value is +0.33340777, and the next hit changes curvature to
-0.03804196. The first downstream RTS value is -0.07912822; published IP
omega is also negative (-5.77320725e-6/mm). Thus the hits move the trajectory
across the reference curvature sign, while the one-pass loss Jacobian remains
the one evaluated at positive reference curvature. A better unsigned
reference momentum alone is insufficient here. No sign-based post-selection
or rejection was added to hide this failure.

Next, review the full reference-trajectory/affine propagation treatment and
check the nonlinear loss relation at the fitted breakpoint before proposing
more source changes. No whole-trajectory iteration or production promotion
was made. GSF/shared KF sources and existing default fit behavior are intact.
At this handoff the NEW switch is built/tested using `build.../run`, not
installed into the shared InstallArea. Its card default remains false.
The standalone numerical helper/tests were checkpointed locally as `686237e`.
The adapter/controller/card integration remains in the working tree alongside
the pre-existing uncommitted RecBreakpoint development. No unrelated changes
were staged and no remote refs were changed.
