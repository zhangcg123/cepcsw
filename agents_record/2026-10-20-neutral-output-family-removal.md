# Neutral output-family removal

The user requested removal of `neutral_free_loss_*`, not removal of the optional
neutral-energy KF measurement. The measurement now steers the existing free-loss
result, like the charged switch. Both switches remain default OFF. The separate
neutral track collection, status/index collections, output property and flat
endpoint/optimizer family are removed. The existing free-loss tuple retains
neutral measurement diagnostics and `free_loss_neutral_ecal_measurement_status`:
0 off, 1 unavailable neutral, 2 applied, 3 no breakpoint, -1 failed neutral fit.
On failure, retry the ordinary free-loss fit with charged measurement if selected;
its existing KF fallback remains intact. No extra fit runs on the unavailable path.
The shared measurement, propagation, smoothing and likelihood algebra is unchanged.

## Validation and deployment

- Rebuilt RecBreakpoint; all seven package numerical CTests pass.
- Seed 1 / input track 0 / events 0,5,11,12,16,17,25,31 from
  `sim_large_barrel_20261018/rec-barrel-1.root` were rerun locally for all four
  charged/neutral OFF/ON combinations. No Condor jobs were submitted.
- Neutral OFF: all 476 pre-existing fields match the pre-neutral baseline
  exactly (NaN-aware), separately for charged OFF and ON: 3,808 comparisons each.
- Neutral ON: existing free-loss endpoint, covariance, per-hit score and NLL
  fields reproduce the former separate neutral-family values exactly. Other
  endpoint families are unchanged: 3,246 checked row/field values per charged
  configuration. Status sequence is 1,2,1,3,1,1,-1,2. In particular event 25
  retries successfully without neutral; events 5/31 apply it.
- Output ROOT schemas contain no `neutral_free_loss_*` fields. The maintained
  card and installed Gaudi configuration have no `OutputTracksNeutralFreeLossRTS`.
- Batch tests: 38/39 pass. The existing full-chain-default test fails because
  the user's submission wrapper selects breakpoint-only; that unrelated script
  and test expectation were not changed.
- Package-only installation uses the normal project InstallArea. Build and
  installed plugin SHA256 both equal
  `72164a985e9fb7fc8df77601991bc4d9735d4450a27e84c68f01595d7e6e2df5`.
- Event 5 was run through installed `gaudirun.py` with verbose full six-state
  and covariance dumps. Local outputs/logs/checker use the prefix
  `/tmp/bp-single-family-` and suffix `20261020`.

Existing ROOT files were not altered. Old prepared cards declaring the removed
output property must be regenerated. No population/endcap study or enabled
backward-filter regression was performed; defaults keep all backward filters OFF.

## Outgoing current focus (verbatim)

Active work on local `breakpoint` is the optional neutral-energy KF measurement
in free-loss Persistent6D RTS. `FreeLossNeutralEcalMeasurement` defaults OFF
and publishes `BreakpointTracksNeutralFreeLossRTS` / `neutral_free_loss_*`
alongside the unchanged selected base free-loss result. The measured scalar
energy updates the joint track/loss state at a virtual post-loss site, before
further material propagation; RTS includes that site. It may combine with the
default-OFF charged-ECAL terminal measurement. No usable neutral, no selected
breakpoint or failed optimization copies the selected base result. All backward
filters remain default OFF; no neutral backward workflow was added. The old
neutral-penalty-only and absolute-prior refitters remain retired.

Seven numerical tests pass. On eight fixed seed-1 tracks, all 476 old flat
branches reproduce exactly with neutral OFF and ON, for charged OFF and ON.
Two tracks apply the new update; five copy for absent neutral/interval and one
falls back after rejecting invalid zero-curvature-crossing trials. This is a
mechanical gate, not physics validation. The saved affine and nonlinear photon
energies can disagree significantly even in converged fits. Next: audit that
linearization limit and cluster/interval ownership, then use held-out categorized
barrel/endcap comparisons before any default-on decision. Details and preserved
outgoing focus: `agents_record/2026-10-20-neutral-energy-kf-measurement.md`.
