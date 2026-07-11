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
| `BHModel` | `"CEPC2GeV85StepConditioned"` | Select the scoped CEPC execution model by default, or explicitly choose `ActsAtlas`. `ActsAtlas` reproduces ACTS's default ATLAS regime thresholds and coefficients but is not CEPC validation. |
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

### Mixture and output

| Property | Default | Purpose |
|---|---:|---|
| `MaxComponents` | `12` | Split/reduction trigger. It is not a strict instantaneous ceiling: one split can temporarily exceed it. |
| `ReductionTargetComponents` | `0` | Target after reduction; `0` means `MaxComponents`. |
| `ReductionMode` | `"KL"` | Select KL moment merging or `TopN` weight pruning. |
| `ReductionMinHitsAfterSplit` | `0` | Minimum successful hit updates each branch must survive after its latest split before reduction. Set to several hits for delayed pruning studies. |
| `ComponentWeightCutoff` | `1e-8` | Remove normalized post-transition components below this weight while preserving at least the largest component. |
| `GSFOutputMode` | `"BestBranch"` | Publish the best branch or `WeightedMean`. |
| `MaterialIPExtrapolation` | `false` | Include material when extrapolating the selected state to the IP. |
| `ReverseFiltering` | `false` | Experimental reverse multi-component filtering audit from the final measurement to the innermost hit. |
| `RetainedLineageSmoothing` | `false` | Run an RTS smoother on each retained forward lineage. Requires `TopN`, geometric IP extrapolation, and `ReverseFiltering=false`. |

### Selection and diagnostics

| Property | Default | Purpose |
|---|---:|---|
| `SelectedEventIndices` | `[]` | Process only listed zero-based event entries. |
| `VerboseDump` | `false` | Print per-track truth/LCIO/GSF summaries. |
| `VerboseSplitDump` | `false` | With `VerboseDump`, print component-flow summaries and tables. |
| `ComponentDebugDump` | `false` | With verbose flow enabled, print every component and update detail. |
| `ComponentDebugMaxHistory` | `240` | Maximum printed component-history length. |
| `MaterialTransitionCSV` | `""` | Optional per-component current and crossed-cradle material audit. Empty disables it. |

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
