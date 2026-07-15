# MaxComp=12 transition-7--8 surface-6 population audit

## Question and samples

The nine-event stratified diagnostic suggested that overshoots might prefer
surface 6. This was tested on the complete topology-clean light-eBrem
MaxComp=12 population with dominant truth transition 7 or 8 and final GSF
residual from +0.5% through +2%. All 57 events were rerun with comprehensive
component dumps. All jobs completed with finite output; disposable logs and
ROOT files are in `/tmp/gsf-maxcomp12-transition78-overshoot-all57-verbose`.

A unique 57-event control set was selected with the same exact truth-transition
composition (28 at transition 7 and 29 at transition 8), final GSF residual
inside +/-0.5%, and nearest available owned loss. The median absolute loss
mismatch is 0.049 percentage point and the maximum is 0.791 point. All control
jobs also completed with comprehensive dumps; disposable products are in
`/tmp/gsf-maxcomp12-transition78-controls57-verbose`.

Stable inputs and extracted outputs are:

- `maxcomp12_transition_7_8_overshoot_0p5_2pct_all57.csv`;
- `maxcomp12_transition_7_8_overshoot_loss_matched_controls57.csv`;
- `maxcomp12_transition_7_8_overshoot_all57_diagnostics.csv`;
- `maxcomp12_transition_7_8_controls57_diagnostics.csv`;
- directory `maxcomp12_transition_7_8_surface_selection/` with eventwise and
  summary tables plus a comparison plot.

All paths above are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/`.

## Result

The apparent surface-6 preference is real in the overshoot population but is
not a universal algorithm preference:

- surface 6 is the first selected radiative surface in 24/57 overshoots
  (42.1%) versus 7/57 controls (12.3%);
- any inward-only radiative selection occurs in 45/57 overshoots (78.9%)
  versus 19/57 controls (33.3%);
- the selected structure includes the truth transition in only 8/57
  overshoots (14.0%) versus 23/57 controls (40.4%);
- 11/57 controls publish identity, while every event in the explicitly
  radiative overshoot selection publishes a radiative state;
- surface 7 or 8 is first in 21/57 overshoots versus 33/57 controls.

As descriptive tests, inward versus not-inward gives an odds ratio of 7.5
with two-sided Fisher p=1.56e-6, and surface-6 versus all other outcomes gives
an odds ratio of 5.19 with p=6.14e-4. These p-values describe the constructed
overshoot/control samples; they are not independent validation or a calibrated
physics decision rule.

## Mechanistic interpretation

The evidence supports a coupled magnitude/location degeneracy rather than a
hard-coded preference for layer 6. A radiative child whose discrete loss is
too large can fit the measurements better when placed one or two surfaces
inward: fewer propagation intervals are exposed to the excessive momentum
jump, while the branch still produces a large interaction-point correction.
Transition 7--8 has enough inward curvature leverage for this neighboring-
surface hypothesis to become competitive. Correctly reconstructed controls
instead concentrate on the truth surfaces 7 and 8 or retain identity.

The reverse workflow starts from the forward mixture and forward posterior
weights, so a coupled wrong-surface/wrong-magnitude branch may already be
strong in the reverse seed after outer-hit information has been processed.
Subsequent inward measurements and KL aggregation can preserve or strengthen
it. The automatic decisive-crossing extractor labels all these cases
`reverse_seed` when it cannot find both exact unmerged comparator IDs in the
logged top-component list; this must not be interpreted as proof that local
measurement likelihood played no role. Previous exact-pair traces, including
340/5 and 26/9, already demonstrate measurement-supported flips.

Thus the second issue happens because surface and loss magnitude are inferred
jointly but only through discrete sequential hypotheses. A too-large loss at
the truth surface and a similar loss shifted inward can be measurement-
degenerate; the latter often wins and then overcorrects the IP momentum. A new
BH component could reduce the magnitude pressure, but only if it makes the
truth-surface hypothesis more competitive without increasing total radiative
prior or strengthening inward merged mass.
