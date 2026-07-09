# RecGsfTracking

`RecGsfTracking` refits `CompleteTracks` with a Gaussian-sum electron model and
writes `GSFTracks`.  Each component now uses the baseline MarlinTrk
`addHit(reference) -> initialise(componentState) -> addAndFit(currentHit)`
update path.  The old alternate KF fitter and initialization experiments have
been removed; historical comparisons remain under `agent_record/`.

## Active configuration

### Physics and material

| Property | Default | Purpose |
|---|---:|---|
| `ElectronHypothesis` | `true` | Enable Bethe-Heitler splitting. |
| `BHModel` | `"Current"` | Select `Current` or experimental `GlobalSim2GeV85`. |
| `BHSplitThreshold` | `1e-4` | Minimum target-layer `t/X0` for a split. |
| `MSOn` | `true` | Enable multiple-scattering process noise. |
| `ElossOn` | `false` | Enable the KalTest dE/dx treatment in addition to BH splitting. |
| `KappaSeedCov` | `1e-7` | Initial curvature variance for the LCIO-derived seed site. |

### Mixture and output

| Property | Default | Purpose |
|---|---:|---|
| `MaxComponents` | `12` | Split/reduction trigger. It is not a strict instantaneous ceiling: one split can temporarily exceed it. |
| `ReductionTargetComponents` | `0` | Target after reduction; `0` means `MaxComponents`. |
| `ReductionMode` | `"KL"` | Select KL moment merging or `TopN` weight pruning. |
| `GSFOutputMode` | `"BestBranch"` | Publish the best branch or `WeightedMean`. |
| `MaterialIPExtrapolation` | `false` | Include material when extrapolating the selected state to the IP. |

### Selection and diagnostics

| Property | Default | Purpose |
|---|---:|---|
| `SelectedEventIndices` | `[]` | Process only listed zero-based event entries. |
| `VerboseDump` | `false` | Print per-track truth/LCIO/GSF summaries. |
| `VerboseSplitDump` | `false` | With `VerboseDump`, print component-flow summaries and tables. |
| `ComponentDebugDump` | `false` | With verbose flow enabled, print every component and update detail. |
| `ComponentDebugMaxHistory` | `240` | Maximum printed component-history length. |

For ordinary production, defaults plus an explicitly selected `BHModel` and
mixture policy are sufficient.  Enable diagnostics only for small selected
event lists.

## Current limitation

The experimental `GlobalSim2GeV85` model is a global five-component retained-
momentum distribution and currently ignores the individual step `t/X0`.
Immediate `TopN=1` reduction normally selects its dominant near-no-loss branch
before later hits can distinguish hard-bremsstrahlung hypotheses.  It improves
fit consistency but does not recover the generated IP momentum in known hard-
loss events.  See `agent_record/2026-07-10-gsf-topn-energy-loss-status.md`.
