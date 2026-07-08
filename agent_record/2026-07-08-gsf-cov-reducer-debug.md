---
name: 2026-07-08-gsf-cov-reducer-debug
description: GSF covariance/reducer fixes and current five-event debug workflow for GlobalSim2GeV85
metadata:
  type: runbook
  date: 2026-07-08
---

# GSF Covariance/Reducer Fixes and Five-Event Debug Workflow

## Current Code Changes

This record captures the post-`GlobalSim2GeV85` debugging changes made on 2026-07-08.

### Per-propagation BH split

`Reconstruction/RecGsfTracking/src/GsfAlgorithm.cpp` now uses the current layer/material estimate only for BH splitting:

```cpp
const double stepTX0 = thicknessInX0(hi.layer);
...
if (stepTX0 > m_bhSplitThresh && m_isElectron) {
  auto children = bhs.split(comp, stepTX0, bz);
}
```

The old accumulated `aTX0` trigger/input was removed. `totalTX0` and `maxTX0Layer` remain diagnostics.

### BH split covariance update

`Reconstruction/RecGsfTracking/src/BetheHeitlerSplitter.cpp` no longer changes only `kappa` during a split. For each child component it now:

- scales the covariance kappa row/column by `1 / fracMomentum`
- adds the BH retained-fraction variance contribution into `cov(2,2)`
- then sets the child `kappa`

This is still within the current architecture of independent weighted KalTest tracks, but the branch state is less inconsistent than before.

### Moment-matching reducer

`Reconstruction/RecGsfTracking/src/GsfMixture.cpp` now reduces components by moment matching at the active last site instead of just keeping the higher-weight mean and adding weight.

For the selected pair it computes:

```text
merged mean = weighted mean of the two component means
merged covariance = weighted covariance + between-component spread
merged weight = w1 + w2
```

The merged state overwrites the representative component's last-site states so subsequent propagation starts from the moment-matched Gaussian approximation. A fallback was also added so reduction still selects a pair when all KL distances are invalid/sentinel.

### IP state source

The IP extrapolation uses the smoothed state from the first real tracker hit:

```cpp
comp->kaltrack->At(1)
```

The temporary `Last()` workaround is not used. The point of the current debug is to understand and fix the smoothed innermost state, not bypass it.

### Diagnostics

Under:

```python
gsf.VerboseDump = True
gsf.VerboseSplitDump = True
```

`RecGsfTracking` prints:

- split-by-split component information
- `DIAG initial`, `DIAG inner`, `DIAG last`, and `DIAG ip` helix states
- per-event split/reduction/component summary

## Build/Install

Use install before runtime tests, because `./run.sh` loads from `InstallArea`:

```bash
source ~/.bashrc
cd /aifs/user/data/zhangcg/gsfdev/CEPCSW
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target RecGsfTracking -j8
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target install -j8
```

Check that the installed library timestamp updates:

```bash
ls -l InstallArea/x86_64-el9-gcc11-opt/lib/libRecGsfTracking.so
```

## Current Five-Event Focus

Current focused seed-1 events:

```text
22, 37, 47, 89, 94
```

These are the five events that are sane after the covariance/reducer fixes and are useful for step-by-step algorithm debugging. Event `15` is intentionally kept separate as a stress case because it still has a large smoothed inner `tanLambda` in the current output.

Persistent debug run card:

```text
DumpGsfTrks/rungsf-light-globalBH-e--2.0-85-seed1-five-debug.py
```

This card selects only:

```python
gsf.SelectedEventIndices = [22, 37, 47, 89, 94]
gsf.BHModel = "GlobalSim2GeV85"
gsf.MaxComponents = 12
gsf.BHSplitThreshold = 1e-4
gsf.VerboseDump = True
gsf.VerboseSplitDump = True
gsf.MaterialIPExtrapolation = False
```

Run it with:

```bash
source ~/.bashrc
cd /aifs/user/data/zhangcg/gsfdev/CEPCSW
source setup.sh
mkdir -p /tmp/gsf_seed1_five_event_debug/logs /tmp/gsf_seed1_five_event_debug/outputs
./run.sh DumpGsfTrks/rungsf-light-globalBH-e--2.0-85-seed1-five-debug.py   > /tmp/gsf_seed1_five_event_debug/logs/run.log 2>&1
```

Useful log filter:

```bash
grep -E "GSF event index|BH Split|DIAG (inner|last|ip)|GSF diagnostics"   /tmp/gsf_seed1_five_event_debug/logs/run.log
```

Expected current behavior for the five-event card:

```text
splits 2
peak-comps 25
final-comps 12
no huge tanLambda at inner/IP
GSF p close to LCIO/truth, except expected energy-loss behavior
```

## Six-Event Validation Snapshot

The six-event diagnostic card in `/tmp/gsf_seed1_diagnosis_current/run_gsf_light_global_bh_seed1_splitdump_max12.py` selected:

```text
15, 22, 37, 47, 89, 94
```

Latest output:

```text
/tmp/gsf_seed1_diagnosis_current/outputs/gsf_light_global_bh_seed1_splitdump.root
```

Kinematic summary from that output:

```text
event  truth_p truth_pt truth_theta   lcio_p  lcio_pt lcio_theta    gsf_p   gsf_pt  gsf_theta  gsf_tanl    gsf_d0    gsf_z0
15      2.0080   2.0004    85.0000   2.0116   2.0039    84.9832   2.7057   2.0032    47.7638    0.9079   -6.6363   15.2825
22      2.0080   2.0004    85.0000   2.0046   1.9971    85.0174   2.0039   1.9964    85.0161    0.0872   -0.0014   -0.0047
37      2.0080   2.0004    85.0000   2.0054   1.9978    85.0253   2.0053   1.9977    85.0086    0.0873   -0.0481   -0.0028
47      2.0080   2.0004    85.0000   2.0087   2.0011    85.0175   2.0086   2.0012    85.0843    0.0860   -0.0236    0.0563
89      2.0080   2.0004    85.0000   2.0015   1.9938    84.9781   2.0003   1.9926    84.9730    0.0880    0.0542   -0.0131
94      2.0080   2.0004    85.0000   1.8435   1.8364    84.9581   1.8433   1.8362    84.9458    0.0884    0.0967   -0.0195
```

Interpretation:

- Events `22, 37, 47, 89, 94` are now suitable for focused algorithm debugging.
- Event `15` is not catastrophic like the earlier `tanLambda ~ 50` cases, but it remains a separate pathology: `pT` is fine while `tanLambda`, `theta`, `D0`, and `Z0` are wrong.

## Notes for Next Session

Do not reintroduce the `Last()` workaround. If event `15` is investigated, keep using `At(1)` and diagnose why the smoothed first-layer state differs sharply from the otherwise sane trajectory.

When testing runtime behavior, always install after building. A plain target build is not enough for `./run.sh`.
