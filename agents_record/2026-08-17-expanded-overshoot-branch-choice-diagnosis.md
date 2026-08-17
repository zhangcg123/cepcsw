# Expanded ECAL overshoot branch-choice diagnosis

Date: 2026-08-17

## Question and frozen setup

The default-off ECAL component-re-ranking prototype was rerun on 20 additional
single-track overshoot candidates to determine when the final branch changes
and why it remains unchanged in other events.  No source or tracked run-card
setting changed.  The diagnostic used the committed defaults:

- tracker-only reverse `BestBranch` with `AggregateWeight`;
- `EcalConstraintRatioThreshold=1.1`;
- Gaussian `log(p/E)` width 0.15;
- likelihood floor 0.05;
- phi and theta cluster windows of 0.10 rad;
- `GSFTracks` preserved and only the paired branch choice examined.

The 20 events came from six seeds and span extreme, moderate, and near-3%
overshoots: seed 7 entries 8, 9, 29, 98; seed 14 entries 1, 64; seed 16
entries 38, 74, 76, 87; seed 26 entries 31, 37, 60, 72; seed 29 entries 0,
39, 61; and seed 49 entries 26, 34, 49.  Each rerun produced one fitted GSF
track.  The event selection used truth to define the diagnostic cohort, but
the runtime ECAL decision used no truth, LCIO momentum, or PFO momentum.

All temporary cards and the 2.2 MB of verbose logs stayed under `/tmp`.  Three
simultaneous detector initializations briefly hit the account process limit;
all jobs nevertheless completed successfully, and later launches used at most
two concurrent jobs.

## Eventwise outcome

| seed/entry | truth pT | tracker pT | paired pT | before | after | decision |
|---|---:|---:|---:|---:|---:|---|
| 7/8 | 12.8059 | 49.7041 | 12.8132 | +288.1% | +0.1% | changed |
| 7/9 | 13.8261 | 14.9974 | 14.9974 | +8.5% | +8.5% | inactive |
| 7/29 | 44.5825 | 47.8510 | 47.8510 | +7.3% | +7.3% | inactive |
| 7/98 | 44.5500 | 45.9127 | 45.9127 | +3.1% | +3.1% | inactive |
| 14/1 | 16.9784 | 51.4908 | 51.4908 | +203.3% | +203.3% | active, same |
| 14/64 | 12.0578 | 17.6907 | 17.6907 | +46.7% | +46.7% | active, same |
| 16/38 | 21.8510 | 62.3554 | 18.9886 | +185.4% | -13.1% | changed |
| 16/74 | 10.9417 | 11.3325 | 11.3325 | +3.6% | +3.6% | active, same |
| 16/76 | 36.5034 | 39.4606 | 39.4606 | +8.1% | +8.1% | inactive |
| 16/87 | 18.2210 | 21.3919 | 21.3919 | +17.4% | +17.4% | active, same |
| 26/31 | 19.0185 | 20.4287 | 20.4287 | +7.4% | +7.4% | inactive |
| 26/37 | 11.7814 | 12.7157 | 12.7157 | +7.9% | +7.9% | inactive |
| 26/60 | 8.3015 | 15.2521 | 8.2901 | +83.7% | -0.1% | changed |
| 26/72 | 21.7698 | 25.8328 | 25.8328 | +18.7% | +18.7% | active, same |
| 29/0 | 37.0138 | 38.4921 | 38.4921 | +4.0% | +4.0% | active, same |
| 29/39 | 9.9671 | 27.1038 | 27.1038 | +171.9% | +171.9% | active, same |
| 29/61 | 19.0398 | 20.5605 | 20.5605 | +8.0% | +8.0% | inactive |
| 49/26 | 22.9173 | 79.6984 | 79.6984 | +247.8% | +247.8% | active, same |
| 49/34 | 13.4515 | 14.5758 | 14.5758 | +8.4% | +8.4% | active, same |
| 49/49 | 38.7199 | 39.9302 | 39.9302 | +3.1% | +3.1% | inactive |

Of these 20 events, 12 activated the ECAL calculation, three changed branch,
and all three changes improved the absolute truth residual.  Nine activated
but kept the tracker branch, and eight remained inactive.

Combining these events with the first six single-track overshoots documented
in `2026-08-17-default-off-ecal-component-constraint-prototype.md` gives 26
focused overshoots: 15 activations, four branch changes, and four improvements.
Three changed events end within 0.3% of truth; seed 16 entry 38 improves greatly
but crosses from +185.4% to -13.1%.  No branch change worsened an event in this
selected overshoot cohort.  This is not clean-track safety evidence.

## Exact branch-choice condition

For a final reverse component `i`, the experimental score is

```text
S_i = T_i [f + (1-f) exp(-0.5 (log(p_i/E)/sigma)^2)]
```

where `T_i` is the existing tracker selection score and `f=0.05`.  A candidate
can beat the baseline only if

```text
T_candidate / T_baseline > L_baseline / L_candidate.
```

Since every likelihood lies in `[0.05,1]`, ECAL supplies at most a factor 20.
Therefore a candidate below 5% of the baseline tracker score can never win,
even in the ideal case where the candidate likelihood is one and the baseline
is at the floor.  This is the principal branch-choice boundary; the activation
threshold only decides whether the comparison is evaluated.

The four successful changes, including the earlier seed 11 entry 41 event,
had candidate-to-baseline tracker-score ratios:

| seed/entry | candidate/baseline tracker score | branch result |
|---|---:|---|
| 11/41 | 0.989 | changed, +92.7% to +0.2% |
| 16/38 | 0.746 | changed, +185.4% to -13.1% |
| 26/60 | 0.194 | changed, +83.7% to -0.1% |
| 7/8 | 0.0549 | changed, +288.1% to +0.1% |

Seed 7 entry 8 is almost exactly the theoretical boundary.  Its baseline score
was 0.5676 and its 17.986 GeV candidate score was 0.03116.  ECAL likelihoods
0.05 and 0.9837 produced final scores 0.02838 and 0.03065, only an 8% winning
margin.  Its correction is therefore real but fragile against ECAL-response or
matching changes.

Seed 16 entry 38 had substantial surviving bimodality.  The 64.049 GeV
baseline and 19.504 GeV candidate tracker scores were 0.5533 and 0.4130.  Their
ECAL likelihoods were 0.05 and 0.7191, giving an unambiguous 0.02766 versus
0.2970 flip.  A 22.235 GeV component was closest to the 22.114 GeV ECAL energy,
but its tracker score was only 0.0113, so the bounded product preferred the
lower 19.504 GeV mode.  This explains the residual -13.1% undershoot.

Seed 26 entry 60 selected a 12.613 GeV-momentum component over the 23.206 GeV
baseline.  The respective tracker scores were 0.1357 and 0.6996; ECAL
likelihoods 0.5070 and 0.0662 yielded final scores 0.0688 and 0.0463.  Although
a 14.639 GeV component was closer to the 15.123 GeV cluster sum, its final
score was only 0.0400.  The selected component nevertheless gives truth-level
pT, showing that the most calorimeter-compatible component need not be the
correct helix branch when calorimeter response fluctuates.

## Why active events remain unchanged

There are two distinct mechanisms.

First, several extreme overshoots have no sufficiently weighted energy-
compatible survivor:

- seed 14 entry 1: the closest-energy component has about `7.6e-21` of the
  baseline tracker score;
- seed 29 entry 39: the closest-energy component has about `2e-17` of the
  baseline tracker score;
- seed 49 entry 26: the closest-energy component has only 0.00267 of the
  baseline tracker score.

All are below the 0.05 recoverability boundary.  They end with 3, 6, and 12
components respectively, so merely reaching `MaxComponents=12` does not imply
that a viable alternative survived.  This agrees with the earlier result that
capacity 24 preserves false modes rather than solving final selection.

Second, in moderate events the baseline may remain the best bounded posterior
even when another component is closer to ECAL energy.  Seed 16 entry 74 is a
useful safety example: reconstructed ECAL energy activates the gate even though
the tracker is only +3.6% high, but the posterior retains the baseline.  Seed
29 entry 0 is similarly only +4.0% high.  The likelihood floor prevents ECAL
from forcing a large change solely because of calorimeter response.

Seed 16 entry 87 and seed 26 entry 72 show the other extreme: their components
closest to ECAL energy have effectively zero tracker posterior (`5.6e-109` and
zero).  The final mixture contains their numerical states but not meaningful
probability mass.  Re-ranking cannot restore that lost evidence.

## Inactive overshoots

Eight events have `max(p/E,E/p)<=1.1`, so no component score is changed.  Two
are almost exactly on the configured boundary: seed 16 entry 76 has 1.09950
and seed 26 entry 37 has 1.09910.  Others range down to 1.035.  This confirms
that the threshold controls the active count near 1.1, but the active-event
evidence shows that lowering it cannot by itself overcome a negligible
candidate tracker posterior.

## Conclusion and next gate

The expanded diagnosis supports a conditional statement, not general
validation: ECAL re-ranking can repair an overshoot when a suitable alternative
survives with enough tracker posterior.  With the current likelihood floor,
the practical necessary condition is a candidate tracker score above about 5%
of the baseline, followed by sufficient ECAL likelihood separation.  Extreme
failures below that boundary are unrecoverable by this final-selection method.

The next informative validation is no longer another overshoot-only selected
sample.  It is a same-code held-out population measurement of:

1. how often a candidate above the recoverability boundary exists;
2. correction, partial-correction, and wrong-change rates;
3. clean/no-eBrem false activation and false branch-change rates;
4. behavior in genuine light/hard and transition-0--4 underestimates;
5. stability versus ECAL energy closure, angular matching, and energy/angle.

Do not lower the likelihood floor, change the threshold, or promote the mode
from these selected overshoots alone.  A stronger ECAL Bayes factor could
recover more extreme cases only by weakening the present protection against
calorimeter fluctuations and clean-track changes.
