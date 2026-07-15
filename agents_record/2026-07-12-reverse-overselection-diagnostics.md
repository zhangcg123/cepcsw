# Reverse radiative-overselection diagnostics

Date: 2026-07-12

## Question

The corrected eBrem-only model left 15/407 surface-owned no-eBrem events whose
reverse BestBranch pT differed from the no-BH control by more than 1%.  The
first hypothesis was that KL merging accumulated several radiative lineages
into a component that defeated the exact identity lineage.

## KL audit: hypothesis rejected

Verbose reverse-reduction logging was added and seed 23 event 8 was rerun.
At the final pre-hit-0 reduction, selected component 392 had weight 0.21890;
its dominant unmerged child already carried 0.21793 (99.6%).  Merging added
only about 0.00097.  The hit-0 likelihood then raised it to 0.376785 versus
0.291278 for the identity, publishing 2.30485 GeV for 2.00036 GeV truth.
Therefore KL weight aggregation is not the cause of this outlier and changing
BestBranch/reduction semantics would not solve it.

The lineage ancestor had weight only 0.00294 before reverse hit 5.  Hit 5 gave
it delta-chi2 17.13, versus 61.65 for the identity, causing radiative
dominance.  The durable log is `/tmp/gsf-kl-audit-23-8.log`.

## Reverse-start weight audit

A diagnostic `ReverseInitialWeightMode=Uniform` reset the retained forward
mixture weights at reverse start.  For seed 23 event 8 it reduced the output
from 2.30485 to 1.96436 GeV but still selected radiation.  The representative
10.4% hard-loss seed 1 event 3 retained its recovery: 1.99117 versus 1.99115
GeV with normal forward-posterior initialization, for 2.00036 GeV truth and
1.79826 GeV LCIO.

Across all 15 clean outliers, however, uniform initialization left every event
more than 1% from its no-BH result and changed 14 events negligibly.  Inherited
forward weights amplify one extreme case but are not the population-level
cause.

## Forward-only and retained-lineage RTS controls

Forward-only BH BestBranch matched the no-BH results within 0.052% for all 15
clean outliers, proving that their false-radiative tail is introduced by the
reverse workflow.  It did not recover the hard event: seed 1 event 3 returned
1.7980 GeV, equal to LCIO.

The existing one-pass retained-lineage RTS smoother with TopN is statistically
clean but is anchored by `KappaSeedCov=1e-7`.  It gave 1.9983 GeV on clean seed
23 event 8 and only 1.7981 GeV on the hard event.  Increasing the seed kappa
variance activated recovery but produced a clean bias:

| Kappa variance | hard pT [GeV] | clean pT [GeV] |
|---:|---:|---:|
| 1e-7 | 1.7981 | 1.9983 |
| 1e-5 | 1.8041 | 1.9893 |
| 1e-4 | 1.9262 | 1.9784 |
| 3e-4 | 1.9868 | 1.9765 |
| 1e-3 | 1.9937 | 1.9758 |
| 1e-2 | 1.9966 | not run |

There is no useful fixed covariance in this scan.  Merely loosening the LCIO
momentum seed trades hard recovery against clean bias.

## Broad single outer-seed reverse diagnostic

`ReverseSeedMode=BestBroad` starts reverse filtering from one best forward
outer state as a numerical guess, discards the forward mixture weights, and
assigns broad diagonal covariance with configurable kappa variance.  At
`ReverseKappaSeedCov=1e-3`, it succeeded on the two representative events:
clean 1.99839 GeV and hard 1.99116 GeV.  Both retained 233/233 hits, but each
had one rejected component update.

The 15-event check rejected it as a general solution.  It fixed only seed 23
event 8 and seed 9 event 1; 13/15 events still differed from no-BH by more than
1%, usually nearly unchanged from the normal reverse result.  It remains an
opt-in diagnostic and must not become the default.

`ReverseSeedMode=IdentityBroad` then seeded the inward fit from the protected
no-radiation forward state alone, with broad covariance.  It preserved the
hard seed-1/event-3 recovery at 1.99197 GeV but likewise left 13/15 selected
clean outliers more than 1% from no-BH.  For typical seed 62 event 9 it started
from one identity state and still selected 2.06522 GeV with weight 0.86594;
there were zero rejected component updates.  This rules out forward mixture
composition as the general cause.

Full per-hit IdentityBroad dumps show that identity is first overtaken mainly
at inner hits 2--9 (events occur at hits 2, 2, 2, 3, 4, 5, 5, 5, 5, 6, 7, and
9); one event changes at hit 114, while two never lose identity under this
diagnostic.  In seed 62 event 9, identity has weight 0.606 after hit 5 and
0.495 after hit 4.  At hit 3, identity has delta-chi2 17.78 and
`logDetS=-18.907`, while the moderate-loss branch has delta-chi2 5.90 and
`logDetS=-18.613`.  The resulting single-hit likelihood ratio is approximately
328:1 for the radiative branch; its posterior weight becomes 0.657 immediately
and reaches 0.866 at the IP.  The relevant preceding material interval is
`t/X0~=0.01018`, where the moderate-loss fitted prior is only a few percent.

The reusable parser is
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/scripts/`
`summarize_reverse_identity_collapse.py`.  The focused logs are under `/tmp/`,
not project records; important examples are `gsf-kl-audit-23-8.log`,
`gsf-identitybroad-clean62-full.log`, and the `gsf-rts-*` logs.

## Current conclusion

The remaining clean degradation is genuine reverse-direction radiative model
selection, not KL aggregation or inherited mixture weights.  Forward-only is
clean but cannot recover the hard IP momentum; the reverse direction is needed
but presently over-selects small radiative corrections in a minority of clean
tracks.  No measurement-evidence threshold was introduced, and WeightedMean
was neither run nor used.

The next analysis is to measure the no-eBrem total-transition residual variance
versus `t/X0` from the same ten production CSVs, after separating the
deterministic mean loss.  Determine whether the exact identity process is
unphysically narrow because ionization/non-radiative straggling is absent from
its process covariance.  Do not shift the identity mean away from `z=1`, do
not double-count deterministic `ElossOn`, and do not add a measurement-evidence
threshold.  Broad categorized reruns must wait until a focused change passes
both clean and hard representatives.
