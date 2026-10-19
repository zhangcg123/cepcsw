# Neutral-energy KF measurement in free-loss Persistent6D RTS

## Request and implementation

User requested an optional neutral-cluster **Kalman measurement**, analogous
to the charged-ECAL option, with no-neutral fallback to the selected ordinary
free-loss or charged-constrained free-loss result. No batch jobs submitted.
All fitter changes stay in RecBreakpoint; shared KF and GSF are unchanged.

`FreeLossNeutralEcalMeasurement` (compiled/card default false) runs a parallel
RTS result. `BP_FREE_LOSS_NEUTRAL_ECAL_MEASUREMENT` is frozen by the batch helper.
It requires FreeLossFit and can combine with FreeLossChargedEcalMeasurement.
All four existing backward filters retain default OFF; no neutral backward
workflow was added. The optional neutral RTS collection is
`BreakpointTracksNeutralFreeLossRTS`; flat prefix is `neutral_free_loss_`.

The existing neutral collector supplies only unique, hit-supported neutral
ECAL clusters within theta +/-10 mrad and phi +/-200 mrad around the input
track ECAL reference. Its default per-cluster uncertainty is
`0.011*sqrt(E) + 0.004*E` GeV, with independent variances summed. Selection
does not use photon truth; the breakpoint location still follows the existing
shared interval selection (default truth-owned layer).

Each free-loss trial uses the same BreakpointFitter. The existing loss mapping
is factored into `applyBreakpointLoss`, then a virtual six-dimensional
measurement site is inserted at its post-loss position, before further
material/geometric propagation. For live kappa=charge/pT, tanLambda and b:

```text
h = sqrt(1+tanLambda^2)/abs(kappa) * (exp(b)-1)
H_kappa = -h/kappa
H_tanLambda = h*tanLambda/(1+tanLambda^2)
H_b = p_after*exp(b)
S = H*P*H^T + sigma_E^2
K = P*H^T/S
x_updated = x_predicted + K*(E_measured-h)
P_updated = (I-KH)*P*(I-KH)^T + sigma_E^2*K*K^T
```

Track/loss correlations are retained. Downstream native measurements and the
optional charged-ECAL terminal update follow. RTS crosses the virtual site
on its way inward. The site is also one scalar observation in the shared
GaussianTrackModel, so the normalized likelihood includes it **once**, not
as a second external penalty. The finite SigmaLogLoss remains unchanged;
Minuit still optimizes its center. No diffuse result is reused.

The neutral measured E/error never change. Predicted, filtered and smoothed
photon-energy estimates/errors, their six-dimensional states/covariances,
innovation chi2 and smoothed affine-measurement chi2 are recorded. The complete
score groups the virtual-site measurement and birth penalty with tracker hit
i+1. Native tracker residuals remain separately available.

Status `neutral_free_loss_measurement_status` / `BreakpointNeutralFreeLossStatus`:
0 disabled/not reached; 1 no usable neutral; 2 applied; 3 no selected interval;
-1 optimization failure. Non-applied outputs copy the selected base RTS,
including its charged update or KF fallback. Input rows excluded or ordinary
fits failing before publication retain absent output index -1.

## Tests and evidence

Reference inputs: `sim_large_barrel_20261018/rec-barrel-1.root`;
zero-based event indices `[0,5,11,12,16,17,25,31]`, input track 0 in each.
Pre-edit installation reference outputs:
`/tmp/bp-neutral-base-c0-20261020.root` and `...-c1-...root`, with charged OFF/ON.
Neutral-OFF rebuilt outputs `...-off-c0-...root` / `...-off-c1-...root` reproduced
**all 476 pre-existing branches exactly**, including vectors and NaNs, for
eight tracks in both configurations. Neutral copied outputs were also checked
against base IP parameters/covariance, ECAL state, per-hit scores/states and NLL.
Verbose event 5 output is `/tmp/bp-neutral-smoke2-20261020.{root,log}`.

All seven package CTests pass, including the new NeutralEnergy test:
finite-difference energy Jacobian; analytic mapping; full six-dimensional
Kalman update versus batch precision solution; downstream refinement;
five-to-six-dimensional Gaussian model; innovation and complete score agreement;
weak-measurement limit; invalid uncertainty and curvature-pole rejection.
38 batch/card tests pass. One pre-existing wrapper-default test fails because
the user-edited subbreakpointjobs.sh defaults to breakpoint-only while the
test expects the full chain; that script and expectation were not changed.

The first physical trial audit identified an invalid zero-curvature crossing:
event 25 could acquire a very good affine neutral score with a nonsensical
nonlinear photon energy. New neutral updates and smoothed neutral states
that cross zero curvature relative to the measurement prediction now reject
that trial. This rejects use of the 1/abs(kappa) Jacobian across its pole;
it is not an arbitrary residual cut or prior-width change.

Final ON outputs: `/tmp/bp-neutral-final-c0-20261020.root` and
`/tmp/bp-neutral-final-c1-20261020.root`. Both contain all eight rows with
unchanged 476 pre-existing branches. Events 0,11,16,17 copy the base because
there is no selected neutral; event 12 copies it because no eligible breakpoint
exists. Events 5 and 31 apply the neutral update. Event 25 rejects the invalid
trial domain and falls back to the selected base. A forced optimizer-failure
test (`BP_FREE_LOSS_MAX_CALLS=1`, event5) also copies the base KF fallback
exactly: `/tmp/bp-neutral-failure-20261020.root`.

| Event | Truth pT | Base free pT | Neutral KF pT | Charged free pT | Charged+neutral pT |
|---|---:|---:|---:|---:|---:|
| 5 | 13.83535957 | 21.94991861 | 14.82766739 | 22.07193202 | 14.99362335 |
| 25 | 31.82990265 | 49.85420380 | 49.85420380 (copy) | 19.28517782 | 19.28517782 (copy) |
| 31 | 15.03544617 | 15.53135948 | 15.05414479 | 14.49271534 | 15.04959106 |

Numbers are GeV; this selected mechanical sample cannot establish population
improvement. Same-final-code base and new branches are produced together.

Installed with the package-only `cmake_install.cmake` into project-default
`InstallArea/x86_64-el9-gcc11-opt`. Built and installed plugin SHA-256 agree:
`ebfb5b7f7bf63bc1910c0818fd2112a2134a8648a929b607a1b76fe344345d4d`.
Installed Configurables expose both ECAL switches with default false. A direct
`source setup.sh; gaudirun.py ...` rerun of events 5 and 31 (without build/run)
produced `/tmp/bp-neutral-installed-20261020.root`, reproducing **all 628
branches exactly** against the final build outputs. No Condor jobs were launched.

## Physics limits

Even without crossing the curvature pole, the affine and nonlinear final
photon energies can differ substantially: event 5 demonstrates this. Both
are intentionally retained. This first implementation does not relinearize
the neutral site after smoothing. Successful convergence is not validation.
All collected neutral energy is assigned to one selected breakpoint. Wrong
ownership, extra emissions and cluster contamination remain risks. Charged
and neutral measurement errors are treated independently despite possible
PFA/calibration correlations. No endcap or held-out population claim is made.

## Preserved outgoing AGENTS focus

Active work on local `breakpoint` is simplifying its fit surface. The two
neutral-cluster-constrained refitters (`FreeLossEcal` and the absolute-loss
RTS) have been retired. Track-wise neutral-cluster fields and the event-wise
`neutral_pfos` tree remain for diagnostics only; they do not steer a fit.
The optional charged-ECAL terminal Kalman measurement remains default-off,
and all backward filters remain default-off. The ordinary, free-loss,
beam-guided, diffuse, and truth-prior approaches remain available.

The installed plugin passes a one-event fitted-breakpoint smoke: ordinary,
free-loss, diffuse and truth-assisted results remain active; new flat tuples
omit retired neutral endpoints but retain neutral diagnostics. Next: examine
charged-cluster independence and held-out clean/light/hard-loss performance
before considering any ECAL default-on steering. The removal, outgoing
focus, and checks are in
`agents_record/2026-10-20-retired-neutral-breakpoint-constraints.md`.
