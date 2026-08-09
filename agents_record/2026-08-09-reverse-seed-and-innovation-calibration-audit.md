# Reverse seed and innovation-likelihood calibration audit

Date: 2026-08-09

## Question and constraints

The outgoing live focus asked whether the reverse innovations or transported
covariances at the frequently decisive relative hit indices 5--8 make the
identity-versus-radiative likelihood ratio directionally inconsistent or
overconfident. It also asked whether reusing the final forward posterior as the
reverse seed accidentally reuses enough information to create the false
radiative modes.

This was a diagnostic-only study. The installed implementation and the frozen
production steering were used without source, header, build, or maintained run
card changes. All generated options, ROOT outputs, logs, parsers, and tables
were kept under `/tmp`; ROOT outputs were removed immediately after each run,
and the remaining temporary diagnostics were removed after this record was
written.

The baseline was the configuration in `DumpGsfTrks/gsf.py.bk` and tag
`gsf-memory-leak-fixed-2026-08-08`: `CEPC2GeV85StepConditioned`,
`MaxComponents=12`, `ReductionTargetComponents=0`, `SymmetricKL`, identity
lineage protection, `ComponentWeightCutoff=1e-4`, forward-posterior reverse
weights, `ReverseKappaSeedCov=100`, `AggregateWeight`, and reverse
`BestBranch` publication.

## Samples and runs

Three samples were evaluated with comprehensive component diagnostics:

- The matched selection-boundary sample contained 19 no-eBrem events whose
  false radiative result was repaired by the previous scalar-density selector
  and seven genuine light-eBrem recoveries which that selector lost. Each event
  was run at reverse seed covariance scales 100 and 1,000,000. All 52
  event-variant runs completed.
- A first outcome-unselected clean control contained 50 events: the first ten
  primary-electron events without a Geant4 `eBrem` step from each of seeds
  1--5. The GSF outcome was not used for selection. All 50 runs completed.
- An independent held-out clean control used the same rule on seeds 6--10,
  again taking ten events per seed. All 50 runs completed.

The authoritative no-eBrem label came from the primary-electron Geant4 step
records (`process_subtype == 3` was absent), not SimTrackerHit momentum. The
input reconstruction files were `tuples285/trk-e--2.0-85-<seed>.root`, and the
labels were read from
`tuples285/gsf_material_steps-e--2.0-85-<seed>.root`.

The 19 clean boundary events were:

```text
(1,41) (3,6) (7,6) (7,8) (14,28) (15,89) (16,14) (16,76)
(17,94) (18,44) (21,38) (24,8) (24,23) (25,76) (33,90)
(34,28) (49,49) (49,59) (50,82)
```

The seven genuine light-eBrem recoveries were:

```text
(13,70) (33,70) (39,29) (41,93) (42,23) (44,41) (50,79)
```

The held-out control events were:

```text
seed 6:  0 1 12 18 21 23 26 27 28 30
seed 7:  7 10 11 13 14 15 21 28 30 32
seed 8:  0 2 3 5 6 13 14 17 21 25
seed 9:  3 5 7 8 12 15 19 22 24 31
seed 10: 1 8 13 16 17 19 28 30 31 32
```

## Result 1: broadening the reverse seed does not repair selection

Changing `ReverseKappaSeedCov` from 100 to 1,000,000 left all 19 selected clean
tails outside the +/-1% core. Their median absolute residual moved only from
2.662% to 2.546%, and only three events changed by more than 0.1 percentage
point. The seven light-eBrem recoveries all stayed inside +/-1%, with their
median absolute residual changing from 0.765% to 0.757%.

The very broad seed was not benign: clean event `(7,8)` moved from a 288% to a
723% residual, while `(14,28)` improved from 2.89% to 1.62% but did not return
to the core. It is therefore not an implementation candidate.

The state-by-state comparison explains the small population effect. The median
absolute identity-update delta-chi-square difference between scales was 0.828
at the first reverse update, 0.554 after 10--19 inward updates, 0.218 after
50--99, 0.038 after 100--199, and 0.010 after 200 or more. The median relative
identity-state transverse-momentum difference was 0.00129 after 100--199
updates and 0.000156 after 200 or more. Long CEPC tracks effectively forget the
forward seed before the inner decisive measurements. The ACTS-like seed reuse
is consequently not the leading cause of these population tails, although a
formal two-filter construction remains mechanically different.

## Result 2: relative hit index is not a physical layer

Navigation records show that relative hit indices 5--8 do not denote fixed
detector layers in the broad 10--50 GeV and theta 40--140 degree sample. For
different trajectories, hit 5 can be in the VXD at radius about 43 mm or in a
silicon system at radii about 235 or 555 mm. Calibration claims must therefore
be grouped by decoded detector system and physical radius, not by a literal
relative index. The earlier phrase "surfaces 5--8" identifies where decisions
appear in the ordered track, not a universal material-layer location.

## Result 3: the apparent VXD reverse undercoverage does not reproduce

In the first 50 clean controls, VXD identity innovations suggested reverse
undercoverage: 52 updates had mean two-dimensional chi-square 2.803, pull RMS
values 1.326 and 1.022, and a summed-chi-square two-sided p-value of 0.0087.
This could have motivated a local covariance correction if it had survived an
independent cohort.

It did not. In the held-out 50 controls, 44 VXD updates had reverse mean
chi-square 1.999, pull RMS values 1.065 and 0.931, and p=0.962. The forward mean
was 2.004. The first-cohort VXD excess is therefore treated as a sampling
fluctuation, not a reproducible reverse-likelihood defect.

A mild first-coordinate pull width near 1.17 at the silicon radius around
235 mm repeated in both clean halves. However, the corresponding forward
innovations show a similar scale, and the false-clean cohort does not show an
elevated median identity chi-square there (median 1.574). This is a monitoring
observation about common measurement modelling, not a reverse-GSF-specific
correction candidate and not evidence to change a shared tracking package.

The two independent controls also passed the relevant final safety check. The
first set had 49/50 GSF results inside +/-1% versus 50/50 for LCIO, with one GSF
tail at seed 3 event 11. The held-out set had 50/50 inside +/-1% for both GSF
and LCIO.

## Result 4: false and genuine radiative modes receive overlapping real evidence

At relative hit 5, the best radiative-versus-identity local log-likelihood
ratio had these medians:

| Cohort | Events/updates | Median local log LR | Positive local LR |
| --- | ---: | ---: | ---: |
| outcome-unselected clean control | 50 | -0.004 | 22/50 |
| selected false-clean boundary | 19 | 0.415 | 17/19 |
| genuine light-eBrem recovery | 7 | 1.201 | 7/7 |

For the false-clean cohort, the median identity-minus-radiative chi-square gain
was 0.903 and the median log-determinant contribution was -0.079. For the
genuine light cohort they were 2.882 and -0.480. Thus the positive result is
not created solely by covariance volume: the accepted measurements themselves
prefer a radiative state in both outcome classes.

At hits 6 and 7, the median log likelihood ratios were much smaller for false
clean events (0.0048 and 0.0057) but remained positive in 12/19 and 16/19;
genuine light medians were 0.115 and 0.194. Hit 8 did not discriminate either
cohort. Pooling hits 5--8 gives a control median of -0.00134 with 88/200
positive updates, compared with 0.00582 and 54/76 for the selected false-clean
updates (Mann-Whitney p=5.85e-5). Outcome selection naturally enriches positive
evidence, but this confirms that likelihood, rather than KL reduction, drives
the selected false cohort.

Physical-region grouping gives the same interpretation. In the VXD, the
median event-level maximum local log LR was 0.421 for the false-clean events
and 1.201 for genuine light events. Their ranges overlap: false clean spans
-0.078 to 14.529 and genuine light 0.216 to 22.386. A threshold of 0.5 would
reject 11/18 false clean cases but also 3/7 genuine recoveries; a threshold of
1 would reject 13/18 false cases but still lose 3/7 genuine recoveries. The
ordinary held-out controls reached only 0.368, but the selected boundary
samples show that there is no truth-free gap between false and genuine modes.

## Result 5: no accidental duplicate measurement was found

The read-only implementation audit found a consistent ordinary reverse loop:

- it starts at `hits.size()-2`, uses `hits[reverseHit+1]` as the transport
  reference, and uses `hits[reverseHit]` as the target;
- it applies the material transition before the target update;
- it calls the baseline `addAndFit` once for each branch at that target, uses
  the exact returned update for the innovation likelihood, and then performs
  cutoff/reduction;
- it decrements `reverseHit` once and does not present the same target twice in
  the ordinary reverse path.

There is deliberate reuse of the final forward posterior as the reverse seed,
but the scale experiment above shows its state and covariance memory is largely
washed out before the relevant inner hits on long tracks. No demonstrated
measurement-ownership or inter-surface transport defect was found.

## Decision and new boundary

This audit does not support an implementation change. In particular, it
rejects the following as evidence-based next steps:

- increasing the reverse seed covariance;
- applying a VXD or relative-hit-5 covariance correction;
- globally rescaling measurement or process covariance;
- adding a local evidence threshold;
- interpreting relative hit indices 5--8 as fixed CEPC layers;
- changing KL ranking, component capacity, or final posterior publication to
  compensate for this likelihood behaviour.

Under the current tracker measurements and Bethe-Heitler hypotheses, the
diagnosed false and genuine radiative branches are statistical/information-
limited boundary cases: both can be supported by the same accepted hit
residual pattern. The current point-estimate GSF has no demonstrated internal
calibration correction that separates them while preserving genuine light
recovery.

The next scientifically motivated question is whether an independent
reconstructed observable, most naturally calorimeter energy or an E/p-like
constraint, can distinguish real energy-loss recovery from tracker-fit
fluctuations without using truth at runtime. That is a new integration scope,
not authorization to modify packages. A truly measurement-disjoint two-filter
GSF remains a possible formal research control, but the seed-memory result does
not identify it as the likely clean-tail solution. Either direction requires a
new explicit design decision and held-out validation before implementation.

The superseded final-selector, KL, capacity-24, and multidimensional BH options
remain rejected for the reasons recorded in the linked 2026-08-08 and
2026-08-09 studies. A new independent light-tail dataset remains necessary for
any production-performance claim.
