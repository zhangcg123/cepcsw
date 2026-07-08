---
name: gsf-bethe-heitler-model
description: The Bethe-Heitler bremsstrahlung parameterization — how splitting works, regimes, data format, and known issues
metadata:
  type: reference
---

# Bethe-Heitler Splitter

## What is tX0?
tX0 = 径迹穿过的材料厚度 / 辐射长度（radiation length, X0）。辐射长度是材料的一个属性，表示电子通过轫致辐射损失 1/e 能量的平均距离。

在代码中，`thicknessInX0()` 逐层累加：
```cpp
tX0 += layer->surface()->innerThickness() / matIn.GetRadLength();
tX0 += layer->surface()->outerThickness() / matOut.GetRadLength();
```
当累计 tX0 超过 `BHSplitThreshold` 时触发 BH 分裂，然后重置计数器。

## 物理预期
轫致辐射在高能电子中主导，近似有：**ΔE/E ≈ tX0**（即穿过 tX0=0.01 的材料，预期能量损失约 1%）。

电离能损：dE/dx ≈ 1.5 MeV/(g/cm²) in Si。Si X0=93.6 mm, density=2.33 g/cm³。tX0=0.01 → 0.936 mm → 0.218 g/cm² → ΔE_ion ≈ 0.33 MeV → 对 1 GeV 电子仅 0.03%。**电离能损可忽略。**

## Source: ACTS AtlasBetheHeitlerApprox<6,5>
The parameterization data is embedded directly in `BetheHeitlerSplitter.cpp` to avoid requiring Eigen/Boost transitive dependencies from ACTS. It uses 6th-degree polynomial coefficients for 6 components across 3 statistics (weight, mean, var).

## Four Regimes

### 1. Negligible material (t/X0 < 0.0001)
- **1 component**, no energy loss
- weight=1.0, mean=1.0 (no momentum change), var=0.0

### 2. Single Gaussian (0.0001 ≤ t/X0 < 0.002)
- **1 component**, single Gaussian approximation
- c = x / ln(2)
- mean = 2^(-c), var = 3^(-c) - 4^(-c)
- At tX0=0.0019: mean=0.998 → **0.2% energy loss** ✓ 物理合理

### 3. Low-x parameterization (0.002 ≤ t/X0 < 0.1)
- **6 components**，Weight: invLogit(poly), Mean: invLogit(poly), Var: exp(poly)
- **At tX0=0.01 (default threshold):** means={0.289, 0.001, 0.010, 0.008, 0.007, 0.009}
  → **71-99.9% energy loss** ✗ 物理完全错误（预期 ~1%）

### 4. High-x parameterization (0.1 ≤ t/X0)
- **6 components**, x capped at 0.2, raw polynomial (no transform)
- At tX0=0.1: means={0.93, 0.54, 0.10, 0.08, 0.08, 0.02} → 逐渐合理
- At tX0=0.2: means={0.93, 0.63, 0.44, 0.47, 0.50, 0.67} → 能量损失 7-56%

## 已知问题：Low-x 区域参数化失败

### 问题 1: 在 tX0=0.002 处存在严重不连续
- 单高斯（tX0=0.0019）：mean=0.998 → 0.2% 能量损失
- 6 组分（tX0=0.002）：mean=0.0005-0.276 → 72-99.95% 能量损失
- **跳跃了 400 倍！**

### 问题 2: Low-x 多项式在 tX0=0.01 时给出物理上不可能的值
- 轫致辐射预期：ΔE/E ≈ 0.01 = 1%
- BH 模型：ΔE/E = 71-99.9%
- 所有子组分被下一个 hit 以 Δχ²≈30 否决

### 根因分析
- BH 参数化是为 ATLAS 设计的，单层 tX0 ~ 0.1，累积极快进入 high-x 区
- CEPC tracker 材料很薄（单层 tX0 ~ 1e-4 到 3e-3），累计 tX0=0.01 仍处于 low-x 区
- invLogit 变换在多项式值接近 ±∞ 时饱和到 0 或 1
- 多项式在 tX0=0.01 范围内的外推产生了极端值

### 实际影响
- 在 CEPC 的测试中，tX0 累积到 0.01 触发分裂，但产生的子组分全部被 KF 拒绝
- **GSF 等价于没做分裂**


## 2026-07-08 implementation update

`BetheHeitlerSplitter` now supports named model options while preserving the existing split interface:

- `Current`: default existing model.
- `GlobalSim2GeV85`: 5-component simulation-derived global retained-fraction model from `BHModelComparisonStudies/globalBHmodelfromSim@2GeV85Degree/`.

`RecGsfTracking` exposes the model through:

```python
gsf.BHModel = "Current"
gsf.BHModel = "GlobalSim2GeV85"
```

The new global model does not depend on `tX0`; this is intentional for this fitted sample. The `split(parent, tX0, bz)` interface keeps `tX0` for compatibility and future models.

Splitter implementation notes:

- Child components are now created before mutating the reused parent, so all children clone from the original parent state.
- Child weights use the original parent weight, not a parent weight already modified by child 0.
- The old note that child 0 kappa was not updated is superseded by this implementation.

GSF diagnostics now separate final fit summaries from detailed split logs:

```python
gsf.VerboseDump = True       # final fit parameter table
gsf.VerboseSplitDump = False # suppress per-split component dump
```

See `agent_record/2026-07-08-global-bh-gsf-run.md` for the current run recipe and light-eBrem test results.

## Split Operation
```
BetheHeitlerSplitter::split(parent, tX0, bz) -> vector<GsfComponent*>
```

## 累积 vs 逐层分裂

### 当前做法：累积 tX0，到阈值一次性分裂
- 逐层累加 tX0，当累计值超过 `BHSplitThreshold`（默认 0.01）时触发分裂
- 分裂后清零计数器，重新累积
- 问题：CEPC tracker 中 0.01 需要累积几十层，但 BHSplitThreshold=0.01 已远超单高斯区域上限（0.002），进入了 low-x 6 组分区域，参数化给出错误值

### 逐层分裂的困境
- 单层 tX0 最大 3.2e-3（VTX/SIT），最小 4.3e-5（TPC 气体）
- 大部分层 tX0 < 0.002 → 落在单高斯区域，公式物理正确
- 但 200+ 层 × 6 组分 clone = 计算量爆炸

### 单高斯区域（0.0001 ≤ tX0 < 0.002）
能量损失用**单一高斯分布**描述，公式解析：
```
c = tX0 / ln(2)
mean = 2^(-c)        ≈ 1 - tX0     （能量保留比例，物理正确）
var  = 3^(-c) - 4^(-c)
```
- 轫致辐射薄材料近似：ΔE/E ≈ tX0
- tX0=0.0019 时 mean≈0.998（0.2% 损失），物理合理
- 这是 BH 参数化中**唯一对 CEPC 场景有效的区域**

### 不连续边界（tX0=0.002）
- 单高斯：mean=0.998 → 0.2% 能量损失
- 6 组分 low-x：mean ∈ {0.276, 0.001, 0.007, 0.005, 0.005, 0.006} → 72-99.9% 能量损失
- **跳跃 400 倍**，物理上完全错误

### 可能的改进方向
1. 延长单高斯区域上限（从 0.002 提高到 0.05 或 0.1），绕过有问题的 low-x 参数化
2. 降低 BHSplitThreshold 到 0.002，确保每次分裂都在单高斯区域内
3. 从 Geant4 模拟重新生成 CEPC 专用的参数化
For each component in the BH mixture:
1. Compute `fracMomentum = max(mean, 0.01)` (guards against zero energy)
2. New kappa = parentKappa / fracMomentum (lower momentum → higher curvature)
3. For i=0: reuse parent component in-place (set weight = parent.weight * mixture[i].weight)
4. For i>0: deep-clone parent via `GsfComponent::clone()`, then overwrite kappa(2,0) in the last site's state vector
5. Weight of each child = parent.weight * mixture[i].weight

**2026-07-08 update**: the child-cloning/weighting path has been fixed so child 0 and cloned children are all derived from the original parent state and original parent weight before per-child kappa updates.

## tX0 累积数据（实测）
在 1.0 GeV e- @ 85° 的 232 个 hit 中：
- 单层最大 tX0：3.2e-3（VTX/SIT 层）
- 单层最小 tX0：4.3e-5（TPC 气体层）
- 全程总 tX0：~0.02
- BHSplitThreshold=0.01 时，触发 1-2 次分裂，均在 tracker 中段（r≈750-1800 mm）

**Why:** Understanding the physics model is essential for tuning the BH splitting threshold or evaluating the quality of the energy loss model.
**How to apply:** Reference this when modifying `BetheHeitlerSplitter.cpp` or tuning `BHSplitThreshold`. See [[gsf-algorithm-flow]], [[gsf-mixture-reduction]].