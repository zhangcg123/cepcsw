# Positive-LCIO residual amplified by GSF: 30-event selection

Date: 2026-07-13

The user requested 30 events from the expanded 2 GeV, theta-85-degree sample
where `CompleteTracks` already slightly overestimates pT and the reverse GSF
increases the positive residual further, as the starting population for later
state-by-state diagnostics.

The reproducible source is
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/topology_clean_event_outcomes.csv`.
The selection requires:

- topology-clean ordinary optimization event;
- positive LCIO residual in `[0.1%, 0.5%)`;
- GSF residual at least 0.25 percentage point above the LCIO residual;
- no-eBrem or light-eBrem category.

There are 32 topology-clean ordinary events before the final category
restriction. Two are hard-eBrem anomalies, 25/9 with 69.94% owned loss and
373/3 with 10.87% owned loss. Excluding them leaves exactly 30 coherent
no-/light-eBrem right-tail events: 20 no-eBrem and 10 light-eBrem. Their LCIO
residuals span +0.109% to +0.470%, and the stored GSF-minus-LCIO amplification
spans +0.369 to +3.286 percentage points.

The durable event list with category, owned loss, both stored residuals, and
amplification is
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/positive_lcio_amplified_30_candidates.csv`.

This is an ID-selection result, not yet a state-level mechanism claim. Because
the catalogue is a stored aggregate-weight result and the active component
budget is now 24, each event should be confirmed with current-code verbose
output as part of the diagnostic production. Do not silently replace an ID if
current code changes its final state; record that drift explicitly.

## Exact identity interpretation to retain

The current conditioned model uses
`z = 1 - ebrem_step_loss_sum_GeV / p_before_GeV`. In each t/X0 bin, transitions
with no Geant4-attributed eBrem form the identity population, while transitions
with positive eBrem supply the radiative spectrum. Consequently:

- the identity weight is data-derived as the zero-eBrem fraction in that t/X0
  bin;
- its location is imposed at exactly `z=1`, not fitted from the positive-loss
  spectrum;
- its variance is imposed as `1e-12`, or `sigma_z=1e-6`;
- the remaining probability and the shapes of g1--g4 come from positive eBrem
  transitions.

Identity means no discrete eBrem correction, not no material effect. The
standard card still applies deterministic ionization loss with `ElossOn=True`,
as well as multiple scattering, propagation, and measurement updates. In the
reverse filter, identity approximately preserves an already positive LCIO pT
residual; every radiative child raises inferred inward pT. The present BH basis
therefore has no child that actively lowers an already overestimated momentum.
The identity lineage is protected from the low-weight cutoff and from KL merge
with radiative lineages, but it is not forced to win final aggregate-weight
selection.

## Interrupted current-default confirmation

A non-verbose current-code `MaxComponents=24`, `AggregateWeight` confirmation
was started under `/tmp/gsf-positive-lcio-amplified-30-current24`. Eight seeds
completed successfully before the broad confirmation was stopped because the
user requested an ID-first handoff rather than a long production: 43, 71, 84,
92, 101, 124, 127, and 146. Their ROOT tuples and logs are disposable `/tmp`
outputs and their pT values were not promoted into the durable table. The
remaining events have not been confirmed against current code. This partial
execution is not validation and must not be mistaken for the requested
state-by-state diagnostic.

## Exact resume point

First rerun all 30 IDs with the current installed 24-component aggregate-weight
default and comprehensive component dumps. Preserve stored-versus-current pT
drift explicitly rather than replacing IDs. For every event, extract the final
identity state, selected state, first decisive divergence, pre-hit prior odds,
exact Gaussian innovation likelihood ratio, post-hit odds, intervening KL
merges, and selected BH surface/mode. Stratify the result by no-eBrem versus
light-eBrem and by amplification size before proposing any model change.
