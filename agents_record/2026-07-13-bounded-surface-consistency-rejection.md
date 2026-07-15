# Bounded surface-consistency selection rejection

Date: 2026-07-13

## Fixed hypothesis

The tested final-component coincidence was

```text
C(c) = 1 - product_s (1 - F_s R_s)
L(c) = 0.05 + 0.95 C(c)
score(c) = aggregate_weight(c) L(c),
```

where `F_s` and `R_s` are aggregate forward and reverse radiative masses at
surface `s`. `C` is a noisy-OR probability under an explicit per-surface
independence approximation. It retains absolute mass scale and therefore
avoids the normalized-cosine tiny-forward-mass failure in 469/6. The fixed
0.05 uninformative floor caps the surface-consistency Bayes factor at 20, so a
component below 5% of the aggregate-weight winner cannot be promoted. This was
a structural no-vanishing-branch bound, not a residual-fit threshold.

## Implementation and reproducibility

A default-off `ReverseSelectionMode=SurfaceConsistency` was implemented only
inside `Reconstruction/RecGsfTracking`. It automatically propagates the
existing aggregate surface/mode fractions and changes only final reverse
component ranking. It does not change BH splitting, measurement likelihood,
cutoff, KL reduction, states, or covariances. The normal default remains
`AggregateWeight`.

Reproducible additions are:

- `scripts/analyze_surface_lineage_consistency.py`: noisy-OR coincidence;
- `scripts/screen_surface_lineage_selection.py`: bounded offline ranking;
- `scripts/compare_selection_direct_ab.py`: same-code tuple A/B comparison;
- filtered explicit-event support in `scripts/run_reverse_selection_sample.py`.

The target built and installed successfully. Python analysis scripts pass
`py_compile`.

## Focused gates

The extracted 19-overshoot/18-control screen predicted two overshoot changes
and no control changes:

- 74/0: pT 2.02256 -> 2.00172 GeV;
- 310/8: pT 2.02210 -> 1.99180 GeV.

All 37 events were rerun verbosely. Online and offline component IDs and pT
agreed exactly. Against the prior surface-mass diagnostic, every final
component count and all 101,713 accepted plus 33 rejected reverse candidate
updates were identical. Only 74/0 and 310/8 changed publication.

The ordinary light representatives 2/7, 234/4, 309/6, 299/7, and 463/7, plus
clean 62/9 and hard 1/3, retained their default states and update counts.
Exact-pair event 11 completed 234/234 hits at about 1.984 GeV. Events 16 and 17
remain unavailable because `/tmp/gsf-match-tracks.root` ends after event 11.

## Full light/clean screen and baseline correction

All 2,132 topology-clean light and 2,032 topology-clean no-eBrem events
completed. Comparison to the stored outcome table initially showed:

| sample | metric | stored AggregateWeight | candidate rerun |
|---|---|---:|---:|
| light | median residual | -0.09686% | -0.09812% |
| light | central-68 half-width | 0.43567% | 0.43758% |
| light | inside 1% | 1769 | 1776 |
| light | beyond 10% | 33 | 32 |
| clean | median residual | -0.00640% | -0.00724% |
| clean | central-68 half-width | 0.14338% | 0.14255% |
| clean | inside 1% | 1917 | 1918 |

This was not a valid direct selection A/B for every flagged event. Clean
116/5 illustrates current/stored drift: current AggregateWeight and
SurfaceConsistency both select the identical weight-1 component at pT 27.3221
GeV, while the stored baseline is about 20.43 GeV. The event is already
pathological, but its difference is not caused by the new ranking.

Every stored-table material change was therefore rerun with current code and
`AggregateWeight`, then compared directly with the candidate tuple:

- light: 37 checked, 9 genuine selection changes, 2 improve and 7 worsen;
- clean: 32 checked, 5 genuine selection changes, 0 materially improve and 4
  materially worsen.

The two light improvements are 74/0 and 310/8. Light degradations include
37/2 from -2.7616% to -4.3269% and 157/7 from -1.4045% to -3.5202%. Clean
degradations include 358/8 from -0.4505% to -1.3243%, 351/6 from -0.0022% to
+0.3411%, and 398/5 from +0.0088% to +0.2654%.

Durable tables are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/surface_consistency_validation/`,
especially `light_ebrem_current_direct_ab.csv` and
`no_ebrem_current_direct_ab.csv`. Disposable logs and tuples are under
`/tmp/gsf-surface-consistency-*` and `/tmp/gsf-current-aggregate-*`.

## Decision and resume point

The bounded noisy-OR likelihood with floor 0.05 is rejected. Absolute mass
protection and the Bayes-factor cap work as designed, but common-surface
coincidence is not sufficiently specific: it rewards wrong radiative modes in
ordinary light events and introduces clean-track corrections. The clean gate
failed, so the full hard, forced-BH muon, 10 GeV/85-degree, and 10 GeV-pT/20-
degree ladders were intentionally not run. Default publication remains
`AggregateWeight`; the opt-in implementation is retained only as a
reproducible rejected diagnostic.

Next, use current-code direct A/B state traces to identify what distinguishes
the two desirable switches from the seven worsened light and five changed
clean events. Do not tune the noisy-OR floor against these residuals; changing
the cap only moves an insufficiently specific decision boundary. A replacement
must add physically meaningful discrimination while retaining absolute mass
scale and the no-vanishing-branch guarantee.
