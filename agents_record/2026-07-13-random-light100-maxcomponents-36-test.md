# Random light-eBrem 100-event `MaxComponents=36` test

Date: 2026-07-13

## Question and fixed population

The user requested the same reproducible random 100-event topology-clean
light-eBrem test with `MaxComponents=36`.  This is a capacity-only diagnostic:
all reconstruction and publication settings are unchanged from the earlier
`MaxComponents=12` and `24` runs.

The fixed sample is the uniform random draw (seed `20260713`) from all 2,132
topology-clean light-eBrem events:

```text
TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/
  topology_clean_2026-07-13/random_light_100_seed20260713.csv
```

It contains 68 `truth_like_lcio_preserved`, 17 `good_recovery`, 8 `missed`,
5 `truth_like_lcio_degraded`, and 2 `overshoot` events.  The selection does not
constrain the LCIO or GSF residual.

## Execution and completeness

The same installed code was run through
`scripts/run_reverse_selection_sample.py` with `--max-components 36`, 16
workers, and the fixed event list.  Outputs are under
`/tmp/gsf-random-light100-max36`.  All 100 selected events from 87 input files
produced finite output.  Every event has the same 232--235 retained hits as its
12- and 24-component counterpart; there are zero hit-count mismatches.  The
ROOT PCM lookup messages in the logs are the pre-existing non-fatal warnings,
not event failures.

## Population result

| MaxComponents | median residual | central-68 half-width | mean absolute residual | RMS | inside +/-1% | inside +/-2% | inside +/-5% |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 12 | -0.0700% | 0.4144% | 0.6231% | 1.2260% | 85/100 | 93/100 | 98/100 |
| 24 | -0.0631% | 0.3463% | 0.6110% | 1.2281% | 83/100 | 93/100 | 98/100 |
| 36 | -0.0832% | 0.3586% | 0.6235% | 1.2329% | 82/100 | 93/100 | 98/100 |

All configurations keep 100/100 inside +/-10%.

Relative to 12, 36 changes 14 signed residuals by more than 0.1 percentage
points.  By absolute residual, 8 events improve by more than 0.1 points and 4
worsen.  Improvements comprise five good recoveries, two truth-like preserved
events, and overshoot 240/4.  Worsenings comprise two good recoveries, one
truth-like degraded event, and truth-like preserved late-loss event 284/1.
Three events leave the +/-1% core and none enter it; there are no +/-2%, +/-5%,
or +/-10% crossings.

Relative to 24, 36 has no material absolute-residual improvement and two
material worsenings.  Event 42/2 moves from -0.7445% to -0.9008%, and event
404/8 loses the striking 24-component recovery, moving from -0.0134% to
-1.0106%.  Event 404/8 is the sole 24-to-36 +/-1% outward crossing.  Thus 36
partly retains the capacity gain over 12 but is strictly less favorable than
24 on this sample's core measures and eventwise material changes.

## Interpretation and decision

Capacity is not a monotonic physics improvement.  The 24-component run can
preserve a truth-compatible mode that 12 loses, but 36 can again alter the KL
aggregate competition and lose that result.  The additional capacity also
continues to preserve false inner-radiation explanations, exemplified by
284/1 remaining at +1.3557% for both 24 and 36 instead of the -0.3822% default.

Do not raise the default from 12 based on this diagnostic, and do not prefer
36 over 24.  A capacity change would still require the prescribed overshoot,
matched-control, clean, hard, and transfer/safety validation populations.

## Durable products

- `random_light_100_maxcomponents_12_vs_36_eventwise.csv`
- `random_light_100_maxcomponents_12_vs_36_summary.csv`
- `random_light_100_maxcomponents_12_vs_36_pt_resolution.{png,pdf}`
- `random_light_100_maxcomponents_24_vs_36_eventwise.csv`
- `random_light_100_maxcomponents_24_vs_36_summary.csv`
- `random_light_100_maxcomponents_24_vs_36_pt_resolution.{png,pdf}`

All products are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/`.
The comparison and plotting scripts now accept explicit baseline/candidate
labels or component counts, so these products are reproducible without
hard-coded 12-versus-24 plot text.
