# Broad ECAL component-constraint population screen

Date: 2026-08-17

## Scope and input audit

This is the first broad population comparison of the same-run ordinary
`GSFTracks` result and the paired `GSFTracksEcalConstrained` result. The root-
level inputs were `gsf_flat_e-_reverse_<seed>.root`, with 100 events expected
per file and `EcalComponentConstraint=True` in the generated cards.

- 403 files were present after the batch stopped changing.
- 401 files contained a usable 100-entry `gsf_tuple`.
- Seeds 330 and 462 were 955-byte files without the tuple tree.
- Nine rows had no constrained result, leaving 40,091 events with finite,
  positive truth, LCIO, ordinary GSF, and constrained GSF pT.
- The constraint changed final branch choice in 201 events, or 0.501% of the
  valid paired population.

This generator sample spans 10--50 GeV and theta 40--140 degrees. The older
5,000-event Geant4 loss/category and secondary-topology tables cover a
different ten-event-per-seed sample and were not joined to these new 100-event
files. This screen is therefore inclusive, not topology-clear or categorized.

## Resolution results

All residuals are `100*(pT_reco/pT_truth-1)`. Histograms use the same paired
events and are normalized by their common population.

| population | method | N | median (%) | width68 (%) | RMS (%) | abs >10% | abs >50% | abs >100% |
|---|---|---:|---:|---:|---:|---:|---:|---:|
| all paired | LCIO | 40,091 | -0.1549 | 3.1739 | 16.59 | 5,165 | 1,503 | 0 |
| all paired | ordinary GSF | 40,091 | -0.08854 | 0.87868 | 237.68 | 3,594 | 1,539 | 170 |
| all paired | GSF + ECAL | 40,091 | -0.08838 | 0.87890 | 237.36 | 3,535 | 1,473 | 118 |
| ECAL-changed | LCIO | 201 | -14.139 | 23.34 | 30.83 | 111 | 26 | 0 |
| ECAL-changed | ordinary GSF | 201 | -0.1632 | 104.56 | 176.29 | 146 | 79 | 52 |
| ECAL-changed | GSF + ECAL | 201 | +0.00261 | 15.70 | 23.54 | 87 | 13 | 0 |

Across the 201 changed events, 149 changes reduced absolute truth error and 52
increased it. ECAL removed all 52 greater-than-100% tails in this changed
subset, but did not act on every extreme event. In particular, seed 446 entry
64 retained the same 14,694.7 GeV ordinary and constrained pT for truth pT
34.36 GeV, a +42,661% residual. Its failure is consistent with the known
surviving-component/posterior-support limitation.

The global central distribution is essentially unchanged because only 0.501%
of events change. Tail counts improve modestly: absolute residual above 10%
falls by 59, above 50% by 66, and above 100% by 52. The global RMS remains
dominated by the unchanged +42,661% outlier and is not an appropriate central-
performance summary.

## Clean-safety boundary

The broad sample exposes false interventions that the earlier selected
overshoot cohort could not measure:

| ordinary-GSF starting region among changed events | N | improved | worsened | constrained abs >1% | constrained abs >3% | constrained abs >10% |
|---|---:|---:|---:|---:|---:|---:|
| abs residual <=1% | 37 | 6 | 31 | 22 | 2 | 0 |
| 1--3% | 10 | 7 | 3 | 5 | 2 | 0 |
| 3--10% | 8 | 8 | 0 | 5 | 2 | 0 |
| above 10% | 146 | 128 | 18 | 131 | 119 | 87 |

Thus the present selector is effective for many large errors but is not yet
clean-track safe. The worst observed degradation was seed 303 entry 21: an
ordinary residual of -35.40% changed to -77.11%, while LCIO was -79.26%.
Truth is used only for evaluation, not by the runtime selector.

## Generated artifacts

Generated plots and CSVs are intentionally uncommitted under:

`TrackingPerformanceStudies/gsf_ecal_constraint_broad_electron_2026-08-17/`

The directory contains normalized global wide/log-tail, +/-5%, and +/-1%
overlays; changed-event wide and +/-10% overlays; the input audit; per-event
paired and changed tables; and metric summaries.

## Conclusion and next gate

The broad screen is encouraging for large tracker failures but rejects any
claim that the default settings are ready for promotion. Keep the prototype
default-off. Diagnose the 52 worsened changes, especially the 31 interventions
starting within the ordinary GSF 1% core, and determine why the largest tails
do not have a selectable energy-compatible component. Before tuning the gate,
matching windows, likelihood width, or likelihood floor, build independent
Geant4 loss and secondary-topology labels for this broad sample and repeat the
comparison by no-eBrem, light, hard, early-transition, energy, angle, and
topology cohorts.
