# Native KalTest machinery for charged and neutral ECAL measurements

## Scope and the three review points

User requested using the same KF machinery as tracker hits for both optional
energy measurements. Changes stay inside RecBreakpoint; shared KalTest,
MarlinTrk, TrackSystemSvc, GSF and batch scripts are unchanged.

The prior review identified:

1. Charged ECAL used finite differences through float EDM states, at different
   propagated pivots, combined with a separately returned native covariance.
   Inferring Q by subtraction produced invalid process penalties (481/1466
   trials in the prior file-2 combined run, affecting 16/21 rows). The precise
   contributions of pivot conventions and finite-difference roundoff were not
   separately measured.
2. The neutral energy linearization can fail badly despite a converged fit:
   file 2 event 166 measured 3.273 +/- 0.033 GeV but the old neutral-only final
   state implied 14.553 GeV, while its affine prediction was 3.270 GeV.
3. Charged energy lacked the curvature-sign-crossing rejection already present
   for neutral energy.

This change replaces the machinery behind point 1 and shares the protection in
point 3. Point 2 remains OPEN. No iterated measurement, new prior, extra cluster
position measurement, or changed optimizer objective was introduced.

## Implementation

- `KalmanAdapter::predictCalorimeter` uses the same native source-site setup
  and `TKalDetCradle::Transport` as tracker propagation. It selects a forward
  barrel/endcap ECAL face with native layer intersections and returns double
  state, F and Q at one pivot. The 5D native transport is lifted to 6D with
  unchanged b and full propagated cross covariances. No finite differences,
  EDM round-trip or Q inferred by subtraction remains on this constrained edge.
- `EnergyMeasurementSite` specializes `TKalTrackSite` only for a scalar affine
  energy observation, using native `TVKalSite::Filter()` for the full 6D state.
  Both charged and neutral observations use it, including a common curvature
  sign check. Their physical energy formulas, input errors and locations remain
  unchanged: charged at the ECAL face, neutral at the virtual post-loss site.
- The existing RTS implementation traverses the charged native F/Q edge and
  the neutral virtual site. Existing outputs, defaults OFF, no-neutral/no-loss
  behavior and failed-neutral retry are unchanged. All backward defaults OFF.
- Verbose dumps include charged source/predicted/filtered states, covariances
  and F/Q. `tests/check_ecal_transport_dump.py` checks covariance closure, noise
  positive semidefiniteness, unchanged propagated b and charge-sign protection.
- Unit tests compare native updates with an independent Gaussian information
  calculation for both energy observations. No handwritten production gain or
  measurement covariance update remains in either energy measurement.

## Validation (local jobs, no Condor)

Seven package CTests passed. Built with the normal LCG105 build tree. CMake/make
reported filesystem clock skew; the changed compilation/link commands completed
and the runtime verbose diagnostics below confirm the new module was exercised.

OFF regression: `rec-barrel-1.root` events 0,5,11,12,16,17,25,31 with all existing
auxiliary outputs enabled. All 515 branches in all 8 rows matched the prior
`/tmp/bp-single-family-off-c0-20261020.root` exactly, including NaN-aware values:
4,120 row/field comparisons. Output `/tmp/bp-native-off-all-20261023.root`.
An earlier OFF check disabled beam/diffuse/truth outputs and therefore differed
in those intentional copies; its tracker/free-loss fields were also exact.

Combined ON focused runs:

- `rec-barrel-1.root`: 0,5,11,16,17, output
  `/tmp/bp-native-both-f1-20261023.root`. The same numbered events in this file
  are not all hard-loss fits: 0,11,17 have no selected loss and copy ordinary;
  16 has no collected neutral and runs charged only; 5 applies both.
- `rec-barrel-2.root`: 1,148,166, output
  `/tmp/bp-native-both-final-f2-20261023.root`. All three apply both and converge.
  Five applied ECAL edges across these two files pass normalized covariance
  closure (<1.6e-16) and positive-semidefinite noise checks. No invalid
  tracker-to-ECAL process penalty occurred in these runs.

Residuals are `100*(pT_reco/pT_truth - 1)`; old outputs are the documented
same-event pre-change combined runs, not a population comparison:

| File:event | Previous both | Native both |
|---|---:|---:|
| 2:1 | -23.75% | -1.7372% |
| 2:148 | -31.76% | -2.1983% |
| 2:166 | -21.24% | +82.2348% |

Event 166 remains a counterexample to physics validation: removing the broken
transport permits the neutral-linearization pathological solution. Its final
photon prediction is about 14.55 GeV for the measured 3.27 GeV. Do not describe
the machinery change as a universal resolution improvement. The complete RTS
quadratic and stored likelihood quadratic agree within 4e-8 on these three fits.

Final separate charged-only and neutral-only reruns of file 2 events 1,148,166
all converged (`/tmp/bp-native-charged-f2-20261023.root` and
`/tmp/bp-native-neutral-f2-20261023.root`). Their residuals are respectively
(-3.7748%, -2.5130%, -23.7278%) and (-1.6423%, -2.1945%, +82.2725%).
The full eight applied charged edges across charged-only and combined verbose
logs pass covariance closure (<2.1e-16) and noise positive-semidefiniteness.
Charged-only produced no invalid trial in these three events; the neutral runs
reject curvature crossings as intended. These checks exercise all four switch
combinations without changing their default values.

## Deployment and remaining checks

Built and tested via `build.105.0.0.x86_64-el9-gcc11-opt/run`; NOT installed.
Installed plugin remains SHA256
`72164a985e9fb7fc8df77601991bc4d9735d4450a27e84c68f01595d7e6e2df5`.
Tested build plugin SHA256
`f939bea2c5f0134fe79e7f8613871a120bb95f506e0b5dff99b253b83cd858f1`.

Do not turn either measurement on by default. Next discussion is review point
2 (neutral affine versus nonlinear energy); endcap, calibration/independence,
cluster ownership and held-out categorized performance still need validation.

## Preserved outgoing AGENTS focus

Active work on local `breakpoint` is the optional neutral-energy KF measurement
in the existing free-loss Persistent6D result. `FreeLossNeutralEcalMeasurement`
defaults OFF and steers `BreakpointTracksFreeLossRTS` / `free_loss_rts_*`.
The separate neutral output family has been removed. The measured scalar
energy updates the joint track/loss state at a virtual post-loss site, before
further material propagation; RTS includes that site. It may combine with the
default-OFF charged-ECAL terminal measurement. No usable neutral, no selected
breakpoint skips the neutral measurement; failed optimization retries without
it, retaining the charged constraint if selected. All backward filters remain
default OFF. The old
neutral-penalty-only and absolute-prior refitters remain retired.

The measurement/RTS algebra is unchanged by the output consolidation. This is
not physics validation. The saved affine and nonlinear photon
energies can disagree significantly even in converged fits. Next: audit that
linearization limit and cluster/interval ownership, then use held-out categorized
barrel/endcap comparisons before any default-on decision. Details and preserved
outgoing focus: `agents_record/2026-10-20-neutral-output-family-removal.md`.
