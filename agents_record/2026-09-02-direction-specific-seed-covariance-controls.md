# Direction-specific GSF seed-curvature controls

Date: 2026-09-02

## Motivation

The outward and fresh-inward GSF filters use independent three-hit geometric
prefits at opposite track boundaries. A single `KappaSeedCov` property could
not test whether excess uncertainty originates in the forward message, the
backward message, or both. The controls were therefore separated without
changing either initializer's geometry or boundary-hit update.

## Interface

- `ForwardKappaSeedCov=-1` controls the three-innermost-hit outward seed.
- `InwardKappaSeedCov=-1` controls the three-outermost-hit fresh inward seed.
- A finite value at or below zero preserves the standard
  `Var(omega)=1e-4` covariance.
- A finite positive value is a direct `Var(kappa)` override and is converted
  internally to `Var(omega)=Var(kappa)*alpha^2` before pivot transport.
- `InwardKappaSeedCov` is inert when `InwardSeedCovarianceScale>0`, because
  that mode copies and scales the final forward component population rather
  than constructing a fresh inward seed.

For generated cards predating this change, deprecated `KappaSeedCov` remains
as an explicit compatibility alias. Its compiled default is zero (disabled).
A nonzero value applies to both direction-local initializers only if both new
properties retain `-1`; mixing the alias with an explicitly changed
directional property is rejected during initialization. New cards must set
the two directional properties and leave the alias at zero.

## Scope and interpretation

Only the curvature covariance entry is changed. The direction-local hit
choice, four other loose covariance entries, pivot transport, and explicit
boundary-hit MarlinTrk update remain unchanged. Tightening either value is a
diagnostic control, not a validated default change. Same-code event and
population comparisons are required before making any physics claim.

The option-surface audit covered the declaration, finite-value validation,
both initializer call sites, the authoritative package README, the maintained
`DumpGsfTrks/gsf.py.bk` card, workflow documentation, and live project status.
Historical generated cards and dated records were intentionally not rewritten.

## Verification and focused evidence

The EL9/LCG 105 `RecGsfTracking` and `RecGsfFlatTuple` targets built and the
install completed. Generated configuration reports 41 properties and the
compiled values `ForwardKappaSeedCov=-1`, `InwardKappaSeedCov=-1`, and
`KappaSeedCov=0`.

A same-code, double-BH-off, fresh-inward, `LocalMeasurement` comparison used
199 topology-clear events. The standard seed corresponds to approximately
`Var(kappa)=123.6` at the tested 3 T field; the tight diagnostic used
`Var(kappa)=1`, separately in each direction. At the passive 5D directional
score threshold 0.95:

| Seed setting | Null false positives / 45,415 | Type-I | Missed losses >=0.2% / 96 | Type-II |
|---|---:|---:|---:|---:|
| standard forward, standard inward | 4,813 | 10.598% | 61 | 63.542% |
| tight forward only | 4,817 | 10.607% | 60 | 62.500% |
| tight inward only | 4,855 | 10.690% | 60 | 62.500% |

Forward-only tightening cannot alter the fresh-inward reverse endpoint; all
199 endpoint pT values were identical. Inward-only tightening changed endpoint
pT by a median 0.00010 GeV and at most 0.00231 GeV. On hard-loss events 11,
16, and 17, independently tightening either seed changed the maximum passive
score by no more than about `7e-5` and the endpoint by no more than 0.0002 GeV.
The standard `-1/-1` rerun reproduced the pre-change event-11/16/17 endpoint
payload exactly: zero mismatches in 192 comparisons across 64 BestBranch,
WeightedMean, FullMixtureMode, and final-mixture-component branches.

This is not evidence for a tighter default. A factor-123 variance reduction
mainly disappears after the subsequent measurement updates. The few threshold
crossings are too small and correlated to support a physics claim, while
inward tightening slightly increases false positives. Seed covariance is
therefore retained as an independent diagnostic axis; it does not resolve the
poor brem-identification power of the current F/B score.
