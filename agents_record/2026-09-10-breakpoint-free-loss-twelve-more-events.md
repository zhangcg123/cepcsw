# Free-loss Minuit: twelve additional events

Follow-up to `2026-09-10-breakpoint-free-loss-minuit-prototype.md`.
User requested a few more events. No fitter/minimizer settings or compiled
code were changed. The same isolated libFreeLossProbe.so and ordinary helper
objects were used. Maintained source/cards/batch scripts and remote unchanged.

## Selection and execution

Twelve previously untested topology-clear single-selected-interval tracks,
six from each prior RTS chi2 comparison category, selected before execution:

- seed1: 3,4,13,31
- seed2: 3,9,10,16
- seed12: 1,2,12,23

Input: preserved `gsf_doublebhoff_freshseed_diagnostic/trk-e--2.0-85-{seed}.root`.
These files contain 100 events; every selected index is within range. Three
local Gaudi jobs (`more_seed1`, `more_seed2`, `more_seed12`) all exited 0.
All 1,136 recorded trial evaluations are valid; repeated uncached minima
reproduce exactly. Eleven winning points have Minuit status0. Seed1:3 has
the exact b=0 coarse-scan winner and sentinel status-99, not a certified
interior minimum. Four winners are at/numerically near the b=0 bound:
1:3,1:31,2:3,12:1. Do not interpret these as proof of absent physical loss.

The same-code ordinary RTS and sigma_b=.001 truth-prior RTS endpoints match
the stored batch pT exactly for all twelve, with generator pT and event/track
keys checked. The original 104-field regression checks still pass for
seed2:68 and seed12:11,16,17; the seed1 repeated-job regression also passes.

## Results

All residual columns are 100*(pT_reco-pT_truth)/pT_truth, in percent.
Ordinary is the maintained zero-centered sigma_b=.001 fit. Free-b RTS uses
the deterministic loss chosen by the outer scalar optimization. Fixed-truth
RTS uses b fixed at truth, sigma_b=0, not the maintained finite-sigma oracle.

| Seed:event | Interval | Truth loss % | Fitted loss % | KF residual % | Ordinary RTS % | Free-b RTS % | Fixed-truth RTS % |
|---|---:|---:|---:|---:|---:|---:|---:|
| 1:3 | 231 | 0.0889 | 0.0000 | -0.1070 | -0.1166 | -0.1166 | -0.1162 |
| 1:4 | 6 | 0.8144 | 3.1460 | -1.0865 | -1.1049 | +2.0494 | -0.3172 |
| 1:13 | 39 | 19.8144 | 19.5110 | -13.9420 | -13.7161 | +0.2844 | +0.5647 |
| 1:31 | 226 | 1.1075 | 0.0000 | -0.0419 | -0.0377 | -0.0377 | -0.0325 |
| 2:3 | 6 | 0.1404 | 0.0000 | -0.2601 | -0.2562 | -0.2554 | -0.1152 |
| 2:9 | 8 | 0.1300 | 0.9145 | +0.2712 | +0.2631 | +0.9664 | +0.3536 |
| 2:10 | 4 | 20.4595 | 47.6131 | -20.7971 | -20.8047 | +51.1221 | -0.4433 |
| 2:16 | 6 | 1.1489 | 0.2738 | -1.4339 | -1.4403 | -1.1817 | -0.3419 |
| 12:1 | 5 | 0.1497 | 0.0000 | -0.5537 | -0.5723 | -0.5722 | -0.4232 |
| 12:2 | 6 | 2.0193 | 0.4841 | -2.0305 | -2.0655 | -1.6004 | -0.0829 |
| 12:12 | 8 | 7.1649 | 7.2600 | -6.7540 | -6.6577 | -0.1261 | -0.2211 |
| 12:23 | 7 | 0.5986 | 2.2288 | -0.8235 | -0.8162 | +1.3796 | -0.2445 |

With a tolerance of .01 percentage point in the change of absolute residual:
4 improve (1:13,2:16,12:2,12:12), 4 worsen (1:4,2:9,2:10,12:23), and 4 are
essentially unchanged. Counting arbitrarily tiny changes instead gives 6
better and 6 worse. Do not present that raw count as six substantial gains.
Within +/-1%: KF6/12, ordinary RTS6/12, free RTS7/12, fixed-truth RTS12/12.
Median absolute residual: KF .955%, ordinary .961%, free .769%, fixed-truth
.281%. This hand-selected small sample does not establish population resolution.

The previous truth-chi2-better subset contains two improvements and four
worsenings. Thus truth having a lower objective than the ordinary fit does
not imply that unconstrained minimization selects a truth-like momentum.

## New failure mode evidence: weak loss discrimination

For seed2:10:

```text
interval = 4 -> 5
ordinary RTS chi2 = 444.4503073683175
fixed-truth chi2 = 444.10448394120687 (see CSV for full machine precision)
free-b chi2 = 443.67611709291083
truth-minus-minimum = 0.42836684829603655
truth loss = 20.4595%; free loss = 47.6131%
ordinary residual = -20.8047%; free residual = +51.1221%
```

The profile remains shallow over a wide range of b: numerical convergence
does not imply a well-determined loss. This is different from the earlier
seed1:47 counterexample, whose wrong solution beat the fixed-truth objective
by roughly143.47. Do not claim a calibrated one-sigma interval from this raw
Delta-chi2 profile; normalization, trial-dependent covariance and one-pass
linearization limitations remain unchanged.

The returned minimum is consistent with sampled points within 3e-7 chi2;
seed1:13 has one numerical trial lower by 2.74e-7, negligible at the configured
tolerance. This is not evidence of global convergence.

## Artifacts and next step

Under `TrackingPerformanceStudies/breakpoint_free_loss_minuit_20260910/`:

- `more_results.csv`: full precision pT, residuals, losses, three hypothesis
  scores, backward endpoint values, prior category and minimizer status.
- `more_summary.json`: counts and conditional sample summaries.
- `more_analysis.log`: regression and per-event summary.
- `runs/more_seed*_trials.csv`, `_minimum.csv`, `_states.txt`, `.root`, `.log`.
- `profile_seed{seed}_event{event}.png` and `_core.png`: per-event scans.
- `results.csv`/`analysis.json`: updated combined results (19 clean events
  plus the separately identified seed12:17 secondary-activity control).

No source rebuild, no production steering change, no ROOT staging or remote
operation. Keep the prior recommendation: do not make this the default.
Investigate the physical score/model constraints using the strong wrong
minimum (1:47), shallow wrong minimum (2:10), and recovered controls
(1:13,12:12) before production integration.
