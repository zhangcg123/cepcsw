# RecGsfTracking

`RecGsfTracking` refits `CompleteTracks` with a Gaussian-sum electron model and
writes the tracker-only result to `GSFTracks`. A default-off ECAL experiment
can additionally write a paired component-selection result to
`GSFTracksEcalConstrained`. Each component uses the baseline MarlinTrk
`addHit(reference) -> initialise(componentState) -> addAndFit(currentHit)`
update path.  The old alternate KF fitter and initialization experiments have
been removed; historical comparisons remain under `agents_record/`.

## Complete configuration reference

Reference date: 2026-08-17. `RecGsfTracking` exposes 40 Gaudi properties in
`src/GsfAlgorithm.h`. “Compiled” below means constructing the algorithm
without a run card. “Active reverse” means the effective no-environment-
override configuration in `options/run_gsf_reverse_template.py`. The
distinction matters because that template enables `ElossOn` and
`ReverseFiltering`, whose compiled defaults are false.

### Physics and material model

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `ElectronHypothesis` | `true` | `true` | Enable electron-hypothesis BH processing. Set false for forced no-BH particle controls. |
| `BHModel` | `CEPC2GeV85StepConditioned` | same | Select the BH Gaussian-mixture parameterization. The five-component conditioned model is active; the six-component conditioned model and `ActsAtlas` are default-off research controls. |
| `BHSplitThreshold` | `1e-4` | same | Minimum component-local outgoing material thickness used to trigger a BH process split. |
| `MSOn` | `true` | `true` | Enable multiple-scattering process noise in the underlying track fit. |
| `ElossOn` | `false` | `true` | Enable the baseline KalTest deterministic energy-loss treatment in addition to BH splitting. |
| `MaterialPathMode` | `CurrentSurface` | same | Material assignment: `CurrentSurface` is active; `DD4hepBetweenSurfaces` is a default-off volume-integration diagnostic. |
| `MaterialIPExtrapolation` | `false` | `false` | Include material effects during final extrapolation to the interaction point. Kept off in the active workflow. |
| `KappaSeedCov` | `1e-7` | same | Forward GSF seed-covariance scale; the small baseline value tightly anchors the start to `CompleteTracks`. |

Material assigned to a measurement surface is owned by the outgoing
transition from that surface. Its inner and outer normal-thickness `t/X0`
contributions are divided by the absolute dot product of the component-local
track tangent and DD4hep surface normal. The final measurement has no outgoing
transition. Forward and reverse workflows apply the same current-surface
ownership in their respective directions.

### Mixture population and reduction

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `MaxComponents` | `12` | `12` | Posterior-reduction trigger/capacity. A BH split is updated before reduction, so this is not an instantaneous ceiling. Keep 24 only as an explicit comparison. |
| `ReductionTargetComponents` | `0` | `0` | Number retained after reduction; zero means use `MaxComponents`. Valid values are zero or `1..MaxComponents`. |
| `ReductionMergeCost` | `SymmetricKL` | same | Pair-ranking cost for moment merging: active `SymmetricKL` or default-off weighted `Runnalls`. Runnalls was tested and rejected for promotion. |
| `ComponentWeightCutoff` | `1e-4` | `1e-4` | Remove normalized target-measurement posterior components below this weight while retaining at least the largest and, when enabled, an identity lineage. |
| `ProtectIdentityLineage` | `true` | `true` | Preserve at least one exact no-radiation lineage through cutoff and reduction when the target component count exceeds one. |

Forward children from transition `i -> i+1` remain expanded through
measurement `i+1`; reverse children from `i+1 -> i` remain expanded through
measurement `i`. The exact innovation likelihood is applied and normalized
before cutoff and reduction. A transition can therefore temporarily require
roughly `MaxComponents * number-of-BH-modes` measurement updates.

### Forward and reverse publication

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `GSFOutputMode` | `BestBranch` | same | Forward/output publication: `BestBranch` or moment-matched `WeightedMean`. Weighted publication is default-off. |
| `ReverseFiltering` | `false` | `true` | Run the independent inward multi-component refit from the complete final forward mixture. This is the active production candidate. |
| `ReverseKappaSeedCov` | `100` | `100` | Multiply every full-mixture reverse-seed covariance by this factor. |
| `ReverseInitialWeightMode` | `ForwardPosterior` | same | Reverse-start weights: active `ForwardPosterior` or default-off `Uniform` diagnostic. |
| `ReverseOutputMode` | `BestBranch` | same | Publish the highest-ranked reverse branch or a moment-matched `WeightedMean`. |
| `ReverseSelectionMode` | `AggregateWeight` | same | Final branch score: active `AggregateWeight`; rejected diagnostics `DominantLineage` and `SurfaceConsistency`. |
| `SurfaceConsistencyUninformativeFloor` | `0.05` | same | Lower bound used only by `SurfaceConsistency`; 0.05 caps its selection Bayes factor at 20. |

`AggregateWeight` selects the component with the largest normalized weight.
`DominantLineage` multiplies that weight by the fraction supplied by its
strongest real pre-merge lineage. `SurfaceConsistency` multiplies it by a
bounded forward/reverse radiative-surface coincidence likelihood. Both
alternatives are retained only to reproduce rejected diagnostics.
`ProtectIdentityLineage` is a reduction safeguard, not another selection mode.

### Experimental ECAL component constraint

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `EcalComponentConstraint` | `false` | `false` | Enable a default-off, two-sided ECAL likelihood that can re-rank the already fitted final reverse components. It requires ordinary reverse filtering, `ReverseOutputMode=BestBranch`, and no CMSSW-like workflow. |
| `EcalConstraintRatioThreshold` | `1.1` | same | Activate re-ranking only when the unconstrained branch has `max(p/E,E/p)` above this value. It must be finite and greater than one. |
| `EcalConstraintLogPSigma` | `0.15` | same | Gaussian width of the component likelihood in `log(p/E)`; it must be finite and positive. |
| `EcalConstraintLikelihoodFloor` | `0.05` | same | Additive likelihood floor in `(0,1]`; 0.05 limits the ECAL re-ranking Bayes factor to 20. |
| `EcalConstraintPhiWindow` | `0.10` | same | Maximum absolute azimuth difference, in radians and in `(0,pi]`, for summing positive-energy `EcalCluster` objects around the extrapolated outer forward-GSF direction. |
| `EcalConstraintThetaWindow` | `0.10` | same | Maximum absolute polar-angle difference, in radians and in `(0,pi]`, for the same cluster-energy sum. A cluster must pass both the phi and theta windows. |

The ECAL observation uses neither truth nor LCIO/PFO momentum. It sums
`EcalCluster` energy inside both configured angular windows around the
extrapolated outer forward-GSF direction and, after the ordinary reverse fit
has finished, multiplies each final component's existing reverse selection
score by

```text
floor + (1 - floor) * exp[-0.5 * (log(p_component/E) / sigma)^2].
```

This is a selection constraint, not a track--calorimeter parameter
combination: it does not alter any fitted component state or covariance.
`GSFTracks` always preserves the unconstrained tracker-only result. When the
experiment is enabled, `GSFTracksEcalConstrained` is created alongside it. If
the ECAL observation is unavailable or the symmetric ratio does not cross the
threshold, the paired output is an exact parameter/covariance copy of the
unconstrained result; otherwise it can publish a different existing reverse
component. The mode is an unvalidated research control and remains inactive in
the production baseline.

### Alternative backward workflows

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `GaussianSumSmoothing` | `false` | `false` | Run the retained-graph experimental Gaussian-sum smoother. It is default-off and forfeits much of the observed hard-loss recovery. |
| `CmsGsfSmoothing` | `false` | `false` | Run the experimental CMSSW-like backward workflow instead of ordinary reverse filtering. |
| `CmsErrorRescaling` | `100` | `100` | Covariance scaling for the CMSSW-like backward seed; inactive unless `CmsGsfSmoothing=true`. |

`ReverseFiltering`, `GaussianSumSmoothing`, and `CmsGsfSmoothing` are
alternative workflows and must not be enabled simultaneously.

### Focused-event and component diagnostics

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `SelectedEventIndices` | empty | empty | Empty processes the normal event stream; otherwise process only the listed zero-based input entries. |
| `VerboseDump` | `false` | `false` | Print general filtering, track, and workflow diagnostics. |
| `VerboseSplitDump` | `false` | `false` | Dump component populations around BH splits, cutoff, and reduction. |
| `ComponentDebugDump` | `false` | `false` | Dump exact component states, innovation quantities, and lineage histories. |
| `SurfaceLineageMassDump` | `false` | `false` | Propagate and print aggregate BH-mode probability mass by surface. |
| `ComponentDebugMaxHistory` | `240` | `240` | Maximum process/lineage history retained per component for debug output. |
| `MaterialTransitionCSV` | empty | empty | Optional path for a component-local outgoing-surface material audit; empty disables it. |

The reverse template connects the first three verbose properties to
`GSF_VERBOSE_COMPONENTS`. Comprehensive focused-event validation normally
enables all three together. Generated logs and CSV files are outputs, not
project-status records.

### Counterfactual loss scan

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `CounterfactualLossScan` | `false` | `false` | Enable a truth-assisted, likelihood-only trial-loss diagnostic. |
| `CounterfactualTruthTransitionMap` | empty | empty | Comma-separated `event:transition` locations at which to test hypothetical losses. |
| `CounterfactualLossFractions` | `0.04,0.05,0.06,0.07,0.08,0.09,0.10,0.12` | same | Fractional momentum losses assigned to trial branches. |
| `CounterfactualLossVariance` | `2e-4` | same | Retained-momentum-fraction variance assigned to each trial branch. |

For example:

```text
CounterfactualTruthTransitionMap = "1:7,4:8"
```

This tests the configured losses at truth transition 7 for event 1 and
transition 8 for event 4. The scan reports validity, accepted-hit count,
cumulative measurement log-likelihood, and final hypothetical momentum.
Trial branches never enter or reweight the live mixture and cannot become the
published track. This is a truth-assisted mechanism study, not a production
loss estimator. When `CounterfactualLossScan=false`, the other three
properties have no effect.

### Collection handles

The data handles are configurable separately from the 40 properties:

| Role | Default collection |
|---|---|
| input reconstructed tracks | `CompleteTracks` |
| output refitted tracks | `GSFTracks` |
| input ECAL clusters | `EcalCluster` |
| paired ECAL-constrained output tracks | `GSFTracksEcalConstrained` |
| truth particles | `MCParticle` |

### Historical `DumpGsfTrks` card compatibility

`DumpGsfTrks/gsf.py.bk` with `method="reverse"` explicitly configures all 40
properties and agrees with the active reverse template, including
`ComponentWeightCutoff=1e-4` and the default-off ECAL experiment. It silently
inherits no configurable property. Its reconstructed-event input list includes
`EcalCluster`, and `keep *` preserves the paired constrained collection when
the experiment is explicitly enabled.

### Configuration-maintenance contract

The 40-property inventory above is part of the configurable interface, not a
one-time snapshot. Any change that adds, removes, or renames a
`RecGsfTracking` property, changes its compiled or active default, or changes
its accepted values must include a dedicated sub-agent configuration audit.
That audit must, in the same change:

1. update this complete property reference and recount the exposed properties;
2. update `options/run_gsf_reverse_template.py` when the active effective
   steering changes;
3. add or update an explicit setting in `DumpGsfTrks/gsf.py.bk`, including an
   intentional inactive value or a documented inapplicability for
   workflow-specific diagnostics, so the historical workflow does not
   silently acquire a future compiled default;
4. record any intentional difference between that card and the active
   baseline in `DumpGsfTrks/README.md`;
5. verify that every property declared in `src/GsfAlgorithm.h` is documented
   here and classified as active, inactive, experimental, or diagnostic.

Do not consider a configurable-property implementation complete until this
documentation and steering audit is complete.

## Current limitation

`CEPC2GeV85StepConditioned` is the constrained eight-knot, five-component
transition model for the 2 GeV pT, 85-degree primary-electron execution sample.
It consumes the component-local transition `t/X0` and returns hypotheses in
`z=p_after/p_before`. Its source artifact and diagnostics are under
`data/CEPC2GeV85StepConditioned/`. This same-sample execution model is not a
general or independently validated CEPC Bethe-Heitler parameterization.

`CEPC2GeV85StepConditioned6` uses the same events, t/X0 bins, interpolation,
and total radiative probabilities. It keeps the no-eBrem, 0--1%, and >20%
components, while replacing the former 1--5% plus 5--20% pair with three
truth-extracted 1--5%, 5--10%, and 10--20% components. Its artifact is under
`data/CEPC2GeV85StepConditioned6/`. It is selectable for comparison and is not
the default or a validated improvement.

## Geant4 transition dataset

`GsfMaterialStepRecorderAnaElemTool` records authoritative Geant4 pre/post-step
truth. Its tuple includes true track-step order, sensitive-volume and touchable
identifiers, track-length coordinates, and momentum directions. Tracker-region
DD4hep constants are explicitly converted to millimetres.

Build outgoing-current transition rows with:

```bash
source setup.sh
build.105.0.0.x86_64-el9-gcc11-opt/run python3 \
  Reconstruction/RecGsfTracking/scripts/build_g4_transition_dataset.py \
  'path/to/gsf_material_steps*.root' \
  --output /tmp/gsf_transitions.csv
```

The builder aggregates primary-electron steps from entry into one sensitive
element to entry into the next. It maps the CEPC TPC lower/upper sensitive
half-layer pair to one reconstructed pad-row anchor. `g4_t_over_x0` remains a
Geant4 diagnostic; `reco_t_over_x0` is intentionally empty until the rows are
matched to the owned `RecGsfTracking` surface transitions.

Compare a G4 transition CSV with the GSF material audit using
`scripts/compare_g4_reco_material_transitions.py`. Select
`--reco-column path_t_over_x0` for the active current-surface calculation or
`--reco-column geometry_path_t_over_x0` for DD4hep volume integration. In
matched event 11, the respective Geant4 and DD4hep totals are 0.0737544 and
0.0739544 X0; the hard-eBrem transition agrees to 0.064%. Select
`--reco-column interval_path_t_over_x0` only for the diagnostic crossed-cradle
sum, which is not authoritative material semantics.
