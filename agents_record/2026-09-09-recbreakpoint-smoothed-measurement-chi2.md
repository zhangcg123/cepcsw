# RecBreakpoint: actual RTS-smoothed hit residual sums

## Request and scope

After the eight-event 1--5% loss comparison, the user asked whether truth
override gives a better global fit chi2, then explicitly requested checking
chi2 after smoothing. This record replaces no earlier number: the preceding
forward/backward filter sums and the post-smoothing measurement score are
different quantities.

No fitter, GSF/shared source, maintained card, build installation, default,
branch or remote was changed. Existing dirty user files remain untouched.
Local test_breakpoint starting checkpoint f7d3b53. The 16 existing RTS outputs
(eight events x ordinary/TruthOverride) are reused without refitting.
All originate from one-pass same-code jobs, not iterative fits.

## Exact score

At each reconstructed hit, reconstruct the native KalTest measurement site
using the existing GEAR/MarlinKalTest layer lookup and ConvertLCIOTrkHit.
Read the full 5D smoothed mean/covariance from the original 17-digit verbose
log. Use native TKalTrackSite::CalcExpectedMeasVec to intersect the local helix
with its actual measurement surface and predict the detector measurement.

For r_i = measured_i - h_i(smoothed_state_i), compute

    J_hit = sum_i r_i^T V_i^-1 r_i

where V_i is native GetMeasNoiseMat(), the hit measurement covariance. Each
selected hit is two-dimensional. All hits enter once, including both ends.
No final-pT residual enters this score. No filter innovations are substituted
for final smoothed residuals; no forward/backward scores are added together.

This is the final trajectory's weighted measurement residual sum, NOT a
complete global MAP objective: it excludes seed/loss-prior and material
process-noise penalties. It is also NOT the sum of per-hit residual pulls
normalized with V-H*C_smoothed*H^T, and is not divided by an assumed NDF.
Therefore a lower number here alone neither establishes a global optimum nor
calibrates a goodness-of-fit probability. Only RTS is compared: the separate
BackwardFilter path is not an RTS smoother.

## Read-only replay and validation

An isolated helper compiled under
/tmp/recbreakpoint_smoothed_residuals.Y6A9CI/ implements no propagation or
Kalman update. It loads saved predicted, filtered and smoothed states, native
geometry and the same original CompleteTracks. It does not link/run the
RecBreakpoint fitter; existing dependency libraries provide measurement
conversion/projection only. The package's generated build flags/link library
list are read without editing its build tree. Helper C++ remains outside the
project source tree; no alternative fitter is installed or selected.

The first attempt stopped in Python configuration before processing events
(keyword arguments unsupported by the temporary Configurable subclass).
Setting its two properties after construction fixed the helper card. All four
replay jobs then completed, with no failed or omitted selected events/hits.

The original tuple and parsed logs match exactly in filtered/smoothed kappa.
All replayed hit indices/cell IDs match the source tuple and original ordered
CompleteTracks hits. Replay refuses unverified one-dimensional measurement
sites (none occurred). Native filtered residuals plus the original predicted-
to-filtered state penalty reproduce EVERY stored native per-hit update chi2;
the maximum absolute discrepancy across 16 tracks is 1.2787904069e-10.
Their sums also reproduce the stored filter_chi2. This checks the measurement
coordinates, units, pivots, hit errors and interpretation of the logged states
before using the same machinery on smoothed states.

Artifacts under
TrackingPerformanceStudies/recbreakpoint_truthoverride_2026-09-09/smoothed_residual_check/:
replay.py, summarize.py, metadata.json, seed*_states.txt,
seed*_residuals.csv/log, results.json, comparison.json/csv.
Per-hit CSV retains residuals, measurement sigmas, native pivot, dimensions,
measurement score and the filter-replay gate. Generated outputs/scripts are
uncommitted. This record is durable numerical evidence.

## Results

Delta = TruthOverride - ordinary; negative favors TruthOverride in J_hit.

| Seed:entry | Hits | Ordinary RTS J_hit | Truth RTS J_hit | Delta |
|---|---:|---:|---:|---:|
| 2:68 | 232 | 416.050941 | 416.605612 | +0.554671 |
| 3:12 | 233 | 454.554446 | 454.491316 | -0.063130 |
| 3:75 | 234 | 444.230262 | 443.529772 | -0.700490 |
| 3:83 | 235 | 451.296920 | 451.309044 | +0.012124 |
| 3:85 | 234 | 433.508932 | 433.621227 | +0.112295 |
| 3:94 | 233 | 426.795914 | 426.768541 | -0.027373 |
| 4:65 | 232 | 408.279788 | 408.334674 | +0.054886 |
| 5:69 | 232 | 462.963698 | 462.952006 | -0.011691 |

TruthOverride lowers J_hit in 4/8 events, compared with 5/8 for the recorded
forward filter chi2. For 3:75, the filter chi2 rises 450.704 -> 453.504,
whereas the actual smoothed measurement score falls 444.230 -> 443.530.
Its pT residual improves +1.9182% -> +0.2642%. Conversely, 2:68 improves pT
+0.6446% -> -0.0145% while J_hit rises 416.051 -> 416.606.

All absolute J_hit differences are below 0.701 despite substantial pT changes
in some events. Thus these actual final-hit residual scores do not uniformly
prefer the truth-loss result. This is consistent with limited discrimination
between these trajectories by the hit data, but does not isolate its cause or
prove optimality, prior dominance, or correctness of the loss-position model.
TruthOverride changes both the loss magnitude and its uncertainty; these
selected examples are not a population validation.

The paired momentum results and selection provenance remain in
2026-09-09-recbreakpoint-truthoverride-loss1to5.md.
