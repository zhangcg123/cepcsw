# Default-off ECAL component-constraint prototype

Date: 2026-08-17

## Outgoing focus

The preceding focus followed completion of the reverse seed, innovation-
coverage, physical-region, and measurement-ownership audits.  Those audits did
not identify a reproducible GSF-specific defect and ruled out another round of
tracker-internal capacity, covariance-scale, selection-mode, or literal-layer
tuning.  The open question was whether a reconstructed observable independent
of the tracker, most naturally calorimeter energy, could distinguish genuine
energy-loss recovery from false tracker-only radiative modes without truth at
runtime.  The frozen tracker-only baseline, rejected internal alternatives,
validation requirements, and non-goals remain authoritative through the
records linked from `AGENTS.md`.

## Authorized prototype

The user authorized a first algorithm trial with four requirements:

- support both high-momentum overshoots and low-momentum, early-transition
  underestimates;
- make the activation threshold configurable;
- constrain final GSF components rather than replacing the track with a
  track-calorimeter combination;
- preserve the original tracker-only GSF parameters.

The implementation stays inside `Reconstruction/RecGsfTracking` and is
default-off.  It does not modify any component state or covariance.  It writes
the normal tracker-only result to `GSFTracks` and, only when enabled, writes a
paired experimental result to `GSFTracksEcalConstrained`.

The ECAL observation uses neither LCIO momentum nor PFO momentum.  It takes the
highest-weight final forward GSF component as an outer reference, projects its
tangent to each `EcalCluster` radius, and sums positive-energy clusters within
both configured azimuth and polar-angle windows.  A phi-only first smoke test
collected a cluster at the wrong polar angle, so the polar-angle requirement
was added before the focused evaluation.

The tracker-only branch activates ECAL re-ranking when
`max(p/E,E/p) > EcalConstraintRatioThreshold`.  Each surviving final reverse
component receives the bounded multiplier

```text
floor + (1 - floor) exp[-0.5 (log(p/E) / sigma)^2]
```

and the branch with the largest tracker selection score times this multiplier
is copied to the paired collection.  This is two-sided and bounded: it can
prefer either a lower- or higher-momentum surviving component, while the
default likelihood floor 0.05 limits the ECAL Bayes factor to 20.

New defaults are:

- `EcalComponentConstraint=false`;
- `EcalConstraintRatioThreshold=1.1`;
- `EcalConstraintLogPSigma=0.15`;
- `EcalConstraintLikelihoodFloor=0.05`;
- `EcalConstraintPhiWindow=0.10` rad;
- `EcalConstraintThetaWindow=0.10` rad.

The feature currently requires ordinary reverse filtering,
`ReverseOutputMode=BestBranch`, and `CmsGsfSmoothing=false`.  A dedicated
property audit found 40 configurable properties in total and confirmed that
all 40 are documented in `Reconstruction/RecGsfTracking/README.md` and set
explicitly in `DumpGsfTrks/gsf.py.bk`.  The maintained card reads
`EcalCluster`, leaves the feature off, and preserves the frozen tracker-only
baseline.

## Mechanical validation

- `RecGsfTracking` built and installed successfully in the normal EL9/LCG 105
  build.
- A three-event enabled smoke run completed and produced both collections.
- For the enabled run, the paired output was exactly equal to `GSFTracks` in
  all three events when selection did not change: all helix parameters,
  reference points, covariance entries, chi-square, and NDF matched.
- A same-code enabled/disabled A/B found `GSFTracks` exactly equal in all three
  events.  Thus enabling the experiment does not mutate the original result.
- Comprehensive component dumps completed for the available early-transition
  underestimate and for the overshoot whose selected branch changed.
- The former canonical hard-loss seed-1 input for events 11, 16, and 17 had
  been deleted and was not relabelled or replaced by unrelated events.

All temporary cards, ROOT files, and logs were kept under `/tmp`.

## Focused physics result

The first check used the current reconstructed-event sample and threshold 1.1.
It is a mechanism test, not a validation population.

Six single-track overshoots above 3% were checked.  Three activated the ECAL
gate.  One changed branch and was repaired; two activated but retained the
same branch because no competing component had enough posterior support.  The
other three stayed inactive.  Seed 10 entry 48 was also inspected separately:
it contained two reconstructed tracks despite the existing table's
topology-clear label, so it is a topology/control event rather than part of the
single-track count.

| seed/entry | truth pT (GeV) | tracker pT (GeV) | constrained pT (GeV) | result |
|---|---:|---:|---:|---|
| 10/6 | 36.2306 | 37.4260 | 37.4260 | inactive |
| 10/36 | 39.5968 | 45.8904 | 45.8904 | active, same component |
| 10/77 | 10.6058 | 11.3915 | 11.3915 | active, same component |
| 10/99 | 41.8522 | 44.3448 | 44.3448 | inactive |
| 11/41 | 11.6818 | 22.5057 | 11.7097 | active, repaired |
| 11/45 | 41.5014 | 42.9897 | 42.9897 | inactive |

In the repaired seed 11 entry 41 event, the tracker posterior was genuinely
bimodal.  The selected high branch had momentum 23.1269 GeV and tracker score
0.2520; a low branch had momentum 12.0329 GeV and tracker score 0.2491.  With
matched ECAL energy 14.1729 GeV, the bounded likelihoods changed their scores
to 0.0138 and 0.1429 respectively, selecting the low branch.  This is the
intended use case: ECAL resolves an ambiguity that the tracker already
retained.

Only one topology-clear event in the current ECAL sample could be joined to
the historical dominant-transition-0--4 category with a tracker underestimate
larger than 3%: seed 11 entry 1.  Its truth and tracker pT were 17.2383 and
15.4493 GeV.  At threshold 1.1 the gate stayed inactive because the matched
energy was 17.6430 GeV and `max(p/E,E/p)=1.079`.  Lowering only the threshold to
1.05 activated the calculation but did not change the branch.  The surviving
18.2564 GeV candidate had tracker score 0.000522, versus 0.660533 for the
selected 16.3517 GeV-momentum branch; their ECAL likelihoods were 0.9757 and
0.8855.  ECAL could not overcome a tracker-score ratio of about 1265 when both
momenta were calorimetrically plausible.

## Interpretation and next gate

The prototype demonstrates a narrow but real capability: independently
reconstructed ECAL energy can repair a false radiative mode when the final GSF
mixture still contains a comparably weighted alternative.  It does not create
a missing alternative, and changing the activation threshold cannot solve
that limitation.  Threshold 1.1 is therefore only an experimental default,
not an optimized or validated cut.

Next work must measure branch changes and false changes on same-code held-out
no-eBrem, light, hard, early-transition, and secondary-topology populations.
It must audit ECAL matching efficiency and energy closure separately across
energy and angle.  The tangent projection, fixed 0.10-rad windows, log-energy
resolution 0.15, likelihood floor 0.05, and threshold 1.1 are all unvalidated.
No setting may be promoted until the ordinary tracker-only output remains
untouched, clean-track safety is demonstrated, useful light/hard recovery is
preserved, and an independent dataset is available.
