# RecBreakpoint first working version — 2026-09-08

## Authorization and scope

The user requested a new branch and independent Reconstruction/RecBreakpoint
package reading CompleteTracks. They explicitly restricted the sixth coordinate
to configured propagation intervals, not every hit, and asked for readable
package-local helpers without modifying ready KF classes. Local branch
`test_breakpoint` starts at `c113c7d` (dev). No remote operations were performed.

Implementation: new package, dedicated options/run_breakpoint.py, and one
add_subdirectory in Reconstruction/CMakeLists.txt. Existing GSF and shared KF
sources are unchanged. Pre-existing edits to DumpGsfTrks/gsf.py.bk, sim.py.bk,
trk.py.bk and subtrkjobs.sh are unrelated and preserved.

The project Git law excludes other cards/build files by default. Permission to
include the new package's CMake files, parent registration and dedicated card in
a checkpoint was asked separately. Unless granted, those remain local while
source and documentation are checkpointed. Generated diagnostics are untracked.

## Fit contract

- Hits are sorted outward by cylindrical radius. BreakpointIntervals entry i
  selects hit[i] -> hit[i+1], not layer i. Default [] is ordinary outward KF.
- At selected edges only, b=log(p_before/p_after) is an independent local
  Gaussian with MeanLogLoss=0 and SigmaLogLoss=0.05 by default. The collinear
  source-surface map is kappa_after=exp(b)*kappa_before.
- A package-local 6x6 Jacobian includes this mapping and native propagation.
  The 5D marginal receives the loss variance, while the loss/target and
  source/target cross covariances are retained for inference.
- Native KalTest updates every real hit once. RTS smoothing conditions earlier
  states and the local losses on downstream measurements. Local and all-hit
  loss means/variances are persisted separately.
- Standard FullLDCTracking-style loose seed covariance, scale=1, MS=true,
  deterministic ionization=false. No BH mixture, truth override or beam spot.
- Native KalTest's sixth coordinate is t0, not reused for loss. The temporary
  b dimension belongs exclusively to the new numerical helper.
- KalmanAdapter retains double mean/covariance/pivots through native KalTest;
  BreakpointTrackSystem is a local subclass exposing protected layer lookup.
  No shared KF class was edited or parallel measurement update hand-coded.
- Covariance smoothing uses the conditional positive-sum form, avoiding severe
  cancellation in P_filtered + G(P_smoothed_next-P_predicted_next)G^T for the
  very loose early seed. Nonpositive covariances still fail; no diagonal jitter.
- Successful BreakpointTracks carry IP/first/last states and hit references.
  Fixed PODIO status/index vectors align to input tracks. A flat diagnostic
  tree records successes and failures. Optional EDM output is commented in the
  dedicated card. Existing output files are refused rather than overwritten.

The single linearization does not enforce b>=0 or find a global nonlinear
optimum. Negative losses remain visible. Selected edges are not automatically
matched to truth. Physical location within an edge remains approximated by its
upstream surface; outward noncurling barrel ordering is the first-version scope.

## Numerical and event gates

EL9/LCG105 module built and installed. Standalone CTest passed covariance
transport, cross-correlation retention, inverse map, finite-difference loss
derivative, process-noise embedding, direct/marginal Gaussian conditioning,
and nonfinite/asymmetric input rejection.

Early adapter experiments found a DDKalTest layer-lookup/teardown failure;
the implemented backend is the same KalTest family used by GSF. A float EDM
round-trip prototype and subtractive RTS covariance failed early smoothing.
The final native double-precision adapter plus conditional covariance resolved
those failures; failed prototype output is not performance evidence.

Final focused input:
`gsf_doublebhoff_freshseed_diagnostic/trk-e--2.0-85-12.root`

SHA256: `cf926cd5445105820e7863e9488e9fe826098ac176bc86f9601e7ffcbf9bd558`

Selected zero-based event indices 11,16,17, 233 hits and one CompleteTracks row
each. Seed 12 is the source named in the earlier beam-boundary mechanical
record; event IDs are not silently substituted from another seed. These tests
did not rerun a topology audit and are not population optimization counts.

Fresh same-code runs used intervals [], [5], [5,7], mean b=0, sigma b=0.05,
MS=true, Eloss=false, SeedScale=1. All nine fits succeeded. Verbose logs include
complete predicted/filtered/smoothed 5D means and covariances at every hit.

| Event | Truth pT | Stored CompleteTracks | Native same-seed KF | No breakpoint | [5] | [5,7] |
|---:|---:|---:|---:|---:|---:|---:|
| 11 | 9.15107727 | 9.09759015 | 9.09556848 | 9.09556915 | 9.50392219 | 9.44865482 |
| 16 | 38.36070251 | 38.26177498 | 38.25095017 | 38.25095312 | 39.00779948 | 39.39417776 |
| 17 | 31.75560379 | 18.22051485 | 18.21669917 | 18.21669917 | 17.92862071 | 17.96651973 |

All pT values are GeV. Empty-list/native relative pT disagreement is below
8e-8. This is not equivalence to the original CompleteTracks production setup.
Selected-edge independent 6D/native covariance closures are below 3e-16 in
these nine fits. The arbitrary selected intervals do not improve these tracks
uniformly; event 17 remains badly underestimated. No performance claim follows.

Additional seed-12 event-11 tests:

- First/last edges [0,231]: success, pT=9.09643595, max closure 4.19e-16.
  Last-edge local and smoothed losses coincide, as there are no later hits.
- [5], mean b=.02, sigma=.03: success, pT=9.43361193, b_smoothed=.03410404.
- [233] (out of range): explicit failed row, status=-1, pT=NaN.
- [5,5]: rejected at initialization, before event processing/output creation.
- Optional PODIO serialization/readback: excluded event 0 has status [0],
  output index [-1] and no fitted tracks; selected event 11 has status [1],
  index [0], one fitted track with three states and all 233 hit references.
  The serialized [5] endpoint also agrees with the flat-only run.

Earlier seed-1 smoke checks included event 16 with two reconstructed tracks;
both were mechanical controls, not counted as topology-clear examples.

Outputs and verbose logs:
`TrackingPerformanceStudies/recbreakpoint_first_working_2026-09-08/`

Main files are seed12_none.root, seed12_h5.root and seed12_h5_h7.root with
matching logs. Boundary/prior/invalid configurations have separately named
outputs/logs. The dedicated card and package README give exact rerun commands.

## Memory migration and next work

The complete pre-edit AGENTS.md is preserved verbatim in
`2026-09-08-agents-before-recbreakpoint.md` (checked identical before edits).
Heading map: Introduction/global status, project laws and essential commands
remain live; outgoing Current focus is preserved in that snapshot and replaced
by this experiment's active focus. All active laws are retained, with narrow
user-authorized package/branch exceptions. No history directories were moved,
so a directory migration manifest is not applicable. No historical file was
deleted; no historical path renaming was performed. AGENTS retains two main
sections. Beam-boundary findings and GSF frozen controls remain valid/paused.

Before any population claim: review interval choice and whether one
linearization/independent Gaussian priors are adequate; evaluate large-loss and
negative-loss behavior; use categorized, same-code comparisons and held-out
tracks with clean-track/tail controls. Do not repair these by silently steering
from truth or modifying shared KF classes.
