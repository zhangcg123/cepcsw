# GSF Development Handoff - 2026-07-05

## Repository State
- Remote login workspace: `/aifs/user/data/zhangcg/gsfdev/CEPCSW`
- Active branch: `gsf-simhit-energy-loss-tuple-20260705`
- Remote tracking branch: `origin/gsf-simhit-energy-loss-tuple-20260705`
- Current remote head before this handoff note: `fba53e145212e719a0ebf75db58782eb2c4b6078`
- The branch was pushed to `git@code.ihep.ac.cn:zhangcg/CEPCSW.git`.
- Older local milestone branches were deleted after confirming they are ancestors of the active branch:
  - `gsf-bh-baseline-20260705`
  - `gsf-bh-thin-gaussian-20260705`
  - `gsf-bh-cepc-two-component-20260705`

## What This Branch Contains
Commits above `origin/master` before this handoff note:

```text
fba53e1 Add GSF sim-hit truth tuple
3e0c460 Record plan to measure CEPC electron energy loss
b9c2447 Add CEPC two-component BH toy model
7965974 Test CEPC thin Gaussian BH splitting
5871a69 Add baseline GSF tracking development state
```

The active branch contains all current GSF/BH development work. It is the only development branch that needs to be kept for now.

## Main Technical Conclusions
- The original ACTS/ATLAS Bethe-Heitler mixture is not suitable for the thin CEPC tracker material regime tested here. At `tX0` around `0.01`, it can create unphysical children near `pT ~ 0.01 GeV`, which are immediately rejected by the next hit.
- A thin one-component Gaussian sanity patch fixes the unphysical momentum scale, but it forces energy loss on every split and worsens chi2 for ordinary tracks.
- The current two-component CEPC toy model is safer: it keeps a dominant no-loss branch and a moderate-loss tail. It avoids the chi2 regression, but it does not yet improve electron pT performance in the small smoke test.
- Therefore the next real development step is to measure CEPC electron energy-loss behavior from simulation truth and tune/replace the BH model based on that.

## New Truth Diagnostic Module
User clarified that the existing flat tuples do not contain simulation information. A new module was added under `Reconstruction/RecGsfTracking`:

- `src/GsfSimHitTuple.h`
- `src/GsfSimHitTuple.cpp`
- registered in `CMakeLists.txt` as `RecGsfSimHitTuple`
- test option: `options/run_gsf_simhit_tuple_test.py`
- analysis helper: `scripts/analyze_simhit_energy_loss.py`

The module writes a ROOT tree named `simhit_tuple` with primary MCParticle summary and per-SimTrackerHit truth information: position, radius, momentum at hit, deposited energy, path length, PDG/link IDs, and retained momentum fraction relative to the primary.

Important limitation: `edm4hep::SimTrackerHit::getMomentum()` is momentum at the hit position. It is not a full Geant4 pre-step/post-step material-loss pair. It is enough for first radius/layer momentum evolution studies, but exact material-by-material BH tuning may still require a Geant4 stepping recorder.

## Validation Already Done
Build:

```bash
./quick_build.sh > outlog/gsf_simhit_tuple_build.log 2>&1
```

Result: build succeeded and installed `libRecGsfSimHitTuple.so` plus python config.

Test:

```bash
./run.sh Reconstruction/RecGsfTracking/options/run_gsf_simhit_tuple_test.py > outlog/gsf_simhit_tuple_test.log 2>&1
```

Result: test succeeded on 5 events and wrote `gsf_simhit_tuple_test.root`.

First diagnostic summary from `trk-e--1.0-85-1.root`:

```text
last-hit retained p/p_primary: 0.90943 0.99465 0.99651 0.01621 0.99470
global retained quantiles 0/1/5/10/50/90/99/100%: 0.00002 0.00223 0.83404 0.83420 0.99607 0.99792 0.99976 0.99988
P(ret<0.99)=0.2053, P(ret<0.95)=0.2053, P(ret<0.90)=0.2044 over dumped sim hits
```

Interpretation: some events/hits show large momentum loss candidates, but the 5-event sample is too small and hit-level momentum must be interpreted carefully.

## Files That Should Not Be Pushed
Generated data and build artifacts are ignored and were not pushed:

- `*.root`
- `outlog/`
- `InstallArea/`
- `build.*/`
- generated plots and local IDE files

Before future pushes, keep checking:

```bash
git status --short --ignored | head -80
```

## Suggested Resume Steps
1. Start from branch `gsf-simhit-energy-loss-tuple-20260705`.
2. Confirm clean state:
   ```bash
   git status --short --branch
   ```
3. Rerun the sim-hit tuple on larger electron samples, not only 5 events.
4. Extend `analyze_simhit_energy_loss.py` to bin retained momentum by detector/radius/layer and identify hard-bremsstrahlung-like events.
5. Decide whether SimTrackerHit momentum is sufficient for CEPC BH tuning. If not, add a Geant4 stepping recorder for primary electron pre/post-step momentum in tracker volumes.
6. Tune a CEPC BH mixture using measured energy-loss distributions, then compare LCIO vs GSF pT summaries and verbose per-event parameter tables.
