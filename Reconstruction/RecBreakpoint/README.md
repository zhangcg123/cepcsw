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
BP_INTERVALS=5 BP_EVENTS=18 BP_SELECTED=11,16,17 BP_VERBOSE=1 \
build.105.0.0.x86_64-el9-gcc11-opt/run \
  gaudirun.py Reconstruction/RecBreakpoint/options/run_breakpoint.py
```

The dedicated card explicitly sets every package steering property.
BP_* overrides are conveniences for tests; edit the card for other choices.
Existing output files are refused, not overwritten. The card's tracker-only
TDR_o1_v01 geometry must match the input.

## Selected intervals, not every hit

`LossStateMode="Persistent6D"` is the compiled and dedicated-card default.
It currently accepts **one breakpoint (or none) and BackwardMode="RTS"**.
Use `LossStateMode="LocalMarginal"` for the earlier local-joint implementation,
multiple selected intervals, or the existing BackwardFilter comparison.
Unsupported combinations fail initialization, never silently fall back.

In LocalMarginal, `BreakpointIntervals=[5,7]` means two independent losses on
`hit[5] -> hit[6]` and `hit[7] -> hit[8]`. Indices are zero-based after
sorting hits by cylindrical radius, **not detector layer numbers**. The tuple
saves cell IDs, radii and z. An empty list disables breakpoints. Duplicate or
negative indices fail initialization; missing intervals fail the affected track.
The first version is intended for outward, noncurling barrel tracks.

At the selected interval the augmented state is introduced:

```text
(drho, phi0, kappa, dz, tanLambda, b)
b = log(p_before / p_after)
kappa_after = exp(b) * kappa_before
fractional loss = 1 - exp(-b)
```

The 6x6 Jacobian includes this loss map and ordinary geometric propagation,
including curvature/loss cross covariance. Native KalTest's sixth coordinate
is a time offset; it is NOT reused as b.

### Persistent6D workflow

Before the chosen interval, the live filter is 5D. At its upstream hit, add
one independent b prior to form a 6D mean and full 6x6 covariance. Apply the
loss map once and propagate to the downstream hit. Thereafter every live
prediction and measurement update remains **six-dimensional**:

```text
ordinary interval: J6 = diag(F_track,1), Q6 = diag(Q_track,0)
b_predicted = b_filtered_at_previous_hit
P_predicted = J6 P_filtered J6^T + Q6
measurement derivative: H6 = [H_track,0]
```

The zero measurement derivative for b does not prevent its update: the full
Kalman gain uses all track/b cross covariances. Native KalTest `Filter()`
performs the complete 6D measurement update through a package-local site.
The site projects only the five helix coordinates into the detector response;
the sixth coordinate is never interpreted as native KalTest's time offset.
Native five-dimensional material/geometric transport supplies the physical
F and Q blocks, embedded in full 6D transport; it performs **no separate 5D hit
update** on this path. The complete 6D posterior becomes the next step's input.

At the end, a joint RTS pass smooths the six-dimensional sequence and crosses
the 6D-to-5D birth boundary using its rectangular transition. It publishes the
smoothed innermost track geometrically extrapolated to IP. The final outward
b mean/variance already contain all downstream measurements. It is one fixed-
linearization fit, not an iterated nonlinear refit. The unchanged LocalMarginal
path remains an explicit equivalence/regression reference.

### Earlier LocalMarginal workflow

1. Make an outward prefit from the first, middle and last usable 2D hits
   (default), or the first three as an explicit comparison. Assign the same
   loose FullLDCTracking-style covariance and update actual hit 0 first.
2. At each selected edge, introduce an independent Gaussian b prior,
   linearize the loss map and propagate its covariance into the helix.
   Other edges remain ordinary five-dimensional KF transitions.
3. Update every real hit once using native KalTest. Retain transition state
   cross covariances and, at breakpoints, loss/state cross covariances.
4. With `BackwardMode=RTS`, run backward Rauch-Tung-Striebel smoothing and recover each loss posterior
   using the downstream smoothed state. Publish the smoothed innermost state
   geometrically extrapolated to the IP.

Alternatively, `BackwardMode=BackwardFilter` copies the last outward posterior
and its complete covariance without scaling. It does not update the last hit
again. It propagates inward and updates each remaining hit with native KalTest.
On selected interval i, it first propagates from i+1 to i, then applies
`kappa_before=exp(-b)*kappa_after` BEFORE updating hit i. This preserves the
outward map's upstream-surface owner; it does not move the loss to i+1.
The inverse map scales all curvature cross covariances and adds the scalar
loss-prior variance through its derivative `-kappa_before`.

Each selected reverse crossing introduces a fresh Gaussian with the configured
MeanLogLoss/SigmaLogLoss, not the RTS or outward fitted loss posterior. The
outward posterior already contains material/hit information, so this is a
deliberately evidence-reusing refit, not an independent Bayesian smoother.
Loss/state cross covariances are retained as the inward recursion continues;
later inner hits refine previously crossed loss means/variances by Gaussian
conditioning on the native KF's state update. They do not trigger an RTS pass
or a second propagation of already visited hits. Final backward filtered hit 0
is propagated to IP using native material-aware MarlinTrk propagation. RTS keeps
its original geometric IP extrapolation unchanged. Neither mode adds a
beam-to-first-hit breakpoint.

The reference KF follows the selected mode: native outward KF plus `smooth()`
and geometric extrapolation for RTS; a copied last-state inward KF and native
IP propagation for BackwardFilter. The latter mirrors the default forward-fit
publication branch of FullLDCTracking, but is still a hit-list refit, not a rerun
of pattern recognition, merging, outlier retries or selection.

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
| LossStateMode | Persistent6D | Persistent6D: live downstream 6D state, one interval and RTS only; LocalMarginal: earlier marginalized/local-joint path |
| SeedScale | 1 | Positive scale of all five loose seed variances |
| SeedHitSelection | FirstMiddleLast | First/middle/last usable 2D hits; FirstThree restores the original selection |
| BackwardMode | RTS | RTS smoothing or BackwardFilter seeded from the full outward posterior |
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

`backward_mode` identifies the algorithm used. `filtered_*` and `local_chi2`
always describe the outward pass. `smoothed_*` vectors retain their old RTS-only
meaning and are empty in BackwardFilter runs. The latter fills
`backward_predicted_kappa`, `backward_filtered_kappa`, their `_variance` fields
and `backward_local_chi2`, indexed in outward hit order. At the last hit, both
backward state vectors contain the copied seed and chi2 is zero (no update).
VerboseDump prints complete backward predicted/filtered 5D states/covariances.
`fitted_log_loss` and `fitted_log_loss_variance` are mode-independent final loss
fields. `local_log_loss` describes the first hit update after introducing the
loss: i+1 outward for RTS, i inward for BackwardFilter. The legacy
`smoothed_log_loss*` aliases are filled only for RTS. Published track chi2/ndf
remain outward-filter bookkeeping in both modes, not a combined goodness-of-fit.
Unknown BackwardMode values fail initialization. Use
`BP_BACKWARD_MODE=BackwardFilter` in the dedicated card to select the new mode.

Only the flat tuple is written by default. Commented PodioOutput lines in the
card allow event-collection serialization without changing the GSF workflow.

## Limits and tests

### Persistent-6D tuple diagnostics (automatic)

`loss_state_mode` identifies the implementation. `persistent_hit_index` lists
all downstream hits from selected interval i's hit i+1 to the final hit.
At row ordinal j in that vector:

- `persistent_{predicted,filtered,smoothed}_mean`: six entries starting at 6*j,
  ordered drho, phi0, kappa, dz, tanLambda, b.
- `persistent_{predicted,filtered,smoothed}_covariance`: 36 row-major entries
  starting at 36*j, including every track/b covariance.
- `persistent_transport`, `persistent_process_noise`: 36 row-major entries
  each, for the incoming six-dimensional propagation. The first transport
  includes the one-time loss mapping; later ones have identity b transport and
  zero b process noise. The independent birth prior is in the input P, not Q.

The b owner is the sole `breakpoint_interval` entry, not the current hit.
Vectors are empty for LocalMarginal or an empty breakpoint list. VerboseDump
also emits full 6D states/covariances. Existing 5D track projections and scalar
loss branches remain available. Unsupported multiple retained losses require
more than six dimensions and are deliberately not approximated by one b.

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
