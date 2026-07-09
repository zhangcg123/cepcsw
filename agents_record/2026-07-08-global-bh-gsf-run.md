---
name: 2026-07-08-global-bh-gsf-run
description: Current GlobalSim2GeV85 BH model implementation, how to run GSF, and light-eBrem validation notes
metadata:
  type: runbook
  date: 2026-07-08
---

# GlobalSim2GeV85 BH Model and GSF Runbook

## Code State

`RecGsfTracking` now has parallel Bethe-Heitler model options:

- `BHModel = "Current"`: default CEPC/ACTS-style model already in the code.
- `BHModel = "GlobalSim2GeV85"`: simulation-derived 5-component global model fitted from 2 GeV, theta=85 deg tracker primary eBrem G4 step truth.

The public splitter interface is unchanged:

```cpp
BetheHeitlerSplitter::split(parent, tX0, bz)
```

For `GlobalSim2GeV85`, `tX0` is intentionally unused because the fitted model is global for this sample, not binned in material thickness. The argument remains in the interface for compatibility and for future tX0-dependent models.

The GSF algorithm also has two diagnostic controls:

- `VerboseDump = True`: print the final per-event fit parameter table.
- `VerboseSplitDump = True`: additionally print split-by-split component details. Default is `True` to preserve old verbose behavior. Set it to `False` for event-list studies where only the final fit parameters are needed.

An optional event whitelist was added:

```python
gsf.SelectedEventIndices = [0, 1, 2]
```

When non-empty, only those event indices run the GSF fit; skipped events still publish an empty `GSFTracks` collection so `PodioOutput` remains consistent.

## Model Parameters

The encoded `GlobalSim2GeV85` components are truncated Gaussian in-range weights on `[0, 1]`:

```text
weight           mean            sigma
0.077416116868   0.677171066692  0.350000000000
0.135334171174   0.999993855825  0.153904803245
0.125841439379   0.999993855825  0.0573680711812
0.101560696051   0.999993855825  0.0195818944450
0.559847576529   0.999993855825  0.00479768891507
```

Source study:

```text
BHModelComparisonStudies/globalBHmodelfromSim@2GeV85Degree/
```

## Standard Build

From the CEPCSW checkout:

```bash
source ~/.bashrc
cd /aifs/user/data/zhangcg/gsfdev/CEPCSW
source setup.sh
./quick_build.sh
```

The 2026-07-08 build passed after these changes. ROOT/KalTest warnings are present but not fatal.

## How To Run Current GSF

Use any existing `RecGsfTracking` option file and set the model explicitly.

Default/current model:

```python
gsf = RecGsfTracking("RecGsfTracking")
gsf.BHModel = "Current"
gsf.VerboseDump = True
```

New global simulation-derived model:

```python
gsf = RecGsfTracking("RecGsfTracking")
gsf.BHModel = "GlobalSim2GeV85"
gsf.VerboseDump = True
gsf.VerboseSplitDump = False   # recommended for fit-parameter scans
```

Selected-event scan example:

```python
evtmax = max(selected_events) + 1
gsf.SelectedEventIndices = selected_events
```

Run command:

```bash
source ~/.bashrc
cd /aifs/user/data/zhangcg/gsfdev/CEPCSW
source setup.sh
./run.sh path/to/option.py > path/to/log.txt 2>&1
```

The log should contain:

```text
Bethe-Heitler model: GlobalSim2GeV85
```

## Light Tracker eBrem Event Test

Event lists:

```text
TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/light_tracker_ebrem_event_indices.csv
TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/light_tracker_ebrem_entry_indices.txt
```

Completed before stopping at user request: seeds 1-5, all selected light tracker eBrem events.

Parsed table:

```text
TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/gsf_light_global_bh_fit_parameters_completed_seed1_5.csv
```

Counts:

```text
seed 1: 62/62
seed 2: 39/39
seed 3: 51/51
seed 4: 40/40
seed 5: 43/43
total : 235/235
```

Summary over completed seeds 1-5:

```text
gsf_p median/mean/q10/q90 = 2.121 / 9.876 / 1.989 / 28.752 GeV
gsf_pT median/mean        = 1.9949 / 1.9529 GeV
gsf_chi2 median/mean     = 0.0 / 63.91
zero-chi2 events          = 186/235
gsf_p > 10 GeV            = 40/235
splits counts             = {0: 2, 1: 49, 2: 184}
final component counts    = {1: 2, 5: 49, 25: 184}
total_tX0 median/mean     = 0.0201 / 0.0200
```

Interpretation: the global BH model is encoded and runnable, but the completed light-eBrem subset still shows unstable GSF fit parameters, especially a high-momentum tail and many zero-chi2 fits. Do not treat this model as validated tracking performance.

## Cleanup Policy

Do not commit per-run verbose logs, per-seed option dumps, or `GSFTracks` ROOT outputs. Keep source changes, compact CSV summaries, and reproducible scripts/records only.
