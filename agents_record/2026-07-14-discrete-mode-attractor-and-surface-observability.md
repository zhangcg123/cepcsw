# Discrete radiative-mode attractor and surface-dependent observability

Date: 2026-07-14

## Scope

This record preserves a mechanism observation made from explicit old/new pT
comparisons after posterior-order reduction was implemented. It supports the
existing light-tail concentration and does not replace or redirect the current
focus in `AGENTS.md`.

## Observation

Within the selected pathological no-eBrem and light-eBrem cases whose LCIO pT
is already close to the approximately 2.0004 GeV truth, reverse GSF repeatedly
moves the result from about 2.00 GeV to a narrow band near 2.05 GeV. This is
not a generic change affecting every truth-like LCIO event: in the stored
inclusive no/light table, 3,322 events have LCIO residual within +/-0.5%,
3,213 remain within +/-1% after GSF, 61 move above +2%, and 43 lie in the
+2% to +4% GSF band. The focused pathology list deliberately concentrates
this minority behavior.

The repeated scale has a direct process interpretation. Eleven of the twelve
explicit pathology reruns contain a selected reverse `g2` mode, usually at
surfaces 5--8, with retained fraction about 0.975--0.977. The characteristic
inverse correction is

```text
2.00 GeV / 0.975 ~= 2.051 GeV.
```

Examples include 248/4 (`6:g2:f0.975626`, NEW pT 2.0718 GeV), 279/0
(`7:g2:f0.975838`, 2.0594 GeV), 127/4 (`6:g2:f0.975579`, 2.0563 GeV),
101/3 (`5:g2:f0.975096`, 2.0564 GeV), 301/6
(`6:g2:f0.975855`, 2.0493 GeV), 266/0 (`5:g2:f0.975314`, 2.0513 GeV),
228/5 (`8:g2:f0.975839`, 2.0518 GeV), 193/9
(`8:g2:f0.975750`, 2.0466 GeV), and 124/4
(`6:g2:f0.975852`, 2.0405 GeV). Transport, other lineage content, and the
starting state prevent an exact eventwise `pT/f` equality, but the repeated
mode and scale identify a discrete radiative-mode attractor rather than a
continuous correction.

## Surface, not LCIO deficit, organizes recovery

The explicit underestimated events separate by true loss surface rather than
by the size of the LCIO deficit:

| seed/event | principal truth transition | Truth pT | LCIO pT | NEW pT | outcome |
|---|---:|---:|---:|---:|---|
| 7/9 | 8 | 2.0004 | 1.7496 | 1.9989 | strong recovery |
| 2/7 | 4 | 2.0004 | 1.8521 | 1.8520 | missed |
| 302/9 | 4 | 2.0004 | 1.9126 | 1.9127 | missed |
| 433/6 | 0 | 2.0004 | 1.9226 | 1.9224 | missed |

Thus a larger LCIO underestimate can be recovered nearly perfectly when the
loss occurs at an informative surface, while smaller deficits remain
unrecoverable when the loss occurs at transitions 0--4. This agrees with the
existing population layer audit: transitions 0--4 are predominantly
information-limited, 5--6 form the boundary, and 7--11 have strong recovery.

## Integrated mechanism understanding

The reverse workflow behaves as a discrete process-hypothesis selector, not a
continuous energy-loss estimator. Two distinct limitations must not be
conflated:

1. **Early-loss non-recovery:** a suitable process component can exist, but
   transitions 0--4 have inadequate inward curvature leverage, so the
   identity/under-correcting hypothesis remains dominant.
2. **Truth-like false correction:** at informative inner surfaces, a
   measurement fluctuation or state/innovation mismatch can make the discrete
   moderate-loss `g2` hypothesis defeat identity, producing the characteristic
   roughly +2.5% mode-locked correction near 2.05 GeV.

The near-invariance under posterior-order reduction shows that this attractor
is not primarily created by deleting children before their target
measurement. Relevant `g2` candidates already survived and receive genuinely
larger calculated posterior support. Prior KL, uniform-start, and
`IdentityBroad` controls likewise argue against KL aggregation or inherited
forward weights as the general cause. The remaining candidates include
identity process/innovation under-dispersion, reverse covariance inconsistency,
measurement correlation or information reuse, and discrete BH-mode mismatch.

## Bounded diagnostic

For each event, compare `NEW/LCIO` with the inverse retained fraction of the
selected process mode and tabulate truth-loss surface/fraction against selected
surface/mode/fraction. At the decisive hit, compare identity and winning
radiative predicted residuals, innovation covariance, delta-chi2,
`log(det S)`, prior odds, and posterior odds. Across topology-clean no-eBrem
controls, test whether the observed identity residual variance exceeds its
predicted innovation variance versus surface and t/X0. Do not replace this
mechanism test with an ad hoc evidence threshold or global covariance tuning.

## New-flow random-100 test

A rerun of the fixed uniform 100-event topology-clean light sample (sampling
seed `20260713`) completed with the new posterior-order flow, aggregate-weight
selection, and `MaxComponents=24`. The population contains 68
`truth_like_lcio_preserved`, 17 `good_recovery`, 8 `missed_recovery`, 5
`truth_like_lcio_degraded`, and 2 `overshoot` events from 87 input files. The
direct reference is the preceding old-order 24-component result on exactly the
same event IDs.

All 100 selected events produced finite tuples. Every new GSF hit count agrees
with its old-24 counterpart; there are zero hit-count mismatches.

| metric | old-order 24 | posterior-order 24 |
|---|---:|---:|
| median residual | -0.06311% | -0.06311% |
| central-68 half-width | 0.34634% | 0.35095% |
| mean absolute residual | 0.61103% | 0.60989% |
| RMS | 1.22811% | 1.22701% |
| inside +/-1% | 83/100 | 83/100 |
| inside +/-2% | 93/100 | 93/100 |
| inside +/-5% | 98/100 | 98/100 |
| inside +/-10% | 100/100 | 100/100 |

Only 2/100 signed residuals change by more than 0.1 percentage point. Event
328/4 has 0.0381% owned loss and changes from Truth/LCIO/OLD/NEW pT of
2.00036/1.98329/1.98252/2.01423 GeV. Its absolute error improves by 0.199
point while crossing from -0.892% to +0.693%. Event 357/2 has 0.0729% owned
loss and changes from 2.00036/1.99529/2.00333/1.99548 GeV; its absolute error
worsens by 0.095 point while crossing from +0.149% to -0.244%. Under the
predefined 0.1-point absolute-error criterion this is one improvement, zero
worsenings, and one signed change of similar absolute quality.

Among the 65 sampled events with LCIO residual within +/-0.5%, exactly one lies
in the +2% to +4% GSF band before the ordering change and exactly one after it.
The known sampled false-selection boundary 284/1 is nearly invariant:
Truth/LCIO/OLD/NEW is 2.00036/1.98322/2.02748/2.02747 GeV. The sampled genuine
recovery 404/8 is also stable at
2.00036/1.97097/2.00009/1.99996 GeV.

This population result strengthens the focused conclusion. Updating all
children before reduction is a cleaner statistical construction but is nearly
neutral on the existing 24-component population: it neither removes nor grows
the sampled 2.05-GeV attractor, preserves the demonstrated recovery, and
changes only two low-loss boundary events materially in signed residual. The
dominant remaining issue is the reverse posterior's discrete state/likelihood
choice and its surface-dependent information, not the old reduction order.

Disposable jobs and comparison tables are under
`/tmp/gsf-random-light100-posterior-order-max24` and
`/tmp/gsf-random-light100-posterior-order-analysis`. The durable comparison
helper is
`Reconstruction/RecGsfTracking/scripts/compare_random_light_posterior_order.py`;
it pairs tuples by the original event index and verifies hit counts.

This random light sample is diagnostic, not broad validation. It contains only
two overshoots and does not replace the prescribed overshoot/control, clean,
hard, full-category, or transfer checks.
