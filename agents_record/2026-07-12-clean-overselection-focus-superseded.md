# Superseded clean-overselection focus

This record preserves the complete outgoing `AGENTS.md` current-focus section
when the user redirected the immediate optimization work toward light-eBrem
events on 2026-07-12. The investigation and its conclusions remain valid; only
the ordering of active work changed.

## Outgoing focus, preserved verbatim

The active concentration is preserving the corrected hard-eBrem recovery while
eliminating reverse radiative over-selection on tracks with no true owned
eBrem. The conditioned artifact now fits eBrem-attributed loss only, avoiding
double counting with deterministic `ElossOn=True`. It contains an exact
`z=1` no-eBrem atom whose lineage is protected through cutoff and KL reduction.

Reconstruction-aligned Geant4 surface ownership classifies the matched
1000-event sample as 407 no-eBrem, 437 light-eBrem, and 156 hard-eBrem events.
On the 407 clean events, LCIO and reverse no-BH have central-68 widths of
0.278607% and 0.275195%; therefore the second reverse refit does not itself
degrade the core. Enabling BH changes only 40 events by more than 0.1%, but 15
by more than 1%, broadening the width to 0.321295% while leaving the median near
zero. On 155 successful hard events, LCIO versus eBrem-only reverse BestBranch
has median residual -12.8099% versus -0.2915%, with 48 versus 74 events inside
1%. Seed 74 entry 4 remains the known failure.

The KL-cluster hypothesis is now rejected. In seed 23 entry 8, 99.6% of the
winning component's pre-hit-0 weight already belongs to one radiative child;
merging adds less than 0.5%. Uniform reverse-start weights fix only this extreme
case partially and leave all 15 outliers above 1%. Forward-only BH matches the
no-BH control within 0.052% for all 15 but loses the representative hard-event
recovery completely. A broad single outer reverse seed preserves the focused
clean and hard results but fixes only 2/15 clean outliers. The remaining
blocker is genuine reverse-direction radiative model selection, not missing
identity protection, KL aggregation, or inherited forward weights.

Proceed in this order:

1. For typical outlier seed 62 entry 9, record the exact BH prior odds and
   accumulated innovation likelihood for the identity and winning radiative
   lineage, and identify the first decisive transition/measurement.
2. This audit is complete: hit 3 supplies an approximately 328:1 likelihood
   ratio for a moderate-loss branch after a `t/X0~=0.01018` interval. Measure
   the no-eBrem total-transition residual variance versus `t/X0` and determine
   whether missing non-radiative straggling makes the identity process
   unrealistically narrow.
   Do not introduce a measurement-evidence gate or promote the failed Uniform,
   BestBroad, or loose-seed-covariance diagnostics to defaults.
3. Validate any model change first with complete verbose clean seed 62 entry 9
   and hard seed 1 entry 3 checks, then repeat events 11, 16, and 17 and run
   the 407 clean and 156 hard surface-owned categories. Require finite complete
   tracks without new rejection or covariance failures.
4. Address the known seed-74/event-4 rejection separately only after the
   selection semantics preserve both the clean core and hard recovery.

Success means retaining the demonstrated categorized hard-loss recovery while
matching, rather than biasing or broadening, the LCIO no-eBrem core. Independent
held-out validation and broad energy/angle coverage remain required before any
production-performance claim.

Current non-goals: adding a new measurement-evidence selection threshold,
using WeightedMean, global covariance tuning, fitting SimHit momentum, treating
ACTS coefficients as CEPC validation, premature runtime optimization, or
additional shared-package changes.

The exact identity construction, corrected category provenance, controls,
eventwise diagnosis, and hard-category evidence are preserved in
`agents_record/2026-07-12-ebrem-only-identity-and-clean-control.md`; the KL,
reverse-seed, forward-only, and RTS diagnostics are preserved in
`agents_record/2026-07-12-reverse-overselection-diagnostics.md`.
