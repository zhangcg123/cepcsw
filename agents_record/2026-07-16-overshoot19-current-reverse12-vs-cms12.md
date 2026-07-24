# Current reverse versus CMSSW-like on overshoot-19 at MaxComponents=12

Date: 2026-07-16

## Scope and configuration

The durable 19-event topology-clean light-eBrem transition-5--11 overshoot
population was rerun same-code after the active default returned to
`MaxComponents=12`. The event IDs came from
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/overshoot_population_and_controls.csv`:

```text
310/8 469/6 371/5 248/6 284/0 479/1 142/4 240/4 65/6 47/4
102/4 26/9 57/3 149/2 443/2 74/0 272/0 200/1 299/7
```

Both runs used the current installed code, the default five-component
`CEPC2GeV85StepConditioned` BH model, `MaxComponents=12`, aggregate-weight
selection, and the full forward-mixture seed with covariance scale 100.
Ordinary reverse used reverse BestBranch publication. CMSSW-like used
`CmsGsfSmoothing=true`, its outermost forward-prediction seed, and
`CmsErrorRescaling=100`. The KL smoother was omitted per the established
routine-comparison scope.

Outputs and logs are under:

- `/tmp/gsf-overshoot19-current-reverse12`
- `/tmp/gsf-overshoot19-current-cms12`

Both workflows completed 19/19. Method-to-method hit counts match for every
event. Fresh `mc_pT`, `lcio_pT`, `gsf_pT`, and hit counts were read directly
from each output tuple with native ROOT. In this environment `uproot` could
inspect the fresh tree metadata but hung while reading the baskets, so it was
not used for the result extraction.

The auditable outputs are:

- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/overshoot19_current_reverse12_vs_cms12.csv`
- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/overshoot19_current_reverse12_vs_cms12_summary.json`

## Results against fresh truth and LCIO

| method | median residual | mean absolute | RMS | central-68 half-width | inside 1% | inside 2% |
|---|---:|---:|---:|---:|---:|---:|
| LCIO | -3.2248% | 4.0741% | 4.6760% | 2.3739% | 0/19 | 3/19 |
| reverse | +1.4784% | 1.8978% | 2.2620% | 0.7488% | 0/19 | 15/19 |
| CMSSW-like | +1.6287% | 1.8563% | 2.0034% | 0.7831% | 3/19 | 13/19 |

Reverse has lower absolute truth error in 11/19 and CMSSW-like in 8/19, with
no ties. CMSSW-like publishes lower pT than reverse in eight events and higher
pT in 11, reproducing the conclusion that it is not a uniform overshoot
damping. Reverse has the slightly smaller mean absolute error by only 0.0415
percentage point and the narrower central-68 half-width. CMSSW-like has the
lower RMS and places three events inside 1%, versus none for reverse.

The largest reverse residuals are 469/6 (+6.1288%), 443/2 (+3.6192%), 47/4
(+3.4356%), and 65/6 (+2.5169%). CMSSW-like partly repairs 469/6 to -3.2122%
and 443/2 to +2.7164%, but its largest remaining/worsened cases include 47/4
(+3.4664%), 272/0 (+2.5691%), and 371/5 (+2.4976%).

## Comparison with the earlier 24-component rerun

The earlier same-code 24-component comparison had reverse/CMSSW-like mean
absolute residuals of 2.0871%/1.8199%, RMS of 2.6872%/1.9726%, and eventwise
wins of 10/9. At 12 components, reverse improves both aggregate error metrics
to 1.8978% MAE and 2.2620% RMS, while CMSSW-like changes modestly to 1.8563%
MAE and 2.0034% RMS. The eventwise split becomes 11/8 in favor of reverse.
This selected overshoot sample does not establish population superiority for
either capacity or workflow.
