# Fixed-b reference-trajectory iterations: implementation and focused gate

## Request and implementation

On 2026-09-12 the user authorized inner reference-trajectory iterations.
Only RecBreakpoint and its dedicated card/helper/docs were changed. Existing
GSF/shared KF sources and user-owned GSF cards/submission script are unchanged.
The user subsequently confirmed no batch jobs before the shared build/install.
No remote operation is part of this change.

FreeLossReferenceIterations=0 preserves the one-pass free fit. Positive values
limit inner passes at each fixed b. Defaults: FreeLossReferenceTolerance=0.001
(max state change in posterior-sigma units and covariance change normalized by
posterior standard deviations), FreeLossReferenceObjectiveTolerance=0.0001
(absolute -2logL change). All3 controls are explicit in the maintained card and
freeze through the batch helper. Tests use20 passes; the card default remains0.

Each pass keeps the ORIGINAL seed mean/covariance, hits, V and b unchanged.
The preceding RTS trajectory supplies reference means/covariances. Native
propagation rebuilds f/F/Q; ReferenceMeasurementSite supplies affine h/H to
native TKalTrackSite::Filter. No separate Kalman gain/update is implemented.
fitLocalRTS shares prediction bookkeeping, affine offsets, covariance transport,
RTS recursion and scoring between paths. No loss-prior-center iteration is
restored. Truth loss amounts do not enter free minimization.

Both state/covariance and objective convergence are required. Invalid or
nonconverged trials cannot be published. The pre-existing Minuit acceptance
policy (which need not beat every saved scan point) is unchanged, preserving
the exact off-path comparison. The selected b is repeated without the scalar
cache and the same converged procedure produces the free RTS output. Backward
starts from that final forward endpoint with scale100 and uses its existing
native recursion; it is not part of reference updates or the likelihood.
Ordinary/truth-prior fits remain one-pass, with the same IP operation and
conditional-on-b covariance conventions.

There are11 new flat fields:3 controls,3 aligned trial-convergence vectors,
2 final reference state/covariance vectors, and3 final convergence summaries.
Invalid trials retain the error string, including nonconvergence details.
No previous tuple/card is rewritten. Mathematical reference:
Reconstruction/RecBreakpoint/docs/reference-trajectory.md.

## Mechanical gates and provenance

Artifacts: TrackingPerformanceStudies/breakpoint_reference_20260912/

- Private build from maintained sources passed, initially leaving installed
  libraries untouched.
- All24 batch tests pass, covering all32 explicit algorithm properties and
  freezing all3 new environment controls.
- Independent dense Gaussian tests pass for zero/singular process noise.
  Added nonidentity-transport, nonzero-affine-offset reference-origin
  invariance test passes. Compiled transport tests pass.
- New private OFF vs preceding installed code, seed2:68: all160 retained fields
  and2784 full verbose state/covariance records exactly equal.
- Ten paired rows:9 non-secondary rows plus secondary control12:17.
  All1040 ordinary/truth field comparisons and16296 corresponding verbose
  state/covariance records exactly equal between OFF and ON.
- Six active optimized rows converge in2--4 reference passes at chosen b.
  Four empty-interval rows remain exact ordinary copies. Trial scans need up
  to7 passes here.12:11 has2 invalid trials, excluded as intended.
- The likelihood quadratic agrees with complete smoothed score within1.3066e-6
  and native affine forward-update sum within2.506e-7. Covariance validity,
  fixed fitted b, zero conditional loss variance, final reference lengths
  and both convergence tolerances are checked.
- Forced maximum1 pass on2:68 rejects all14 coarse trials for nonconvergence.
  Both free results exactly copy input KF with status3. Ordinary/truth remain.
- Requested38:106 and19:146 could not run: reused tracker files contain100
  events. Both modes stop at EOF, not algorithm failure. They are explicitly
  excluded and replaced by available overshoots48:54 and1:67.
- Smoke timing old43.67s/new66.21s; peak RSS1190780/1190360KiB.
  This is not a memory-leak or population-performance validation.

After the user's no-batch confirmation, the configured package build and
package-only install passed. Built/installed library SHA256:
e8afbdfdcdf9499c9db84a92ffbd4098cf09db3de790885ad2fb8e5f3e19b7fd
Installed off/on2:68 match their private counterparts in all171 fields.
Installed defaults verified: iterations0, tolerances0.001/0.0001,
SigmaLogLoss0.001 and BackwardSeedScale100. Both numerical executables pass.
Build-time ROOT PCM lookup warnings were present; installed event/configuration
gates succeeded. Generated outputs are not committed.

## Focused results

Residual=100*(pT_reco/pT_truth-1). Zero-based seed:event. Same maintained code and
identical tracker input for each A/B. The secondary control is not counted as
single-track optimization evidence.

| Seed:event | KF residual % | Old free RTS % | Reference free RTS % | Old free backward % | Reference free backward % | Inner passes |
|---|---:|---:|---:|---:|---:|---:|
| 2:68 | -2.519150 | 0.782981 | 0.825949 | 0.823689 | 0.858868 | 2 |
| 12:11 | -0.584490 | -0.283713 | -0.265734 | -0.251062 | -0.223918 | 2 |
| 12:16 | -0.257888 | -0.287240 | -0.287240 | -0.191094 | -0.191094 | 0 |
| 12:17 (secondary control) | -42.622679 | -42.636309 | -42.641951 | -42.610789 | -42.610861 | 3 |
| 14:1 | -0.019761 | 171.604991 | 171.322232 | 172.005133 | 172.001499 | 3 |
| 1:67 | -5.816683 | 155.427357 | 155.528523 | 156.884913 | 156.886359 | 4 |
| 48:54 | -0.075218 | 171.559670 | 171.556294 | 171.352188 | 171.351806 | 2 |
| 12:0 | 0.086157 | 0.054159 | 0.054159 | 0.068531 | 0.068531 | 0 |
| 12:6 | -0.024182 | -0.045323 | -0.045323 | -0.012657 | -0.012657 | 0 |
| 12:15 | 0.087718 | 0.065090 | 0.065090 | 0.122497 | 0.122497 | 0 |

The three extreme overshoots still select b approximately1 (63.21% loss bound).
Reference iteration does NOT resolve these failures. Other changes are small
or mixed. The test establishes mechanical self-consistency and preservation
of existing paths, not better IP resolution. The forward/smoothed reference
mismatch is not established as the dominant cause of the bad minima.

## Status

Implemented, built and installed; the new path is opt-in. Use
BP_FREE_LOSS_REFERENCE_ITERATIONS=20 or fit.FreeLossReferenceIterations=20.
No automatic interval finder, simultaneous multiple-loss optimizer or broader
objective change is included. The next physics investigation requires a
separate decision; do not claim success from the lower/converged score.

The project-status-curator workflow preserved outgoing AGENTS in
2026-09-12-agents-before-reference-trajectory.md and verified the global
status/law prefix byte-exact. Current focus points here for the completed gate.
