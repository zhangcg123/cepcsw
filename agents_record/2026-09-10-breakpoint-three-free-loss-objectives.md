# Three independent free-loss objectives — same100 events

User authorized independent Minuit minimizations of forward, backward and
complete smoothed chi2. Completed300 optimizations on exactly the frozen
100-event sample from `2026-09-10-breakpoint-free-loss-hundred-events.md`.
No maintained RecBreakpoint/GSF/shared-KF implementation, library or run
card was changed. No remote operation. This is an isolated diagnostic, not
integration into the maintained package or a calibrated likelihood fit.

## Implementation

New isolated directory:
`TrackingPerformanceStudies/breakpoint_free_loss_three_objectives_20260910/`.
The previous `breakpoint_free_loss_minuit_20260910/` prototype/library/results
are preserved unchanged. The new wrapper reuses exactly the existing compiled
BreakpointFitter and KalmanAdapter objects. Only the selected scalar changes:

```text
BP_PROBE_OBJECTIVE=Forward  -> pair.rts.chi2
BP_PROBE_OBJECTIVE=Backward -> sum(pair.backward.backwardChi2)
BP_PROBE_OBJECTIVE=Smoothed -> pair.rts.smoothedTotalChi2
```

All three run independent scans and MIGRAD searches; neither the previous
RTS-optimized b nor truth b initializes the other objectives. Existing
fixed-b trial fits compute both endpoints and all scores; all are logged.
The selected b is fixed while KF+RTS+backward run. Same settings: one
truth-selected interval, sigma_b=0 conditional trial, b=[0,1], MS on,
Eloss off, FirstMiddleLast, forward/backward seed scales1, identical grid,
three starts, numerical step .002 and Minuit tolerances. The ordinary
sigma_b=.001 and truth-prior pairs remain unchanged in each flat tuple.

The backward objective uses the existing copied-forward-endpoint seed,
including its dependence on trial b; it is not an independent-data likelihood.
No scores are added together, and covariance normalization/log-determinant
terms are not added. No full free-b endpoint uncertainty is published.

## Mechanical gates and execution

- Six focused jobs: seed2:68 and seed12:11,16,17 for each selector. All104
  ordinary flat fields reproduce prior results exactly. Seed12:16 has
  multiple intervals and is a mechanical-only skip for the scalar profiler;
  seed12:17 remains a secondary-activity control, excluded from the100 sample.
- Smoothed-objective trials/minima and full fitted state/covariance dumps
  reproduce the original prototype exactly in the focused gate.
- At common coarse-grid b values, all three selectors give exactly identical
  forward/backward/smoothed scores and endpoint momenta; only the chosen
  scalar differs. Selected FCN values equal the named score in every valid row.
- Twelve population jobs, four seeds times three objectives, each25 events,
  two jobs at a time. All jobs return0, all300 optimizations save results.
- All104 ordinary flat fields match the previous100-event files exactly
  across all300 rows. Full Smoothed trial/minimum CSV contents (old columns)
  and state dumps reproduce exactly for all100 events.
- Repeated uncached minima are exact. No sampled valid point improves the
  reported minimum by more than1e-3 chi2; this does not prove global convergence.

| Objective | Status0 | Scan winner (-99) | Status3 | Lower bound | Upper bound | Interior | Invalid trial evaluations |
|---|---:|---:|---:|---:|---:|---:|---:|
| Forward |86|13|1|26|6|68|11|
| Backward |92|6|2|28|3|69|13|
| Smoothed |86|14|0|26|8|66|11|

Status3 results are non-converged Minuit results, retained and tagged, not
declared successful minima. Scan winners are finite candidate solutions,
not certified interior minima. Boundary proximity tolerance1e-6 in b. No
event is removed from the performance table because its outcome is bad.

## Same100-event pT comparison

Residual=100*(pT_reco-pT_truth)/pT_truth, percent. Width68=(q84-q16)/2.
The objective label is NOT an endpoint label: both endpoints are evaluated
at each independently optimized b.

| Objective | Published endpoint | Median absolute residual % | Width68 % | RMS % | Within +/-1% | Beyond +/-5% | Beyond +/-100% |
|---|---|---:|---:|---:|---:|---:|---:|
| Forward |RTS|0.3582|2.0211|40.5852|71|18|6|
| Backward |RTS|0.3183|0.8578|31.8674|73|17|4|
| Smoothed |RTS|0.3582|2.0211|41.4650|70|18|7|
| Forward |Backward|0.3233|2.0404|40.6037|71|18|6|
| Backward |Backward|0.3126|0.8715|31.8890|73|17|4|
| Smoothed |Backward|0.3233|2.0404|41.4848|70|18|7|

Reference KF: median absolute .2158%, width68 2.0205%, RMS8.8007%,74/100
within1%, no >100% errors. Ordinary RTS: .2220%,2.0073%,8.7932%,73/100,
no >100% errors. Thus the backward objective narrows the central interval
and reduces some catastrophic tails relative to the other free fits, but
does not establish an overall improvement over KF or solve the tail problem.

RTS absolute-residual comparison with ordinary RTS, tolerance .01 percentage
point: Forward35 better/37 worse/28 unchanged; Backward36/37/27;
Smoothed34/38/28. Backward endpoint comparisons with ordinary backward:
Forward36/36/28; Backward36/36/28; Smoothed35/37/28.

## Front-layer failures persist

All >100% errors remain in VXD-starting intervals (including VXD->ITK).
Among17 such events, >20% errors are10/8/11 and >100% errors6/4/7 for
Forward/Backward/Smoothed objectives respectively. No >100% errors occur
in the38 ITK-starting or45 TPC-starting intervals for any selector.

RTS residual examples:

| Seed:event | Forward objective | Backward objective | Smoothed objective |
|---|---:|---:|---:|
|3:17|+171.6836|+171.6836|+171.6836|
|3:59|+145.6984|-9.6117|+145.6984|
|4:11|+172.0641|+5.2173|+172.0643|
|5:62|+171.2563|+72.6898|+171.2563|
|6:74|+133.1176|+107.6677|+133.1176|

Seed4:11 backward objective selects b=.05 (4.877% loss), against truth
0.009618%. Its coarse-grid score range over b=[0,1] is only3.18e-5 chi2;
the forward and smoothed ranges are .04359 and .04202. Avoid interpreting
the backward-selected loss as a well-measured physical energy loss. Its
smaller overshoot here partly reflects a different preference along an
almost unconstrained direction, not new detector information. The precise
origin of small score trends remains unseparated.

## Artifacts and maintained status

In the study directory:

- `prototype/FreeLossProbe.h`: only scalar selection and additional score labels.
- `run.py`: separate objective jobs; requires focused gate before population.
- `check_gate.py`, `gate/regression.json`: exact focused regression evidence.
- `analyze.py`: population regression, common-b comparisons, metrics and plots.
- `hundred/results_100.csv/.md`:100 rows with three losses and six endpoints.
- `hundred/results_long.csv`:300 objective/event rows with all three scores.
- `hundred/summary.json`, `resolution_summary.csv`: statistics/status/locations.
- `hundred/objectives_{rts,backward}_{core,tails}.png`: comparisons.
- `hundred/profiles_seed4_event11.png`, `profiles_seed3_event36.png`,
  `profiles_seed6_event38.png`: relative objective profiles, not confidence PDFs.
- `hundred/seed*`: flat tuples, all trial/minimum CSVs, full-state dumps and logs.

Project-status-curator use: complete outgoing AGENTS.md was snapshotted to
`2026-09-10-agents-before-three-objective-freefit.md`. Both existing substantive
headings are retained. Global introduction, laws, scope and compile commands
are byte-for-byte unchanged. Only superseded free-fit authorization/status and
the current next action were replaced; all removed text is in the snapshot.
No history directory was moved, so a directory-migration manifest is not
applicable. Historical links remain intentional and no history files were
deleted. The status now distinguishes this tested isolated prototype from the
unchanged maintained package. No source integration is authorized or performed.
