# KL surface-lineage mass diagnostic

Date: 2026-07-13

## Question and implementation boundary

The transition-5--11 audit showed that representative process-signature
strings are insufficient after KL merging: the retained Kalman history belongs
to one real lineage, while its aggregate weight includes other merged
lineages. The next question was therefore whether the selected aggregate
actually contains quantitative support for the truth loss surface, the common
one-hit-inward surface, or both.

This did **not** change the KL distance, merge pairing, moment calculation,
component cutoff, final selection, or published state. A default-off
`SurfaceLineageMassDump` diagnostic now propagates marginal aggregate-weight
fractions for every `(hit, BH mode)` through forward and reverse splitting and
KL moment merging. Fractions combine using the same incoming component-weight
fractions as the state moments. Component-local paths that do not split at a
surface contribute explicit inferred `no_process_mass`; this is distinct from
the exact identity `g0` atom.

Implementation and reproducible analysis are confined to
`Reconstruction/RecGsfTracking`:

- `GsfComponent.h/.cpp`: opt-in forward/reverse mode-fraction maps and clone
  propagation;
- `GsfMixture.cpp`: weight-correct fraction aggregation in `momentMerge`;
- `GsfAlgorithm.h/.cpp`: the default-off property, split annotations, reverse
  seed transfer, and parseable verbose dumps;
- `options/run_gsf_reverse_template.py`: environment switch
  `GSF_SURFACE_LINEAGE_MASS_DUMP`;
- `scripts/summarize_final_surface_lineage_mass.py`: primary-track-aware final
  component extraction;
- `scripts/analyze_surface_lineage_consistency.py`: truth/inward marginals and
  continuous forward/reverse overlap quantities;
- `scripts/summarize_final_lineage_competitors.py`: corrected to inspect the
  first truth-matched GSF track instead of a possible last short secondary.

## Build and behavior-neutral gates

`RecGsfTracking` built and installed successfully in the configured EL9/LCG
105 build. Overshoot 299/7 was run with the diagnostic both enabled and
disabled. Both executions had exactly 12 final components, 2,788 accepted and
4 rejected reverse candidate updates, seven splits/reductions, selected ID
328 at weight 0.990786, dominant fraction 0.306344, and IP pT 2.0302 GeV.
Thus the opt-in bookkeeping did not perturb the filtered result.

All 19 overshoots and all 18 matched controls completed with the diagnostic.
The 37 disposable verbose logs are under
`/tmp/gsf-surface-lineage-population`; all report successful application
termination. The extracted mode masses close with inferred no-process mass to
within `2e-6`, the six-decimal text precision.

The available exact-pair file `/tmp/gsf-match-tracks.root` still ends after
event 11. Event 11 completed with 234/234 hits; enabled and disabled runs were
identical: 12 final components, 2,794 accepted and 2 rejected reverse candidate
updates in the current-surface card, selected ID 376 at weight 0.860277, and IP
pT 1.98365 GeV. Events 16 and 17 could not be repeated from that truncated
source. The two candidate rejections are card/source behavior present with the
diagnostic off, not introduced by the bookkeeping.

## Primary-track extraction correction

Some topology-clean selected events contain an additional short reconstructed
track even though the truth-matched primary has no non-primary tracker
SimHits. The previous final-competitor summarizer selected the last reverse
mixture in each log. For example, seed 443/event 2 has a complete 228-hit
primary at pT 2.07465 GeV and a later six-hit track at 1.73697 GeV. The
extractors now explicitly use the first GSF track, matching the resolution
tuple convention. Recomputing the representative-string competitor counts
leaves the published population totals unchanged: truth-surface competitors
survive in 15/19 overshoots and 15/18 controls, are within 10:1 in 5/19 and
10/18, and a representative forward-consistent competitor survives in 10/19
and 6/18.

## Population result

The quantitative marginals strengthen the surface diagnosis. In the selected
aggregate component:

| population | median truth-surface radiative mass | median one-hit-inward mass | truth > inward | inward > truth |
|---|---:|---:|---:|---:|
| 19 overshoots | 0.121411 | 0.880671 | 2 | 16 |
| 18 controls | 0.477557 | 0.269787 | 11 | 6 |

For 299/7 specifically, the representative reverse signature selects `g2` at
hit 8, but the selected aggregate contains radiative mass 0.739747 at hit 8
and 0.326120 at the truth hit 9. KL merging therefore preserves material
truth-surface support that is invisible in the representative string, while
still leaving the inward surface dominant.

Durable derived tables are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/`:

- `final_selected_surface_lineage_mass.csv`;
- `final_all_component_surface_lineage_mass.csv`;
- `final_component_surface_consistency_scores.csv`;
- corrected `final_lineage_competitor_survival.csv`.

## Consistency-score screening and decision

No selection change is accepted yet. Normalized cosine similarity is unsafe:
it can amplify negligible forward radiative mass into a large score. In
469/6, this changes the default pT 2.12290 GeV component to a 1.83664 GeV
under-corrected component even at moderate score strength.

The unnormalized radiative dot product is more conservative. In an offline
ranking `log(weight) + lambda * dot`, lambda 5 switches 2/19 overshoots and
improves both without changing controls; lambda 7.5 switches and improves 3/19
with no control changes; lambda 10 improves 4/19 overshoots and one control.
At lambda 15 it begins selecting extremely small-weight branches and worsens
two overshoots and two controls. Since the dot product can exceed one when a
lineage contains multiple radiative surfaces, lambda has no calibrated
probabilistic meaning. This scan is mechanism evidence, not a tuned candidate
and not authorization for a new threshold or surface rule.

## Resume point

The next step is to formulate a bounded, probabilistically interpretable
continuous consistency quantity that retains absolute radiative-mass scale,
then screen it offline on all final components. It must avoid the 469/6
tiny-forward-mass failure and must not select vanishing-weight branches. Only
after a parameterization is fixed without truth-residual tuning should it be
implemented as a default-off selection diagnostic and pass the full focused,
clean, hard, energy/angle, and muon-control ladder. Do not modify KL reduction
itself on the current evidence.
