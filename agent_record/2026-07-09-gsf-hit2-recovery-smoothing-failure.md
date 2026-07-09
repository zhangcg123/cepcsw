---
name: 2026-07-09-gsf-hit2-recovery-smoothing-failure
description: GlobalSim2GeV85 event-15 and multi-event debug showing early AddAndFilter recovery causes unphysical smoothed inner/IP state
metadata:
  type: debug-log
  date: 2026-07-09
---

# GSF Hit-2 Recovery and Smoothed Inner-State Failure

## Scope

This note records the 2026-07-09 debug session after replacing `GlobalSim2GeV85` with the distinct-mean 5-component Gaussian model.

Main run card used by the user:

```text
run_gsf_light_global_bh_seed1.py
```

Focused debug cards were created under `/tmp` only:

```text
/tmp/run_gsf_light_global_bh_seed1_event15_debug.py
/tmp/run_gsf_light_global_bh_seed1_events_debug.py
```

Main logs:

```text
/tmp/gsf_event15_debug_clean/logs/run.log
/tmp/gsf_events_debug/logs/run.log
```

## Logging Improvement

`Reconstruction/RecGsfTracking/src/GsfAlgorithm.cpp` verbose split logging was made more readable.

Normal `VerboseSplitDump=True` now prints compact summaries instead of every component line:

```text
MIX <stage> hit=<i> n=<N> sumW=<sum> pT=[min,max]
  top0/top1/top2 components by weight
HIT hit=<i> accepted=<n> recovered=<n> rejected=<n> dchi2=[min,max]
POST-SPLIT hit=<i> r=<r> survivors=<n> measurement=(x,y,z)
DIAG initial/inner/last/ip
```

Full per-component detail remains available with:

```python
gsf.ComponentDebugDump = True
```

The module was rebuilt and installed after this change:

```bash
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target RecGsfTracking -j8
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target install -j8
```

Build completed with the usual external KalTest/ROOT warnings.

## Event 15 Finding

For event 15, the final/outer branch is physically sane, but the smoothed inner state used for IP output is not.

Final selected component after the last hit:

```text
MIX final-smoothed hit=-1 n=3 pT=[2.001, 2.012]
selected bestWeight=0.999783 outputMode=BestBranch
```

Diagnostic states:

```text
DIAG last:
  kappa=-0.497095
  tanl=0.0876861
  pT about 2.01 GeV

DIAG inner:
  kappa=-0.947291
  tanl=-57.4424

DIAG ip:
  d0=-11.8004 mm
  z0=667.5 mm
  tanl=-57.4424
```

The published table therefore becomes:

```text
LCIO pT=2.0036 eta=0.0877 p=2.011
GSF  pT=1.0556 eta=-4.7440 p=60.648 z0=667.5 mm
```

Conclusion: the outer/final fit is not the immediate problem. The published IP parameters are bad because `extrapolateToIP_geometric()` uses `kaltrack->At(1)` after `SmoothAll()`, and that smoothed first-hit state has become unphysical.

## Critical Hit-2 Pattern

In event 15, hit 2 is the earliest catastrophic point:

```text
BH Split before hit 2: 5 comps -> 25 comps
HIT hit=2 accepted=0 recovered=25 rejected=0 dchi2=[-1,-1]
MIX reduce begin hit=2 n=25 target=3 mode=KL
MIX after-reduce hit=2 n=3 pT=[1.627, 2.003]
```

Definitions:

- `accepted`: `comp->kaltrack->AddAndFilter(*st)` succeeded. A real measurement update and `DeltaChi2` were computed.
- `recovered`: `AddAndFilter(*st)` failed, but the predicted state pivot was already at the measurement position (`pivotResidual < 1e-3`), so the code manually copied the predicted state as filtered and kept the component.
- `rejected`: `AddAndFilter(*st)` failed and the component was deleted.

For recovered components, no real measurement residual/Kalman update/DeltaChi2 is applied. The diagnostic uses `dchi2=-1` to mark this.

Therefore hit 2 keeps all 25 post-split branches without a proper hit-2 measurement update, then immediately reduces them to 3. Backward smoothing later propagates through this damaged early history and can create a nonphysical first-hit state.

## Why AddAndFilter Fails Despite Being On The Surface

KalTest `TVKalSystem::AddAndFilter()` does:

```cpp
GetState(TVKalSite::kFiltered).Propagate(next);
if (next.Filter()) { Add(&next); ... }
```

`TVKalSite::Filter()` then calls:

```cpp
CalcExpectedMeasVec(prea, h)
CalcMeasVecDerivative(prea, fH)
```

Both rely on `TKalTrackSite::CalcXexp()`, which calls:

```cpp
ms.CalcXingPointWith(*hel, xx, phi)
```

So after propagation has already placed the predicted state pivot on the target measurement surface, `Filter()` asks KalTest to find the track-surface crossing again. If the predicted state is already exactly on the surface, the crossing solve can fail. KalTest source even has the relevant comment:

```cpp
if (!CalcXexp(a,xxv,phi)) return 0; // hit on S(x) = 0
```

This explains why the recovery condition sees a tiny pivot residual while `AddAndFilter()` returns false: propagation succeeded geometrically, but the later crossing/measurement calculation inside `Filter()` failed in an already-on-surface edge case.

## Why The Pattern Is Common

A small multi-event debug was run for selected seed-1 light-eBrem events:

```text
events: 0, 1, 2, 15, 22, 24, 36, 37
log: /tmp/gsf_events_debug/logs/run.log
```

Summary:

```text
event  hit2 result              total recoveries  last tanl   inner/IP tanl   GSF p
0      accepted=25 recovered=0   26               0.0865      -63.67          7.224
1      accepted=0  recovered=25  47               0.0889      -28.93          6.083
2      accepted=0  recovered=25  37               0.0861       76.49        306.435
15     accepted=0  recovered=25  35               0.0877      -57.44         60.648
22     accepted=0  recovered=25  45               0.0865       -0.345         2.031
24     accepted=0  recovered=25  43               0.0894      -52.11         10.675
36     accepted=0  recovered=25  57               0.0894       -0.721         2.437
37     accepted=0  recovered=25  48               0.0875        0.152         2.014
```

The all-25 recovery at hit 2 occurs in 7/8 tested events. Event 0 differs at hit 2 but still has many recoveries elsewhere and a bad inner/IP state.

The pattern is common because the GSF call path structurally creates it:

1. The code splits before measurement update.
2. Early VXD hits are close together.
3. Hit 1 often gives `1 -> 5` branches.
4. Hit 2 often gives `5 -> 25` branches.
5. All children are near-duplicate KalTest histories aimed at the same target surface.
6. If the already-on-surface crossing edge case occurs for that geometry, all children fail `AddAndFilter()` together and are recovered together.

## Current Interpretation

The main failure is not that the final branch momentum at the outer tracker is wildly wrong. It is that the early KalTest history is damaged by recovered, unmeasured split branches. `SmoothAll()` then back-propagates through that damaged history, producing an unphysical smoothed first-hit state. Since the output IP state is derived from `At(1)`, the published GSF parameters can become wildly different from LCIO.

## Suggested Next Steps

1. Do not interpret current `GlobalSim2GeV85` tracking performance as validated.
2. Debug/fix the hit update recovery path before larger GSF comparisons.
3. Possible experiments:
   - If `AddAndFilter()` fails but pivot residual is tiny, try constructing the measurement update without re-solving the surface crossing, or nudge the pivot slightly before filtering.
   - Do not reduce immediately after an all-recovered split; preserve branches until a real measurement update occurs.
   - Treat recovered components as lower confidence or reject them when all components recover at a split hit.
   - Compare output from last-hit state versus smoothed inner/IP state to isolate smoothing damage.
4. Keep using compact logging; enable `ComponentDebugDump=True` only for a single event/hit when full component histories are needed.

## 2026-07-09 Component-Count Controls

Later controls separated the early `AddAndFilter()` recovery issue from the catastrophic final IP/direction output.

### MaxComponents=1, TopN=1

Run setup:

```python
gsf.MaxComponents = 1
gsf.ReductionTargetComponents = 1
gsf.ReductionMode = "TopN"
```

Because the split condition is `comps.size() < MaxComponents`, this setting performs no BH split. It is effectively the single KalTest branch inside the GSF wrapper.

Results for selected events 10, 12, 14, 15 were healthy and close to LCIO/truth:

```text
event 10: GSF pT/eta/phi=2.0010/0.0879/2.9267, d0/z0=-0.0580/-0.0031 mm, splits=0, reductions=0
event 12: GSF pT/eta/phi=1.9656/0.0882/-1.0876, d0/z0= 0.0167/-0.0289 mm, splits=0, reductions=0
event 14: GSF pT/eta/phi=2.0004/0.0864/0.1732, d0/z0=-0.0314/ 0.0213 mm, splits=0, reductions=0
event 15: GSF pT/eta/phi=2.0031/0.0877/2.3202, d0/z0= 0.0050/-0.0047 mm, splits=0, reductions=0
```

This shows the basic single-branch GSF/KalTest wrapper and output conversion can produce sane parameters.

### MaxComponents=2, TopN=2, normal BH state modification

Run setup:

```python
gsf.MaxComponents = 2
gsf.ReductionTargetComponents = 2
gsf.ReductionMode = "TopN"
```

The original `BetheHeitlerSplitter` was active, including the usual kappa and covariance changes:

```text
newKappa = parentKappa / fracMomentum
kappa covariance rows/columns scaled by 1/fracMomentum
cov(2,2) += bhKappaVar
```

All four selected events were still healthy:

```text
event 10: GSF pT/eta/phi=1.9996/0.0879/2.9267, d0/z0=-0.0564/-0.0033 mm, splits=1, reductions=1, peak-comps=5, final-comps=2
event 12: GSF pT/eta/phi=1.9651/0.0882/-1.0875, d0/z0= 0.0163/-0.0272 mm, splits=1, reductions=1, peak-comps=5, final-comps=2
event 14: GSF pT/eta/phi=2.0000/0.0864/0.1732, d0/z0=-0.0310/ 0.0212 mm, splits=1, reductions=1, peak-comps=5, final-comps=2
event 15: GSF pT/eta/phi=2.0026/0.0877/2.3202, d0/z0= 0.0054/-0.0047 mm, splits=1, reductions=1, peak-comps=5, final-comps=2
```

This means one BH split with normal kappa/covariance modification is not by itself enough to trigger the catastrophic final IP/direction failure.

### MaxComponents=2, TopN=2, no BH state modification

A temporary splitter patch was used only for this test:

```cpp
if (false && child->kaltrack->GetEntriesFast() > 0) {
```

This keeps split children in the GSF workflow but skips all direct child state/covariance mutation. The original splitter was restored and rebuilt after the test.

Final track parameters were again healthy:

```text
event 10: GSF pT/eta/phi=2.0010/0.0879/2.9267, d0/z0=-0.0580/-0.0031 mm, splits=1, reductions=1, peak-comps=5, final-comps=2
event 12: GSF pT/eta/phi=1.9656/0.0882/-1.0876, d0/z0= 0.0167/-0.0289 mm, splits=1, reductions=1, peak-comps=5, final-comps=2
event 14: GSF pT/eta/phi=2.0004/0.0864/0.1732, d0/z0=-0.0314/ 0.0213 mm, splits=1, reductions=1, peak-comps=5, final-comps=2
event 15: GSF pT/eta/phi=2.0031/0.0877/2.3202, d0/z0= 0.0050/-0.0047 mm, splits=1, reductions=1, peak-comps=5, final-comps=2
```

But the hit-2 recovery issue remained in this no-modification setup:

```text
event 10: hit=2 r=22.1 mm accepted=0 recovered=2 rejected=0
event 12: hit=2 r=22.1 mm accepted=2 recovered=0 rejected=0, dchi2=[1.17e-07, 1.17e-07]
event 14: hit=2 r=22.1 mm accepted=0 recovered=2 rejected=0
event 15: hit=2 r=27.6 mm accepted=0 recovered=2 rejected=0
```

### Updated Interpretation

There are now two related but distinct observations:

1. The early hit-2 `AddAndFilter()` recovery issue is real and persists even when BH child kappa/covariance/state mutation is disabled. It is not caused by the kappa modification.
2. The catastrophic final GSF IP/direction output does not appear with `MaxComponents=1` or with `MaxComponents=2, TopN=2`, even with one normal BH kappa-modifying split. It appears in the larger/default component evolution (`MaxComponents=12`, target 3), so the trigger is likely larger component multiplicity, repeated split/reduce cycles, KL/moment reduction history effects, or smoothing through a more damaged early multi-component history.

Do not collapse these into one statement: fixing the final bad track parameters and fixing the early recovered-hit path may require different changes.

## 2026-07-09 MaxComponents=25 Target=5 Reducer Controls

The next control kept normal BH state modification enabled and increased the component budget:

```python
gsf.MaxComponents = 25
gsf.ReductionTargetComponents = 5
```

This allows repeated split/reduce cycles with peak component count 25 and final count 5.

### TopN reducer

Run setup:

```python
gsf.ReductionMode = "TopN"
```

Results:

```text
event 10: hit2 accepted=0  recovered=25 rejected=0; GSF pT/eta/phi=1.9827/ 0.8449/ 1.1615; d0/z0= 1523.3019/-16.1183 mm; splits=10 reductions=9 total A/R/J=1285/60/0
event 12: hit2 accepted=25 recovered=0  rejected=0; GSF pT/eta/phi=1.9322/ 0.3424/-0.9278; d0/z0=-1332.2957/-15.0606 mm; splits=9  reductions=8 total A/R/J=1245/75/0
event 14: hit2 accepted=0  recovered=25 rejected=0; GSF pT/eta/phi=1.9848/-0.6388/ 3.0453; d0/z0=   69.7772/  7.6603 mm; splits=9  reductions=8 total A/R/J=1240/80/0
event 15: hit2 accepted=0  recovered=25 rejected=0; GSF pT/eta/phi=2.0049/ 0.4039/-1.8390; d0/z0=  -14.1621/  5.0341 mm; splits=8  reductions=7 total A/R/J=1255/40/0
```

All four selected events have bad final IP/direction output in this setting. Event 12 is important because hit 2 is fully accepted, not recovered, but the final d0 is still catastrophically wrong.

### KL reducer

Run setup:

```python
gsf.ReductionMode = "KL"
```

Results:

```text
event 10: hit2 accepted=0  recovered=25 rejected=0; GSF pT/eta/phi=0.2405/-6.7180/-2.4760; d0/z0=  11.5267/-332933.9375 mm; splits=10 reductions=9 total A/R/J=1278/67/0
event 12: hit2 accepted=25 recovered=0  rejected=0; GSF pT/eta/phi=1.9484/ 0.5381/-0.8209; d0/z0=  28.2663/    -10.0420 mm; splits=9  reductions=8 total A/R/J=1262/58/0
event 14: hit2 accepted=0  recovered=25 rejected=0; GSF pT/eta/phi=2.0370/-0.4766/ 3.0255; d0/z0=-338.9374/      4.7333 mm; splits=9  reductions=8 total A/R/J=1228/92/0
event 15: hit2 accepted=0  recovered=25 rejected=0; GSF pT/eta/phi=1.0663/-4.7602/ 2.4798; d0/z0=  -2.6731/    957.9103 mm; splits=8  reductions=7 total A/R/J=1253/42/0
```

KL is also bad. For events 10 and 15 it is worse than TopN. Event 12 again proves that hit-2 recovery is not required for final IP failure.

### Updated Reducer/Component Interpretation

The evidence now separates three effects:

1. `AddAndFilter()` recovery at early hits is a real KalTest/surface-crossing edge case and persists even when BH kappa/covariance mutation is disabled.
2. The final catastrophic IP/direction output requires larger component evolution. It is absent for `MaxComponents=1` and `MaxComponents=2, TopN=2`, but present for `MaxComponents=25, target=5` with both TopN and KL.
3. Because event 12 fails with hit 2 fully accepted, the final IP failure is not only an all-recovered-hit-2 problem. The repeated split/reduce/smooth history itself can create a bad smoothed inner state.

## 2026-07-09 Parallel Pure-KF Fitter Mode

A selectable fitter path was added to `RecGsfTracking`:

```python
gsf.FitterMode = "GSF"  # default, existing multi-component workflow
gsf.FitterMode = "KF"   # new single-branch KalTest workflow
```

The KF path reuses the same LCIO seed extraction, hit/layer matching, initial KalTest site construction, smoothing, IP extrapolation, and output table. It skips all BH splitting, component cloning, mixture weighting, and reduction.

A second option controls whether the single KF branch uses the same pivot-copy recovery fallback:

```python
gsf.KFRecoveryMode = "None"       # default, raw AddAndFilter only
gsf.KFRecoveryMode = "PivotCopy"  # recover if predicted pivot is already on the hit
```

### Raw KF, recovery disabled

With `FitterMode="KF"` and default `KFRecoveryMode="None"`, raw KalTest `AddAndFilter()` fails very early:

```text
event 10: AddAndFilter failed at hit 1, r=16.6 mm
event 12: AddAndFilter failed at hit 1, r=16.6 mm
event 14: AddAndFilter failed at hit 1, r=16.6 mm
event 15: hit1 accepted, AddAndFilter failed at hit 2, r=27.6 mm
```

This proves the recovery issue is not introduced by GSF splitting. It is already present in the single-branch KalTest hit-update path.

### KF with PivotCopy recovery

With:

```python
gsf.FitterMode = "KF"
gsf.KFRecoveryMode = "PivotCopy"
```

selected events 10, 12, 14, and 15 reproduce the healthy single-branch result and are close to LCIO/truth:

```text
event 10: hit1 recovered, hit2 recovered; KF pT/eta/phi=2.0010/0.0879/ 2.9267; d0/z0=-0.0580/-0.0031 mm; total A/R/J=230/3/0
event 12: hit1 recovered, hit2 accepted;  KF pT/eta/phi=1.9656/0.0882/-1.0876; d0/z0= 0.0167/-0.0289 mm; total A/R/J=230/2/0
event 14: hit1 recovered, hit2 recovered; KF pT/eta/phi=2.0004/0.0864/ 0.1732; d0/z0=-0.0314/ 0.0213 mm; total A/R/J=229/3/0
event 15: hit1 accepted,  hit2 recovered; KF pT/eta/phi=2.0031/0.0877/ 2.3202; d0/z0= 0.0050/-0.0047 mm; total A/R/J=230/1/0
```

### Updated KF/GSF Interpretation

The single-branch KF control now establishes a baseline:

1. Raw `AddAndFilter()` can fail at the first VXD hits even without BH splitting or GSF components. This is a KalTest/DDKalTest crossing/update edge case.
2. The pivot-copy recovery is enough for a single branch to continue and produce LCIO-like final parameters.
3. Therefore the catastrophic GSF final IP failure is not caused by recovery alone. It needs the multi-component history: repeated split/reduce cycles, branch selection/merging, or smoothing through histories that have mixed recovered and updated sites.
4. The strongest current suspect remains reduction/merge consistency: the reducer changes a component last-site state/covariance, but the earlier KalTest site history still belongs to one surviving branch. Backward smoothing through that inconsistent history can produce a nonphysical inner/IP state.

## 2026-07-09 GSF KF Mode Switched to Baseline KalTestTool Workflow

Changed `RecGsfTracking` `FitterMode="KF"` from the direct `TKalTrack::AddAndFilter` diagnostic loop to the baseline `ITrackFitterTool` path, defaulting to `KalTestTool/KalTest111`.  The KF mode now builds the same broad initial covariance style used by `RecSiTracking`/`RecTrkGlobal`, calls `KalTestTool::Fit(...)`, uses the tool-produced `AtIP` state, and pushes the fitted `MutableTrack` into `GSFTracks`.  GSF mode is unchanged.

Validation after `cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target install -j8`:

| event | hits in fit / input | outliers | LCIO pT eta phi d0 z0 | baseline-KF pT eta phi d0 z0 | chi2/ndf LCIO -> KF |
|---:|---:|---:|---|---|---|
| 10 | 234/234 | 0 | 2.0017 0.0880 2.9304 -0.0084 0.0004 | 2.0009 0.0880 2.9256 -0.0126 -0.0067 | 467.5/462 -> 472.8/462 |
| 12 | 233/233 | 0 | 1.9658 0.0874 -1.0824 -0.0009 -0.0006 | 1.9660 0.0874 -1.0874 0.0129 0.0041 | 543.1/460 -> 549.1/460 |
| 14 | 233/233 | 0 | 2.0006 0.0866 0.1774 -0.0009 -0.0024 | 2.0005 0.0866 0.1725 0.0014 0.0128 | 431.9/460 -> 431.9/460 |
| 15 | 232/232 | 0 | 2.0036 0.0877 2.3251 0.0038 -0.0023 | 2.0040 0.0877 2.3201 0.0081 -0.0070 | 413.1/458 -> 412.4/458 |

Conclusion: the hit-recovery failures seen in the earlier pure-KF diagnostic were caused by driving low-level `TKalTrack::AddAndFilter` directly from GSF, not by the baseline KalTestTool workflow.  With the baseline workflow inside GSF, these same events fit all hits with zero outliers and no manual recovery.

## 2026-07-09 Baseline-KF d0/z0 Bias Check and Finalisation Note

Checked `FitterMode="KF"` with the baseline `KalTestTool` path on events 0-19 from `trk-e--2.0-85-1.root`. All fitted tracks used all input hits with zero outliers.

Summary of `abs(KF)-abs(LCIO)`:

| quantity | abs(KF) > abs(LCIO) | mean abs delta [mm] | median abs delta [mm] | mean signed KF-LCIO [mm] |
|---|---:|---:|---:|---:|
| d0 | 15/20 | +0.00698 | +0.00425 | +0.00735 |
| z0 | 13/20 | +0.00338 | +0.00335 | -0.00238 |

Interpretation: the current pure-KF mode is baseline-fitter-like but not bit-for-bit the LCIO `CompleteTracks` workflow. It calls `KalTestTool::Fit`, so smoothing/finalisation occurs through `KalTestTool::finaliseTrack` (`marlintrk->smooth(lastHit)` plus the forward-fit temporary-track propagation to IP). It no longer uses GSF's old direct `TKalTrack::SmoothAll()` plus manual first-site IP extrapolation. Remaining differences from LCIO likely come from pre-finalisation orchestration: hit ordering, initial-state choice, retry/outlier policy, and the fact that GSF refits the already-produced `CompleteTracks` hit list rather than rerunning full `FullLDCTrackingAlg`.

Important for the hit-recovery study: in the baseline-style pure-KF path on events 0-19, the explicit GSF `AddAndFilter` recovery does not happen at all. The path does not call the GSF manual `TKalTrack::AddAndFilter` loop. It delegates fitting to `KalTestTool`/MarlinTrk; any failed low-level `addAndFit` inside `KalTestTool::finaliseTrack` is currently only debug-logged by the baseline tool and does not correspond to the GSF recovery counter.

## 2026-07-09 Baseline KalTestTool IP-Refit Failure Check

Instrumented `KalTestTool::finaliseTrack` temporarily around the forward-fit IP temporary-track loop:

```cpp
mTrk->addAndFit(hit, deltaChi, DBL_MAX)
```

This exposed a GSF KF-mode bug first: passing `m_kfFitBackward.value()` directly was not equivalent to the baseline algorithms.  The baseline maps the user property with `(_backward ? IMarlinTrack::backward : !IMarlinTrack::backward)`.  GSF pure-KF now does the same mapping, so default `KFFitBackward=false` exercises the baseline forward-finalisation branch.

After the mapping fix, events 10/12/14/15 entered the `KalTestTool` forward IP-refit branch with `fit_backwards=1`, attempted 233/232/232/231 internal `addAndFit` calls respectively, and had zero failures.  On events 0-19 the same check gave total internal IP-refit failures = 0.  All events used all hits with zero outliers.

Conclusion: the hit-recovery problem we saw earlier is not reproduced inside the baseline-style `KalTestTool` workflow, including its forward IP-refit `addAndFit` loop.  The recovery issue remains specific to the old GSF direct `TKalTrack::AddAndFilter` driving path.

## 2026-07-09 Scope Note: Pure-KF d0/z0 Offset vs Hit-Recovery Issue

After matching GSF pure-KF fit-direction handling to the baseline mapping, the `d0/z0` offset relative to LCIO remains: in events 0-19, `abs(d0_KF) > abs(d0_LCIO)` for 15/20 events with mean `abs(KF)-abs(LCIO) = +0.00700 mm`, and `abs(z0_KF) > abs(z0_LCIO)` for 13/20 events with mean `+0.00338 mm`.  This appears to be a separate pure-KF-vs-full-LCIO orchestration difference, not the hit-recovery issue.

For the hit-recovery question, the important current finding is unchanged: the baseline-style `KalTestTool` workflow, including the forward IP-refit `addAndFit` loop inside `finaliseTrack`, showed zero internal failures on events 0-19.  Therefore the recovery issue remains localized to the old GSF direct `TKalTrack::AddAndFilter` component-driving path, not to the baseline-style wrapper workflow.
