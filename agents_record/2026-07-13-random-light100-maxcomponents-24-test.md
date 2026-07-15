# Random 100 light-eBrem MaxComponents 12 versus 24 test

Date: 2026-07-13

## Sample and configuration

A uniform reproducible sample of 100 events was drawn from all 2,132
topology-clean light-eBrem events with Python `random.Random(20260713).sample`.
There was no residual, loss, or outcome restriction. The durable sample is
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/random_light_100_seed20260713.csv`.

The sample contains 68 `truth_like_lcio_preserved`, 17 `good_recovery`, 8
`missed_recovery`, 5 `truth_like_lcio_degraded`, and 2 `overshoot` events.
Owned loss spans 0.000357% to 9.899%. This composition is a random light sample,
not an overshoot-enriched optimization set.

Both sides used current-code `AggregateWeight` publication and identical
default settings except `MaxComponents=12` versus `24`. The previous four-
event verbose comparison supplied the focused component-dump gate, so the
100-event population expansion ran without full dumps. Both passes completed
100/100 events across the same 87 input files. Every event has identical hit
counts between capacities.

Disposable outputs are `/tmp/gsf-random-light100-max12` and
`/tmp/gsf-random-light100-max24`. Durable outputs are:

- `random_light_100_maxcomponents_12_vs_24_eventwise.csv`;
- `random_light_100_maxcomponents_12_vs_24_summary.csv`.

## Population result

| metric | MaxComponents 12 | MaxComponents 24 |
|---|---:|---:|
| median residual | -0.0700% | -0.0631% |
| central-68 half-width | 0.4144% | 0.3463% |
| mean absolute residual | 0.6231% | 0.6110% |
| RMS | 1.2260% | 1.2281% |
| inside +/-1% | 85 | 83 |
| inside +/-2% | 93 | 93 |
| inside +/-5% | 98 | 98 |
| beyond 10% | 0 | 0 |

Thirty events differ above numerical precision and 14 move by more than 0.1
percentage point in signed residual. Using a 0.1-point absolute-error change,
9 improve, 4 worsen, and one crosses zero with similar absolute error. Two
events newly leave +/-1%; none newly leave +/-2%, +/-5%, or +/-10%.

Material changes by original outcome are:

- `good_recovery`: 6 improve, 1 worsens, and 1 crosses zero with similar
  absolute error;
- `overshoot`: the sampled 240/4 improves;
- `truth_like_lcio_preserved`: 2 improve and 1 worsens;
- `truth_like_lcio_degraded`: both changed events worsen.

The central width and 9:4 eventwise balance favor 24 components in this sample,
but containment and RMS do not improve consistently. With only 100 events and
only two overshoots, this is diagnostic evidence rather than a population
performance result.

## Decisive traces

The strongest improvement and worst degradation were rerun verbosely at both
capacities; logs are under `/tmp/gsf-random-light100-traces-max12` and
`/tmp/gsf-random-light100-traces-max24`.

For good-recovery 404/8, truth loss is 1.574% at transition 7. At 12
components, the winner uses near-identity `g1` at hit 7 and finishes at
-0.9529%. At 24, the winner uses `g2` at the correct hit 7 and finishes at
-0.0134%. Higher capacity preserves the appropriate radiative mode.

For truth-like-LCIO 284/1, the dominant 1.184% loss is very late at transition
68. At 12 components, the winner uses near-identity `g1` at hit 7 and finishes
at -0.3822%. At 24, a false inner `g2` at hit 4 wins and finishes at +1.3557%.
Higher capacity preserves an incorrect inner-radiation explanation and creates
the largest degradation.

## Decision and resume point

Keep `MaxComponents=12` as the default. Doubling capacity is not a simple
tail cure: it can preserve a truth-surface radiative mode in genuine recovery
events, but it also preserves false inner modes for LCIO-truth-like or late-
loss events. The mixed 85-to-83 change inside 1% and the 284/1 failure prevent
promotion from this sample.

The next capacity-specific question is whether final selection can distinguish
the 404/8-type correct-surface retained mode from the 284/1-type false inner
mode without adding an ad hoc measurement threshold. If capacity itself is
pursued further, use the full 15-event +1--2% overshoot set and matched
truth-like controls, followed by full clean and hard safety samples before any
default change.
