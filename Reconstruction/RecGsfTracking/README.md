# RecGsfTracking

`RecGsfTracking` refits `CompleteTracks` with a Gaussian-sum electron model and
writes `GSFTracks`.  Each component now uses the baseline MarlinTrk
`addHit(reference) -> initialise(componentState) -> addAndFit(currentHit)`
update path.  The old alternate KF fitter and initialization experiments have
been removed; historical comparisons remain under `agents_record/`.

## Active configuration

### Physics and material

| Property | Default | Purpose |
|---|---:|---|
| `ElectronHypothesis` | `true` | Enable Bethe-Heitler splitting. |
| `BHModel` | `"CEPC2GeV85StepConditioned"` | Select the five-component scoped CEPC model by default, the parallel six-component `CEPC2GeV85StepConditioned6`, or the `ActsAtlas` reference. |
| `BHSplitThreshold` | `1e-4` | Minimum target-layer `t/X0` for a split. |
| `MaterialPathMode` | `"CurrentSurface"` | Select legacy current-layer material or opt-in `DD4hepBetweenSurfaces` volume integration. |
| `MSOn` | `true` | Enable multiple-scattering process noise. |
| `ElossOn` | `false` | Enable the KalTest dE/dx treatment in addition to BH splitting. |
| `KappaSeedCov` | `1e-7` | Initial curvature variance for the LCIO-derived seed site. |

Material assigned to a measurement surface is owned by the outgoing
transition from that surface. Its inner and outer normal-thickness `t/X0`
contributions are divided by the absolute dot product of the component-local
track tangent and DD4hep surface normal. The final measurement has no outgoing
transition. Forward and reverse workflows apply the same current-surface
ownership in their respective direction and are alternative published paths.

Both workflows now reduce target-surface posterior mixtures. Forward process
children from transition `i -> i+1` remain expanded through measurement
`i+1`; reverse children from `i+1 -> i` remain expanded through measurement
`i`. The exact innovation likelihood is applied and normalized before the
low-weight cutoff and KL reduction. This ordering can temporarily require
up to roughly `MaxComponents * number-of-BH-modes` measurement updates at a
material transition.

### Mixture and output

| Property | Default | Purpose |
|---|---:|---|
| `MaxComponents` | `24` | Posterior-reduction trigger. It is not an instantaneous ceiling: a BH split remains expanded through the target measurement update. |
| `ReductionTargetComponents` | `0` | Target after reduction; `0` means `MaxComponents`. |
| `ComponentWeightCutoff` | `1e-8` | Remove normalized target-measurement posterior components below this weight while preserving at least the largest component. |
| `ProtectIdentityLineage` | `true` | Preserve at least one exact no-radiation lineage through low-weight cutoff and reduction when the reduction target is greater than one. Disable only for controlled ablation studies. |
| `GSFOutputMode` | `"BestBranch"` | Publish the best branch or `WeightedMean`. |
| `MaterialIPExtrapolation` | `false` | Include material when extrapolating the selected state to the IP. |
| `ReverseFiltering` | `false` | Experimental reverse multi-component filtering audit from the final measurement to the innermost hit. |
| `CmsGsfSmoothing` | `false` | CMSSW-like two-filter GSF workflow. Seeds the backward mixture from the final forward prediction, rescales its covariance, applies the final hit backward, combines collapsed forward-updated and backward-predicted moments at interior surfaces, and publishes the collapsed innermost backward-filtered mixture at the IP. Mutually exclusive with the other smoothing/reverse workflows. |
| `CmsErrorRescaling` | `100` | Full covariance multiplier for the CMSSW-like backward seed, matching CMSSW's default `ErrorRescaling`. |
| `GaussianSumSmoothing` | `false` | Run the KL reduction-aware Gaussian-sum smoother. It records the forward reduction graph, conditions contributors through each KL merge, applies exact RTS transport links, and moment-reduces the backward mixture on each common surface. Requires geometric IP extrapolation and `ReverseFiltering=false`. Use `GSFOutputMode="WeightedMean"` to publish the smoothed mixture. The independent reverse filter remains available through `ReverseFiltering`. |

### Selection and diagnostics

| Property | Default | Purpose |
|---|---:|---|
| `SelectedEventIndices` | `[]` | Process only listed zero-based event entries. |
| `VerboseDump` | `false` | Print per-track truth/LCIO/GSF summaries. |
| `VerboseSplitDump` | `false` | With `VerboseDump`, print component-flow summaries and tables. |
| `ComponentDebugDump` | `false` | With verbose flow enabled, print every component and update detail. |
| `ComponentDebugMaxHistory` | `240` | Maximum printed component-history length. |
| `MaterialTransitionCSV` | `""` | Optional per-component current and crossed-cradle material audit. Empty disables it. |
| `CounterfactualLossScan` | `false` | Run isolated likelihood-only trial-loss branches at a configured truth transition and one transition inward. The branches never enter selection, cutoff, reduction, or output. |
| `CounterfactualTruthTransitionMap` | `""` | Comma-separated zero-based `event:transition` targets for the counterfactual scan. |
| `CounterfactualLossFractions` | `0.04,...,0.12` | Trial fractional momentum losses. Configure explicitly for focused scans. |
| `CounterfactualLossVariance` | `2e-4` | Retained-momentum-fraction variance used by every trial branch in a scan. |

For ordinary production, defaults plus an explicitly selected `BHModel` and
mixture policy are sufficient.  Enable diagnostics only for small selected
event lists.

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
