# Independent FSR-only H_F reference — 2026-11-13

User requested the fourth hypothesis: floating FSR with no material breakpoint.
Only RecBreakpoint implementation, its existing maintained card comments,
focused test and documentation changed. No shared tracking, simulation, PFA,
GSF, public configurable or default property value changed.

## Fit and output contract

The existing ExperimentalBeamSpotFsrLoss/BP_BEAM_SPOT_FSR_LOSS switch (default
ON) now also books fsr_only_110_* and attempts H_F for every processed selected
track before truth-location matching. It clears intervals and per-interval
prior centers and directly calls the existing zero-material FSR fitter. There
is no loss coordinate, material-prior optimization or location scan. The native
5D tracker/beam fit profiles one nonnegative ECAL excess and exports its own
post-FSR electron plus collinear massless photon and full joint covariance.
No pre-FSR state is constructed. A guard rejects a returned material breakpoint
or persistent 6D material state. It does not depend on truth hooks to fit.

The serializer is shared with the existing four FSR families; their branch
names, values and statuses are preserved. New H_F has the same19 suffixes,
plus ecal_observed_energy/error, resolved_ecal_sum_applied and
measurement_dimensions. Status2/mask3 is valid, -2 explicit failure, 0 not
attempted. Missing observations do not inherit another fit. The existing FSR
switch OFF omits all FSR branches including H_F; independent H0 remains.

H_F follows H0's configured observation: ElectronPhoton with collected neutrals
uses C+N with quadrature uncertainty; ElectronOnly uses C. No separate neutral
update or extra neutral p4. Existing H_betaF110 unconditionally includes N even
in ElectronOnly mode, so that nondefault case is not a common-observation
four-way comparison. No existing physics routing was changed to hide this.

The available model grid is H0 (no material/no FSR), H_F (no material/FSR),
H_beta (material/no FSR) and H_betaF (material/FSR). No score penalty, threshold
or automatic model choice is implemented. Fits must have compatible observations
and successful states before score comparison. FSR means effective collected
excess, not truth photon energy; wrong hypotheses can absorb material loss or
contamination. Local affine conditioning/closure limits remain; no new nonlinear
relinearization or physics-validation claim.

## Verification

Artifacts: /tmp/bp-fsr-only-20261113/ (uncommitted).
19 compiled RecBreakpoint tests pass, including46 FSR profiling/conditioning
numerical cases. The expanded FsrFamilyTupleTest checks independent family
persistence, covariance sizes, fitted/reset/failed statuses. All65 batch-helper
tests pass; card syntax and git diff whitespace checks pass.

Native same-input A/B uses the retained REC_ECALTRUTH inputs only: source1
selected events3,15,112; source2 event135; source4 event1. Five unique Higgs
events/ten selected electron tracks, not historical particle-gun event labels.
Eight paired configurations cover normal Truth, Manual empty material, explicit
ElectronOnly, missing ECAL, missing beam and FSR OFF. Verbose dumps include every
H_F smoothed5D mean and covariance. Old/native baseline is installed H0 plugin
SHA a2f3eaf4a87209cc739cdd18f866416c361a993eb98cbba907f6b547af9e2272;
new private plugin loading is proven by /proc child maps and SHA metadata.

All 300533 old main-tree and 1260 old side-tree field values agree exactly (NaN-aware), including invalid old fits. The two Manual-empty rows reproduce all19 old zero-material FSR family fields exactly; H_F also agrees exactly between Truth-with-material and Manual-empty for those same tracks. 1 H_F row survives the known truth-hook association failure.

Native checks additionally verify nonnegative F/variance, boundary-zero and
positive solutions, finite5/4/36/64 parameter/p4/covariance vectors, massless
collinear photon p4, electron mass, matching H0 observation routing/dimensions,
no material closure and valid local closure. Missing beam/ECAL fails cleanly;
unselected rows reset and FSR-OFF branches are absent.

Exact configuration counts:

```json
{
  "core2": {
    "unselected": 15,
    "valid": 2,
    "charged_only": 2,
    "positive_fsr": 2
  },
  "core1": {
    "unselected": 72,
    "valid": 6,
    "charged_only": 3,
    "boundary_zero": 3,
    "sum": 3,
    "positive_fsr": 3
  },
  "truth_failure": {
    "unselected": 38,
    "valid": 2,
    "charged_only": 1,
    "boundary_zero": 1,
    "despite_truth_failure": 1,
    "sum": 1,
    "positive_fsr": 1
  },
  "empty": {
    "unselected": 30,
    "valid": 2,
    "charged_only": 1,
    "boundary_zero": 1,
    "sum": 1,
    "positive_fsr": 1
  },
  "electron_only": {
    "unselected": 30,
    "valid": 2,
    "charged_only": 2,
    "boundary_zero": 1,
    "positive_fsr": 1
  },
  "missing_ecal": {
    "unselected": 30,
    "missing_observation": 2
  },
  "missing_beam": {
    "unselected": 30,
    "missing_observation": 2
  },
  "fsr_off": {}
}
```

## Installation and preserved production

All500 existing H0-campaign jobs had completion markers before installation;
no campaign was submitted, rerun or overwritten. Only the RecBreakpoint package
was installed using its cmake_install.cmake; five previous install files were
saved under the artifact rollback directory. Installed plugin SHA: `ba82a23202fa11e75928a8e245a5396b03e8678ae1e5734b45831a6d260d2bcd`.
A default-install source2/event135 rerun reproduces the private test output in
both trees exactly; actual loaded path is proven in its native metadata.
Existing500 H0 outputs do not gain H_F without another BP rerun. REC inputs,
old tuples and the plot gallery were not modified.

The historical particle-gun hard-loss11/16/17 fixtures remain unavailable.
This is focused mechanical regression, not population momentum/recoil validation
or a calibrated four-hypothesis selection. No performance improvement is claimed.
