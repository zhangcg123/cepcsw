# Plan: Rollback 2026-06-28 Changes (3-Hit Prefit + Q/P Refinement)

## Context
回滚到 2026-06-28 改动之前的状态，即去掉：
- 3-hit 解析预拟合（AnalyticalPrefit）
- 多轮 q/p 扫描（QPRefinement）
- 可配置 KappaSeedCov（恢复为硬编码 1e-7）
- 相关的 lambda 封装（runGsfPass，它仅为支持多次调用而引入）

## 需要修改的文件

### 1. `Reconstruction/RecGsfTracking/src/GsfAlgorithm.h`（~10行删除）
删除 6 个 Gaudi::Property：
- `m_kappaSeedCov`（line 81）
- `m_useAnalyticalPrefit`（line 83）
- `m_qpRefinement`（line 85）
- `m_qpRefinementChi2Thresh`（line 86）
- `m_qpRefinementSteps`（line 87）
- `m_qpRefinementStepSize`（line 88）

### 2. `Reconstruction/RecGsfTracking/src/GsfAlgorithm.cpp`（~120行删除+重写）
- `makeInitialSite` 函数签名和实现：`kappaCov` 参数改为函数内硬编码 `1e-7`
- Step 3：删除 3-hit 解析预拟合 block（lines 386-408），保留一句 `kappaSeed = (bz != 0) ? (seed.omega / alpha) : 1e-5;`
- 删除 `runGsfPass` lambda（lines 412-478），将 forward filter 逻辑恢复为内联
- 删除 Step 4b q/p refinement（lines 492-552）
- Verbose dump：删除 kappa 对比行和 q/p refit 行（lines 675-683）

### 3. `Reconstruction/RecGsfTracking/options/run_gsf_e.py`（~8行删除）
删除新属性的设置：
- `gsf.AnalyticalPrefit = True`
- `gsf.KappaSeedCov = 1e-4`
- `gsf.QPRefinement = True`
- `gsf.QPRefinementChi2Thresh = 3.0`
- `gsf.QPRefinementSteps = 3`
- `gsf.QPRefinementStepSize = 0.1`

### 4. `agents_record/DEVELOPMENT.md`
更新状态：标注回滚完成，当前基线为"回滚后原始版本"

## 验证步骤
1. `./build.sh` 编译通过
2. 用 `run_gsf_e.py` 跑一条径迹（或少量 events），对比 GSF 和 LCIO 的 track parameters（pT, eta, phi, d0, z0, chi2/ndf），确认回滚后结果合理
