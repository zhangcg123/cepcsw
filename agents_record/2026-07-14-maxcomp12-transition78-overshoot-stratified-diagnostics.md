# MaxComp=12 transition-7--8 overshoot stratified diagnostics

## Scope

MaxComponents=12 was adopted as the diagnostic working capacity, without
changing the active production/default setting of 24. Nine topology-clean
light-eBrem events were selected from the transition-7--8, final GSF residual
+0.5% to +2% population in three strata:

- low-loss false corrections: 302/6, 320/4, 340/5;
- moderate-loss overshoots: 41/5, 53/4, 200/1;
- strong-loss recoveries that overcorrect: 26/9, 187/4, 261/6.

The stable event list is
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/maxcomp12_transition_7_8_overshoot_diagnostic_events.csv`.
All nine comprehensive-dump jobs completed with finite output. Disposable
logs and ROOT files are in
`/tmp/gsf-maxcomp12-transition78-overshoot-9-verbose`. Extracted state data are
in `maxcomp12_transition_7_8_overshoot_9_diagnostics.csv` beside the event
list.

## MaxComp=12 mechanism

Seven of nine selected radiative structures are inward of the Geant4-owned
dominant transition. The exceptions are low-loss 302/6 and 320/4, which select
outward surface-8 and surface-9 g2 modes for truth transition 7. The remaining
surface/mode choices are:

- 340/5: truth 8, selected surface-6 g2;
- 41/5 and 53/4: truth 7, selected surface-6 g2;
- 200/1: truth 8, selected surface-7 g2;
- 26/9 and 187/4: truth 7, selected surface-6 g3;
- 261/6: truth 7, selected surface-6 g2 plus surface-5 g3.

The three strong-loss cases have overwhelming inherited reverse-start
radiative odds and negligible final identity weight. Their coarse g3 or mixed
inward corrections explain why genuine recovery passes beyond truth. Low-loss
302/6 also begins with strong radiative odds despite essentially truth-like
LCIO. For 320/4, 340/5, and 41/5, the logged leading radiative component begins
behind identity but later wins; the current automatic extractor labels these
`reverse_seed` because the top-component dump does not retain both exact
competitors at the crossing. Do not interpret that label as evidence that no
measurement-driven flip occurred; 340/5 was previously established as a
near-boundary likelihood-supported false selection.

These traces reinforce the existing mechanism: transition 7--8 supplies
strong correction leverage, but the selected discrete loss mode and often its
surface are wrong. KL capacity changes can repackage the posterior but are not
a calibrated physical discriminator.

## Bounded MaxComp=24 check

Stored same-event tuples show MaxComp=24 reduces absolute residual in 8/9
events. Improvements exceed 0.2 percentage point only for 26/9 (0.434), 53/4
(0.231), 200/1 (0.253), and 320/4 (0.265). Those four were rerun with
comprehensive MaxComp=24 dumps; all completed. Logs are in
`/tmp/gsf-maxcomp24-transition78-capacity-sensitive-4-verbose`, and durable
state data are in `maxcomp24_transition_7_8_capacity_sensitive_4_diagnostics.csv`.

Only 26/9 changes selected radiative structure materially: MaxComp=12 selects
surface-6 g3 and gives +1.202%, while 24 retains a surface-8/surface-6 g2
structure and gives +0.765%. Events 53/4, 200/1, and 320/4 retain the same
reported surface/mode signature at both capacities but change pT, weights, and
state through different KL representation histories. Thus 24 provides some
tail protection, but it does not introduce a general surface/mode discriminator.

## Decision

Use MaxComp=12 for the next broad mechanism-screening diagnostics because it
is sufficient to expose the dominant wrong-surface/wrong-mode pattern and is
nearly identical in population central performance. Retain MaxComp=24 as the
active default and as a required validation capacity for any proposed physics
change. A 24-component rerun is warranted when a 12-component candidate lies
near an identity/radiative boundary or when the selected g2/g3 structure is
capacity-sensitive; it is not necessary for every exploratory trace.
