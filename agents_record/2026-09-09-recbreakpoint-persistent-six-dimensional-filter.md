# RecBreakpoint persistent six-dimensional filtering

## Request and scope

User requested a real 6D KF: after crossing a configured eBrem interval, all
downstream states must explicitly carry b and its uncertainty. Implemented
on test_breakpoint inside RecBreakpoint only, starting from 8e9de9e. Shared
KalTest/MarlinTrk/TrackSystemSvc and GSF sources/cards are unchanged. No remote
or branch operations. Generated tests began Sept8 and remain under
TrackingPerformanceStudies/recbreakpoint_persistent6d_2026-09-08/.

## Active contract

- New compiled/dedicated-card default LossStateMode=Persistent6D. Exactly one
  breakpoint or an empty list is supported, with BackwardMode=RTS. Other
  combinations fail initialization. Independent multiple retained losses need
  more than six dimensions and are not collapsed into a common b.
- LossStateMode=LocalMarginal preserves the earlier implementation, including
  multiple breakpoints and BackwardFilter. No GSF configurable changed.
- Before the configured edge i -> i+1 the live state is 5D. At the upstream
  state introduce an independent Gaussian b prior, with the existing mean and
  variance settings. Apply kappa_after=exp(b)*kappa_before once, then propagate.
- Every downstream state has six live coordinates and a complete 6x6
  covariance. The b owner stays interval i; it does not move to the current hit.
  On ordinary later edges J6=diag(F_track,1), Q6=diag(Q_track,0). The mean/variance
  of b remain unchanged in prediction; track/b cross covariance propagates.
- KalmanAdapter uses native 5D material/geometric prediction to supply F/Q
  blocks and physical helix transport. That call is prediction-only. It does
  not perform a 5D hit update and subsequently reconstruct b.
- Package-local LossMeasurementSite calls native KalTest Filter() with a full
  six-dimensional predicted state/covariance. Its native hit projection is
  evaluated using the five helix coordinates, with H_b=0. The full gain updates
  b through cross covariance. The native sixth-coordinate t0 interpretation
  is explicitly avoided. No hand-coded parallel measurement update was added.
- The entire native six-dimensional posterior becomes the next prediction's
  input. Later hits update b and its variance directly during outward filtering.
- A variable-dimension joint RTS pass smooths the downstream 6D sequence and
  crosses the birth boundary with its rectangular 6x5 transport. The innermost
  smoothed five-dimensional track is geometrically extrapolated to IP, as in
  the earlier RTS path. This is not iterative relinearization.

Native process-noise, seed, material and hit settings are unchanged. The loss
still has an unconstrained fitted sign, is placed at the upstream hit, and is
not automatically located or given truth magnitude. This change is not a
solution to the earlier negative-peak or early-layer information limitations.

## Outputs

Existing track and scalar-loss outputs remain. New default-on flat branches:
loss_state_mode; persistent_hit_index; persistent_{predicted,filtered,smoothed}
_mean and _covariance; persistent_transport; persistent_process_noise.

For persistent hit ordinal j, mean occupies six entries and every matrix 36
row-major entries. Coordinate5 is b. The sole breakpoint_interval entry names
its owner. The first transport includes the loss map; subsequent ones have
zero track derivative with respect to b at fixed current helix and identity b
transport. All process-noise b rows/columns are zero; the birth uncertainty is
in the independent prior, not a repeatedly injected process noise.
Full predicted/filtered/smoothed 6D verbose dumps are also available.
LocalMarginal and empty-breakpoint runs have empty persistent vectors.

Run through the dedicated card with BP_LOSS_STATE_MODE=Persistent6D,
BP_BACKWARD_MODE=RTS, BP_INTERVALS=5 (example). BP_LOSS_STATE_MODE=LocalMarginal
restores the old path. Existing other dedicated-card controls are unchanged.
The dedicated options and build files remain uncommitted under the existing
repository law; source, headers and durable documentation are checkpointed.

## Build and mechanical checks

RecBreakpoint built in EL9/LCG105. Global cmake --install stopped on an unrelated
missing CEPCSW.components in Analysis/TotalInvMass. Package-local installation
succeeded using cmake --install build.105.0.0.x86_64-el9-gcc11-opt/Reconstruction/RecBreakpoint.
The standalone test initially encountered a read-only ccache temp directory;
explicit /usr/bin/g++ bypassed that wrapper and CTest passed. NFS clock-skew
warnings were present; actual sources were compiled and installed, and new
runtime properties/branches were checked in produced files.

Transport unit tests include persistent b variance/cross-covariance transport
over twenty ordinary steps, no repeated b derivative, zero b process noise,
physical loss derivatives and the earlier joint/conditional-update checks.

Same-code runs used FirstMiddleLast, SeedScale1, MS=true, Eloss=false,
MaxChi2PerHit=1e100, MeanLogLoss0, SigmaLogLoss0.05 unless stated. Native KF
references and full verbose diagnostics enabled. Twenty-four successful fit
rows across sixteen jobs (one smoke pair, then eleven event/configuration pairs)
cover five distinct events. Two additional invalid-setting jobs correctly
failed initialization: Persistent6D with two intervals and with BackwardFilter.

The initial seed12:11 [5] smoke pair passed before the regression campaign.
Regression pairs: seed12 events11/16/17 with [5] and with []; seed5:84 [5];
seed5:92 [6]; seed12:11 [5] with prior0.02; seed12:11 first edge[0] and last
edge[231]. Hard-event fixed[5] tests are mechanical controls, not claims that
this is each event's unique correct truth interval. Seed5 negatives retain
the audited single-eBrem truth-selected intervals from the twenty-event study.

Checks passed for statuses, exact event/cell/truth/KF pairing, expected row
counts, all downstream 6D dimensions, finite positive-definite full
covariances, full J P J^T+Q recursion, unchanged predicted b/variance between
hits, zero later b derivatives/noise, native measurement variance reduction,
endpoint b equality to final live b, and constant smoothed b over downstream
surfaces. Empty-list runs retain the native KF reference gate.

Across the eleven regression event/configuration pairs:

- Stored IP pT differences: exactly zero (the endpoint passes through EDM
  serialization; this does not mean all internal doubles are identical).
- Maximum fitted b difference: 2.57963e-11.
- Maximum fitted b variance difference: 5.64761e-16.
- Maximum outward filtered kappa difference: 4.51405e-10.
- Maximum transition covariance closure: 3.41790e-16.

results_smoke.json, results_regression.json and check_*.log retain all values
and checks. This supports fixed-linearization equivalence on the tested cases,
not an unconditional equivalence claim for different nonlinear iterations.

## Endpoint comparison

Momenta in GeV. Every LocalMarginal value below equals the fresh Persistent6D
value at stored precision. KF is CompleteTracks, not the mode-matched empty
refit. Entries are zero-based.

| Seed:entry | Setup | Truth pT | KF pT | LocalMarginal pT | Persistent6D pT |
|---|---|---:|---:|---:|---:|
| 12:11 | [5] | 9.15107727 | 9.09759015 | 9.50620040 | 9.50620040 |
| 12:16 | [5] | 38.36070251 | 38.26177498 | 39.01869670 | 39.01869670 |
| 12:17 | [5] | 31.75560379 | 18.22051485 | 17.91896149 | 17.91896149 |
| 12:11 | [] | 9.15107727 | 9.09759015 | 9.09490795 | 9.09490795 |
| 12:16 | [] | 38.36070251 | 38.26177498 | 38.25051517 | 38.25051517 |
| 12:17 | [] | 31.75560379 | 18.22051485 | 18.21618641 | 18.21618641 |
| 5:84 | [5] | 36.68881607 | 36.42239996 | 35.76436192 | 35.76436192 |
| 5:92 | [6] | 30.11129570 | 29.88802765 | 29.44185295 | 29.44185295 |
| 12:11 | [5], prior0.02 | 9.15107727 | 9.09759015 | 9.65961572 | 9.65961572 |
| 12:11 | [0] | 9.15107727 | 9.09759015 | 9.09490795 | 9.09490795 |
| 12:11 | [231] | 9.15107727 | 9.09759015 | 9.09565749 | 9.09565749 |

## Explicit downstream b evolution example

Seed12:11, breakpoint5, prior b=0, sigma0.05. Values below are live outward
posteriors, not losses reconstructed afterward from RTS:

| Hit | b mean | b sigma |
|---:|---:|---:|
| 6 | -0.00336346 | 0.04949077 |
| 7 | +0.01694656 | 0.04621982 |
| 8 | +0.02995687 | 0.04501471 |
| 119 | +0.03596829 | 0.04432537 |
| 232 | +0.03810427 | 0.04419619 |

Seed5:92 similarly starts positive at h7 (b0.0112437), becomes negative by
h119 (-0.0105832), and ends at -0.0159522 with sigma0.0197361. Its -2.22323%
pT residual is unchanged. Explicit persistence makes this development visible;
it does not remove wrong-sign fitting or produce additional information.

## Project-memory migration

Used project-status-curator for the changed active implementation contract.
The complete pre-edit AGENTS.md is preserved in
2026-09-08-agents-before-persistent6d.md. Mapping: section1/global status,
active laws and essential commands are retained byte-for-byte; section2's
outgoing local-joint focus is preserved in that snapshot and replaced by the
persistent-6D focus. Both top-level substantive sections remain. No historical
directory was moved or file removed, so no directory-migration manifest was
needed. Earlier tests and the twenty single-eBrem results remain applicable
to LocalMarginal and are not relabeled as new physics validation.
