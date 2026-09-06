# Directional seed covariance scales

Date: 2026-09-06

## Decision

The curvature-specific `ForwardKappaSeedCov` and `InwardKappaSeedCov`
properties were retired. Their replacement properties are `ForwardSeed` and
`BackwardSeed`, each with compiled, active-template, and maintained-card
default `1.0`.

Both properties must be finite and positive. A value of `1` assigns the full
FullLDCTracking-style loose diagonal prefit covariance:

```text
Var(d0)        = 1e6
Var(phi)       = 1e2
Var(omega)     = 1e-4
Var(z0)        = 1e6
Var(tanLambda) = 1e2
```

Any other accepted value uniformly multiplies all five variances. There is no
longer a public omega/kappa conversion or a nonpositive sentinel. The three-
hit geometric prefit central values and explicit boundary-hit MarlinTrk update
are unchanged.

`ForwardSeed` always controls the fresh outward initializer. `BackwardSeed`
controls only the fresh backward initializer selected by
`InwardSeedCovarianceScale<=0`; it is inert when a positive inward scale
copies and scales the final forward mixture.

Old cards assigning `KappaSeedCov`, `ForwardKappaSeedCov`, or
`InwardKappaSeedCov` are intentionally rejected and must be regenerated.

## Mechanical validation

The EL9/LCG-105 `RecGsfTracking` and `RecGsfFlatTuple` targets built and
installed successfully. A comprehensive verbose run selected events 11, 16,
and 17 from the existing seed-1 diagnostic input. Every initialized outward
and fresh-backward track reported `prefitCovarianceScale=1`; the run completed
and wrote both GSF EDM and flat-tuple outputs. The selected events contained
four reconstructed input tracks. One secondary track retained the pre-existing
FullMixtureMode numerical fallback while the other endpoint publications and
the application completed normally; this is not a seed-scale validation
failure.

A focused event-11 run with `ForwardSeed=2` and `BackwardSeed=2` completed and
reported scale 2 in both initializers. A zero forward scale was rejected during
algorithm initialization with the required finite-positive diagnostic. The
installed configurable exposes 37 algorithm properties, including the two new
names and excluding both retired directional names. The complete property
reference documents all 37; the maintained card explicitly steers 36 and
deliberately inherits only `RecordTruthMaterialIntervals`.
