# KL smoother on the durable MaxComponents=12 overshoot-50 sample — 2026-07-15

## Question and exact sample

The new KL reduction-aware Gaussian-sum smoother was run on the user's durable
50-event sample originally selected from MaxComponents=12 reverse-GSF outputs:

- dominant truth transition 7 or 8;
- old reverse-GSF positive residual between +0.5% and +2%;
- 50 deterministic seed/event IDs across 48 reconstructed seed files.

Truth, LCIO, and the recorded original conditioned-model reverse-filter values
come from
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/conditioned6_overshoot50_comparison.csv`,
using its `old_gsf_*` columns. The old values are the original
`CEPC2GeV85StepConditioned`, MaxComponents=12, AggregateWeight reverse-filter
same-code rerun, not the rejected six-component BH candidate.

## New run configuration and audit

The smoother rerun used:

- `BHModel=CEPC2GeV85StepConditioned`;
- `MaxComponents=12` and KL reduction only;
- `GaussianSumSmoothing=true` and `ReverseFiltering=false`;
- `GSFOutputMode=WeightedMean`;
- the reconstructed `tuples285/trk-e--2.0-85-<seed>.root` inputs.

All 50 events completed in 48/48 seed jobs. Every log contains successful
application termination and its expected fitted track count. There is no
smoothing-graph, covariance, or fatal error in the valid run directory
`/tmp/gsf-kl-smoother-overshoot50-valid`.

An initial accidentally launched simulation-only input batch was stopped after
the files reported missing reconstructed hit collections. It is excluded from
all results; its directory is `/tmp/gsf-kl-smoother-overshoot50`.

The reusable runner now accepts `--workflow` and `--input-pattern`, and the
template exposes `GSF_OUTPUT_MODE`. The comparison parser is
`Reconstruction/RecGsfTracking/scripts/compare_smoother_overshoot_sample.py`.

## Results against truth

| Estimator | Mean residual | Mean absolute | Median absolute | RMS | Inside 1% | Inside 2% |
|---|---:|---:|---:|---:|---:|---:|
| LCIO | -2.5059% | 2.5189% | 1.6622% | 3.4402% | 15/50 | 29/50 |
| Recorded reverse filter | +1.0458% | 1.0458% | 0.9291% | 1.1348% | 28/50 | 50/50 |
| KL smoother | -2.5129% | 2.5258% | 1.6755% | 3.4511% | 15/50 | 29/50 |

Eventwise, the smoother is closer to truth than the reverse filter in 15
events and worse in 35. It is closer than LCIO in 16 events and worse in 34.

The smoother output is essentially LCIO-like across this selected sample. Its
mean pT shift from LCIO is -0.000140 GeV and median shift -0.000096 GeV. Only
one event changes by more than 1 MeV, and none changes by more than 5 MeV.
Forty-eight smoother residuals remain negative and two positive.

The largest apparent improvement relative to the reverse filter is clean-like
320/4: LCIO is -0.0289%, reverse overshoots by +1.9702%, and the smoother is
-0.0414%. This removes the false reverse correction but is slightly worse than
LCIO itself.

The largest regression relative to reverse is hard-loss 187/4: LCIO is
-8.5842%, reverse recovers to +1.1193%, and the smoother stays at -8.5915%.
The largest change relative to LCIO is 416/5, where the smoother worsens
-6.7851% to -6.9878%; the reverse filter is +0.6084%.

## Interpretation

The new smoother eliminates the defining positive overshoot behavior primarily
because it does not carry the reverse filter's material-loss correction back
to the IP. It does not solve the sample by moving the estimates toward truth:
its aggregate performance is statistically and eventwise almost identical to
LCIO and slightly worse on the displayed metrics. Conversely, the reverse
filter genuinely recovers substantial underestimation for many events but
systematically overshoots this deliberately selected population.

This sample was selected on reverse-filter overshoot and is not an unbiased
resolution sample. It is nevertheless a direct mechanism test: the KL smoother
is safer against these old positive overshoots but forfeits nearly all of the
hard-loss recovery that made the reverse workflow useful.

Durable outputs:

- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/kl_smoother_overshoot50_comparison.csv`
- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/kl_smoother_overshoot50_summary.json`

