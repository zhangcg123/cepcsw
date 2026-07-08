# GSF light tracker eBrem component diagnostics

Date: 2026-07-08
Branch: `gsf-simhit-energy-loss-tuple-20260705`

## Scope

Current focus is only the five seed-1 light tracker eBrem events:

```text
[22, 37, 47, 89, 94]
```

The hard tracker eBrem checks are explicitly out of scope for this note.

## Diagnostic code added

Temporary/uncommitted diagnostic logging was added in:

- `Reconstruction/RecGsfTracking/src/GsfAlgorithm.cpp`
- `Reconstruction/RecGsfTracking/src/GsfMixture.cpp`
- `Reconstruction/RecGsfTracking/src/GsfMixture.h`

The diagnostics are gated by existing `VerboseDump=True` and `VerboseSplitDump=True`.
They print:

- component tables after BH split, after normalization, before hit update, after hit update, after reducer, and after smoothing
- hit update accept/recover/reject messages with `dchi2` and weight changes
- reducer merge pair, symmetric KL distance, weights, kappas, covariance determinants, and merged result
- final selected component index and selected weight

The current diagnostic edits are not committed.

## Build and run

Build/install used:

```bash
source ~/.bashrc
cd /aifs/user/data/zhangcg/gsfdev/CEPCSW
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target RecGsfTracking -j8
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target install -j8
```

Five-event light eBrem diagnostic run:

```bash
mkdir -p /tmp/gsf_light_ebrem_component_diag/logs /tmp/gsf_light_ebrem_component_diag/outputs
source setup.sh
GSF_DEBUG_OUTPUT_DIR=/tmp/gsf_light_ebrem_component_diag/outputs   ./run.sh DumpGsfTrks/rungsf-light-globalBH-e--2.0-85-seed1-five-debug.py   > /tmp/gsf_light_ebrem_component_diag/logs/run_five_diag.log 2>&1
```

Outputs:

```text
/tmp/gsf_light_ebrem_component_diag/logs/run_five_diag.log
/tmp/gsf_light_ebrem_component_diag/outputs/gsf_light_global_bh_seed1_five_debug.root
```

## Component lifecycle counts

```text
event 22: splits=2 reduce_blocks=1 merges=13 accepts=2745 recovers=20
event 37: splits=2 reduce_blocks=1 merges=13 accepts=2736 recovers=29
event 47: splits=2 reduce_blocks=1 merges=13 accepts=2734 recovers=31
event 89: splits=2 reduce_blocks=1 merges=13 accepts=2784 recovers=29
event 94: splits=2 reduce_blocks=1 merges=13 accepts=2748 recovers=17
```

Global counts in the five-event light sample:

```text
events:          5
BH splits:       10
reduce blocks:   5
merge decisions: 65
invalid symKL:   0
hit rejects:     0
hit recovers:    126
```

The reducer is numerically healthy for these light events; there are no `symKL=1.0000e+30` invalid-distance cases.

## Why LCIO and GSF look similar

For event 22, the first BH split creates one large-loss branch and four near-no-loss branches:

```text
parent pT ~1.997
child[0] pT ~1.352  weight 0.0774
child[1-4] pT ~1.997 total weight ~0.9226
```

After the second split and reduction to 12 components, the mixture still contains loss branches:

```text
pT ~0.916  weight 0.0060
pT ~1.352  total weight ~0.143
pT ~1.997  total weight ~0.851
```

The later hit likelihoods strongly suppress the large-loss branches. Around hit 20 in event 22, the extreme branch has approximately:

```text
comp[00] weight ~4e-260, chi2 ~13142
```

By the final smoothed state, all meaningful components have almost identical momentum near the LCIO-like solution:

```text
event 22 final meaningful components:
comp 05 w=0.206962 pT=1.99400 chi2=394.8
comp 08 w=0.164275 pT=1.99400 chi2=394.8
comp 10 w=0.133297 pT=1.99400 chi2=394.8
comp 11 w=0.370102 pT=1.99400 chi2=394.8  selected
```

This pattern explains the small visible momentum difference: GSF splits as expected, but the measurement likelihood drives surviving high-weight branches back to the near-no-loss / LCIO-like trajectory.

## Final five-event comparison

```text
event  LCIO p   GSF p   LCIO chi2/ndf  GSF chi2/ndf
22     2.004    2.004   404.0/458      394.8/456
37     2.006    2.005   456.1/458      448.0/456
47     2.009    2.008   420.1/458      409.3/456
89     2.001    1.999   458.2/466      443.5/464
94     1.843    1.844   702.9/458      764.2/456
```

For events 22, 37, 47, and 89, GSF gives a small chi2 improvement but almost no momentum shift.

Event 94 is different: both LCIO and GSF sit near `p ~1.84 GeV`, far from truth `2.008 GeV`; GSF does not recover the truth momentum and worsens chi2.

## Current interpretation

For these light eBrem events, the lack of large GSF improvement is not due to missing split/reduction. The split and reducer both run. Instead, the large-loss branches are strongly disfavored by subsequent hit likelihoods, and the surviving weighted components collapse to nearly identical LCIO-like pT.

The recovery path exists but is not dominant by rate: about 126 recoveries versus 13747 normal accepts in the five-event light sample. It clusters in early split layers, so it remains worth understanding, but it does not by itself explain the LCIO-like final momentum in these events.


## Detailed algorithm workflow from event 22

The relevant code path is in `Reconstruction/RecGsfTracking/src/GsfAlgorithm.cpp`.

### 1. Start from one LCIO-seeded component

The algorithm builds one initial `GsfComponent` from the LCIO seed and first hit:

```cpp
auto* site = makeInitialSite(seed, hits[0], bz, kappaSeed, m_kappaSeedCov);
initComp->weight = 1.0;
initComp->kaltrack = new TKalTrack();
initComp->kaltrack->Add(site);
std::vector<GsfComponent*> comps = {initComp};
```

So before material splitting, the GSF is exactly one KalTest track seeded from the LCIO track.

### 2. At each hit, material split happens before measurement update

For every next hit, the code checks the target-layer material thickness:

```cpp
if (stepTX0 > m_bhSplitThresh && m_isElectron && comps.size() < m_maxComponents) {
    auto children = bhs.split(comp, stepTX0, bz);
}
```

The splitter clones the full branch history and changes only the kappa state/covariance at the last site:

```cpp
newKappa = parentKappa / fracMomentum;
child->weight = parentWeight * mixture[i].weight;
```

For event 22 at hit 1, one parent splits into five children:

```text
parent pT ~1.997
child[0] pT ~1.352  weight 0.0774
child[1] pT ~1.997  weight 0.1353
child[2] pT ~1.997  weight 0.1258
child[3] pT ~1.997  weight 0.1016
child[4] pT ~1.997  weight 0.5598
```

The first component is the strong-loss hypothesis. The other four are near-no-loss hypotheses with different Gaussian widths/weights from the BH mixture.

### 3. Early hit update recovery preserves weights

At hit 1 and hit 2, the event 22 diagnostic shows `hit-update recover`, not normal `hit-update accept`. Recovery keeps the predicted state as filtered but does not calculate a `DeltaChi2`, so the weights remain the BH prior weights.

At hit 2, after the second split and before the measurement update, there are 25 children. They are reduced to 12 before the hit update.

### 4. Reducer merges nearest components by symmetric KL

The reducer is in `Reconstruction/RecGsfTracking/src/GsfMixture.cpp`.

It repeatedly finds the closest pair by symmetric KL distance at the last site, then moment-matches common branch history:

```cpp
double d = klDistance(comps[i], comps[j], bz);
momentMerge(comps[bi], comps[bj], bz);
keep->weight = keep->weight + drop->weight;
```

For the five light eBrem events the reducer is numerically healthy:

```text
invalid symKL: 0
```

After hit-2 reduction in event 22, the 12 components are still physically distinct:

```text
comp[00] pT=0.916  weight=0.0060
comp[01] pT=1.352  weight=0.0210
comp[02] pT=1.352  weight=0.1219
comp[03-11] pT=1.997 total weight ~0.851
```

So the reducer has not erased the energy-loss hypotheses at this point. It mostly combines duplicated near-identical branches produced by the 5x5 split.

### 5. Later normal hit updates kill large-loss branches

Once normal `AddAndFilter()` succeeds, each component weight is multiplied by the hit likelihood:

```cpp
comp->weight *= exp(-0.5 * min(dchi, 100.0));
```

By hit 20 in event 22, the strong-loss branch has become impossible under the hit sequence:

```text
comp[00] weight ~4e-260  pT=2.153  chi2=13142.7
```

The remaining meaningful components are all near the LCIO-like momentum:

```text
comp[03] weight=0.0214 pT=1.99874 chi2=15.51
comp[05] weight=0.2104 pT=1.99895 chi2=15.50
comp[08] weight=0.1657 pT=1.99884 chi2=15.50
comp[10] weight=0.1343 pT=1.99881 chi2=15.49
comp[11] weight=0.3629 pT=1.99796 chi2=15.53
```

This is the first clear point where the answer appears: the components still have different labels and weights, but the components with non-negligible weights now occupy nearly the same track state.

### 6. Smoothing makes the surviving components even more similar

After all hits are processed, the algorithm smooths each surviving branch independently:

```cpp
for (auto* c : comps)
    c->kaltrack->SmoothAll();
```

Then it publishes the highest-weight branch by default:

```cpp
if (bestIdx < 0 || comps[i]->weight > comps[bestIdx]->weight)
    bestIdx = i;
```

Final smoothed event 22 components:

```text
comp[01] w=0.000653 pT=1.99403 chi2=402.1
comp[02] w=0.003689 pT=1.99403 chi2=402.1
comp[03] w=0.021309 pT=1.99399 chi2=394.8
comp[04] w=0.039443 pT=1.99400 chi2=394.8
comp[05] w=0.206962 pT=1.99400 chi2=394.8
comp[06] w=0.018444 pT=1.99399 chi2=394.8
comp[07] w=0.029708 pT=1.99400 chi2=394.8
comp[08] w=0.164275 pT=1.99400 chi2=394.8
comp[09] w=0.012120 pT=1.99400 chi2=394.8
comp[10] w=0.133297 pT=1.99400 chi2=394.8
comp[11] w=0.370102 pT=1.99400 chi2=394.8  selected
```

The published GSF track is therefore almost identical to LCIO because the only surviving high-weight components are all fitted to almost the same trajectory.

### Why many different components share the same result

The main reason is that most BH mixture components in the new global model are clustered at retained fraction very close to 1.0. After two splits, many branch histories differ by Gaussian label/weight/covariance, not by a visibly different mean kappa. The one real large-loss hypothesis is present, but the hits assign it enormous chi2 and nearly zero weight.

Therefore the final mixture contains multiple components with different ancestry and different weights, but the measurement sequence has pulled their mean states to nearly identical values. The best-branch output then naturally matches LCIO-like momentum.

### Practical conclusion for the five light events

For events 22, 37, 47, and 89:

- split works
- reducer works
- large-loss hypotheses are created
- large-loss hypotheses are killed by hit likelihoods
- surviving components converge to one LCIO-like solution
- GSF improves chi2 slightly but does not materially change momentum

Event 94 is different: the whole fit remains near the low-momentum LCIO solution and GSF does not recover truth.


## Extended component ancestry dump

Additional configurable debug fields were added after the initial component-table dump.

New properties:

```python
gsf.ComponentDebugDump = True          # print component ancestry histories
gsf.ComponentDebugMaxHistory = 360     # truncate long ancestry strings
```

The fields are added to `GsfComponent`:

```text
debugId
debugHistory
```

The splitter appends one ancestry step per BH split:

```text
g<i>[w=<gaussian weight>,f=<retained fraction>,s=<sigma>]
```

The reducer wraps merged histories:

```text
merge(history_a | history_b)
```

The normal component table now prints `id=...`; full ancestry is printed only when `ComponentDebugDump=True`.

### Ten-event light eBrem history run

Temporary remote run card:

```text
/tmp/gsf_light_ebrem_component_diag/run_seed1_ten_component_history.py
```

Selected seed-1 light events:

```text
[0, 1, 2, 4, 5, 22, 37, 47, 89, 94]
```

Run output:

```text
/tmp/gsf_light_ebrem_component_diag/logs/run_seed1_ten_component_history.log
/tmp/gsf_light_ebrem_component_diag/outputs_ten_history/gsf_light_global_bh_seed1_ten_component_history.root
```

The log has about 262k lines with ancestry enabled.

Counts:

```text
event 0:  splits=2 reduces=1 merges=13 accepts=2760 recovers=17 invalidKL=0  selected=11 w=0.33
event 1:  splits=2 reduces=1 merges=13 accepts=2753 recovers=24 invalidKL=13 selected=11 w=0.317271
event 2:  splits=2 reduces=1 merges=13 accepts=2759 recovers=18 invalidKL=0  selected=11 w=0.367283
event 4:  splits=2 reduces=1 merges=13 accepts=2765 recovers=12 invalidKL=13 selected=11 w=0.311363
event 5:  splits=2 reduces=1 merges=13 accepts=2760 recovers=17 invalidKL=0  selected=0  w=0.995948
event 22: splits=2 reduces=1 merges=13 accepts=2745 recovers=20 invalidKL=0  selected=11 w=0.370102
event 37: splits=2 reduces=1 merges=13 accepts=2736 recovers=29 invalidKL=0  selected=11 w=0.347336
event 47: splits=2 reduces=1 merges=13 accepts=2734 recovers=31 invalidKL=0  selected=11 w=0.368117
event 89: splits=2 reduces=1 merges=13 accepts=2784 recovers=29 invalidKL=0  selected=11 w=0.359208
event 94: splits=2 reduces=1 merges=13 accepts=2748 recovers=17 invalidKL=0  selected=0  w=1
```

Events 1 and 4 show invalid reducer KL sentinels in this larger light sample, so the invalid-KL issue is not exclusive to hard eBrem. It did not appear in the original five focused events.

### Event 22 ancestry answer

Final high-weight event-22 components with almost identical fitted result:

```text
comp 05 w=0.206962 pT=1.994000 chi2=394.8
  merge(merge(seed->g1[w=0.1353,f=1.0000,s=0.1539]->g4[w=0.5598,f=1.0000,s=0.0048] | seed->g4[w=0.5598,f=1.0000,s=0.0048]->g1[w=0.1353,f=1.0000,s=0.1539]) | merge(seed->g1[w=0.1353,f=1.0000,s=0.0196] | ...))

comp 08 w=0.164275 pT=1.994000 chi2=394.8
  merge(seed->g2[w=0.1258,f=1.0000,s=0.0574]->g4[w=0.5598,f=1.0000,s=0.0048] | seed->g4[w=0.5598,f=1.0000,s=0.0048]->g2[w=0.1258,f=1.0000,s=0.0574])

comp 10 w=0.133297 pT=1.994000 chi2=394.8
  merge(seed->g4[w=0.5598,f=1.0000,s=0.0048]->g3[w=0.1016,f=1.0000,s=0.0196] | seed->g3[w=0.1016,f=1.0000,s=0.0196]->g4[w=0.5598,f=1.0000,s=0.0048])

comp 11 w=0.370102 pT=1.994000 chi2=394.8
  seed->g4[w=0.5598,f=1.0000,s=0.0048]->g4[w=0.5598,f=1.0000,s=0.0048]
```

The ancestry makes the mechanism explicit: these are different combinations of the near-no-loss Gaussians `g1`, `g2`, `g3`, and `g4`. Their retained fractions are all printed as `f=1.0000` at this precision, so their mean kappa evolution is almost identical. Their different final weights come from different products/sums of BH prior weights and small later likelihood differences. Their fitted states are nearly identical because the hit sequence constrains them to the same track trajectory.

For event 22 specifically, component 11 has the largest weight because it follows the dominant near-no-loss Gaussian twice:

```text
g4 weight 0.5598 x g4 weight 0.5598 -> large prior branch
```

This directly explains why components can have the same final pT/chi2 but different weights.


## Reduction policy test: MaxComponents 25, reduce to 5

Implemented a configurable reduction target:

```python
gsf.MaxComponents = 25
gsf.ReductionTargetComponents = 5
```

Behavior:

- `MaxComponents` remains the runtime ceiling used by the split condition.
- When component count reaches or exceeds the ceiling, the reducer compresses to `ReductionTargetComponents`.
- Default `ReductionTargetComponents = 0` preserves the previous behavior: reduce only to `MaxComponents` when count is greater than the ceiling.

The relevant condition now reduces when either:

```text
components > MaxComponents
```

or when a lower target is configured and:

```text
components >= MaxComponents
```

This fixes the previous deadlock where `5 -> 25` with `MaxComponents=25` would block further splitting because `components < MaxComponents` was false, but no reduction was triggered.

### Verification run

Temporary card:

```text
/tmp/gsf_light_ebrem_component_diag/run_seed1_two_thr1e3_reduce25to5.py
```

Settings:

```python
gsf.BHSplitThreshold = 10e-4
gsf.MaxComponents = 25
gsf.ReductionTargetComponents = 5
gsf.ComponentDebugDump = True
```

Output:

```text
/tmp/gsf_light_ebrem_component_diag/logs/run_seed1_two_thr1e3_reduce25to5.log
/tmp/gsf_light_ebrem_component_diag/outputs_thr1e3_reduce25to5/gsf_light_seed1_two_thr1e3_reduce25to5.root
```

Observed split/reduce locations:

```text
event 0:
  splits:  hit 6 r=234.7 tX0=0.0016 comps=1
           hit 7 r=345.2 tX0=0.0016 comps=5
           hit 8 r=555.0 tX0=0.0016 comps=5
           hit 232 r=1808.6 tX0=0.0032 comps=5
  reduces: hit 7  n=25 -> target=5
           hit 8  n=25 -> target=5
           hit 232 n=25 -> target=5

event 22:
  splits:  hit 5 r=234.7 tX0=0.0016 comps=1
           hit 6 r=344.7 tX0=0.0016 comps=5
           hit 7 r=555.6 tX0=0.0016 comps=5
           hit 231 r=1807.4 tX0=0.0032 comps=5
  reduces: hit 6  n=25 -> target=5
           hit 7  n=25 -> target=5
           hit 231 n=25 -> target=5
```

So the requested mechanics work: once the runtime component count reaches 25, it is reduced to 5 and later high-material layers can split again.

### Warning from this test

The physics result is not yet validated. Event 22 produced a pathological final total momentum:

```text
pT: LCIO 1.9966, GSF 1.9966 GeV
p:  LCIO 2.004,  GSF 45.474 GeV
chi2/ndf: 0.4/456
```

This means the new split/reduce schedule can create an unphysical final `tanLambda`/total-momentum state, especially after the late outermost split. This needs diagnosis before treating the 25->5 policy as a production setting.


## 2026-07-08 configurable TopN reducer

Implemented an optional reducer mode for RecGsfTracking:

```python
gsf.MaxComponents = 25
gsf.ReductionTargetComponents = 5
gsf.ReductionMode = "TopN"  # default remains "KL"
```

Code changes:
- `GsfMixture::reduceTopN(...)` keeps the highest-weight N components, deletes the rest, and renormalizes weights.
- `RecGsfTracking.ReductionMode` selects `KL` moment-merge reduction or `TopN` pruning at the existing reduction point.
- Reduction logs now print the mode and, for TopN, the keep/drop ranking.

Build/install passed:

```bash
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target RecGsfTracking -j8
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target install -j8
```

Smoke card:

```text
/tmp/gsf_light_ebrem_component_diag/run_seed1_event22_topn_smoke.py
/tmp/gsf_light_ebrem_component_diag/logs/run_seed1_event22_topn_smoke.log
```

Smoke result for event 22 with `BHSplitThreshold=1e-4`, `MaxComponents=25`, `ReductionTargetComponents=5`, `ReductionMode=TopN`:

```text
reductions: 7 TopN reductions, each 25 -> 5
final components: 5
best weight: 0.472292
GSF pT: 1.9974 GeV
GSF p: 2.081 GeV
GSF chi2/ndf: 1.4/456
GSF d0: 503.8764 mm
GSF z0: 12.4306 mm
```

This confirms TopN avoids invalid-KL pair selection, but the aggressive split setting still gives a bad published track/IP state. The remaining issue is therefore not the KL reducer alone.


## 2026-07-08 reduce after measurement update

Changed the GSF reduction ordering. The previous code path was:

```text
BH split before hit
normalize
reduce mixture if needed
measurement update
normalize
```

The current code path is now:

```text
BH split before hit
normalize
measurement update for all split children
normalize posterior weights
reduce mixture if needed
normalize
```

This applies to both reducer modes:

```python
gsf.ReductionMode = "KL"    # default moment-merge reducer
gsf.ReductionMode = "TopN"  # weight-rank pruning reducer
```

The diagnostic log message now includes `after measurement`, for example:

```text
MIX reduce begin hit=231 n=25 max=25 target=5 mode=TopN after measurement
```

Build/install passed after this change:

```bash
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target RecGsfTracking -j8
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target install -j8
```

Smoke test:

```text
/tmp/gsf_light_ebrem_component_diag/run_seed1_event22_topn_smoke.py
/tmp/gsf_light_ebrem_component_diag/logs/run_seed1_event22_topn_after_update.log
```

Event 22 result with `BHSplitThreshold=1e-4`, `MaxComponents=25`, `ReductionTargetComponents=5`, `ReductionMode=TopN`:

```text
splits: 8
reductions: 7
peak components: 25
final components: 5
best weight: 0.452199
GSF pT: 1.9974 GeV
GSF p: 2.081 GeV
GSF chi2/ndf: 1.4/456
GSF d0: 503.8764 mm
GSF z0: 12.4306 mm
```

The reducer is now conceptually on the correct side of the measurement update. This did not cure the event-22 published IP pathology. The final/outer site remains reasonable, but the published state still comes from the selected branch's `At(1)` state, which is already bad after smoothing:

```text
last site: kappa=-0.50119, tanl=0.08647
IP state:  d0=503.876 mm, z0=12.431 mm, tanl=-0.292828
```

Remaining diagnosis: why the smoothed `At(1)` state of the selected component is inconsistent with the good last-site state.
