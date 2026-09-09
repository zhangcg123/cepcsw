# Free-loss Minuit: 100 new events

User requested 100 events for the isolated free fitting method. Completed on
branch `breakpoint` with no maintained source/card/workflow changes or remote
operations. The exact prototype and fitted-score definition are in
`2026-09-10-breakpoint-free-loss-minuit-prototype.md`; the additional twelve
event study is in `2026-09-10-breakpoint-free-loss-twelve-more-events.md`.

## Fixed selection and configuration

100 distinct, previously untested topology-clear single-selected-interval
tracks: first25 eligible events below index100 in each of seeds3,4,5,6. No
cuts on loss magnitude, prior pT residual or prior chi2 preference. This
selection naturally contains69 previous truth-prior-chi2-better and31
ordinary-chi2-better cases. It is not an inclusive no-eBrem population.

Input files: `gsf_doublebhoff_freshseed_diagnostic/trk-e--2.0-85-{3,4,5,6}.root`.
All file lengths were checked before launching. Full seed/event/track manifest
and original batch quantities were frozen in `hundred/selection.csv` before
execution. Events do not overlap the earlier nineteen clean examples.

Same `libFreeLossProbe.so`, no rebuild. Trial b is fixed, sigma_b=0, and
existing LocalMarginal forward/RTS/backward code is called for each trial.
Minuit2/MIGRAD minimizes the complete RTS quadratic objective. The backward
endpoint uses the RTS-selected b; it is not separately optimized. Truth
selects the interval only; truth loss amounts enter solely in a check after
the blind search. Original sigma_b=.001 ordinary and truth-prior pairs remain
published unchanged in the flat tuples. MS on, Eloss off, FirstMiddleLast
seed, forward/backward scales1. Bounds b=[0,1], meaning loss=[0,63.2121%].
All 100 truth amounts lie inside the search range.

## Execution and validity

- Four local Gaudi jobs,25 selected events each, two jobs concurrently.
- All four jobs return0;100 ordinary rows and100 finite profile results.
- 10,120 recorded trial evaluations;11 invalid extreme trial points receive
  penalty scores and validity=false. No event-level profile failure.
- 86 winning results have Minuit status0. Fourteen retain a best coarse-scan
  point (sentinel-99), not a certified interior Minuit minimum.
- 66 interior winners,26 at/near lower bound,8 at/near upper bound, using
  proximity tolerance1e-6 in b. Converged status does not remove boundary bias.
- Repeated uncached minimum evaluations reproduce exactly. No sampled valid
  point beats the reported minimum by more than1e-3 chi2. This does not prove
  global convergence or a calibrated likelihood.
- All100 generator pT, ordinary RTS pT, ordinary backward pT and maintained
  finite-sigma truth-prior RTS pT match the stored batch exactly, by explicit
  seed/event/track identity. Source/library provenance hashes remain unchanged.
- Job times95.0,101.3,99.9,102.0 seconds; peak RSS1,198,816–1,209,444 KiB.
  These are per-job measurements, not a memory-leak validation.

## Results

Residual =100*(pT_reco-pT_truth)/pT_truth, percent. All100 finite profile results
are included, including boundary and coarse-scan winners. No tail trimming.

| Method | Median absolute residual % | Width68 % | RMS about zero % | Within +/-1% | Outside +/-5% | Outside +/-100% |
|---|---:|---:|---:|---:|---:|---:|
| KF |0.2158|2.0205|8.8007|74/100|15|0|
| Ordinary RTS |0.2220|2.0073|8.7932|73/100|15|0|
| Free-loss RTS |0.3582|2.0211|41.4650|70/100|18|7|
| Ordinary backward |0.2212|2.0051|8.7921|74/100|15|0|
| Free-loss backward |0.3233|2.0404|41.4848|70/100|18|7|
| Fixed-truth RTS diagnostic |0.1439|0.2308|5.3905|97/100|3|0|

Width68=(q84-q16)/2 of the full residual distribution. Fixed-truth diagnostic
uses b fixed at truth with sigma_b=0, not the maintained sigma_b=.001 oracle.

Relative to ordinary RTS,34 improve,38 worsen,28 are essentially unchanged
in absolute residual, using a .01-percentage-point tolerance. Corresponding
backward counts:35 improve,37 worsen,28 unchanged. Therefore the current
unregularized quadratic-score minimization is not an overall improvement in
this100-event diagnostic sample. Backward endpoints do not fix the tails.

## Clean-track counterexamples and weak objective preference

| Seed:event | Interval | Truth loss % | Fitted loss % | Ordinary RTS residual % | Free RTS residual % | Fixed-truth chi2 minus free chi2 |
|---|---:|---:|---:|---:|---:|---:|
|4:11|0|0.009618|63.212056|+0.085801|+172.064325|0.042020|
|3:17|2|0.016124|63.212056|-0.032229|+171.683588|1.395114|
|5:62|4|0.555308|63.212055|-0.133079|+171.256292|3.134037|
|3:59|0|10.297336|63.212021|-9.611666|+145.698449|0.015271|

For4:11, a negligible score advantage selects the maximum allowed loss and
destroys an initially accurate momentum. This directly demonstrates that a
reported minimum is not evidence of a well-determined or physical loss.
The broad score/normalization and interval loss-placement issues remain
unresolved; this trial does not isolate their relative contributions.
Do not promote this optimizer to the maintained production configuration.

## Artifacts

Base: `TrackingPerformanceStudies/breakpoint_free_loss_minuit_20260910/`.

- `run_hundred.py`: fixed manifest, source/library verification and execution.
- `analyze_hundred.py`: event matching, failure accounting, scores and plots.
- `hundred/campaign.json`, `selection.csv`, `job_status.json`: steering/provenance.
- `hundred/results_100.csv`: full100 rows, endpoint pT/residuals, losses, score
  comparisons, boundary/minimizer/trial validity tags.
- `hundred/results_100.md`: readable100-row table including both continuations.
- `hundred/summary.json`, `resolution_summary.csv`, `analysis.log`: summaries.
- `hundred/rts_core.png`, `rts_tails.png`, `backward_core.png`,
  `backward_tails.png`: three-curve comparisons with KF. Core +/-1%, tails
  +/-200% so all tails are visible. Normalized per full sample, no window
  renormalization. Tail y-axis logarithmic, core linear; no Gaussian fitting.
- `hundred/fitted_vs_truth_loss.png`: scatter with bound status.
- `hundred/seed{3,4,5,6}*`: flats, scalar trials/minima, verbose fitted states,
  job logs, exit-status records and resource measurements.

No ROOT files or other generated study artifacts are staged or committed.
Only this durable diagnostic record is checkpointed locally.
