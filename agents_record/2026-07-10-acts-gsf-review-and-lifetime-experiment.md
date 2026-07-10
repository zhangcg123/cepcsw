# ACTS GSF review and component-lifetime experiment

Date: 2026-07-10

This record preserves the outgoing component-lifetime focus, the experiment
performed under it, and the design review that changed the immediate work
order.  It is historical evidence; `AGENTS.md` remains the authority for the
current focus.

## Outgoing focus

The concentration was energy-loss inference and component lifetime rather
than the resolved hit-update recovery problem.  The experimental
`GlobalSim2GeV85` model supplied the same five retained-momentum hypotheses at
every qualifying split and ignored the individual material step `t/X0`.  Its
dominant prior was the near-no-loss component `z=0.99995` with weight 0.5793.
With immediate TopN target 1, lower-`z` components were normally deleted after
one following hit, before outer-hit curvature could identify a hard loss.

The primary validation events and pre-experiment results were:

| event | truth pT [GeV] | LCIO pT [GeV] | GSF pT [GeV] |
|---:|---:|---:|---:|
| 11 | 2.000 | 1.793 | 1.793 |
| 16 | 2.000 | 1.812 | 1.812 |
| 17 | 2.000 | 1.579 | 1.579 |

The intended sequence was to retain 3-5 hypotheses for several hits, track
branch momentum/weight/chi-square/age/ancestry, expose the true predicted
measurement and residual, fit a step-conditioned BH model from primary-
electron Geant4 truth, represent loss as a pre/post-material transition, and
then validate backward smoothing before any broad performance study.

## Delayed-reduction experiment

`ReductionMinHitsAfterSplit` was introduced as an experimental control with a
compatibility-preserving default of zero.  Components gained parent ID,
generation, and successful-hit age diagnostics.  With five components,
TopN target 1, and a three-hit minimum age, events 11, 16, and 17 retained all
five hypotheses for three measurement updates before reduction.

The delayed run completed with full hits but did not improve IP momentum:

| event | truth pT [GeV] | LCIO pT [GeV] | delayed GSF pT [GeV] |
|---:|---:|---:|---:|
| 11 | 2.0004 | 1.7934 | 1.7933 |
| 16 | 2.0004 | 1.8118 | 1.8117 |
| 17 | 2.0004 | 1.5790 | 1.5789 |

At event 11 hit 2, the approximately 1.217 GeV branch had the smallest step
chi-square increment (0.000731) but retained only its 0.0345 prior weight; the
near-no-loss branch retained weight 0.5793.  At hit 3 the near-LCIO branches
had accumulated chi-square near 10.32 while the lower-momentum branches had
13.72 and 16.82, so TopN selected the near-no-loss branch.  Component lifetime
alone at this setting did not establish hard-loss recovery.

The property is therefore provisional.  Some lifetime protection may remain
useful, but this exact delayed-TopN policy is not a validated production rule.

## Prediction and residual diagnostics

Verbose component output was extended with the predicted 3D surface crossing,
measured 3D hit position, signed global residual `measurement - prediction`,
local predicted and measured coordinates, local residuals, hit uncertainties,
and measurement-only pulls.  These calculations run only when
`VerboseDump`, `VerboseSplitDump`, and `ComponentDebugDump` are all enabled.

For event 11 at radius 43.1 mm, the five post-split branches gave:

| pre-update pT [GeV] | local m0 residual [mm] | m0 hit-only pull | delta chi2 |
|---:|---:|---:|---:|
| 0.656 | -0.10430 | -20.86 | 6.886 |
| 1.217 | -0.02647 | -5.29 | 3.774 |
| 1.749 | +0.00129 | +0.26 | 1.435 |
| 1.784 | +0.00257 | +0.51 | 1.458 |
| 1.793 | +0.00288 | +0.58 | 1.466 |

The branches do separate geometrically.  The hit-only pull is not the Kalman
innovation pull because it divides by detector uncertainty alone.  The much
smaller delta chi-square shows that predicted-state covariance substantially
softens the likelihood.  The standalone GSF-local intersection helper still
reports no crossing for the first three hits even though MarlinTrk updates them;
therefore it is not authoritative.  The exact predicted bound state, residual,
projector, and innovation covariance must come from the actual MarlinTrk update
path.

## ACTS comparison

The current ACTS `main` GSF was reviewed at commit
`d33613d3f95a26779deba20d766ac748d965b3a4` (2026-07-10).  The review covered
`GaussianSumFitter`, `GsfActor`, `GsfUtils`, `GsfOptions`,
`BetheHeitlerApprox`, mixture reduction/merging, the multi-component stepper,
and unit tests.

The most important finding is that `RecGsfTracking` updates weights with only

```text
w <- w * exp(-deltaChi2/2)
```

whereas ACTS uses the full Gaussian innovation likelihood

```text
w <- w * det(S)^(-1/2) * exp(-deltaChi2/2)
S = H * Ppred * H^T + R
```

before normalization.  Omitting `det(S)^(-1/2)` prevents a statistically
correct comparison of broad and narrow BH branches.  This invalidates strong
conclusions drawn from the delayed-lifetime weights until the full likelihood
is available.

Other weaknesses established by the comparison:

- The current splitter rewrites curvature and covariance in the preceding
  stored site instead of creating a distinct pre-material to post-material
  transition.
- Nominal layer `t/X0` is not corrected for the component incidence path or
  evaluated at the component local position.
- `Current` uses a CEPC thin-material toy mixture for the tracker-step regime;
  `GlobalSim2GeV85` applies a whole-sample distribution repeatedly and ignores
  step `t/X0`.  Neither is a validated CEPC transition model.
- `SmoothAll` is not a reverse multi-component filtering pass and cannot undo
  a material transition that was never represented explicitly.
- The local KL reducer moment-merges all historical sites, potentially
  combining states with different pivots and creating a synthetic history
  without recomputed Jacobians or likelihoods.  Reduction should operate on
  current-surface components.
- A fresh one-reference-hit MarlinTrk object is created for every component at
  every hit; the authoritative internal prediction and innovation are not
  exposed to the GSF.
- Hits are ordered only by transverse radius, which is not a general track-path
  or material-crossing order.
- TopN target 1 is not ACTS-like.  A more representative baseline retains
  roughly 4-5 components, uses KL reduction and a low-weight cutoff, and uses
  the full posterior likelihood.
- Weighted-mean output currently publishes chi-square/NDF from the best branch,
  so the state and fit-quality metadata can describe different objects.
- Focused automated tests are missing for posterior weights, path correction,
  direction-reversed material transitions, and surface-local merging.

ACTS is a design reference, not a CEPC physics validation.  Its default ATLAS
BH approximation must not be assumed valid for CEPC, and ACTS itself does not
claim a dedicated full component smoother.

## Resulting work order

The ACTS review superseded the earlier decision to treat component lifetime as
the first unresolved mechanism.  The required order became:

1. obtain the exact MarlinTrk prediction, residual, measurement projector, and
   innovation covariance on the current surface;
2. apply and validate the full innovation likelihood, including the determinant
   term;
3. establish explicit pre-material/post-material state semantics and
   component-dependent path-corrected `t/X0`;
4. fit and validate a Geant4 step-conditioned CEPC BH model;
5. use 4-5 components with current-surface KL reduction and a low-weight cutoff,
   then decide whether an age policy is still necessary;
6. implement and validate reverse multi-component propagation to the IP;
7. only then run broad GSF-versus-LCIO studies.

Every implementation step must be checked with comprehensive verbose component
dumps on at least one focused event, followed by events 11, 16, and 17 when the
step is stable.  Build success, finite output, or lower chi-square alone is not
validation.
