# RecGsfTracking

`RecGsfTracking` refits `CompleteTracks` with a Gaussian-sum electron model and
writes the tracker-only result to `GSFTracks`. A default-off ECAL experiment
can additionally write a paired component-selection result to
`GSFTracksEcalConstrained`. Each component uses the baseline MarlinTrk
`addHit(reference) -> initialise(componentState) -> addAndFit(currentHit)`
update path.  The old alternate KF fitter and initialization experiments have
been removed; historical comparisons remain under `agents_record/`.

## Complete configuration reference

Reference date: 2026-08-19. `RecGsfTracking` exposes 40 Gaudi properties in
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
| `MaterialPathMode` | `DD4hepBetweenSurfaces` | same | Material assignment for both outward and inward propagation. The default integrates the complete DD4hep volume interval between matched measurement endpoints in canonical inner-to-outer order; `CurrentSurface` remains an explicit comparison control. |
| `MaterialIPExtrapolation` | `false` | `false` | Include material effects during final extrapolation to the interaction point. Kept off in the active workflow. |
| `KappaSeedCov` | `1e-7` | same | Forward GSF seed-covariance scale; the small baseline value tightly anchors the start to `CompleteTracks`. |

Material between consecutive accepted measurements is owned by the outgoing
transition from the current measurement to the next one. The final
measurement has no outgoing transition, and `MaterialPathMode` governs both
propagation directions. In the default `DD4hepBetweenSurfaces` mode, the
outward component's current measurement state is the segment start and the
already matched target `TrackerHit` global point is the endpoint. Inward
filtering evaluates the same bounded interval in canonical inner-to-outer
order using the matched inner and outer hit points; the separate reverse flag
still controls the BH response. The material manager integrates the DD4hep
volume interval after finite-point, matched-surface, and propagation-direction
checks. This avoids independently re-solving a bounded target-surface
intersection after the hit was accepted. The returned material segments must
cover the requested endpoint distance. If TGeo navigation from an exact
boundary omits the leading volume, the query is retried from 1 micrometre
inside the interval and the leading cap is restored from `materialAt` at its
midpoint. A retry that still does not cover the interval is invalid rather
than silently accepting a partial material path.
In the `CurrentSurface` control, the owning surface's inner and outer normal
thicknesses are divided by the absolute dot product of the component-local
track tangent and DD4hep surface normal. Outward propagation evaluates the
current filtered state; inward propagation evaluates the same physical surface
at the reverse component's target crossing before the measurement update.

The DD4hep endpoint and representative inner-VXD ownership are mechanically
validated, but the collapsed interval's BH energy-loss response and population
momentum performance remain under study. Default status is therefore a
steering decision, not a claim of production physics validation.

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

### Flat-tuple paired-track branches

`RecGsfFlatTuple` keeps the existing ordinary tracker-only branches unchanged
and, when `GSFTracksEcalConstrained` is present in the event store, fills a
parallel constrained-track scalar set:

| Branches | Meaning |
|---|---|
| `gsf_pT`, `gsf_p`, `gsf_eta`, `gsf_theta`, `gsf_phi`, `gsf_d0`, `gsf_z0`, `gsf_omega`, `gsf_tanl`, `gsf_chi2`, `gsf_ndf`, `gsf_nhits`, `gsf_type` | Ordinary unconstrained `GSFTracks` result. |
| `ecal_gsf_pT`, `ecal_gsf_p`, `ecal_gsf_eta`, `ecal_gsf_theta`, `ecal_gsf_phi`, `ecal_gsf_d0`, `ecal_gsf_z0`, `ecal_gsf_omega`, `ecal_gsf_tanl`, `ecal_gsf_chi2`, `ecal_gsf_ndf`, `ecal_gsf_nhits`, `ecal_gsf_type` | Paired `GSFTracksEcalConstrained` result. |
| `ecal_gsf_available` | One when a constrained track is present for the tuple row; otherwise zero. |
| `ecal_gsf_changed` | One when the constrained and ordinary AtIP track parameters or fit quality differ; otherwise zero. |
| `res_pT_gsf`, `res_pT_ecal_gsf` | Ordinary and constrained fractional pT residuals relative to the first truth particle. |

The constrained branches always exist in newly produced flat files. When the
experiment is off or the paired collection is absent, `ecal_gsf_available=0`
and its scalar/residual fields are zero. The constrained track deliberately
has no duplicate hit-vector branches: the experimental collection copies the
ordinary GSF tracker hits, so `gsf_hit_*` is the common hit information.

### Historical `DumpGsfTrks` card compatibility

`DumpGsfTrks/gsf.py.bk` with `method="reverse"` explicitly configures all 40
properties and agrees with the active reverse template, including
`ComponentWeightCutoff=1e-4` and the default-off ECAL experiment. It silently
inherits no configurable property. Its reconstructed-event input list includes
`EcalCluster`, and `keep *` preserves the paired constrained collection when
the experiment is explicitly enabled. Its `RecGsfFlatTuple` instance writes
both the ordinary `gsf_*` and paired `ecal_gsf_*` scalar branch sets.

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

For BH-model dataset production, the recorder can also write a default-off
`dd4hep_surface_tuple` using the same DD4hep `MaterialManager::materialsBetween`
primitive and `length/radLength` sum as `DD4hepBetweenSurfaces` in the GSF. It
uses the midpoint of each sensitive traversal as a measurement anchor, treats
the adjacent TPC lower/upper sensitive half-volumes as one pad row, and stores
the DD4hep material composition together with the clipped Geant4 step material
and eBrem loss inside the same midpoint-to-midpoint bounds. It also evaluates
the identical endpoints in reverse order and stores
`dd4hep_reverse_path_tX0`, `reverse_segment_count`, `reverse_valid`, and
`dd4hep_reverse_materials`; these are direction-closure diagnostics, not a
second material definition. Both endpoint orders enforce the same complete-
coverage invariant used by the GSF. The tuple exposes `coverage_repaired`,
`reverse_coverage_repaired`, the original `initial_covered_length_mm` values,
and the final `covered_length_mm` values for auditing boundary recovery.
Enable the tree with:

```python
steprec.RecordDD4hepSurfaceIntervals = True
```

This mode requires the complete raw step stream: `RecordZeroLoss=True`,
`MinStepLengthMm=0`, and `MinAbsLossGeV=0`. Initialization fails instead of
silently constructing incomplete intervals when those requirements are not
met. The original `g4step_tuple` is unchanged, and no additional tree is
created while the option remains off.

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
