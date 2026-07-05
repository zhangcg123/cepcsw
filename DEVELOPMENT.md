# GSF Tracking Development Log

## 当前状态
- 回滚到 2026-06-28 之前：**无 3-hit 预拟合、无 q/p 扫描、kappaCov=1e-7**
- 测试通过（5 events, 1.0 GeV e- @ 85°），输出正常
- Verbose dump 已增强：BH split 时显示 parent/child kappa + 下一 hit 的预测 vs 测量

## 测试结果 (Baseline, 2026-07-05)
### 1.0 GeV e- @ 85°, 5 events
- GSF pT 与 LCIO pT 差异 < 0.15%，几乎完全一致
- Event 2: LCIO pT=0.83（truth=1.0），GSF 继承了错误
- BH 分裂在部分事件触发（splits=1-2），但 6 组分最终坍缩为 1

### 0.5 GeV e- @ 85°, 5 events
- 8 条径迹中 3 条 LCIO pT 偏差 > 12%，GSF 忠实复制错误
- 极端：event 1 trk 1: GSF d0 从 0.0001 跳到 -139.9 mm（径迹爆炸）
- 唯一改善：event 4 χ²/ndf 从 268/196 降到 190/190（拟合更平滑，但 pT 未变）

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

## 下一步
- [ ] 验证 BH 参数化：检查 tX0=0.01 时的 6 组分多项式输出是否合理
- [ ] 检查 split() 中 kappa 更新方式：`newKappa = parentKappa / fracMomentum` 是否正确
- [ ] 考虑降低 BHSplitThreshold 或调整 split 策略
- [ ] 【远期】从 Geant4 模拟生成 CEPC 专用的 BH 参数化：单电子穿过 tracker 各层材料，记录实际能量损失分布，EM 算法拟合高斯混合，替代现有 ACTS 数据

## 待解决问题
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
- New plan recorded in `agent_record/plans/2026-07-05-measure-cepc-electron-energy-loss.md`.
