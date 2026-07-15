# Counterfactual truth-surface versus inward loss scan

## Purpose and implementation

A default-off diagnostic was implemented inside `RecGsfTracking` to test the
causal hypothesis that missing intermediate BH loss support pushes radiative
hypotheses inward. It does not alter the live GSF mixture. At the configured
Geant4-owned dominant transition it finds the highest-weight protected exact
identity branch, deep-clones that common state, and creates isolated branches
with the same trial fractional loss applied either at the truth transition or
one transition inward. A no-process baseline is also retained.

Every isolated branch traverses the same remaining measurements through the
existing baseline-compatible MarlinTrk `initialise -> addAndFit` path. It
accumulates the exact prior-independent innovation evidence
`-0.5 * sum(dchi2 + logDetS)`. The trial branches receive no process prior,
additional BH splitting, cutoff, KL reduction, or final-selection access and
cannot modify the published track.

New properties, all inert by default, are:

- `CounterfactualLossScan=false`;
- `CounterfactualTruthTransitionMap=""` using `event:transition` entries;
- `CounterfactualLossFractions`;
- `CounterfactualLossVariance=2e-4`.

The standard reverse template and `run_reverse_selection_sample.py` expose the
diagnostic through environment/CLI controls. The package builds and installs.
The README documents the properties. Reproducible extraction and summary are
implemented by `analyze_counterfactual_loss_scan.py` and
`summarize_counterfactual_loss_scan.py`.

## Focused mechanical validation

Overshoot 26/9 and its nearest-loss transition-7 control 42/3 were run with
comprehensive dumps at MaxComp=12. Every counterfactual branch retained all
eight inward measurements. Scan-on and same-code scan-off runs have identical
selected IDs, weights, signatures, IP states, and tuple pT for both events.

With trial retained-fraction variance `2e-4`:

- 26/9 prefers 7% loss at inward surface 6 (`logL=52.4572`) over the best
  truth-surface trial, 4% at surface 7 (`logL=46.9513`), giving
  `delta logL(truth-inward)=-5.5058`;
- control 42/3 prefers 6% at truth surface 7 (`logL=55.7352`) over 6% at
  surface 6 (`logL=24.8672`), giving `delta logL=+30.8681`.

Extending the scan down to 0.5%, 1%, 2%, and 3% does not change either focused
optimum. Repeating the full 0.5--12% focused scan with zero trial variance and
g3-like variance `0.0017` preserves the surface decisions: 26/9 has delta
logL -5.833 and -5.481, while 42/3 has +31.036 and +31.362. Thus the focused
contrast is not an artifact of the nominal trial variance.

## Population scan

All 57 MaxComp=12 transition-7--8 overshoots with final residual +0.5% through
+2% and all 57 exact-transition, nearest-owned-loss controls inside +/-0.5%
were scanned. The complete grid is 0.5%, 1%, 2%, 3%, 4%, 5%, 6%, 7%, 8%, 9%,
10%, and 12% at the truth and one-inward surfaces, plus the baseline: 25
branches per event. All 2,850 branches are valid and retain every expected
measurement (eight hits for truth transition 7, nine for transition 8).

Durable products are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/counterfactual_loss_scan_transition_7_8/`:

- per-branch and per-event CSVs for overshoots and controls;
- `population_summary.csv`;
- PNG/PDF likelihood and preferred-loss plots;
- live-GSF diagnostic tables used for the non-interference audit.

Results:

- the best truth-surface loss beats the best inward loss in only 13/57
  overshoots, versus 24/57 controls;
- median delta logL truth-minus-inward is -1.108 for overshoots and -0.0967
  for controls; the central-68 intervals are [-10.833,+0.321] and
  [-3.235,+1.161];
- 21 overshoots versus 10 controls strongly prefer inward placement by more
  than 2 log-likelihood units; six in each sample strongly prefer truth;
- the median independently optimized loss is 2% at both surfaces and in both
  samples;
- only 6/57 overshoots have a best truth-surface loss of at least 5%, and only
  one of those six actually prefers truth placement;
- among the 11 selected lineages containing g3, only two prefer truth
  placement, their median delta logL is -6.159, and only one combines a
  truth-surface win with a preferred loss in the 5--8% region.

As descriptive comparisons, truth-surface wins give two-sided Fisher p=0.0447
between constructed samples, and the delta-logL distributions give two-sided
Mann-Whitney p=0.00503. These are not held-out validation or a production
selection calibration.

## Interpretation and decision

The scan directly rejects the proposed simple causal mechanism for most of the
population. Even after optimizing loss magnitude independently at each
surface, the inward hypothesis remains more likely in 44/57 overshoots. A new
5--8% truth-surface component would target only six events' truth-surface
optima, and only one of those has truth-surface evidence stronger than inward.
Adding or repartitioning a BH component is therefore not supported as the next
default model change.

The remaining mechanism is genuinely surface-dependent measurement evidence
and/or inherited common-state bias, not merely coarse g3 support. Next work
should decompose the cumulative delta logL hit by hit and compare common-state
construction (forward-posterior versus a controlled independent/broad seed)
before changing BH support. The scan's fixed-variance and single-process
branches are diagnostic counterfactuals, not production components; only the
focused pair has explicit variance sensitivity checks.
