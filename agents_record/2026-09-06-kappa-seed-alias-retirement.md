# Common kappa-seed covariance alias retirement

On 2026-09-06, the deprecated common `KappaSeedCov` property was removed from
`RecGsfTracking`. It had provided no independent algorithm behavior: any
nonzero value copied the same scalar into the effective forward and inward
kappa-seed covariance controls, and zero disabled the alias. It also rejected
cards that combined a nonzero alias with either non-default directional
property.

The only supported controls are now `ForwardKappaSeedCov` and
`InwardKappaSeedCov`. Each finite value at or below zero selects the standard
`Var(omega)=1e-4`; a positive value is interpreted as `Var(kappa)` for that
direction. The outward initializer always consumes the forward value. The
inward value is consumed only by a fresh reverse seed selected with
`InwardSeedCovarianceScale<=0`; a positive scale copies and scales the final
forward covariance instead.

The active reverse template now exposes independent
`GSF_FORWARD_KAPPA_SEED_COV` and `GSF_INWARD_KAPPA_SEED_COV` environment
controls, both defaulting to `-1`. The maintained `DumpGsfTrks/gsf.py.bk`
card retains the same explicit directional values and no common alias. Two
tracked legacy focused cards that had selected `KappaSeedCov=1e-7` now assign
`1e-7` to both directional properties, preserving their former effective
initialization.

The resulting public surface has 37 `RecGsfTracking` properties. The
maintained card explicitly steers 36 and deliberately inherits only
`RecordTruthMaterialIntervals=true`. The option audit found 186 untracked
generated cards with 186 alias assignments: 86 use `0.0` and 100 use `1e-7`.
They were not modified and must be regenerated or migrated to the two
directional properties. Seventeen dated records retain intentional historical
mentions.

## Validation gate

- `RecGsfTracking` and `RecGsfFlatTuple` built and installed successfully in
  the EL9/LCG-105 development build.
- A card that assigns `KappaSeedCov` is rejected because the Gaudi property no
  longer exists.
- A same-input 18-entry reverse A/B run used a fresh inward seed and enabled
  both forward and inward BH splitting. The old alias value `-1` and new
  directional values `-1/-1` were bit-identical for every persisted
  BestBranch, WeightedMean, and FullMixtureMode endpoint field, including
  availability/status, five helix parameters, pT/p, eta/theta, chi-square,
  NDF, type, and hit count.
- Zero-based events 11, 16, and 17 completed with comprehensive verbose
  component dumps. Both initializers reported `Var(omega)=1e-4` and
  `Var(kappa)=123.627784` at 3 T.

These gates establish mechanical equivalence only, not new physics
validation.
