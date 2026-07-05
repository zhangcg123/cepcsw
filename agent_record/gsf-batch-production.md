---
name: gsf-batch-production
description: How batch jobs are submitted and run on CEFS/HTC
metadata:
  type: reference
---

# GSF Batch Production

## Infrastructure
- **CEFS path**: `/cefs/higgs/zhangcg/cepc/28Jun2026/CEPCSW/`
- **Local dev path**: `/aifs/user/data/zhangcg/gsfdev/CEPCSW/`
- **Batch system**: HTCondor via `hep_sub` (IHEP batch submission)
- **Memory**: 8000 MB per job
- **Events per job**: 1000 (configurable via `NEVT` in `subtrkjobs.sh`)

## Key Scripts

### `dump_gsftrk.sh` (43 lines)
Located at project root: `/aifs/user/data/zhangcg/gsfdev/CEPCSW/dump_gsftrk.sh`

Args: `particle momentum_mag momentum_trn theta seed nevt`

Runs 3 stages sequentially:
1. Copy `sim.py.bk` → substitute parameters via `sed` → `./run.sh`
2. Copy `trk.py.bk` → substitute parameters via `sed` → `./run.sh`
3. Copy `gsf.py.bk` → substitute parameters via `sed` → `./run.sh`

Working directory: `cd /cefs/higgs/zhangcg/cepc/28Jun2026/CEPCSW/`

### `subtrkjobs.sh` (19 lines)
Located at project root: `/aifs/user/data/zhangcg/gsfdev/CEPCSW/subtrkjobs.sh`

Submits batch jobs via `hep_sub`:
```bash
for particle in mu-
  for seed in {1..2}
    for theta in 85  # 135 is commented out
      for trans_mom in {0.5,1.0,2.0}
        mom = trans_mom / sin(theta * pi/180)  # total momentum
        hep_sub dump_gsftrk.sh -mem 8000 -g cms -o outlog/... -e outlog/... -argu ...
```

### Template Files (in `DumpGsfTrks/`)
- `sim.py.bk` — Geant4 simulation (particle gun, TDR_o1_v01 geometry)
- `trk.py.bk` — Full digitization + tracking chain
- `gsf.py.bk` — GSF refit + flat tuple

Templates use placeholder values (12340, 'mu-', etc.) that are replaced by `sed`.

### Generated Config Files
Pattern: `{stage}-{particle}--{pT}-{theta}-{seed}.py`
- 36 generated files for electrons (e-: 0.5/1.0/2.0 GeV × 85° × seed 1/2)
- 36 generated files for muons (mu-: 0.5/1.0/2.0 GeV × 85° × seed 1/2)
- Each stage (sim, trk, gsf) × each parameter point = 3 files per point

## Output Logs
`/cefs/higgs/zhangcg/cepc/28Jun2026/CEPCSW/outlog/trk_{particle}_{pT}_{theta}_{seed}.out`
`/cefs/higgs/zhangcg/cepc/28Jun2026/CEPCSW/outlog/trk_{particle}_{pT}_{theta}_{seed}.err`

**Why:** Reference for running or modifying the batch production pipeline.
**How to apply:** When you need to produce more samples, add new parameter points, or debug batch failures. See [[gsf-data-flow]], [[gsf-project-overview]].