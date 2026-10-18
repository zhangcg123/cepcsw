# Explicit charged-ECAL face update and terminal RTS edge (2026-10-19)

## Outgoing focus, preserved verbatim

Active work on local `breakpoint` is the charged-ECAL energy measurement in
the ordinary free-loss Persistent6D fit. `FreeLossChargedEcalMeasurement` is
installed and default-off. It updates the outer tracker 6D state with one
energy-only scalar KF observation, then RTS-smooths inward; no ECAL cluster
position is fitted. OFF exactly reproduced 593 existing flat-tuple branches
for the focused same-input regression. ON converged on three linked-cluster
tracks; a no-cluster track reproduced the tracker-only fit. These are
mechanical gates, not physics validation. The provisional charged-cluster
resolution and possible CyberPFA track-energy reuse are the immediate risks.
The neutral-ECAL free-loss branch remains a separate default-on study option;
all backward filters remain default-off.

Next: audit charged-cluster independence/calibration, then compare held-out
clean/light/hard-loss performance and association failures. Do not add ECAL
position constraints or promote this charged measurement to default-on from
the focused smoke tests. Exact behavior, outgoing focus and tests are in
`agents_record/2026-10-19-charged-ecal-free-loss-kf.md`.

## Reason for the change

The first version propagated a copy of the outer tracker state to ECAL for
energy prediction, but applied the scalar update to the last tracker-hit
state. That is only equivalent to an explicit ECAL-node update if its
transport Jacobian, process covariance, and cross covariance are exact. Its
scalar propagation-noise remainder was approximate; the earlier record's
phrase "update there" was imprecise.

## Current implementation

Only the optional `FreeLossChargedEcalMeasurement` branch changed. It remains
default-off and acts only in the tracker-only free-loss Persistent6D fit.
`KalmanAdapter::propagateToCalorimeter` supplies the native ECAL-face mean and
five-parameter covariance. The fitter numerically differentiates all five
input track parameters to form the last-tracker-to-ECAL transport; the same
breakpoint loss coordinate persists unchanged. Native ECAL covariance plus
the transported loss cross covariance form a six-dimensional predicted ECAL
state. A one-dimensional energy measurement updates that state with its
cluster error, without a position measurement. The terminal RTS edge uses
the last-tracker/ECAL cross covariance to smooth the last tracker state; the
existing tracker RTS then continues inward. The published
`free_loss_rts_calo_*` state is the updated ECAL-face state.

The native propagation API does not expose a last-tracker-to-ECAL transition
cross covariance. Therefore the transport is numerically differentiated and
the process covariance is inferred as propagated covariance minus transported
input covariance. This is a model approximation, not a claim of exact native
material-process equivalence. The charged-cluster energy itself is not yet a
calibrated independent observation of the post-brem electron: CyberPFA may
reuse track momentum, and unresolved photons may enter the charged shower.

The ECAL transition process penalty and postfit energy residual are included
in the final smoothed quadratic. The Minuit trial objective remains the
tracker likelihood plus the conditional ECAL-energy likelihood. All
non-ECAL endpoints and the charged-ECAL OFF path remain separate.

## Mechanical checks

- `RecBreakpoint` built under LCG 105; six package CTests passed.
- Same-input event 3 with the charged switch OFF reproduced all 609 existing
  flat-tuple branches exactly, including NaN fields.
- The first five linked-cluster ON events (seed 1: 3, 38, 41, 48, 54) completed
  with verbose diagnostics and finite ECAL-face publication. The output
  ECAL momentum agreed with the first-order energy-update prediction at the
  0.0004 GeV level or better in these five.
- The fixed 100-event barrel hard-loss/no-collected-hit-supported-neutral
  cohort produced 100 rows. The explicit-face ON fit converged in 98; seed 1
  event 112 and seed 5 event 58 fell back to input KF. Every converged fit
  published an updated ECAL-face state (`free_loss_rts_calo_status=1`). The
  prior last-tracker-update implementation converged in 70/100. These are
  same-selection runs, not yet same-final-code OFF/ON performance validation.
- Relative to the earlier tracker-only OFF output, median absolute IP pT error
  was 0.370% OFF versus 0.770% explicit-face ON; 28 tracks improved and 72
  worsened. RMS decreased from 4.32% to 2.46% because of outliers, so a single
  width measure is misleading. The charged measurement is not validated for
  physics use.
- The tested sample is barrel only. No endcap ECAL-face gate or independent
  charged-cluster truth association was available in this check.
- The final rebuilt plugin reproduced the event-3 ON endpoint and diagnostics
  exactly against the 100-event build. The build and project-wide installed
  `libRecBreakpoint.so` hashes matched after `cmake --install`.
