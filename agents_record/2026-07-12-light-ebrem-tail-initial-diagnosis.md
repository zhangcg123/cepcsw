# Expanded-sample light-eBrem tail: initial diagnosis

The four events in this record were initially chosen from the most extreme
tail. The user subsequently clarified that optimization should not spend time
on such catastrophic outliers. Their evidence remains preserved here, but the
active focus now uses representative 1--10% residual events and category-level
quantiles instead.

## Population result

Reconstruction-aligned surface ownership on 499 matched seeds gives 2045
no-eBrem, 2148 light-eBrem, and 797 hard-eBrem events. Seed 464 is absent
because its 951-byte flat tuple has no `gsf_tuple`. In the light category,
LCIO versus reverse BestBranch GSF has median residual -0.2131% versus -0.0971%,
1550 versus 1778 events inside 1%, 1973 versus 2064 inside 5%, and 20 versus
37 events beyond 10%. Thus the global result improves while a minority GSF
tail remains.

The durable category and plot tables are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/` followed
by `full_500_sample_2026-07-12/`.

## Four verbose representatives

| Seed/entry | Owned eBrem | LCIO residual | Reverse GSF residual | First obvious inner excursion |
|---|---:|---:|---:|---|
| 483/2 | 1.050% | -0.845% | +84.832% | hit 3, top pT jumps from about 1.98 to 11.23 GeV |
| 383/2 | 0.00197% | -0.124% | -58.854% | hit 6 moves to 1.83 GeV; hits 5--3 become extreme |
| 57/4 | 6.753% | -6.709% | +31.355% | hit 5 starts correction; hit 1 jumps to 2.93 GeV |
| 295/6 | 8.459% | -8.175% | +28.315% | hit 5 starts correction; hit 2 jumps to 3.51 GeV |

For most of the inward traversal, dominant components remain tightly grouped
near the outer fitted momentum. The catastrophic choices are localized to a
few innermost measurements rather than accumulated gradually. At the final IP,
the exact no-radiation branch remains present but has weights between roughly
`1e-29` and `1e-70` in these examples. This does not by itself prove that
inherited weights cause the failure: a radiative branch near the correct pT is
already dominant over most outer hits, and the innermost likelihoods choose
among radiative alternatives.

Focused controls further localize the mechanism. Reverse filtering without BH
splitting gives residuals -0.852%, -0.158%, -6.725%, and -8.191% for 483/2,
383/2, 57/4, and 295/6: it avoids the catastrophic tail but, as expected, does
not recover genuine light losses. Forward-only BH gives -41.503%, -0.169%,
-6.711%, and -8.186%; it also fails to recover the genuine 57/4 and 295/6
losses. Reverse radiative selection is therefore necessary for the observed
light-loss recovery but is also the immediate source of these overcorrections.

## Interpretation and next audit

The current evidence favors a local hypothesis-selection problem at the inner
silicon measurements, not a general failure of all 200-plus measurements. The
next audit must record, for the eventual winner and a truth-compatible
competitor, the exact pre-update weight, `delta-chi2`, `logDetS`, posterior
odds, transition child and KL contribution at each decisive hit. Only after
that decomposition can the process prior, innovation model, and reduction be
distinguished.

Mechanism-specific solution directions are: improve the physical conditioning
and variance of the near-unity/non-radiative process core if it is too narrow;
or make output/reduction use consistent unmerged-lineage evidence if KL changes
selection semantics. Do not add an event-level measurement-evidence gate,
global covariance scaling, or a truth-dependent light-category rule.
