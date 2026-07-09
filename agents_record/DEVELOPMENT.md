# GSF Tracking Development Log

> Historical append-only log moved from the repository root on 2026-07-10. Its
> top “Current Stage” predates the July 9-10 component-update repair. Use root
> `AGENTS.md` for the current concentration and this file only for chronology.

## Current Stage (2026-07-05)

The active development path is now based on true Geant4 pre/post-step material truth, not SimHit hit-position momentum.

### Update (2026-07-06): G4 step recorder refactored to event-per-entry

`GsfMaterialStepRecorderAnaElemTool` restructured so `g4step_tuple` has **one TTree entry per event** instead of one per step. Step-level data stored in `vector<>` branches.

Key changes:
- `PrimaryOnly` default changed from `true` to `false` — records all tracks, not just primaries.
- `EndOfEventAction` fills the tree once per event; `UserSteppingAction` pushes step data into vectors.
- Coordinates in **mm**, momenta in **GeV/c**.
- Updated `G4MaterialStepComparison/macros/compare_g4step_e_mu.C` to read `vector<>` branches.
- All three config sites updated: `sim.py.bk`, `run_gsf_material_step_recorder_test.py`, and C++ default.

Current validated tool:

```text
GsfMaterialStepRecorderAnaElemTool
tree: g4step_tuple (one entry per event)
event-level: event_id, step_count
step vectors: pre_p, post_p, retained, loss, step_tX0, material, process, mid_x/y/z/r, ...
```

Current conclusion:

- `SimTrackerHit::getMomentum()` is useful for qualitative detector-level cross-checks, but not sufficient for final CEPC BH fitting.
- The new G4 recorder confirms the electron loss tail with true pre/post-step truth.
- In the 200 e- vs 200 mu- comparison at 1 GeV, theta=85 deg:
  - electron mean event loss: 0.007116 GeV
  - muon mean event loss: 0.000898 GeV
  - electron/muon mean loss ratio: 7.92
  - endpoint loss ratio: 19.06
  - material budget ratio: 1.03

Current RAG entry to read first:

```text
agents_record/current-stage-and-todos.md
```

## Current TODOs

1. Build a dedicated `g4step_tuple` analysis script.
2. Produce larger true G4-step electron samples at the baseline point first: 1 GeV, theta=85 deg.
3. Decide the exact BH fitting target, likely primary electron `eBrem` steps with `z = post_p/pre_p` binned by `step_tX0`.
4. Fit a CEPC-specific BH mixture and compare with the current ACTS/ATLAS parameterization.
5. Keep SimHit-level files only as cross-checks.
6. Decide how to store or ignore large generated ROOT outputs before committing code/docs.

## Current Important Files

```text
Simulation/DetSimAna/src/GsfMaterialStepRecorderAnaElemTool.h
Simulation/DetSimAna/src/GsfMaterialStepRecorderAnaElemTool.cpp
Reconstruction/RecGsfTracking/options/run_gsf_material_step_recorder_test.py
G4MaterialStepComparison/
SimHitEnergyLoss/root_files/
```

## Historical Log Below

The sections below are kept for provenance. Some earlier next-step items, especially SimHit-only fitting and BH toy-model tests, are superseded by the true G4 pre/post-step truth path above.

## 关键发现（按时间顺序，每次在前次基础上追加）

### 发现 1: Seed Curvature 问题 (2026-07-05)
- GSF 用 LCIO 的 omega 直接转换 kappaSeed，初始协方差 `kappaCov=1e-7`（极紧）
- KF 高度信任种子值，forward filter 无法纠正错误的 curvature
- 改善方向：增大 kappaCov、3-hit 预拟合

### 发现 2: 放宽 KappaSeedCov 无效 (2026-07-05)
- 即使 KappaSeedCov=1e-4，GSF pT 与 LCIO pT 差异仍 < 0.15%
- KF 的 forward filter 用 hit 测量约束的是局部几何，不是绝对动量标度
- 单独放宽协方差无法改善 pT

### 发现 3: 3-hit 预拟合也无改善（用户确认）
- 即使改用外半层 hit 的几何关系重算 curvature seed，也无法改善 pT 精度
- 说明问题不在 seed 上

### 发现 4: BH 分裂产生的子组分全部被数据否决 (2026-07-05)
- Verbose dump 显示：BH split 产生 5 个极端能量损失子组分（pT≈10 MeV，99% 能量损失）
- 下一个 hit（仅 5mm 外）立即以 Δχ²≈30 拒绝，权重归零
- 只有 child[0]（无能量损失，kappa 不变）存活
- **GSF 等价于没做分裂**

### 发现 5: BH 参数化在低 tX0 区域完全失败 (2026-07-05)
- 深入分析 BH 多项式参数化：在 tX0=0.01 时，6 组分 mean 值 = {0.289, 0.001, 0.010, 0.008, 0.007, 0.009}
- 物理预期：轫致辐射 ΔE/E ≈ tX0 = 1%；BH 模型给出 71-99.9%
- **在 tX0=0.002 处存在严重不连续**：单高斯 mean=0.998（0.2% 损失）→ 6 组分 mean=0.001-0.276（72-99.9% 损失），跳跃 400 倍
- 根因：BH 参数化为 ATLAS 设计（单层 tX0~0.1），CEPC tracker 材料很薄（单层 tX0~1e-4 到 3e-3），累计到 0.01 仍处于 low-x 区，invLogit 多项式在此范围外推产生了极端值

## 根因分析演进
- 初疑：seed covariance 太紧 → 验证后发现无效
- 再疑：seed curvature 初始值不对 → 3-hit 预拟合也无效
- 终疑：**BH 参数化在低 tX0 区域完全失败**——6 组分 mean 值比物理预期大 70-100 倍
- 当前方向：BH 参数化根本不适合 CEPC tracker 的薄材料场景，需要改用单高斯近似或重新参数化

## 总体目标
- [ ] GSF pT 分辨率显著优于 LCIO baseline（尤其在低 pT 和电子场景）
- [ ] 降低 BAD track 比例（当前 ~8.5% @ 2GeV/85°）
- [ ] 代码合入主分支（commit + PR）
- [ ] 扩展到多径迹物理事件

## 已完成
- 2026-06-28: 实现 3-hit 解析预拟合（外半层取点）和 q/p 多轮扫描
- 2026-07-05: 建立 agent_record 知识库（10 个 RAG + 本日志 + plans/）
- 2026-07-05: 回滚 3-hit 预拟合和 q/p 扫描，恢复原始基线
- 2026-07-05: KappaSeedCov 改为可配置属性（默认 1e-7），测试 1e-4 效果有限
- 2026-07-05: Verbose dump 增加 BH split 诊断（parent/child kappa、Post-split 预测 vs 测量）
- 2026-07-05: 发现 BH 子组分全部被数据否决——GSF 等价于没做分裂

## 历史下一步（已被顶部 Current TODOs 取代）
- [ ] 验证 BH 参数化：检查 tX0=0.01 时的 6 组分多项式输出是否合理
- [ ] 检查 split() 中 kappa 更新方式：`newKappa = parentKappa / fracMomentum` 是否正确
- [ ] 考虑降低 BHSplitThreshold 或调整 split 策略
- [ ] 【远期】从 Geant4 模拟生成 CEPC 专用的 BH 参数化：单电子穿过 tracker 各层材料，记录实际能量损失分布，EM 算法拟合高斯混合，替代现有 ACTS 数据

## 历史待解决问题（部分已由 G4 pre/post-step 路线回答）
- BH 参数化是否适用于 CEPC tracker 材料（tX0 范围、材料成分）？
- split 后 kappa 更新是否应该用加权平均而非直接替换？
- 多径迹事件如何处理？
- KL 归并保留高权重组分而非加权平均，是否引入偏差？

### 发现 6: First BH sanity patch started (2026-07-05)
- Created branch `gsf-bh-thin-gaussian-20260705` from baseline commit `5871a69`.
- First development step: bypass the ACTS/ATLAS low-x 6-component parameterization for `tX0 < 0.1` and use the single-Gaussian thin-material formula instead.
- Also fixed the existing split update bug: `child[0]` reused the parent component but did not update its kappa; now all returned children/components update the last-site kappa.
- Expected immediate behavior: for `tX0≈0.01`, retained momentum should be about 0.99 instead of producing children near `pT≈0.01 GeV`.
- Next validation: rebuild and run `run_gsf_test.py`, then compare verbose dump and pT summary.


### 发现 7: Thin-Gaussian sanity patch result (2026-07-05)
- Build succeeded with `./quick_build.sh`.
- 5-event smoke test succeeded: `./run.sh Reconstruction/RecGsfTracking/options/run_gsf_test.py`.
- Positive result: BH split is now physically sane at `tX0≈0.01`; examples changed from children near `pT≈0.01 GeV` to `pT≈0.99 GeV`:
  - event 1 split: parent `pT≈1.000` → child `pT≈0.990`, second split `0.990` → `0.980`
  - event 2 split: parent `pT≈0.997` → child `pT≈0.987`
- Negative result: forcing a single deterministic thin-Gaussian energy-loss component worsens chi2 for normal tracks:
  - event 1 GSF chi2/ndf changed from baseline `448.9/454` to `545.0/454`
  - event 2 changed from `414.9/454` to `517.1/454`
  - event 5 changed from `452.5/452` to `556.3/452`
- pT summary from `plot_pt_resolution.py gsf_test.root`:
  - LCIO mean/RMS = `-3.3899% / 6.6364%`
  - GSF mean/RMS = `-3.3184% / 6.6739%`
- Interpretation: bypassing the broken low-x ACTS/ATLAS mixture fixes the unphysical split scale, but a one-component forced-loss model is too rigid. The next model should keep a no-loss/small-loss competition, or add process-noise/covariance treatment, instead of replacing the state with a single shifted kappa.

### 发现 8: Two-component CEPC toy mixture started (2026-07-05)
- Created branch `gsf-bh-cepc-two-component-20260705` from commit `7965974`.
- Replaced the forced single-component thin-Gaussian branch with a two-component CEPC toy mixture for `tX0 < 0.1`:
  - dominant no-loss branch: `mean=1.0`
  - moderate-loss tail branch: weighted so total `E[p/p0]≈exp(-tX0)`
- Motivation: the single-component patch fixed the unphysical BH scale but worsened chi2 because every split forced energy loss. This test lets the filter choose between no-loss and moderate-loss hypotheses.


### 发现 9: Two-component CEPC toy mixture result (2026-07-05)
- Build succeeded with `./quick_build.sh`.
- 5-event smoke test succeeded.
- Positive result: the model now creates physically plausible competing hypotheses:
  - at `tX0≈0.01`, child[0] keeps `pT≈1.000`, child[1] gives moderate tail `pT≈0.900`
  - components survive the next hit instead of being immediately killed as unphysical
- Positive result versus the one-component patch: chi2 returned to the baseline level because the no-loss branch remains available:
  - event 1 GSF chi2/ndf `448.9/454`
  - event 2 GSF chi2/ndf `414.9/454`
  - event 5 GSF chi2/ndf `452.5/452`
- pT summary from `plot_pt_resolution.py gsf_test.root`:
  - LCIO mean/RMS = `-3.3899% / 6.6364%`
  - GSF mean/RMS = `-3.4015% / 6.6325%`
- Interpretation: this is a safer BH model than the single forced-loss patch, but still not a performance improvement. The fitter almost always selects the no-loss branch in the current 1 GeV/85deg smoke test. Next useful tests should target samples/events with real hard bremsstrahlung or tune the tail weights/means with truth energy-loss information.

### 发现 10: Need simulation truth for CEPC BH model (2026-07-05)
- Current BH tuning is limited by lack of direct CEPC electron energy-loss truth.
- Next planned step is to inspect existing `sim/trk/gsf/gsf_flat` ROOT files for pre/post material momentum information.
- If existing outputs are insufficient, add a lightweight Geant4 truth recorder for primary electron steps in tracker volumes.
- New plan recorded in `agents_record/plans/2026-07-05-measure-cepc-electron-energy-loss.md`.

### 发现 11: Added RecGsfSimHitTuple for simulation-hit energy-loss diagnostics (2026-07-05)
- User clarified that existing flat tuples do not contain the needed simulation information.
- Added a new module under `Reconstruction/RecGsfTracking`: `RecGsfSimHitTuple`.
- It reads configurable EDM4hep `SimTrackerHitCollection`s and writes `simhit_tuple` to ROOT with:
  - primary MCParticle production/end momentum and retained fraction where available
  - per-sim-hit position, radius, time, EDep, path length, quality, cellID
  - per-sim-hit truth momentum at hit position: `hit_px/py/pz`, `hit_p`, `hit_pT`
  - MCParticle link IDs and PDG
  - retained momentum relative to primary: `hit_retained_vs_primary = hit_p / mc_p`
- Added test option: `Reconstruction/RecGsfTracking/options/run_gsf_simhit_tuple_test.py`.
- Added analysis script: `Reconstruction/RecGsfTracking/scripts/analyze_simhit_energy_loss.py`.
- Build succeeded; test ran over 5 events and produced `gsf_simhit_tuple_test.root`.
- First diagnostic result for `trk-e--1.0-85-1.root`:
  - 5 entries, 233-244 primary electron sim hits per event
  - last-hit retained p/p_primary: `0.90943 0.99465 0.99651 0.01621 0.99470`
  - global hit retained quantiles 0/1/5/10/50/90/99/100%: `0.00002 0.00223 0.83404 0.83420 0.99607 0.99792 0.99976 0.99988`
- Important caveat: `SimTrackerHit::getMomentum()` is momentum at the hit position, not a full Geant4 pre/post-step pair. This is still useful for layer/radius momentum evolution, but true per-step loss may need a deeper Geant4 stepping recorder later.

### 发现 12: First large-statistic SimHit energy-loss analysis — e- vs μ- (2026-07-06)
- Analyzed 10k e- and 10k μ- events @ 1.0 GeV, θ=85° (10 seeds × 1000 each).
- Analysis run by deepseek-v4-pro (Claude Code harness); output at `SimHitEnergyLoss/bh_eloss_emm1.0m85.root` and `bh_eloss_mumm1.0m85.root`.
- **Electrons lose ~26% momentum on average** through the tracker; muons only ~4%.
- **e- tail is a genuine bremsstrahlung continuum**: 51.6% of hits < 0.99, 44.4% < 0.95, 41.6% < 0.90. μ- tail is flat (5.4%→4.4%), consistent with geometric exit effects.
- **67% of e- events have at least one hard loss** (>1% retained drop); 56% have >10% loss.
- **First hard loss at R~300-330 mm** (TPC inner region). ITK Barrel is the biggest per-hit loss region (e- retained drops from 0.85→0.65).
- **MC endpoint momentum is zero for all events** — likely a Geant4 output issue; this branch is unusable.
- **Key implication**: the current BH toy model's "no-loss branch almost always wins" is wrong. Truth shows ~56% of e- events experience significant energy loss. A proper multi-component mixture must be fitted to these distributions.
- Full analysis record: [[2026-07-06-simhit-energy-loss-first-analysis]] (in `agents_record/`).

### Priority update: analysis and truth-definition decision (2026-07-05)
- High priority before broader pT/theta scans: improve the existing SimHit energy-loss analysis so electron/muon retained-momentum summaries, radius/layer bins, overflow handling, and artifact/control checks are reliable.
- High priority before BH fitting: decide whether `SimTrackerHit::getMomentum()` is sufficient for CEPC BH tuning or whether a Geant4 primary-track pre/post-step momentum recorder is required.
- Broader scans and fitted CEPC BH mixture work should follow these two items.

### Finding 13: SimTrackerHit momentum sufficiency and tuple extension (2026-07-05)
- Assessment recorded in `agents_record/2026-07-05-simtrackerhit-momentum-sufficiency.md`.
- Conclusion: SimTrackerHit momentum is solid enough for the electron-vs-muon qualitative conclusion, but not enough for final CEPC BH parameter fitting.
- Extended `RecGsfSimHitTuple` with explicit hit-to-hit diagnostic branches ordered by SimTrackerHit time:
  - previous hit index and previous hit momentum/position/time
  - `hit_step_retained_vs_prev`, `hit_step_loss_vs_prev`
  - `hit_step_dp`, `hit_step_dr`, `hit_step_ds`, `hit_step_dt`
  - `hit_loss_vs_primary`
- These branches approximate momentum evolution between recorded sensitive hits. A Geant4 stepping recorder is still needed for true pre/post-step material loss.

### Finding 14: Real Geant4 pre/post-step recorder added (2026-07-05)
- Added `GsfMaterialStepRecorderAnaElemTool` in `Simulation/DetSimAna`.
- It records real `G4Step` pre/post momentum and material information into `g4step_tuple`.
- Key branches: `pre_p`, `post_p`, `retained`, `loss`, `material`, `material_radlen`, `step_tX0`, `pre_volume`, `post_volume`, `process`.
- Added smoke-test option: `Reconstruction/RecGsfTracking/options/run_gsf_material_step_recorder_test.py`.
- Remote validation passed: build succeeded and a 5-event simulation produced `gsf_material_steps_test.root` with 335 `g4step_tuple` entries.
- This recorder supersedes SimTrackerHit momentum for final CEPC BH mixture fitting.

### Finding 15: True G4 pre/post-step e-mu comparison with 200 events each (2026-07-05)
- Generated 200 primary e- and 200 primary mu- samples at 1 GeV, theta=85 deg with `GsfMaterialStepRecorderAnaElemTool`.
- Outputs: `gsf_material_steps_e200.root` and `gsf_material_steps_mu200.root`, both with `g4step_tuple`.
- Electron: 15214 steps, mean event loss 0.007116 GeV, endpoint retained mean 0.982889, 40 `eBrem` steps.
- Muon: 14638 steps, mean event loss 0.000898 GeV, endpoint retained mean 0.999102, 429 `muIoni` steps.
- Material budgets are comparable: mean event `t/X0` differs by about 3%.
- Electron/muon mean event loss ratio is 7.92; endpoint loss ratio is 19.06.
- Conclusion: the stronger electron energy-loss tail is confirmed with true G4 pre/post-step truth, not only SimTrackerHit momentum.
- Reproducibility files and text summary are in `G4MaterialStepComparison/`.

### Current TODO Record Added (2026-07-05)
- Added `agents_record/2026-07-05-current-gsf-todos-after-g4-step-comparison.md`.
- It centralizes the prioritized next steps after the true G4 pre/post-step e/mu comparison.
- Highest priorities: larger true G4-step electron samples, dedicated `g4step_tuple` analysis, precise eBrem BH fitting target, then CEPC-specific BH mixture fitting.

### Finding 16: RecGsfTracking AddAndFilter pivot recovery (2026-07-07)
- Reproduced the electron GSF issue with `./run.sh DumpGsfTrks/rungsf-e--1.0-85-1.py` at `EvtMax=5`: patched stale-component guard avoided the old segfault, but most tracks failed immediately at hit 1 with `all components rejected`, producing incomplete GSF output.
- Diagnosis: `TKalDetCradle::Transport` could successfully pivot the predicted state to the next measurement position, but the subsequent KalTest re-crossing inside `Filter()` returned false on that same site. The predicted-state pivot residual was exactly zero in the failing VXD cases.
- Fix in `Reconstruction/RecGsfTracking/src/GsfAlgorithm.cpp`: when `AddAndFilter()` fails, recover only the narrow case where a predicted state exists and its pivot is already at the measurement position (`pivotResidual < 1e-3 mm`) by installing the predicted state as the filtered state and keeping the component. Non-recovered failures are deleted, so stale one-site components are not kept alive.
- Clean validation after rebuild/install: `./run.sh DumpGsfTrks/rungsf-e--1.0-85-1.py` exited 0, processed 5 events, fitted counts were `1/1`, `1/1`, `1/1`, `2/2`, `1/1`, with no `all components rejected`, no `FATAL`, and no segmentation violation. Outputs `gsf-e--1.0-85-1.root` and `gsf_flat-e--1.0-85-1.root` were finalized with nonzero size.

### Finding 17: Global simulation-derived BH model option added (2026-07-08)
- Added a parallel Bethe-Heitler splitter option, selectable with `RecGsfTracking.BHModel = "GlobalSim2GeV85"`; the default remains `"Current"` and keeps the existing tX0-dependent implementation.
- The new option encodes the simulation-derived global tracker eBrem retained-fraction model from `BHModelComparisonStudies/globalBHmodelfromSim@2GeV85Degree/`.
- The model is independent of `tX0` by construction, but the splitter interface remains `split(parent, tX0, bz)` so existing GSF call sites do not change.
- Encoded components are the bounded/truncated-Gaussian mixture normalized on `[0,1]` from the study record. Current split logic uses component weights and mean retained fraction to generate hypotheses; variances are retained in the model table for provenance/future covariance treatment.
- Fixed the splitter child-construction order so secondary children clone the original parent state before child[0] mutates the reused parent. This preserves intended component weights and kappa hypotheses for both `Current` and `GlobalSim2GeV85` models.
- Validation: `./quick_build.sh` succeeded. A 5-event smoke test with `BHModel = "GlobalSim2GeV85"` initialized as `Bethe-Heitler model: GlobalSim2GeV85`, produced 5 children at BH splits, and terminated successfully.
