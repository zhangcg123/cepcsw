---
name: gsf-batch-production
description: How batch jobs are submitted and run for GSF/sim-hit studies
metadata:
  type: reference
---

# GSF Batch Production

## Infrastructure
- **Current dev path**: `/aifs/user/data/zhangcg/gsfdev/CEPCSW`
- **Configurable workdir**: scripts default to the current dev path via
  `WORKDIR=${CEPCSW_GSFDEV_DIR:-/aifs/user/data/zhangcg/gsfdev/CEPCSW}`.
- **Batch system**: IHEP HTCondor wrapper `hep_sub`
- **Batch PATH setup**: `export PATH=/cvmfs/common.ihep.ac.cn/software/hepjob/bin:$PATH`
- **Memory in existing scripts**: `-mem 8000`
- **Group in existing scripts**: `-g cms`
- **Events per job in existing scripts**: `NEVT=1000`

The old hard-coded path `/cefs/higgs/zhangcg/cepc/28Jun2026/CEPCSW` has been removed from the active scripts. Use `CEPCSW_GSFDEV_DIR` only if running the same scripts from another checkout.

## Existing Full Chain Scripts

### `subtrkjobs.sh`
Located at project root.

Purpose: submit many parameter points through `hep_sub`.

Current loop shape:
```bash
NEVT=1000
for particle in mu-
do
  for seed in {1..2}
  do
    for theta in 85
    do
      for trans_mom in {0.5,1.0,2.0}
      do
        mom=$(echo "scale=3; $trans_mom / (s($theta * 3.1415926 / 180))" | bc -l)
        hep_sub dump_gsftrk.sh \
          -mem 8000 -g cms \
          -o ${WORKDIR}/outlog/trk_${particle}_${trans_mom}_${theta}_${seed}.out \
          -e ${WORKDIR}/outlog/trk_${particle}_${trans_mom}_${theta}_${seed}.err \
          -argu ${particle} ${mom} ${trans_mom} ${theta} ${seed} ${NEVT}
      done
    done
  done
done
```

Notes:
- `trans_mom` is the target transverse momentum used in filenames.
- `mom = trans_mom / sin(theta)` is the total gun momentum/energy passed to the simulation template.
- The current script still loops over `mu-`; change to `e-` for electron production.

### `dump_gsftrk.sh`
Located at project root.

Arguments:
```text
particle momentum_mag momentum_trn theta seed nevt
```

It runs three stages sequentially:
1. **Simulation**: copy `DumpGsfTrks/sim.py.bk` to `runsim-${particle}-${pT}-${theta}-${seed}.py`, replace placeholders, run with `./run.sh`.
2. **Digitization + tracking**: copy `DumpGsfTrks/trk.py.bk` to `runtrk-${particle}-${pT}-${theta}-${seed}.py`, replace input/output names and seed/event count, run with `./run.sh`.
3. **GSF refit**: copy `DumpGsfTrks/gsf.py.bk` to `rungsf-${particle}-${pT}-${theta}-${seed}.py`, replace input/output names, run with `./run.sh`.

Outputs from the full chain:
```text
sim-${particle}-${pT}-${theta}-${seed}.root
trk-${particle}-${pT}-${theta}-${seed}.root
gsf-${particle}-${pT}-${theta}-${seed}.root
gsf_flat-${particle}-${pT}-${theta}-${seed}.root
```

## Template Files
Located in `DumpGsfTrks/`:
- `sim.py.bk`: Geant4 particle-gun simulation using `TDR_o1_v01/TDR_o1_v01.xml`.
- `trk.py.bk`: digitization + tracking chain using sim tracker hits.
- `gsf.py.bk`: GSF refit and flat tuple.

Templates use placeholders such as `12340`, `mu-`, `sim_v01.root`, `rec_v01.root`, `trk_v01.root`, `gsf_v01.root`, and `gsf_flat_v01.root`; the shell scripts replace them with `sed`.

## Large-Statistic BH Energy-Loss Study
For CEPC Bethe-Heitler tuning, do **not** run large-stat GSF tracking first. The goal is to measure electron energy-loss behavior from simulation truth. Use a lighter workflow:

```text
particle gun simulation
  -> RecGsfSimHitTuple
  -> ROOT/Python aggregation
```

Recommended adaptation:
- Create a new submission script, for example `sub_simhit_jobs.sh`, modeled on `subtrkjobs.sh`.
- Create a new worker script, for example `dump_simhit_energy_loss.sh`, modeled on the simulation section of `dump_gsftrk.sh` plus a tuple stage.
- Reuse `DumpGsfTrks/sim.py.bk` for particle-gun simulation.
- Skip `trk.py.bk` and `gsf.py.bk` unless a later validation subset needs full reconstruction.
- After simulation, run a `RecGsfSimHitTuple` option over the sim output to produce compact tuple files.

The tuple stage should configure:
```python
from Configurables import RecGsfSimHitTuple
simtuple = RecGsfSimHitTuple("RecGsfSimHitTuple")
simtuple.OutputFile = "simhit_tuple-${particle}-${pT}-${theta}-${seed}.root"
simtuple.PrimaryOnly = True
simtuple.ElectronOnly = True
simtuple.SimHitCollectionNames = [
    "VXDCollection",
    "ITKBarrelCollection",
    "ITKEndcapCollection",
    "TPCCollection",
    "OTKBarrelCollection",
    "OTKEndcapCollection",
]
```

Suggested scan dimensions:
```text
particle: e- first, then e+ as cross-check
pT or momentum: 0.5, 1.0, 2.0, 5.0, 10.0, 20.0 GeV
theta: barrel and transition/endcap points, e.g. 20, 35, 50, 65, 85, 100, 130, 160 deg
phi: use full 0-360 random gun first; add fixed phi scan only if material non-uniformity matters
seeds/events: at least O(1000) events per bin; larger for hard-bremsstrahlung tails
```

Recommended output naming:
```text
sim-${particle}-${pT}-${theta}-${seed}.root
simhit_tuple-${particle}-${pT}-${theta}-${seed}.root
outlog/simhit_${particle}_${pT}_${theta}_${seed}.out
outlog/simhit_${particle}_${pT}_${theta}_${seed}.err
```

## What To Measure From Tuples
Use `Reconstruction/RecGsfTracking/scripts/analyze_simhit_energy_loss.py` as the starting point, then extend it for large statistics:
- retained momentum fraction `hit_p / mc_p` by event
- retained momentum by detector, radius, and layer proxy
- first hard-loss radius
- hard-loss probability per detector/layer
- core/no-loss fraction and tail fraction
- dependence on momentum and theta

Important limitation: `edm4hep::SimTrackerHit::getMomentum()` is the momentum at the hit position, not a Geant4 pre/post-step pair. It is useful for first layer/radius evolution studies; exact material-by-material loss may still require a Geant4 stepping recorder.

## Hygiene Before Submission Or Push
- Ensure `outlog/` exists before submitting jobs.
- Check scripts with `bash -n`.
- Check generated data are ignored and not staged:
  ```bash
  git status --short --ignored | head -80
  ```
- ROOT outputs, logs, `InstallArea/`, and build directories should not be pushed.

**Why:** Reference for running or modifying the batch production pipeline and for preparing large-stat CEPC electron energy-loss samples.
**How to apply:** Use the full chain only for GSF validation. Use the lighter sim-hit tuple workflow for BH model tuning. See [[gsf-data-flow]], [[gsf-project-overview]], and [[2026-07-05-measure-cepc-electron-energy-loss]].
