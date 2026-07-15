# MaxComponents 12 versus 24 focused overshoot test

Date: 2026-07-13

## Question and selection

The user requested a focused rerun of light-eBrem overshoots with default GSF
settings except for a higher component budget. From the 15 topology-clean
light overshoots with default GSF residual between +1% and +2%, four events
were selected to span owned-loss scale:

| event | owned loss | stored default residual | dominant truth transition |
|---|---:|---:|---:|
| 371/5 | 1.176% | +1.047% | 6 |
| 57/3 | 2.118% | +1.238% | 7 |
| 26/9 | 5.512% | +1.238% | 7 |
| 240/4 | 8.455% | +1.789% | 6 |

The explicit list is
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/maxcomponents_overshoot_1to2pct_events.csv`.

## Configuration and execution

Both sides were rerun with the current code, `AggregateWeight` publication,
the default CEPC conditioned BH model, current-surface material workflow, KL
reduction, and full verbose component diagnostics. The only configuration
difference was `MaxComponents=12` versus `MaxComponents=24`;
`ReductionTargetComponents=0` followed that maximum in both runs.

All four events completed in both configurations with all primary-track hits:
371/5 233/233, 57/3 234/234, 26/9 233/233, and 240/4 232/232. No covariance
failure occurred. Reverse candidate rejection counts were 0/0, 2/4, 2/2, and
0/0 for 12/24 components respectively. The extra two rejected candidates in
57/3 arise from evaluating a larger candidate population; the complete track
and selected state remain finite.

Disposable verbose outputs are under
`/tmp/gsf-maxcomponents-overshoot-baseline12` and
`/tmp/gsf-maxcomponents-overshoot-24`. The durable direct comparison is
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/maxcomponents_12_vs_24_overshoot_1to2pct.csv`.

## Result

| event | residual at 12 | residual at 24 | absolute-error change | assessment |
|---|---:|---:|---:|---|
| 371/5 | +1.0474% | +1.1401% | +0.0927 pp | slight worsening |
| 57/3 | +1.2377% | +1.1600% | -0.0776 pp | slight improvement |
| 26/9 | +1.2399% | +0.7678% | -0.4721 pp | clear improvement |
| 240/4 | +1.7297% | +1.5619% | -0.1678 pp | improvement |

Thus two of four improve by more than 0.1 percentage point, one improves
slightly, and one worsens slightly. This is promising but not uniform.

The final mixtures contain 12 versus 24 components for the first three events
and 12 versus 23 for 240/4. All still undergo the same number of split and
reduction surfaces, so the test changes KL clustering and retained aggregate
moments rather than eliminating reduction.

## State interpretation

For 371/5 and 57/3, the representative reverse process signature is unchanged
when capacity doubles. Both retain a `g2` at hit 5; their small pT changes are
therefore aggregate-moment effects from different KL merges, not a new
representative process path. The truth transitions are 6 and 7 respectively.

For 26/9, the 12-component winner uses `g3` at hit 6, one hit inward of truth
transition 7. The 24-component winner instead uses milder `g2` modes at hits 6
and 8, straddling truth, and reduces the overshoot by 0.472 percentage point.

For 240/4, truth is transition 6. The 12-component winner has `g2` at hit 7
and `g3` at hit 5; the 24-component winner moves the `g2` onto truth hit 6
while retaining `g3` at hit 5, reducing the overshoot by 0.168 percentage
point.

## Decision and resume point

Do not change the default `MaxComponents=12` from this four-event test. Higher
capacity can preserve a more truth-aligned process combination in some
overshoots, but it also changes KL moments without changing representative
lineage and is not monotonically beneficial. This does not overturn the prior
population finding that KL deletion is not the recurring overshoot cause.

If capacity is pursued, the next bounded test is the full set of 15
topology-clean +1% to +2% overshoots with same-code 12/24 direct A/B, followed
by matched controls and clean/hard safety samples. Trace whether improvements
specifically correlate with replacement of an inward strong mode by
truth-aligned or straddling milder modes. Do not infer broad performance from
these four events.
