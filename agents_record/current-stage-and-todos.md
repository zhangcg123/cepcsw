---
name: current-stage-and-todos
description: Current GSF development stage, validated truth source, and prioritized TODOs
metadata:
  type: status
  date: 2026-07-05
---

# Current Stage and TODOs

## Current Stage

The project has moved from SimHit-level momentum diagnostics to a true Geant4 pre/post-step truth source for CEPC Bethe-Heitler tuning.

Current validated truth source:

```text
GsfMaterialStepRecorderAnaElemTool
tree: g4step_tuple
one entry per event, step data in vector<> branches
key variables: pre_p, post_p, retained, loss, step_tX0, material, process
PrimaryOnly = false (records all tracks)
```

The SimHit-level study remains useful as a detector-level cross-check, but it is no longer the source for final BH parameter fitting because `SimTrackerHit::getMomentum()` is momentum at the sensitive hit position, not a material-step pre/post pair.

## Validated Facts

1. The existing ACTS/ATLAS BH parameterization is not reliable for CEPC thin tracker material. At low `tX0`, it produces unphysical large-loss components.
2. The true G4 pre/post-step recorder builds and runs in CEPCSW.
3. A 5-event electron smoke test produced `gsf_material_steps_test.root` with 335 true G4 steps.
4. A 200 e- vs 200 mu- comparison at 1 GeV, theta=85 deg confirms the electron loss tail with true G4 pre/post-step truth:

```text
electron mean event loss = 0.007116 GeV
muon mean event loss     = 0.000898 GeV
electron/muon loss ratio = 7.92
endpoint loss ratio      = 19.06
mean material t/X0 ratio = 1.03
```

Conclusion: the electron bremsstrahlung / energy-loss tail is real and stronger than muon, now confirmed with the correct truth source.

## Resolved Operational Issues

- The 2026-07-07 electron GSF refit segmentation fault is repaired. `agents_record/2026-07-07-temp-gsf-electron-refit-crash.md` is now provenance only, not an active handoff. Do not spend the next session reinstalling/revalidating that old crash unless a new run reproduces it.

## Current Files To Know

Code:

```text
Simulation/DetSimAna/src/GsfMaterialStepRecorderAnaElemTool.h
Simulation/DetSimAna/src/GsfMaterialStepRecorderAnaElemTool.cpp
Simulation/DetSimAna/CMakeLists.txt
Reconstruction/RecGsfTracking/options/run_gsf_material_step_recorder_test.py
DumpGsfTrks/sim.py.bk  (integrated as of 2026-07-06)
```

Generated comparison material:

```text
G4MaterialStepComparison/
gsf_material_steps_e200.root
gsf_material_steps_mu200.root
```

SimHit cross-check files:

```text
SimHitEnergyLoss/root_files/electron/
SimHitEnergyLoss/root_files/muon/
```


## Current Study Directories To Read

There are two separate top-level study areas that should both be checked before continuing analysis:

```text
G4MaterialStepComparison/studies/
TrackingPerformanceStudies/
```

`G4MaterialStepComparison/studies/` contains the true Geant4 material-step studies by energy/theta point. It is the place for `g4step_tuple` material truth summaries, primary eBrem categories, tracker process-loss spectra, and electron/muon material controls.

`TrackingPerformanceStudies/` contains tracking-resolution studies. The current important subdirectory is `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/`, which is an LCIO `CompleteTracks` performance study joined to material-step eBrem categories. Its useful no/light/hard eBrem categories are based on tracker-volume primary eBrem only (`pre_volume` contains VXD/ITK/TPC/OTK/SIT/SET); all-material eBrem is computed but is not the performance split. It is not automatically a GSF-vs-LCIO study unless extended to read `gsf_flat`/`GSFTracks`.

Latest regenerated 2 GeV, theta=85 deg tracking-performance result uses seeds 1..15. Tracker-volume category counts are `no_tracker_ebrem=1373`, `light_tracker_ebrem=1561`, `hard_tracker_ebrem=566`. For transverse momentum only, no-tracker-eBrem electrons have median/q16/q84 `0.0542/-0.0884/0.1737%` and `|pT residual|>10% = 13/1373`; muon LCIO has median/q16/q84 `0.0668/-0.0569/0.1826%` and `|pT residual|>10% = 2/3500`. Conclusion: no-tracker-eBrem electron pT core is close to muon, but tails are still larger, so quote both core quantiles and tail fractions.

User guidance as of this stage: current GSF output is known not to work properly, so do not spend effort checking GSF-vs-LCIO results yet. Switch to direct BH-model diagnostics. New study area: `BHModelComparisonStudies/current_bh_vs_g4step_2p0_theta85/`. First step is understanding the actual current `BetheHeitlerSplitter.cpp` model by plotting its weighted Gaussian mixture curves; next step is overlaying G4 tracker-volume primary eBrem `z = post_p/pre_p` truth.



### Active BH Model Finding

The current `BetheHeitlerSplitter.cpp` model is not validated. Model-only plots in `BHModelComparisonStudies/current_bh_vs_g4step_2p0_theta85/` show a sharp discontinuity at `tX0 = 0.1`: the weighted mean retained fraction changes from about `z=0.951` at `tX0=0.05` to about `z=0.144` at `tX0=0.10`. This is suspicious because the physical BH shape should evolve smoothly with material thickness. Next work should fine tune the current BH model against G4 tracker-volume primary eBrem truth before returning to GSF validation.


### 2026-07-08 Global BH Model Status

A parallel `GlobalSim2GeV85` BH model is now encoded in `BetheHeitlerSplitter` from the tracker primary eBrem `E_f/E_i` fit in `BHModelComparisonStudies/globalBHmodelfromSim@2GeV85Degree/`. The default remains `Current`. Select the new model with:

```python
gsf.BHModel = "GlobalSim2GeV85"
```

Current runbook and test summary are in `agents_record/2026-07-08-global-bh-gsf-run.md`. The current covariance/reducer fixes and five-event debug workflow are in `agents_record/2026-07-08-gsf-cov-reducer-debug.md`. A selected light-tracker-eBrem scan completed seeds 1-5 only (`235/235` selected events) before the user requested stopping the remaining jobs. The model is runnable but not validated: completed events still show a high GSF momentum tail (`40/235` with `gsf_p > 10 GeV`) and many zero-chi2 fits (`186/235`).


### 2026-07-09 GSF Hit-2 Recovery / Smoothing Failure

A focused debug of `run_gsf_light_global_bh_seed1.py` with the new distinct-mean `GlobalSim2GeV85` model shows the current GSF output problem is dominated by early KalTest hit-update recovery and backward smoothing, not simply by the final outer fitted branch. In event 15 the final selected branch at the last hit has sane `tanl=0.0877` and `pT~2.01 GeV`, but the smoothed first-hit state used for IP output has `tanl=-57.4`, producing `p=60.6 GeV` and `z0=667.5 mm`.

The critical pattern is usually at hit 2 after `1 -> 5 -> 25` early splitting: `AddAndFilter()` fails for all 25 components, all are recovered because their predicted pivots are already at the measurement surface, no real `DeltaChi2` is applied, and the reducer immediately compresses those unmeasured branches to 3. This damaged early history later makes `SmoothAll()` produce an unphysical first-hit state. Multi-event debug found all-25 hit-2 recovery in 7/8 tested events. Full note: `agents_record/2026-07-09-gsf-hit2-recovery-smoothing-failure.md`.

Next work should focus on the direct GSF component update path. The three-flow comparison in `agents_record/2026-07-10-remove-dead-gsf-init-flows.md` shows that pure KF has no recoveries, GSF Seed still recovers early hits even with `MaxComponents=1`/`TopN=1`, and `BaselineEarlyFit` avoids those max1 early recoveries by bypassing direct `AddAndFilter` for hits 0-3. Therefore the priority is to make the GSF component update itself baseline-compatible, or replace the direct per-component update mechanism with something using the same site/update construction as the baseline workflow. Increasing `GSFInitialisationFitHits` is only a safeguard that moves the handoff point, not the real fix. Do not interpret GSF-vs-LCIO performance until this update path is fixed or reliably bypassed.

Explicit next TODO: compare the baseline predict/site/update chain against the current GSF per-component chain, then implement the baseline method inside `Reconstruction/RecGsfTracking` only. The baseline chain is `KalTestTool::Fit` -> `MarlinTrk::createFit` -> `IMarlinTrack::initialise` -> `MarlinDDKalTestTrack::fit` -> `MarlinDDKalTestTrack::addAndFit`, where each hit site is created and updated through the same working wrapper path. The current GSF chain directly clones a hit, creates a `TKalTrackSite`, calls `comp->kaltrack->AddAndFilter`, and then uses a recovery shortcut if that fails. The fix is to make the GSF component update follow the baseline predict/site/update workflow directly, while keeping GSF-specific mixture logic such as BH splitting, weights, and reduction. Do not modify KalTest, TrackSystemSvc, MarlinTrk, DDKalTest, or other shared packages. Do not hand-code a parallel Kalman update in GSF.

## Prioritized TODOs

### 1. Build a dedicated G4-step analysis script

Read `g4step_tuple` and produce:

- event-level endpoint retained momentum
- event-level cumulative loss
- single-step `z = post_p/pre_p`
- single-step loss tails
- material and process breakdown
- `loss` vs `step_tX0`
- `eBrem`-only distributions vs all-step distributions
- electron vs muon control plots

### 2. Produce larger true G4-step electron samples

The 200-event comparison validates the direction but is not enough for stable BH fitting.

Start with the baseline point:

```text
particle: e-
energy: 1 GeV
theta: 85 deg
truth source: GsfMaterialStepRecorderAnaElemTool
```

Muon is now mainly a control sample; it does not need equal statistics unless needed for validation plots.

### 3. Decide the exact BH fitting target

Likely target:

```text
primary electron eBrem steps only
z = post_p / pre_p
conditioned or binned by step_tX0
```

Open decision: exclude ionization/transportation momentum changes from the BH fit, or model them separately as non-BH effects.

### 4. Fit the CEPC-specific BH mixture

After larger G4-step samples exist:

- fit the CEPC-specific `z` distribution vs `tX0`
- compare to the current ACTS/ATLAS BH parameterization
- produce a CEPC-specific mixture table or replacement model
- validate inside GSF tracking

### 5. Clean output and commit policy

Keep code/documentation changes separate from large generated ROOT outputs. Decide whether ROOT outputs stay in working storage, move to CEFS/AIFS output storage, or are ignored by git.

## Historical / Superseded Work

The previous SimHit analysis and BH toy mixture tests are useful provenance, but they are not the current path for final BH fitting. The current path is true G4 pre/post-step truth -> G4-step analysis -> CEPC-specific BH mixture fit.

## 2026-07-10 baseline-compatible GSF component update progress

Implemented a scoped `Reconstruction/RecGsfTracking` bridge that lets the GSF split/update/reduce loop use the baseline-style hit update sequence for each component: add the previous/reference hit to a temporary MarlinTrk track, initialise it from the component state, call baseline `addAndFit` on the current hit, export the updated state, append that state into the component history, update the component weight using the returned `dchi2`, then run the existing GSF reduction. This preserves the GSF mixture workflow around the baseline update: BH split before hit update, per-component baseline update, likelihood/weight update, then TopN/KL reduction.

Focused validation with `MaxComponents=1`, `TopN=1` on events 10, 12, 14, and 15 shows no recovery or all-rejected signatures, full hit-count GSF tracks, track parameters close to LCIO, and nonzero chi2 comparable to LCIO: event 10 `465.3/460` vs LCIO `467.5/462`, event 12 `550.3/458` vs `543.1/460`, event 14 `431.3/458` vs `431.9/460`, event 15 `410.4/456` vs `413.1/458`.

Limitation: the accepted baseline `TKalTrackSite` object is not directly transferred into the persistent GSF component because the public `IMarlinTrack` API hides that ownership. The current implementation exports the baseline-updated `TrackState` and rebuilds a component site from it, with explicit GSF-side chi2 bookkeeping (`fitChi2`) copied through clone and moment-merged through reduction. This is baseline-compatible at the workflow/API level, but not a full internal KalTest site ownership merge. Next validation should run the real working point `MaxComponents=9`, `TopN=3` to test split/reduce cooperation beyond the max1 smoke case.



## 2026-07-10 GSF verbose prediction diagnostic motivation

The next verbose diagnostic must expose the true per-component KF prediction at the current measurement surface, not just the `TrackState.referencePoint`. The current dump labels the component state passed to the baseline update as `predict`, but that state reference point is only the parameterization pivot. Before update it is normally the previous component site pivot; after `getTrackState(trkHit, ...)` it is naturally the measured hit position because the returned state is parameterized at that hit. Therefore identical pivot coordinates across components do not prove the components predict the same measurement, and the updated state reference point matching the measured hit is expected by construction.

The required diagnostic is instead:

```text
component id / branch / weight / pT after split
predicted crossing or predicted measurement on the current hit surface
measured tracker-hit position
residual = measured - predicted, ideally in global and/or local measurement coordinates
updated state after the baseline-style addAndFit
weight and chi2 change
survival through reduction
```

Motivation: the recovery issue was fixed operationally by moving GSF component updates onto the baseline-style `addHit(reference) -> initialise(componentState) -> addAndFit(currentHit)` path, but we still need to verify that each split GSF component is being propagated to the same physical measurement surface in the same way as the baseline KF. This diagnostic should distinguish prediction/extrapolation problems from measurement association, residual/covariance, update, likelihood weighting, and reduction effects. Do not interpret `TrackState.referencePoint` as a predicted hit location.

## 2026-07-10 multi-component validation and current blocker

The baseline-style update was tested with real splitting and TopN reduction on events 10, 12, 14, and 15 (`MaxComponents=9`, target 3). No recovery shortcut occurred, final momenta were physical, and the old catastrophic smoothing output was absent. This closes the immediate component-update recovery blocker.

The remaining physics blocker is energy-loss inference. A quiet events 10-19 run with `MaxComponents=2`, `TopN=1` follows LCIO rather than recovering generated momentum in hard-loss events 11, 16, and 17. Immediate TopN=1 normally keeps the `GlobalSim2GeV85` near-no-loss child (`z=0.99995`, prior weight 0.5793) after only one hit and deletes hard-loss alternatives before later hits provide enough curvature leverage. The global model also ignores per-step `t/X0`.

Next: retain 3-5 components across several post-split hits before reduction, inspect events 11/16/17, then implement a step-`t/X0`-conditioned model and an explicit pre/post-material transition. Full evidence and the simplified active argument list are in `agents_record/2026-07-10-gsf-topn-energy-loss-status.md` and `Reconstruction/RecGsfTracking/README.md`.
