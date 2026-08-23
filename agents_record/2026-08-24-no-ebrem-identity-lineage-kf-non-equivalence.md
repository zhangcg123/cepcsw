# No-eBrem identity-lineage and baseline-KF non-equivalence

Date: 2026-08-24

## Scope

This record preserves a side investigation that is important for clean-track
preservation but does not replace the active material/BH-consistency focus in
`AGENTS.md`.  No source change was made for this conclusion.

The question was whether a reverse GSF track should reproduce the LCIO
`CompleteTracks` result when the selected history is the no-radiation identity
lineage, especially when the default-off truth BH-loss oracle supplies the
exact no-loss response `z=1` in an event with no Geant4 eBrem.

## Empirical evidence

A current-workflow comparison selected the first 100 topology-clear events in
source order after requiring valid truth/material scopes and, for every
nonzero Geant4 eBrem interval, a valid above-threshold BH opportunity in both
the forward and reverse passes.  The sample contained 48 no-eBrem, 40 light-
eBrem, and 12 hard-eBrem events.

For the 48 no-eBrem events, truth override was slightly worse than the LCIO
baseline even though it prevented the most conspicuous false radiative mode of
ordinary reverse GSF:

| Metric | LCIO | Reverse truth override |
|---|---:|---:|
| Median absolute pT residual | 0.1158% | 0.1554% |
| 68% absolute quantile | 0.1678% | 0.2233% |
| Width68 | 0.1656% | 0.2333% |
| Within 1% | 46/48 | 45/48 |

Fourteen events improved relative to LCIO and 34 worsened.  Two events had
pre-existing catastrophic `CompleteTracks`; removing only those two left LCIO
mean absolute residual 0.1367% versus 0.1901% for truth override, so the clean-
core conclusion did not depend on those tails.

A focused current-code no-eBrem control (seed 1, zero-based entry 0) isolated
the mechanism:

- 20 truth-oracle calls all used `z=1`;
- exactly one no-radiation component existed throughout;
- no KL reduction or branch ambiguity occurred;
- all 232 reverse measurement updates were accepted;
- LCIO AtIP pT was 35.5999 GeV;
- forward GSF pT was 35.5930 GeV;
- reverse truth-override AtIP pT was 35.6091 GeV.

The difference therefore exists without real eBrem, mixture selection, or
rejected hits.

Ordinary reverse and truth-override pT were numerically equal in 15 of the 48
no-eBrem events.  That equality does **not** establish that ordinary reverse
published the identity lineage: the flat tuple records the final track state,
not the selected lineage signature.  A component-history dump is required to
distinguish identity survival, identity posterior rank, and identity
publication.  Seed 1 entry 41 is an explicit ordinary-reverse false-radiation
failure: LCIO residual -0.2623%, ordinary reverse +79.4183%, and truth override
-0.3230%.

## Technical conclusion

`IdentityLineage` means a no-radiation process history.  It does not mean a
copy of the input LCIO state, and in the current implementation it is not a
baseline-equivalent KF history.

The non-equivalence is structural:

1. The forward GSF takes the mean from `CompleteTracks::AtFirstHit`, discards
   its fitted covariance, constructs a custom diagonal seed covariance, and
   performs fresh hit-by-hit `initialise -> addAndFit` updates.  The input
   `AtFirstHit` can itself be a state from the already completed/smoothed
   baseline fit, so it is not the original baseline KF prior.
2. The reverse method is an independent correlated refit, not the baseline
   backward smoother.  It starts from the final forward GSF posterior, scales
   its full covariance by `ReverseKappaSeedCov=100`, and processes the hits
   inward again.
3. The reverse output is the newly fitted selected branch after the GSF's own
   IP extrapolation.  It is not the baseline fitter's `CompleteTracks::AtIP`
   state.
4. The truth oracle replaces only the already executed BH response.  For
   `z=1`, `splitWithRetainedFraction` uses one unit-weight component with mean
   one and a variance floor of `1e-12`.  The mean operation is an identity, but
   the complete refit is not.  The variance-floor effect is far too small to
   explain the observed clean-core broadening.
5. Multiple scattering, deterministic energy loss, measurement fluctuations,
   transport covariance, hit handling, and reference-state construction
   remain active.  Supplying exact eBrem loss does not supply the true track
   state or force the fitted state to generator truth.

Consequently, selecting an identity lineage can remove a false radiative
history while still producing a pT different from LCIO.  A numerical
coincidence is possible, but equality is not an invariant of the current
algorithm.

## Relation to the implemented smoother

`GaussianSumSmoothing` is a real component-aware, KL-reduction-aware smoothing
workflow and is distinct from the independent reverse refit.  In the strict
single-Gaussian limit, a correct RTS-style smoother can reduce to a standard
Kalman smoother **if** its forward filtered/predicted states, covariances,
transport, measurements, process noise, accepted hits, and reference-state
construction are identical to the baseline.

The current GSF smoother cannot by itself establish LCIO equivalence because
it smooths the non-equivalent GSF forward graph described above.  In the
maintained smoother workflow it also publishes a moment-matched
`WeightedMean`; with real BH calls that output can mix identity and radiative
components even if identity has the largest individual weight.

Thus the smoother is a suitable mathematical foundation for a regenerated
baseline-equivalent identity solution, but only after the complete
one-component forward chain is made baseline-equivalent.

## Possible regeneration path without copying LCIO output

If the requirement is that GSF itself regenerate the LCIO solution instead of
copying `CompleteTracks::AtIP`, first build a default-off one-component
KF-equivalence control inside `RecGsfTracking`:

1. use the same raw, unsmoothed baseline seed mean and full covariance;
2. use the identical particle hypothesis, hit order, accepted-hit/outlier
   policy, measurement update, material model, and process noise;
3. disable BH branching and component reduction, or use an exact zero-variance
   `z=1` transition for the diagnostic;
4. store and compare every predicted, filtered, and smoothed state and
   covariance against the baseline fitter;
5. use the same smoothing and IP-reference construction;
6. require event-by-event closure of track parameters, covariance, chi2, NDF,
   and accepted hits before adding radiative alternatives;
7. once closure is established, keep the baseline-equivalent identity
   component isolated from radiative KL merging while the radiative histories
   evolve in parallel.

The baseline MarlinTrk fitting interface should be reused rather than
hand-coding a parallel Kalman update.  Current steering changes such as
`z=1`, `ReverseKappaSeedCov=1`, one component, or disabled KL reduction are not
sufficient on their own because they do not make initialization, forward
filtering, smoothing, and IP construction equivalent.

This is a design conclusion and proposed validation control, not authorization
to change the production source or the active reverse default.

## Related records

- `agents_record/2026-08-21-truth-bh-loss-oracle-control.md`
- `agents_record/2026-08-22-passive-truth-material-interval-final-tuple.md`
- `agents_record/2026-08-22-truth-bh-invalid-scope-fallback-and-validity-tag.md`
- `agents_record/2026-08-22-global-one-loss-expanded-event-gate.md`
