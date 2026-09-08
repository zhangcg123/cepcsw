# RecBreakpoint

Experimental electron breakpoint Kalman refitter on `test_breakpoint`.
Reads `CompleteTracks`, refits their reconstructed hits and publishes
`BreakpointTracks`. Existing GSF/KF sources and batch cards are unchanged.
This is a first-order fit, not a physics-validated replacement.

## Build and run

From the CEPCSW repository root:

```bash
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target RecBreakpoint -j4
cmake -P build.105.0.0.x86_64-el9-gcc11-opt/Reconstruction/RecBreakpoint/cmake_install.cmake
BP_INPUT=/absolute/path/to/tracker.root \
BP_OUTPUT=/absolute/path/to/new_breakpoint_flat.root \
BP_INTERVALS=5,7 BP_EVENTS=18 BP_SELECTED=11,16,17 BP_VERBOSE=1 \
build.105.0.0.x86_64-el9-gcc11-opt/run \
  gaudirun.py Reconstruction/RecBreakpoint/options/run_breakpoint.py
```

The dedicated card explicitly sets every package steering property.
BP_* overrides are conveniences for tests; edit the card for other choices.
Existing output files are refused, not overwritten. The card's tracker-only
TDR_o1_v01 geometry must match the input.

## Selected intervals, not every hit

`BreakpointIntervals=[5,7]` means two independent losses on
`hit[5] -> hit[6]` and `hit[7] -> hit[8]`. Indices are zero-based after
sorting hits by cylindrical radius, **not detector layer numbers**. The tuple
saves cell IDs, radii and z. An empty list disables breakpoints. Duplicate or
negative indices fail initialization; missing intervals fail the affected track.
The first version is intended for outward, noncurling barrel tracks.

At a selected interval only, the local state is:

```text
(drho, phi0, kappa, dz, tanLambda, b)
b = log(p_before / p_after)
kappa_after = exp(b) * kappa_before
fractional loss = 1 - exp(-b)
```

The 6x6 Jacobian includes this loss map and ordinary geometric propagation,
including curvature/loss cross covariance. Native KalTest's sixth coordinate
is a time offset; it is NOT reused as b.

1. Make an outward prefit from the first, middle and last usable 2D hits
   (default), or the first three as an explicit comparison. Assign the same
   loose FullLDCTracking-style covariance and update actual hit 0 first.
2. At each selected edge, introduce an independent Gaussian b prior,
   linearize the loss map and propagate its covariance into the helix.
   Other edges remain ordinary five-dimensional KF transitions.
3. Update every real hit once using native KalTest. Retain transition state
   cross covariances and, at breakpoints, loss/state cross covariances.
4. Run backward Rauch-Tung-Striebel smoothing and recover each loss posterior
   using the downstream smoothed state. Publish the smoothed innermost state
   geometrically extrapolated to the IP.

Marginalizing the Gaussian loss before updating the hit, then conditioning
it through its retained cross covariance, is equivalent to an augmented
linear-Gaussian update. Ordinary recursion stays 5D; independent local losses
do not permanently increase the state dimension. Covariance smoothing uses
a positive-sum conditional form to avoid cancellation of loose seed errors.

## Helpers

- `AugmentedTransport`: ROOT-independent 6D Jacobian/covariance arithmetic.
- `TrackState`: double-precision helix, covariance and pivot; EDM conversion
  only at seed/publication and optional reference boundaries.
- `BreakpointTrackSystem`: package-local subclass exposing existing
  MarlinKalTest layer lookup, without shared implementation edits.
- `KalmanAdapter`: native KalTest propagation, process noise and hit updates;
  temporary tracks/sites own their hits and states through RAII.
- `BreakpointFitter`: selected loss transitions, forward filtering, smoothing.
- `RecBreakpoint`: configuration, event input/output, tuple and verbose dumps.

## Configuration

| Property | Compiled default | Meaning |
|---|---|---|
| InputTracks | CompleteTracks | Tracks supplying reconstructed hits |
| OutputTracks | BreakpointTracks | Successful refitted tracks |
| BreakpointIntervals | [] | Selected outward ordered-hit intervals |
| MeanLogLoss | 0 | Common independent Gaussian b-prior mean, finite in [0,5] |
| SigmaLogLoss | 0.05 | Positive finite b-prior sigma |
| SeedScale | 1 | Positive scale of all five loose seed variances |
| SeedHitSelection | FirstMiddleLast | First/middle/last usable 2D hits; FirstThree restores the original selection |
| MaxChi2PerHit | 1e100 | Native hit-acceptance limit; rejection fails the track |
| MSOn | true | Baseline multiple-scattering noise |
| ElossOn | false | Baseline deterministic ionization correction |
| TruthDiagnostics | false | Optional generator-electron pT reference only |
| VerboseDump | false | Complete predicted/filtered/smoothed states and covariances |
| VerifyKFReference | false | Independent native MarlinTrk rerun with same seed/hits |
| SelectedEventIndices | [] | Zero-based input events; empty selects all |
| OutputFile | breakpoint_flat.root | New flat ROOT output |

The card enables TruthDiagnostics; truth never selects intervals or enters
the fit. Ambiguous multi-electron generator events have NaN truth pT. A scalar
generator reference is not reconstructed-track truth matching, and event
selection does not imply topology-clear selection.

`SeedHitSelection` operates on the radius-ordered usable 2D hits, skipping
one-dimensional hits. For N usable hits, FirstMiddleLast selects positions
`0, N//2, N-1` (upper middle for even N); FirstThree selects `0,1,2`.
Both require at least three usable hits and do not silently fall back.
The initial covariance/SeedScale, propagation start, and hit-update order are
unchanged. The optional native-KF reference uses the same selection.
`BP_SEED_HIT_SELECTION=FirstThree` selects the legacy mode in the dedicated
card. The seed uses downstream hit positions only to construct its starting
helix, not their fitted measurement covariance. This is not a global loss fit.

Fixed-name PODIO collections `BreakpointStatus` and `BreakpointOutputIndex`
are input-row-aligned. Status: 1 success, -1 failure, 0 excluded event.
The output index is -1 when no fit exists. Successful tracks contain IP,
first-hit and last-hit states. Chi2/ndf are innovation bookkeeping, not a
calibrated goodness-of-fit test with fitted losses and priors.

The ROOT tree `breakpoint` has one row per attempted input track, including
failures. It saves event/track indices, status, generator/CompleteTracks/fitted
pT, optional native-reference pT, ordered hit geometry, local chi2, filtered and
smoothed curvature/variance, selected intervals, and local/all-hit loss
posteriors with variances. `covariance_transport_closure` compares the separate
6D prediction's helix marginal with the actual native prediction, normalized
to covariance units; a discrepancy above 1e-3 fails the track.
`seed_hit_selection` and `seed_hit_indices` retain the effective mode and three
actual ordered-hit indices in each attempted track row; selection failure leaves
the index vector empty. The indices are also printed with VerboseDump=true.

Only the flat tuple is written by default. Commented PodioOutput lines in the
card allow event-collection serialization without changing the GSF workflow.

## Limits and tests

One linearization about the configured loss mean is performed. The loss is
located at the upstream measurement surface. This version does not fit its
position within the interval, select intervals automatically, use BH mixtures,
enforce positive losses, or iterate to a nonlinear optimum. Negative fitted
losses are retained, not clipped. Large-loss results need particular caution.
There is no beam-spot update. Material/mass conventions remain native KalTest;
this does not introduce the GSF DD4hep material-path machinery.

VerifyKFReference with an empty interval list checks IP pT against native
MarlinTrk (relative tolerance 1e-4). This is not equality to the stored
CompleteTracks fit, whose settings/seeding may differ.

Numerical tests can also run independently:

```bash
cmake -S Reconstruction/RecBreakpoint -B /tmp/recbreakpoint-build
cmake --build /tmp/recbreakpoint-build
ctest --test-dir /tmp/recbreakpoint-build --output-on-failure
```

Focused evidence is in
`agents_record/2026-09-08-recbreakpoint-first-working-version.md` at repository
root. Execution and covariance closure are mechanical checks only; clean-track
safety and categorized, held-out momentum validation remain open.
