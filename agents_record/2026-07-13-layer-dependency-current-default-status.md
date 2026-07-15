# Layer dependency and current-default status (2026-07-13)

## Active configuration and sample

The optimization focus returns to the current default reverse-GSF workflow:
`ReverseFiltering=True`, `ReductionMode=KL`, `ReverseOutputMode=BestBranch`,
`ReverseSelectionMode=AggregateWeight`, and
`BHModel=CEPC2GeV85StepConditioned`, with default component limits, cutoff,
covariances, and material ownership. TopN RTS, native KalTest smoothing, broad
covariance scans, and dominant-lineage publication are default-off diagnostics
and must not be mixed into default performance results.

The active light sample has 2,132 topology-clean events after excluding every
event with non-primary tracker SimHits. The dominant transition index orders
reconstruction-owned Geant4 surface intervals outward from the IP; it is not
yet a direct named detector-layer identifier.

## Layer/surface dependency

For events where LCIO is biased enough to classify recovery, the default
reverse-GSF recovery rate changes sharply with dominant-loss surface:

| dominant transition | eligible | recovered | missed | recovered fraction |
|---|---:|---:|---:|---:|
| 0--2 | 30 | 2 | 28 | 6.7% |
| 3--4 | 60 | 5 | 55 | 8.3% |
| 5--6 | 163 | 93 | 70 | 57.1% |
| 7--8 | 213 | 193 | 20 | 90.6% |
| 9--11 | 34 | 31 | 3 | 91.2% |

Controlling for dominant loss magnitude strengthens the result. For 3--5%
loss, recovery is 0/6, 3/17, 36/46, 54/55, and 6/6 over the same bands: 0%,
17.6%, 78.3%, 98.2%, and 100%. For 5--7% loss it is 0/4, 0/10, 19/27,
21/21, and 3/3.

Default LCIO/GSF pT-resolution panels use 0.5%-wide bins over `[-15%,15%]`
and are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/loss_surface_dependency_2026-07-13/`.
The six-panel plot includes 0--2, 3--4, 5--6, 7--8, 9--11, and aggregate `>11`:

| transition | N | LCIO median | GSF median | LCIO inside 1% | GSF inside 1% |
|---|---:|---:|---:|---:|---:|
| 0--2 | 73 | -0.459% | -0.451% | 43 | 42 |
| 3--4 | 142 | -0.545% | -0.490% | 82 | 82 |
| 5--6 | 499 | -0.349% | -0.175% | 332 | 383 |
| 7--8 | 522 | -0.543% | -0.106% | 306 | 445 |
| 9--11 | 84 | -0.509% | -0.108% | 50 | 74 |
| >11 | 812 | -0.073% | -0.044% | 736 | 743 |

The `>11` population is heterogeneous and mostly truth-like. Its central GSF
shape improves slightly, but GSF has 14 versus 6 LCIO events outside the plot
range and 794 versus 798 inside 5%; it is not a uniform late-loss category.

## Mechanistic evidence

The dependence is not a gross global BH-weight error. Independent training and
expanded-sample extractions find total radiative probabilities compatible
within 0.87 standard deviations in every t/X0 bin.

Loss-matched missed 433/6 (3.8069% at transition 0) and recovered 369/1
(3.8266% at transition 6) isolate measurement leverage. In 369/1,
truth-compatible weight grows from 0.09% after hit 6 to 38.5% after hit 5 and
finishes at 81.5%. In 433/6, a compatible branch survives but never reaches
0.1% and finishes near 0.0003%. KL deletion and absent process support are
excluded for that miss; inner hits never provide sufficient likelihood.

TopN RTS and default-KL native-smoother covariance scans are negative for
reverse-missed events at transitions 0--7. The historical hard 1/3 covariance
recovery does not generalize and must not redirect default optimization. Full
smoother evidence is preserved in
`agents_record/2026-07-13-inner-loss-retained-lineage-rts-test.md` and
`agents_record/2026-07-13-default-kl-native-smoother-kappa-scan.md`.

## Active interpretation and resume point

Layer dependency is a central project result and optimization boundary:

- transitions 0--4 are predominantly information-limited tracker-only cases;
- transitions 5--6 are the recoverability boundary;
- transitions 7--11 are normally recoverable, so their remaining misses,
  partial corrections, and overshoots are actionable algorithm diagnostics.

Continue only with the current default reverse workflow. Pair similar-loss,
same-surface events in transitions 5--11 and record decisive-hit process priors,
exact innovation likelihoods, posterior odds, and KL survival. Quantify the
fractions attributable to prior limitation, measurement support, and reduction
before testing one mechanism-specific change. Do not tune the algorithm to
force transitions 0--4, and do not add a global covariance or BH-prior scale,
ad hoc evidence threshold, or default smoother.

Beam-spot and calorimeter-informed multimodal constraints remain possible
future studies for information-limited inner losses, but neither belongs to
the current default optimization and neither is implemented or validated.
