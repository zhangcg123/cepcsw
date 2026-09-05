# Forward output selector retirement

On 2026-09-06, `GSFOutputMode` was removed from `RecGsfTracking`, the active
reverse template, and the maintained `DumpGsfTrks/gsf.py.bk` card. The
property had selected `BestBranch` or `WeightedMean` only when both
`GaussianSumSmoothing` and `ReverseFiltering` were false and ordinary forward
GSF wrote a single `GSFTracks` collection.

The selector was introduced by commit `47678cd` before smoother and reverse
published their endpoint representations in separate collections. In the
maintained smoother/reverse workflows it was unreachable because those paths
always publish `GSFTracksBestBranch`, `GSFTracksWeightedMean`, and
`GSFTracksFullMixtureMode` together.

Ordinary forward GSF remains mechanically available. It is now fixed to its
former compiled and maintained-card default: the maximum-normalized-weight
final component is extrapolated to the IP and written to `GSFTracks`. It no
longer offers a moment-matched single-collection endpoint. The shared
`weightedMixtureAtIP` implementation remains required for the automatic
smoother/reverse `GSFTracksWeightedMean` collection.

The resulting public surface has 38 `RecGsfTracking` properties. The
maintained card explicitly steers 37 and deliberately inherits only
`RecordTruthMaterialIntervals=true`. The option audit found 186 untracked
generated historical cards with 286 `GSFOutputMode="BestBranch"` assignments.
They were not modified and must be regenerated rather than edited in place.

## Validation gate

- `RecGsfTracking` and `RecGsfFlatTuple` built and installed successfully in
  the EL9/LCG-105 development build.
- A card that assigns `GSFOutputMode` is rejected because the Gaudi property
  no longer exists.
- The ordinary-forward workflow was run before and after the removal on the
  same first 18 entries with forward BH splitting enabled. All 18 rows were
  bit-identical for `gsf_pT`, `gsf_p`, all five helix parameters, eta/theta,
  chi-square, NDF, track type, hit counts, and the truth-relative pT residual.
- Zero-based events 11, 16, and 17 completed with comprehensive verbose
  component dumps after the removal.

These gates establish mechanical equivalence to the former
`GSFOutputMode=BestBranch` behavior only, not new physics validation.
