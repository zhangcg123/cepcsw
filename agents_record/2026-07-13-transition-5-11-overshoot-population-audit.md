# Transition 5--11 overshoot population audit

Date: 2026-07-13

## Scope

This audit follows the topology-clean resurvey and studies the full population
of ordinary light-eBrem overshoots whose Geant4-owned dominant transition is
5--11. It deliberately does not infer a mechanism from a single event. The 19
overshoots are compared with 18 unique, same-surface, loss-matched non-overshoot
controls selected reproducibly by
`select_overshoot_surface_controls.py`.

Overshoots by transition are:

- transition 5: 310/8, 469/6;
- transition 6: 371/5, 248/6, 284/0, 479/1, 142/4, 240/4, 65/6, 47/4;
- transition 7: 102/4, 26/9, 57/3, 149/2, 443/2;
- transition 8: 74/0, 272/0, 200/1;
- transition 9: 299/7.

The matched controls are 459/4, 283/1, 407/6, 174/8, 370/8, 61/7, 315/7,
22/0, 5/4, 37/6, 452/6, 43/9, 192/0, 42/3, 209/0, 308/6, 432/7, and 402/1.
The exact pairing table is
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/overshoot_same_surface_controls.csv`.

## Complete current-default traces

All 19 overshoots and all 18 controls were rerun with the current default
reverse workflow and complete component diagnostics. Every run completed. Raw
logs are under `/tmp/gsf-overshoot-default`,
`/tmp/gsf-overshoot-controls-default`, `/tmp/gsf-overshoot-procdiag`,
`/tmp/gsf-overshoot-signature`, and
`/tmp/gsf-overshoot-controls-signature`. `/tmp` logs are disposable; the
durable derived tables are:

- `overshoot_best_branch_trajectories.csv` and
  `overshoot_control_best_branch_trajectories.csv`;
- `overshoot_decisive_odds.csv`, `overshoot_control_decisive_odds.csv`, and
  `overshoot_decisive_odds_with_process.csv`;
- `overshoot_selected_process_signatures.csv` and
  `overshoot_control_selected_process_signatures.csv`.

The generating scripts are
`summarize_reverse_overshoot_trajectories.py`,
`extract_overshoot_decisive_odds.py`, and
`summarize_reverse_process_signatures.py` in the same survey directory.

To make the selected lineage auditable, default-neutral diagnostics were added
to `GsfComponent`: the last reverse process hit, component, and retained
fraction, plus a compact selected process signature. The reverse filter now
prints these fields in verbose mode. This metadata does not change filtering,
selection, or published states. The package built and installed successfully.

## Population-level findings

The largest selected-branch pT correction occurs at hit 0, 4, 5, 6, or 7 for
3, 2, 9, 4, and 1 overshoots, respectively. The matched controls peak at hits
2, 3, 4, 5, 6, and 7 for 1, 1, 1, 7, 3, and 5 events. Relative to the truth
transition, the overshoot correction is a median two hits inward versus one
hit for controls; 10/19 paired overshoots correct later than their controls,
7/19 at the same relative hit, and 2/19 earlier.

This is not simply a weak final-choice problem. The selected final weight has
median 0.856 for overshoots and 0.955 for controls. With a strict 10:1 odds
definition, 9/19 overshoots have a strong prior advantage at the decisive hit,
7/19 have a strong innovation-likelihood advantage, and 4/19 have both. The
population is therefore heterogeneous: process history and measurement
likelihood each dominate a substantial subgroup.

KL reduction is not a recurring primary cause. Winner-weight amplification at
the audited reduction has median 1.155 for overshoots versus 1.140 for
controls; the overshoot maximum is 1.494 and none exceeds 2. The associated pT
shift is below 1% in every audited event.

The most repeated pattern is discrete loss-mode mismatch. Eighteen of 19
overshoots select exactly one non-identity process child; 240/4 selects two.
Eleven selected lineages contain component g2 and nine contain g3, with 240/4
in both groups; none selects g1 or g4. Controls have 16 single-child, one
identity-only, and one two-child lineage; 14 contain g2 and four contain g3.

For selected g2 lineages, the overshoot median truth dominant loss is 2.154%,
the selected modeled loss is 2.427%, the median modeled-minus-truth mismatch is
+0.360 percentage points, and the final median GSF residual is +1.277%.
Matched g2 controls have median truth loss 2.869%, modeled loss 2.430%, mismatch
-0.0037 points, and final residual -0.086%.

For selected g3 lineages, the overshoot median truth loss is 5.529%, the
selected modeled loss is 10.543%, the median mismatch is +5.356 points, and
the final median residual is +1.789%. The four g3 controls have medians 6.147%,
10.448%, +4.270 points, and +0.263%, respectively. Thus the g3 mismatch is not
sufficient by itself, but it is recurrent and more damaging in the overshoot
population.

Comparing the largest selected-branch pT gain with the truth dominant loss,
16/19 overshoots have positive excess and 8/19 exceed two percentage points;
the median excess is +1.151 points (16--84% interval +0.104 to +7.506).
Controls are positive in 9/18 and exceed two points in 5/18, with median
+0.069 points (interval -1.711 to +3.620). This independently supports an
overlarge/transient correction mode while showing that later measurements can
settle the same discrete mode in some controls.

## Interpretation and next test

The recurring population pattern is incomplete/intermediate support in the
five-component conditional loss mixture, not one bad event and not one global
selection failure. The source truth table split at 10% shows that the current
broad 5--20% g3 component combines a 5--10% mode near 7.2% loss with a 10--20%
mode near 14.1%, producing the executed mode near 10.5% at representative
t/X0. This is consistent with the selected-lineage audit.

The prior experiment that replaced g3 by a hard 5--10/10--20 split is not a
solution: it failed to improve missed/partial examples, changed 469/6 from
+6.13% to -6.39%, and worsened hard 1/3 from -0.46% to -0.79%. The current
five-component model remains restored.

The next experiment must therefore be a default-off, conservative support
diagnostic that preserves the established g3 support and total radiative
probability while adding limited intermediate support. It must be evaluated on
the full 19-event overshoot population and matched controls, not optimized on
one representative. Any candidate is rejected unless it improves population
counts/quantiles while preserving clean 62/9, hard 1/3, hard events 11/16/17,
and then the full clean/light/hard categories.

## Intermediate-support test and surface-lineage follow-up

A default-off diagnostic partitioned 10%, 25%, or 50% of the existing g3
weight into same-sample 5--10% support while preserving the original g3 mode
and total radiative probability. Across all 19 overshoots, every fraction
improved 8 and worsened 11; median absolute-residual changes were +0.0128,
+0.0137, and +0.0156 percentage points. The 18 controls split 9 improved and 9
worsened at every fraction. The candidate is rejected; the default remains
fraction zero. The exact table is `intermediate_support_population_scan.csv`.

Selected process surfaces reveal a stronger repeated pattern. Fifteen of 19
overshoots select their radiative child one hit inward of the Geant4-owned
dominant transition; controls select the truth-owned surface in 9/18 and one
hit inward in 6/18. This is not reverse-index convention: a truth transition
between hits i and i+1 is represented by the reverse split before update i.

Full final-mixture diagnostics show that a truth-surface branch survives in
15/19 overshoots and 15/18 controls, but is within 10:1 of the winner in only
5/19 overshoots versus 10/18 controls. Representative forward/reverse surface
consistency survives in 10/19 overshoots and 6/18 controls; it points to the
truth surface in 7/10 and 4/6 respectively. A hard consistency veto is
therefore unjustified. The durable competitor table is
`final_lineage_competitor_survival.csv`.

The next diagnostic should propagate quantitative unmerged lineage mass by
process surface through KL merges. Current string signatures belong only to
the retained representative, while aggregate component weight sums multiple
histories. Surface-lineage marginals are needed before testing any
forward/reverse consistency score; do not select using representative strings.
