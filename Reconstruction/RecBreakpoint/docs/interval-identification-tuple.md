# Baseline KF interval-identification dataset

`RecBreakpointIdentification` is a separate algorithm in the RecBreakpoint
plugin. Its card is `options/run_interval_identification.py`. It reads existing
REC and writes only a new ROOT file, refusing to overwrite an existing file.
It does not call RecBreakpoint, BreakpointFitter, diffuse, free-loss, or ECAL
refitting, and never updates an input collection. No shared KF source changes.

## Configuration and isolated build-tree execution

The card explicitly lists all properties. Defaults: standard KalTest backend,
MS and ionization enabled, MaxChi2PerHit=100, independent standard loose seed
variances `(1e6,100,1e-4,1e6,100)` in EDM `(d0,phi,omega,z0,tanLambda)` units.
Both directions use the standard first/middle/last 2D-hit prefit; these seed
means already use hits at both ends. They are truth-independent, NOT strict
disjoint-hit evidence. Backward does not copy forward covariance. Forward is
smoothed with native KalTest `smooth()`, not an augmented RTS implementation.

```bash
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target RecBreakpoint -j4
BPID_INPUT="$PWD/sim_large_barrel_20261001/rec-barrel-1.root" \
BPID_OUTPUT=/tmp/new_interval_identification.root \
BPID_EVENTS=18 BPID_SELECTED=0,11,16,17 BPID_VERBOSE=1 \
build.105.0.0.x86_64-el9-gcc11-opt/run env \
  PYTHONPATH="$PWD/build.105.0.0.x86_64-el9-gcc11-opt/Reconstruction/RecBreakpoint/genConfDir:$PYTHONPATH" \
  LD_LIBRARY_PATH="$PWD/build.105.0.0.x86_64-el9-gcc11-opt/lib:$PWD/build.105.0.0.x86_64-el9-gcc11-opt/Reconstruction/RecBreakpoint:$LD_LIBRARY_PATH" \
  gaudirun.py Reconstruction/RecBreakpoint/options/run_interval_identification.py
```

This does not install the plugin. After a separately authorized package install,
the usual gaudirun environment can use the same card without the explicit
build-tree paths. `BPID_TRUTH=0` skips all truth access. `BPID_EVENTS` defaults
to 10; an empty `BPID_SELECTED` processes all events up to that limit.
Current REC lacks EventHeader: leave ReadEventHeader=false and identify a row
by SourceFile metadata + zero-based event_index + input_track_index. To enable
headers for other files, also add their collection to PodioInput.

## Schema

Tree `interval_identification`, one row per CompleteTracks track. File object
`schema_and_configuration` records units, seed/settings, geometry and source.
The first version orders hits by increasing cylindrical radius, matching the
barrel campaign. It is not validated for curling tracks or general endcap order.

* `hit_*`: original object/collection/cell ID, original order, decoded
  system/layer/side, global xyz and packed 3D covariance. Rejected hits remain.
* `baseline_*`: stored CompleteTracks IP state/covariance, pT, chi2, ndf.
* `forward_*`, `backward_*`: per-hit predicted/updated states, full covariance,
  reference point, local chi2, native innovation/vector covariance, projector,
  local measurement/noise, actual transition Jacobian/process noise and previous
  accepted hit. Empty matrix slots mean unavailable, never a zero measurement.
* `forward_smoothed_*`: native smoother states/covariances and included-hit
  residuals. Backward smoothing is not run; its smoothing slots stay empty.
* `forward_production_*`: a separate copied-outer-state inward continuation,
  following KalTestTool's endpoint finalization, with no inward chi2 cut.
  This checks reproduction of stored CompleteTracks. It is NOT the independent
  backward feature pass and must not be interpreted as disjoint evidence.
* `interval_*`: adjacent hit indices and straight chord distance. The chord is
  NOT propagation path length. Runtime material effects are represented by the
  saved Q matrices. Reconstructed DD4hep t/X0 and explicit local surface axes
  are not yet recorded; do not substitute truth t/X0 as a classifier input.
* `truth_hit_*`: association multiplicity, unique simulated-hit ID, weight and
  exact Geant4 hook identity/fraction/status. Ambiguous matches are not guessed.
* `truth_interval_*`: ALL N-1 intervals on a successful provenance match,
  endpoint steps/fractions, G4 t/X0, eBrem momentum loss, incoming momentum,
  retained fraction, emission counts and boundary flags. A failed match gives
  negative truth_status and unknown labels, never negative-class zero losses.
* `truth_ebrem_*`: primary-electron G4 eBrem steps, pre/post xyz, momenta,
  total energies (kinetic + electron mass), momentum loss and interval assignment.
  Interval -1 is before first hook, -2 after last, -3 unresolved. Assignment
  status 1 strict interior, 2 endpoint-step, 3 before, 4 after, 0 unresolved.
  Truth source intentionally supports primary electron/positron only.
* `truth_match_purity`: fraction of all track hits linked to the chosen MC.
  `truth_linked_hit_fraction` separately reports coverage. Tied maximum track
  association weights do not publish an arbitrary particle's momentum.

Native state order is `(drho,phi0,kappa=q/pT,dz,tanLambda)`, NOT EDM helix order.
Matrices are row-major; per-hit matrix vectors have N outer entries. Measurement
dimension determines residual/projector shapes. Predicted quantities are native
double; updated/smoothed states pass through the public float EDM TrackState
interface. Seed values are stored before pivot transport. Predictions reference
the measurement-site pivot returned by the public state interface.

The compiled native KF has six coordinates: the five helix parameters plus
timing t0 (NOT breakpoint loss b). The ordinary predicted/state covariance
branches contain the five-helix marginal; `native_dimension`,
`native_predicted_*`, `native_projector`, `native_transport` and
`native_process_noise` preserve the complete native prediction. No artificial
t0 posterior is reconstructed from the five-parameter EDM interface.
In the 18-event gate every measurement had zero derivative with respect to t0,
so all smoothed residual scores were available. For a different geometry with
a nonzero derivative, smoothed_status=3 explicitly withholds the residual score
while preserving the five-parameter smoothed state.

`innovation` is evaluated passively as m-h(x_pred), using the native layer's
measurement function and derivative. `backend_reported_innovation` also retains
the public interface's reconstructed residual. These can differ because the
interface builds its residual from the post-update residual. `local_chi2` is
the actual baseline acceptance score; `innovation_chi2` is r^T S^-1 r from the
saved prediction. They are not silently identified as the same quantity.

Update status is the raw MarlinTrk code (0 success, -999 unattempted).
diagnostic_valid=1 means native matrices available; -1 invalid quadratic;
0 unavailable (-100 means accepted update without usable native diagnostics).
Direction status=1 means a completed pass, -1 failure (inspect
per-hit statuses and error). smoothed_status=1 means state and residual score
available, 2 means valid state but non-positive-definite residual covariance,
-1 state retrieval failure, 0 unavailable.

The smoothed residual uses the SAME affine measurement model as the forward
update: r_s = r_pred - H(x_s-x_pred), covariance V-H P_s H^T. This is not an
innovation and is not added to the forward chi2. Numerical failure is flagged;
no diagonal jitter or absolute-value repair is silently applied.

## Validation requirements

Schema 2 additionally records default-on reconstructed geometry material:
`interval_chord_tx0` and `interval_chord_reverse_tx0` integrate DD4hep material
along the straight segment between adjacent reconstructed hit positions, in
each direction. These are dimensionless t/X0, independent of all truth hooks.
They are NOT the exact curved KF material path and do not change its Q or mean
energy loss. Keep `truth_interval_tx0` as a separate supervision/check field.
The endpoints retain the usual sensitive-midpoint to sensitive-midpoint
convention (half layer + gap + half layer when applicable).

Both scans store `interval_chord_material_[reverse_]status`,
`interval_chord_material_[reverse_]segments` and
`interval_chord_material_[reverse_]covered_mm`. Status 1 means complete;
2 means complete after a 1-micrometre-or-smaller starting-boundary repair;
-1 means invalid/incomplete and t/X0 is NaN, never a false zero. Coverage is
checked within max(0.001 mm, 1e-6 * chord length). The repair accounts for its
removed cap using the cap-midpoint material. Opposite-direction values are
retained independently, not averaged to hide a navigation discrepancy.

Run a verbose focused event, then entries 11,16,17. Check matrix sizes, covariance
validity, accepted-hit indexing, truth loss/count ownership and before/after
boundaries. Repeat with BPID_TRUTH=0 and require every non-truth branch identical:

```bash
python Reconstruction/RecBreakpoint/tests/check_interval_identification.py on.root off.root
```

No classifier training or population claim is authorized by schema success.
Stored CompleteTracks can differ from a rerun on its accepted-hit subset: the
producer has retries, direction changes and a separate final inward continuation.
The independent backward feature pass is deliberately not that continuation.
Keep this distinction explicit when comparing endpoints.
