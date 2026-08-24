# Full-mixture-mode endpoint

Date: 2026-08-24

## Boundary and definition

The ordinary reverse and KL-smoother workflows now publish a third,
row-aligned endpoint in addition to BestBranch and WeightedMean. The new
FullMixtureMode endpoint is the maximum of the complete final five-dimensional
Gaussian-mixture density at the interaction point in KalTest helix coordinates
`(drho, phi0, kappa, dz, tanLambda)`. It is not a component selector and is not
the previously discussed one-dimensional `q/p` marginal mode spliced into four
other summary parameters.

Every positive-weight surviving component is extrapolated to the IP. A strict
component-set preparation rejects the mode calculation if any such component
cannot provide a finite mean and positive-definite covariance. The deterministic
multistart search seeds the mixture mean, all component means, and all pairwise
weight-interpolated means. Generalized Gaussian mean-shift iterations and local
Newton refinement locate stationary maxima. The highest-density valid maximum
is published, with local covariance `[-H log p(x_mode)]^-1`.

This is a coordinate-dependent experimental posterior summary. Mechanical
availability does not establish momentum-performance improvement or make it a
production selection.

## Output contract

For `GaussianSumSmoothing=true` or `ReverseFiltering=true`, every successful
output track has three parallel EDM rows:

- `GSFTracksBestBranch`: selected component;
- `GSFTracksWeightedMean`: first two global mixture moments;
- `GSFTracksFullMixtureMode`: joint five-dimensional mixture-density maximum.

The new `GSFFullMixtureModeStatus` user-data collection is row-aligned with
`GSFTracksFullMixtureMode`:

| Code | Meaning |
|---|---|
| `1` | mode and local covariance succeeded |
| `0` | workflow not applicable; no mode collection is produced |
| `-1` | incomplete/invalid component set |
| `-2` | no valid stationary maximum found |
| `-3` | invalid local Laplace covariance |
| `-4` | method endpoint unavailable |

On a negative status, the mode collection deliberately stores an exact
BestBranch parameter/covariance fallback so row alignment is preserved. Both
WeightedMean and FullMixtureMode inherit BestBranch chi-square/NDF because
neither mixture summary has a unique branch fit quality. They share the same
tracker-hit list and do not duplicate hit-vector branches.

The flat tuple adds `fullmixture_gsf_*`,
`fullmixture_gsf_available`, `fullmixture_gsf_changed`,
`fullmixture_gsf_status`, and `res_pT_fullmixture_gsf`. The schema always
exists. It is populated only when the mode collection exists and remains
unavailable/zero for forward-only, CMS-like, and global-loss workflows.

There is deliberately no new Gaudi property. FullMixtureMode follows the
existing automatic multi-view publication contract and is default-on for
smoother/reverse. `GSFOutputMode` remains a forward-only compatibility
selector; `ReverseSelectionMode` affects only BestBranch.

## Mechanical validation

The focused EL9/LCG-105 `RecGsfTracking` and `RecGsfFlatTuple` targets built and
installed successfully. Runtime gates used the installed current worktree:

- Five reverse events from
  `/tmp/gsf_interval_definition_ab_20260823/silicon5/rec_v01.root` produced
  successful status `1` for every row. Their final component populations were
  11--12, and the deterministic search used 67--79 starts.
- One KL-smoother event from the same input produced status `1`; BestBranch,
  WeightedMean, and FullMixtureMode were exactly identical, as expected from
  the smoother's common inner mean/covariance.
- Required verbose reverse gates for selected event indices 11, 16, and 17 in
  `trk_large_20260823/trk-e--2.0-85-1.root` all completed. Every published
  first track had status `1`; event 16 also contained a second fitted input
  track whose mode search succeeded.
- EDM inspection confirmed `GSFTracksBestBranch`, `GSFTracksWeightedMean`,
  `GSFTracksFullMixtureMode`, and `GSFFullMixtureModeStatus`. Flat-tuple
  inspection confirmed the full `fullmixture_gsf_*` schema and finite values.

These checks establish build, persistence, row alignment, and deterministic
optimization on the focused sample only. They are not a population A/B or a
physics-performance validation.
