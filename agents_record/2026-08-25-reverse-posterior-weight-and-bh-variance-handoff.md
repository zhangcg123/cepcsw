# Reverse posterior-weight and BH-variance handoff

Date: 2026-08-25

## Next-session objective

The next session starts from two connected reverse-GSF design questions:

1. Persist the posterior-weight evolution for every evaluated reverse
   component, rather than only the final surviving mixture.
2. Make the within-component Bethe-Heitler (BH) uncertainty injected into the
   prediction covariance explicitly scalable, then determine whether that
   uncertainty is obscuring measurement discrimination or protecting a wrong
   branch.

No implementation was made before the disconnect. The production baseline and
the newly added default-on final-mixture component record remain unchanged.

## Confirmed current mechanics

The ordinary reverse pass starts from the normalized final forward-posterior
weights unless `ReverseInitialWeightMode=Uniform` is selected. At a material
split, child `k` receives

```text
child_prior_weight = parent_weight * BH_mode_weight[k]
```

At the next accepted inward measurement, the current code uses the exact
innovation result from the baseline MarlinTrk update:

```text
log_unnormalized_posterior_i =
    log(prior_weight_i)
    - 0.5 * (local_dchi2_i + log(det(S_i)))

S_i = H_i * predicted_covariance_i * H_i^T + R_i
```

The common measurement-dimension `log(2*pi)` term cancels during
normalization. The posterior is normalized across all accepted components,
then the weight cutoff and KL reduction are applied. Consequently, local
chi-square is only one part of the weight update; the prediction-covariance
volume also contributes through `log(det(S))`.

For retained momentum fraction `z` with one BH-mode variance `Var(z)`, the
current first-order covariance mapping is

```text
forward: kappa_after  = kappa_before / mean(z)
         Q_BH(kappa)  = kappa_before^2 * Var(z) / mean(z)^4

reverse: kappa_before = mean(z) * kappa_after
         Q_BH(kappa)  = kappa_after^2 * Var(z)

P_new = F * P_old * F^T + Q_BH
```

The track-state covariance is stored in `omega = alpha*kappa`, so the code
adds `alpha^2 * Q_BH(kappa)` to the omega variance. Cross-covariances involving
curvature receive the deterministic Jacobian scaling. The BH mode spread is
kept as separate components until a KL merge; it is not part of this
within-mode `Q_BH` term.

This is a first-order delta-method approximation, not an exact transformed
moment calculation. It omits second-order terms such as
`Var(kappa)*Var(z)` in the reverse product and uses `1/mean(z)` rather than
`E[1/z]` in the nonlinear forward mapping.

## Requirement 1: record every posterior evaluation

The record must make one reverse measurement decision reconstructible. A
component weight without its provenance cannot determine whether the branch
won because of the residual, an enlarged prediction covariance, its BH prior,
or earlier accumulated evidence. The proposed minimum per-evaluation fields
for review are:

- event, input-track, output-track, reverse hit/interval, and matched-surface
  indices;
- component, parent, and BH mode/lineage identifiers;
- accepted/rejected update status and later cutoff/merge fate;
- normalized prior weight and BH mode weight;
- exact local `dchi2`, `logDetInnovation`, and therefore unnormalized log
  posterior;
- normalized posterior weight before cutoff and KL reduction;
- component kappa and its predicted kappa variance before the measurement.

The conceptual requirement is every evaluated reverse component, including
components that are later removed. For failed measurement evaluations there
is no finite posterior; they must still receive an explicit rejection status
rather than disappearing from the audit.

The exact persistence layout and default need review before implementation.
Recording every component at every hit can be much larger than the existing
final flat tuple. The implementation must not silently truncate the population
needed to explain a crossover. It should remain package-local and must not
change the live weights or component lifecycle.

## Requirement 2: scale only injected BH process variance

The hypothesis is that a broad within-mode `Q_BH` changes both terms of the
measurement likelihood:

```text
larger S -> potentially smaller local_dchi2
larger S -> larger log(det(S)) penalty
```

The net effect is residual- and component-dependent. It can flatten
discrimination, preserve an incorrect broad component, or conversely penalize
it; this must be measured from the complete posterior evolution rather than
inferred from final weights.

The clean experimental control is a scale on the added within-mode variance:

```text
P_new = F * P_old * F^T + bh_variance_scale * Q_BH
```

For an explicitly named variance scale, `0` removes only the added within-mode
BH variance and `1` exactly reproduces current behavior. It must not change:

- the BH component means;
- the BH component prior weights;
- the number of BH modes;
- the deterministic `F * P * F^T` transformation;
- multiple-scattering or measurement covariance;
- final selection or publication rules.

If the eventual property is instead described as a sigma scale, the variance
term must be multiplied by its square; the naming/semantics must be settled
before coding. Any new property triggers the project-law requirement for a
dedicated option-surface sub-agent and synchronized updates to the package
README and maintained `DumpGsfTrks/gsf.py.bk`.

## Ordered restart

1. Freeze and print the exact reverse baseline used for the comparison.
2. Review the complete per-evaluation record schema and storage-volume
   boundary before source changes.
3. Implement and mechanically validate passive posterior recording first.
4. Use the record to identify, in bad and matched good controls, the first hit
   where branch posterior ordering changes and decompose the change into
   prior, `dchi2`, and `logDetS` contributions.
5. Only then add the explicit within-mode BH variance scale, with `1` as the
   unchanged reference, and run same-code values such as `0`, an intermediate
   control, and `1` on the same events.
6. Apply the focused verbose gate, hard events 11/16/17, clean-track controls,
   and held-out population checks before proposing any default change.

Success means demonstrating that the scale predicts and corrects a specific
posterior crossover without sacrificing the no-eBrem core. If posterior
ordering is insensitive to the injected variance, retain scale `1`, record the
closure, and continue downstream rather than tuning it.

Current non-goals are changing the production default, modifying source before
review, changing BH means/weights or material paths, tuning cutoff/KL/capacity
or reverse seed covariance, and changing code outside
`Reconstruction/RecGsfTracking`.
