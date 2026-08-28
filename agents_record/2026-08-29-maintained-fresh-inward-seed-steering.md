# Maintained fresh inward-seed steering

Date: 2026-08-29

## Decision

The maintained `DumpGsfTrks/gsf.py.bk` reverse campaign now explicitly sets:

```python
gsf.KappaSeedCov = -1.0
gsf.InwardSeedCovarianceScale = -1.0
gsf.ReverseInitialWeightMode = "ForwardPosterior"
```

The nonpositive inward value selects one fresh standard-KF-style backward
seed, including the explicit outermost-hit update, and starts recursion at
`N-2`. It is not a negative covariance multiplier. The same nonpositive
`KappaSeedCov` selects the standard `Var(omega)=1e-4` covariance for both the
fresh forward and fresh inward initializers.

`ReverseInitialWeightMode` is intentionally retained as complete explicit
steering but is inert in this mode: the fresh inward initializer has one root
with unit weight. It matters only for a positive inward scale, where it selects
forward-posterior or uniform weights for the copied final-forward population.

## Boundary

This is maintained campaign steering only. The compiled and active reverse-
template `InwardSeedCovarianceScale` default remains 100, and no C++ behavior,
property interface, or production-validation claim changes here. The prior
maintained value 1 and its correlated-prior rationale remain preserved in
`2026-08-29-fresh-inward-standard-kf-initialization.md`.
