---
name: gsf-build-system
description: How to build the GSF tracking code, CMake configuration, dependencies
metadata:
  type: reference
---

# GSF Build System

## CMakeLists.txt Location
`Reconstruction/RecGsfTracking/CMakeLists.txt`

## Two Gaudi Modules

### 1. RecGsfTracking (main algorithm library)
Sources: `GsfComponent.cpp`, `GsfMixture.cpp`, `BetheHeitlerSplitter.cpp`, `GsfAlgorithm.cpp`

Links against:
- `DDKalTest_LIBRARIES` — DD4hep-based Kalman filter measurement layers
- `KalTest_LIBRARIES` — Core Kalman filter engine (TKalTrack, THelicalTrack, etc.)
- `TrackSystemSvcLib` — HelixTrack utilities
- `Gaudi::GaudiKernel` — Gaudi framework
- `k4FWCore::k4FWCore` — EDM4hep data handling
- `EDM4HEP::edm4hep`, `EDM4HEP::edm4hepDict` — Event data model
- `LCIO_LIBRARIES` — LCIO I/O
- `DataHelperLib` — Data helper utilities

### 2. RecGsfFlatTuple (flat ntuple writer)
Sources: `GsfFlatTuple.cpp`

Links against:
- `Gaudi::GaudiKernel`, `k4FWCore::k4FWCore`, `EDM4HEP::edm4hep`, `EDM4HEP::edm4hepDict`
- `ROOT_LIBRARIES` — TFile, TTree

## Build Integration
In `Reconstruction/CMakeLists.txt` (line 10):
```cmake
add_subdirectory(RecGsfTracking)
```
This replaced the commented-out `# add_subdirectory(RecActsTracking)`.

## Include Paths
Both modules expose `src/` as a public include directory, so headers like `GsfAlgorithm.h` are accessible as `#include "GsfAlgorithm.h"` from within the module.

## Installation
Both modules are installed via `install(TARGETS ... EXPORT CEPCSWTargets)`.

## Build Command
```bash
cd /aifs/user/data/zhangcg/gsfdev/CEPCSW
source setup.sh   # 加载 CVMFS 外部库（CEPCSWEXTERNAL 等环境变量）
./build.sh        # cmake 配置 + make + install
```
`setup.sh` 从 `/cvmfs/cepcsw.ihep.ac.cn/prototype/releases/externals/` 加载 LCG 外部依赖。
`build.sh` 在 `build/` 目录下执行 cmake，安装到 `InstallArea/`。

## Fast Incremental Build
改动少量文件后，用 `quick_build.sh` 一键编译+安装：
```bash
./quick_build.sh
```
等价于手动执行：
```bash
cd build.105.0.0.x86_64-el9-gcc11-opt && make -j 8 && cd -
```
只编译改动过的文件，几秒完成。

**注意**：make 后必须 install 到 InstallArea，否则运行时加载的是旧 .so：
```bash
cd build.105.0.0.x86_64-el9-gcc11-opt && cmake --install . && cd -
```
如果 make 没检测到源文件变化（`Nothing to be done`），可以删除 stale .o 强制重编：
```bash
rm build.105.0.0.x86_64-el9-gcc11-opt/lib/libRecGsfTracking.so
make -j 8 && cmake --install .
```

## Run Command (Test)
```bash
./run.sh Reconstruction/RecGsfTracking/options/run_gsf_test.py
```
**默认测试脚本**：`options/run_gsf_test.py`，EvtMax=5, VerboseDump=True, 无 flat tuple, 输出到 `gsf_test.root`。输入文件为 `trk-e--1.0-85-1.root`。

批次生产用 `DumpGsfTrks/` 下的配置（EvtMax=200, VerboseDump=False, 含 flat tuple）。

**Why:** Reference for build configuration, adding new source files, or understanding dependencies.
**How to apply:** When adding a new .cpp file, add it to the SOURCES list in CMakeLists.txt. See [[gsf-project-overview]].