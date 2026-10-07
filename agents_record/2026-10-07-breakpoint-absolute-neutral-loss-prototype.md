# Absolute neutral-energy-loss RTS prototype, 2026-10-07

## Outgoing live focus preserved

Before this prototype, `AGENTS.md` focused on diagnosing RecBreakpoint's
early-loss ambiguity with ECAL. The 72-file rerun had track-matched charged
PFO/ECAL, event-level neutral PFO/ECAL, and primary eBrem-photon emission,
ECAL-entry and last-step truth. In its 11,820-track comparison, adding all
neutral clusters narrowed the hard-eBrem energy residual but broadened the
no-eBrem and light-loss cores. The next action was selective neutral-cluster
association by reconstructed geometry, using photon-path truth only to
validate it. Event-level photons were not matched to CyberPFO neutral
clusters, and ECAL observables were passive. Diffuse RTS had underperformed
beam-guided free loss, especially for light loss and VXD intervals. Complete
evidence remains in `2026-10-03-breakpoint-ecal-workflow.md`,
`2026-10-03-breakpoint-photonlast-ecal-performance.md`, and
`TrackingPerformanceStudies/breakpoint_photonlast_20261003/README.md`.

## New implementation and mechanical evidence

`Reconstruction/RecBreakpoint` now has a separate, default-off
`AbsoluteNeutralLossRTS` endpoint. The ordinary log-loss RTS/backward,
free-loss, diffuse, and truth outputs are unchanged. The new endpoint is
`BreakpointTracksAbsoluteNeutralRTS`, with row-aligned status/index and
`absolute_neutral_*` flat branches. Status 2 means a fit ran, 1 an ordinary
RTS copy because no single interval/cluster was available, 0 disabled,
-1 a failed fit with ordinary copy, and -2 an ordinary fit failure.

The candidate uses reconstructed neutral CyberPFO ECAL clusters with actual
hits. Their angular centers must be within 10 mrad in theta and 200 mrad in
phi of the input track's ECAL direction. Each cluster is counted once; photon
truth never enters selection. Its energy error is provisionally
`0.011 sqrt(E/GeV) GeV + 0.004 E`, with independent variances summed. The
selected total energy and error initialize the sixth KF coordinate
`L = p_before - p_after` in GeV. At the chosen breakpoint, the internal
curvature mapping is `kappa_after = kappa_before * p_before/(p_before-L)`,
where `p_before = sqrt(1+tanLambda^2)/abs(kappa_before)` because internal
`kappa` is inverse pT. There is no second ECAL likelihood update. The same
native hit update and 6D RTS pass as the persistent log-loss fitter are used.

Mechanical checks: `RecBreakpoint` built; the birth Jacobian agreed with
central finite differences to <3e-10 relative on a representative state;
12 selected events ran locally with verbose dumps on events 1:5 and 1:25.
The 12-event result is **not** population validation:

| Seed:event | Truth pT | Ordinary RTS | Absolute neutral RTS | Status |
|---|---:|---:|---:|---:|
| 1:5 | 13.835 | 11.138 | 11.148 | 2 |
| 1:25 | 31.830 | 18.349 | 23.829 | 2 |
| 1:31 | 15.035 | 13.809 | 15.054 | 2 |
| 1:64 | 9.845 | 8.149 | 10.101 | 2 |
| 2:1 | 32.526 | 23.868 | 28.751 | 2 |
| 2:3 | 16.993 | 14.328 | 16.148 | 2 |
| 2:10 | 16.590 | 12.636 | 12.636 | -1 |
| 2:11 | 19.598 | 12.334 | 12.640 | 2 |
| 3:1 | 29.683 | 24.405 | 25.150 | 2 |
| 3:52 | 45.195 | 36.608 | 59.092 | 2 |
| 4:46 | 14.293 | 13.115 | 13.120 | 2 |
| 4:92 | 10.504 | 9.630 | 9.634 | 2 |

Median absolute pT residual over these selected events is 18.39% ordinary
versus 13.44% absolute. By contrast, the older same-event *log-loss ECAL
prior* diagnostic had 1.64% median and is recorded at
`TrackingPerformanceStudies/breakpoint_ecal_refit_repro_20261007/README.md`.
The absolute mode is therefore not ready to replace it. In 2:10, the
single-pass pre-break momentum was below the selected ECAL loss, so the new
mode copied ordinary RTS. The severe 3:52 overshoot and weak 1:5 response
are consistent with a poor linearization of the nonlinear absolute-loss
birth map around the early forward state; this is a hypothesis, not yet a
validated cause. A one-time adjustment of only the loose prefit seed mean
had negligible effect and was removed. Next: inspect the fitted/predicted
state and curvature at the breakpoint on these events, then design a true
relinearized 6D fit rather than tuning a fallback threshold.

Local outputs are `/tmp/absolute_neutral_final_{1,2,3,4}.root` and matching
logs. Python batch-helper unit tests: 32/33 passed; the lone pre-existing
failure is `test_submission_arguments_without_real_scheduler`, expecting
`-g cms` while the locally modified submission helper uses `-g higgs`.
The package-specific CMake install succeeded and a no-library-override run
of event 1:5 reproduced status 2 and pT 11.1475167799 GeV. A full-tree
`cmake --install` first stopped at an unrelated missing
`build.../CEPCSW.components` in `Analysis/TotalInvMass`; the package-local
install script was used to install only RecBreakpoint afterward.
