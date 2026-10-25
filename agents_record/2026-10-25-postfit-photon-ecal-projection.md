# Passive post-fit photon ray and ECAL uncertainty

User requested implementing a photon ECAL position with uncertainty after the
augmented KF/free-loss fit, without introducing additional KF parameters,
then drawing a few examples together with the internal timing-cluster hits.

## Implementation

Changes are confined to `Reconstruction/RecBreakpoint` and its tests/build
registration. `PhotonCaloProjection` constructs a native `TStraightTrack`
from `fit.smoothed[breakpoint.index]`, the fitted upstream site where the
outward loss mapping is applied. Photon origin/direction are reconstructed,
not taken from Geant4. The configured breakpoint selector remains unchanged
(truth layer ownership in the tested cards).

`KalmanAdapter::calorimeterSurfaces()` exposes existing native ECAL entrance
faces from the tracking geometry. The ray uses the generic `TVSurface`
intersection implementation, bypassing charged-helix specializations, and
selects the nearest valid positive intersection. No cylindrical-radius or
cluster-position substitute is used. These are the native tracker-defined
ECAL entrance faces, not a separate audit of actual first ECAL crystal entry.

The surface-aware Jacobian differentiates both the ray and intersection
distance. Full smoothed five-dimensional covariance, including correlations,
is propagated to ECAL xyz and z/phi. For this collinear model the direct
derivatives with respect to curvature and b are zero: the full 5D marginal
contains all relevant augmented-posterior location information. No diagonal
approximation, loss-coordinate removal from the fitter, new parameter,
measurement, process noise or optimization change is introduced.

Default-on tuple-only families are `rts_photon_calo_*` and
`free_loss_rts_photon_calo_*`, 15 branches each. They save status/error,
breakpoint hit, runtime surface index, origin/direction, ECAL xyz and 3x3
covariance, full 2x2 z/phi covariance (mm/rad units), upstream source state,
covariance/pivot and 3x5 derivative. Fitted b and the signed energy diagnostic
`p_before*(1-exp(-b))` are also saved. Status 1 means valid geometry, not an
identified positive-energy photon; 0 is unavailable/no fitted breakpoint
state (including KF fallback), -1 is a projection failure. Failures never
invalidate the existing electron endpoint.

No backward, diffuse, truth or cumulative-constraint family was extended.
This does not implement constraints/free-loss fitting in diffuse tracking.
No new property or run-card steering was required. Neither the earlier PFA
export nor this breakpoint update is installed in the default runtime.

## Validation

- New `RecBreakpointPhotonProjection` CTest checks native cylinder/endcap
  intersections, source non-mutation, the surface-aware Jacobian against
  finite differences, full correlated covariance versus 6,000 sampled rays,
  and missing-geometry failure. z/phi cross-correlation is checked explicitly.
- All nine breakpoint numerical CTests pass.
- Same settings, installed runtime versus new build: events 0,1,11,16,17,
  38,41,48,54,60,78 from seed 1, all constraints 000 and all backward filters
  OFF. All **817 pre-existing flat branches match exactly on all 11 rows**,
  including NaNs and vector contents; only 30 passive branches are added.
- Twenty displayed events rerun with the same 000 settings: all eight checked
  baseline/ordinary/free/truth/diffuse endpoint and free NLL fields match the
  original batch rows exactly. The original batch ran 111 additionally, but
  its retained unconstrained 000 endpoints are the comparison here.
- Detailed verbose per-hit state/covariance dumps are retained in the local
  logs. No simulation or batch submission was performed.

Build-only module SHA256:
`abb51393afc0ff19189fd4632eb4d64d2bd066f25ff2b9aa7fa1ffce28e55849`.
Unchanged installed module SHA256:
`6704f6c2b441b4556db1f110a35ee7f4baf1921338f25b80f2cef6066db62af7`.
Local reference/new/example tuple and log paths use
`/tmp/bp-photon-{installed,new,examples}-20261025.{root,log}`.
Reproduction cards: `/tmp/bp_photon_projection_20261025.py` and
`/tmp/bp_photon_examples_20261025.py`.

## Updated displays

Overwrote the same 22 PNGs in
`TrackingPerformanceStudies/breakpoint_ecal_timing_hits_20261025/`:
black triangle = free-loss 000 electron at ECAL; magenta star = its collinear
photon-ray hypothesis; red cross = timing-cluster centre. Both location
covariances are displayed at true scale as 68.27% joint 2D Gaussian ellipses
(chi-square boundary about 2.296). Photon marginal sigmas and signed energy
estimate are printed. Binning/color normalization/hit contents are unchanged;
both z and phi window spans are doubled about the original hit window.

Eight of the ten hard no-neutral examples have a fitted breakpoint and valid
photon projection: entries **3,38,41,48,54,60,69,78**. The remaining hard
examples **26,57** have no eligible selected breakpoint; the ten no-eBrem
controls also have no photon point. No placeholder photon was invented.

For the eight hypotheses, sigma_z is 0.059--0.150 mm and sigma_phi is
0.0049--0.0548 mrad. Such ellipses are much smaller than a 5 mm x 2 mrad bin.
Entry 38's fitted photon energy is only about 2.7 keV despite its >1% truth
loss: a valid ray is a conditional geometric hypothesis, not evidence that
the free-loss fit recovered bremsstrahlung. Labels explicitly distinguish this.

## Limits / next checks

The uncertainty is conditional first-order tracking covariance. It does not
include uncertainty in emission position within the owning layer, finite
photon opening angle, conversion/scattering, detector/alignment uncertainty,
or Minuit prior-centre uncertainty beyond the existing conditional fit.
Physical truth coverage, transitions between ECAL faces and barrel/endcap
population closure have not been established. The electron and photon
predictions share tracking information; their two marginal covariances must
not be added as if independent when deriving a separation score.
