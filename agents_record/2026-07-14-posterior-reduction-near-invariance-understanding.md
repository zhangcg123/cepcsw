# Posterior-reduction near-invariance: mechanism understanding

Date: 2026-07-14

## Scope

This record preserves the interpretation of focused reruns made after changing
the split/measurement/reduction order. It is supporting mechanism evidence and
does **not** replace or redirect the current concentration in `AGENTS.md`.

The new order updates every BH child with the target measurement and exact
innovation likelihood before cutoff and KL/TopN reduction. The runs used the
installed `RecGsfTracking`, `MaxComponents=24`, the
`CEPC2GeV85StepConditioned` model, reverse `BestBranch`, aggregate-weight
selection, and comprehensive component dumps.

## Proof that the new implementation ran

The new logs contain the forward stages `posterior-pre-reduce` and
`posterior-post-reduce`, plus `reverse-pre-reduce`. They show as many as 120
temporary children receiving the target update before reduction to at most 24.
For example, `/tmp/new-gsf-explicit-248-4.log` contains forward posterior
mixtures of 25--93 components before reduction and reports `peak-comps 120`.
These runtime signatures come from the new installed flow, not merely from a
reused tuple.

## Completed explicit comparisons

The first block uses the directly preceding current-24 audit as `GSFOLD` and
is the closest available old/new comparison. All values are pT in GeV.

| seed/event | category | Truth | LCIO | GSFOLD | NEW | observation |
|---|---|---:|---:|---:|---:|---|
| 248/4 | light | 2.0004 | 2.0045 | 2.07187 | 2.0718 | unchanged false correction |
| 233/4 | light | 2.0004 | 2.0003 | 2.06096 | 2.0608 | unchanged false correction |
| 279/0 | no-eBrem | 2.0004 | 2.0033 | 2.05940 | 2.0594 | unchanged false correction |
| 127/4 | no-eBrem | 2.0004 | 2.0010 | 2.05631 | 2.0563 | unchanged false correction |
| 101/3 | no-eBrem | 2.0004 | 2.0039 | 2.05644 | 2.0564 | unchanged false correction |
| 181/1 | no-eBrem | 2.0004 | 2.0000 | 2.05040 | 2.0491 | 1.3 MeV improvement; still false |
| 301/6 | no-eBrem | 2.0004 | 2.0010 | 2.04931 | 2.0493 | unchanged false correction |
| 266/0 | light | 2.0004 | 2.0032 | 2.05131 | 2.0513 | unchanged false correction |
| 43/7 | no-eBrem | 2.0004 | 2.0042 | 2.05050 | 2.0505 | unchanged false correction |
| 228/5 | no-eBrem | 2.0004 | 2.0101 | 2.05562 | 2.0518 | 3.8 MeV improvement; still false |
| 193/9 | light | 2.0004 | 2.0035 | 2.04650 | 2.0466 | essentially unchanged/slightly worse |
| 124/4 | no-eBrem | 2.0004 | 2.0036 | 2.04489 | 2.0405 | 4.4 MeV improvement; still false |

All twelve completed with finite full-hit tracks. Event 181/1 retained 228/228
hits but had an unusually high GSF chi2/ndf of 3309.4/448; this is flagged and
must not be hidden by the pT-only comparison.

Additional historical representatives were also rerun, but their `GSFOLD`
values came from older diagnostic workflows and are not strict one-setting
A/B references:

| seed/event | Truth | LCIO | historical GSFOLD | NEW | interpretation |
|---|---:|---:|---:|---:|---|
| 2/7 | 2.0004 | 1.8521 | 1.8538 | 1.8520 | early loss remains missed |
| 7/9 | 2.0004 | 1.7496 | 2.0047 | 1.9989 | strong recovery retained; error improves |
| 433/6 | 2.0004 | 1.9226 | 1.9224 | 1.9224 | transition-0 loss remains missed |
| 302/9 | 2.0004 | 1.9126 | 1.9126 | 1.9127 | early loss remains missed |

Seed-1 events 1/9, 1/3, and 1/5 were not rerun because
`tuples285/trk-e--2.0-85-1.root` is absent. Event 342/8 had not started when
the running sequence was interrupted. Generated ROOT files and comprehensive
logs remain disposable outputs under `/tmp/new-gsf-explicit-*`.

## Mechanism learned from the near-invariance

The new ordering is statistically preferable and removes a machinery
ambiguity, but the near-identical endpoint pT values show that premature
pre-measurement reduction was not the dominant cause of these persistent false
selections. The physically relevant radiative candidates already survived the
old flow, and evaluating every child at the target measurement still allows
the same radiative state to win.

The strongest direct example is 248/4. The old and new runs select the same
reverse process signature, including surface 6 mode g2 with retained fraction
0.975626, and the output changes by less than 0.1 MeV. Existing exact odds for
this event give a radiative-to-identity likelihood ratio about 1,193 and
posterior odds about 84:1. Other persistent examples are even stronger:
279/0 has a likelihood ratio about 1.13 million and posterior odds about
34,000:1; 124/4 has a likelihood ratio about 808 and posterior odds about
22:1. These are no-eBrem or negligible-loss tracks, so the wrong radiative
choice is being made by the calculated reverse posterior rather than being
created by deletion of an unevaluated child.

This agrees with prior controls: KL aggregation did not generally create the
decisive crossing; uniform reverse-start weights did not fix the population;
and `IdentityBroad` still allowed inner measurements to select radiation.
Forward-only filtering stays clean but cannot recover hard losses. The
remaining problem is therefore localized to reverse-direction state/likelihood
modeling, not split/merge ordering or inherited mixture composition alone.

## Current technical hypothesis and bounded next diagnostic

The evidence supports a concrete hypothesis: the exact identity/no-radiation
state is insufficiently flexible, or its reverse innovation prediction is
inconsistent, so a discrete g2/g3 radiative state acts as an artificial
curvature correction at an inner measurement. Possible contributors include
missing non-radiative process variance, reverse covariance inconsistency,
measurement correlation or information reuse, and discrete BH-mode mismatch.
The evidence does not yet distinguish these contributors and does not justify
a global covariance change or an ad hoc measurement-evidence threshold.

The next mechanism diagnostic should compare the identity and winning
radiative state at each decisive hit using the predicted residual, innovation
covariance `S`, delta-chi2, `log(det S)`, prior odds, and posterior odds. Across
topology-clean no-eBrem controls, test whether observed identity residual
variance exceeds its predicted innovation variance as a function of surface
and t/X0, after separating deterministic mean loss. Compare the same quantities
with the established overshoot/control population. This follows the active
focus rather than replacing it.

## Validation boundary

These focused reruns are causal diagnostic evidence, not category-level
performance validation. They show that the posterior-ordering change alone
does not cure the persistent false-radiative tail. No broad claim is permitted
until the required overshoot/control, ordinary light, clean, hard, full
no/light/hard, and transfer-control ladder is completed.
