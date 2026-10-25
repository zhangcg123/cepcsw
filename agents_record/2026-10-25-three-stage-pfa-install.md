# Three-stage PFA ECAL export and default installation — 2026-10-25

## Scope and products

User request: save all three ECAL cluster stages, implement and install.
This authorizes the scoped RecPFACyber source/runtime change, not deployment
of unrelated RecBreakpoint development. No batch jobs were submitted.

| Stage | Cluster collection | Hit collection |
|---|---|---|
| EnergyTimeMatchingAlg internal EcalCluster | EcalClusterTiming | EcalClusterTimingHits |
| TrackClusterConnectingAlg internal TrkMergedECAL | TrkMergedECAL | TrkMergedECALHits |
| Existing final PFO-associated output | EcalCluster | RecECALBarrel |

CreateClusterSnapshot is the single exporter for both retained upstream maps.
It exports internal longitudinal energy and shower centre; hits are copied
after rebuilding the complete constituent list on a private clone. It never
refreshes or mutates live input clusters. Timing includes earlier track-guided
clustering; middle includes existing fragment absorption and energy correction.
No additional export calibration, track associations or PFOs are added.
Final PFA code/output and breakpoint steering are unchanged. Absent internal
stages produce empty snapshots. Custom internal names need explicit adaptation.

Maintained rec.py.bk and the frozen batch card use the standard internal names
and PodioOutput keep *. No card change was needed. New rec runs save all three;
old tuples do not gain fields retroactively. Rerun reconstruction with required
digitized inputs; simulation need not be rerun if those inputs survive.

## Build and exact local validation

Built CrystalCaloRec in build.105.0.0.x86_64-el9-gcc11-opt successfully.
Existing clock-skew warnings remain. Reused the clean 20-frame input
/tmp/ecal-timing-input-20261025.root, derived from rec-barrel-1.root entries:

```text
0,1,3,4,8,10,11,18,20,21,22,26,38,41,48,54,57,60,69,78
```

Input source: sim_large_barrel_20261018/rec-barrel-1.root. Old PFA/PID outputs
were excluded when creating the clean input, avoiding duplicate keep-* branches.
Do not directly reconstruct a full old rec file without removing old outputs.

Local run card: /tmp/pfa_three_stages_20261025.py; PFA_SNAPSHOT_STAGE selects
full, timing, connected, installed. The two stopped-chain runs truncate AlgList
and its three parallel configuration lists at the respective stage.

Outputs: /tmp/pfa-three-stages-{full,timing,connected}-20261025.root, with logs.
The maintained checker passed:

```bash
python3 Reconstruction/RecPFACyber/tests/check_timing_cluster_export.py \
  /tmp/pfa-three-stages-full-20261025.root \
  /tmp/pfa-three-stages-timing-20261025.root \
  sim_large_barrel_20261018/rec-barrel-1.root \
  --entries 0,1,3,4,8,10,11,18,20,21,22,26,38,41,48,54,57,60,69,78 \
  --connected /tmp/pfa-three-stages-connected-20261025.root
```

- 32 timing fields equal the timing-only output exactly.
- 32 middle fields equal the connected-only output exactly.
- 199 checked final-PFA fields equal original rec entries exactly.
- Finite nonnegative snapshot energies, valid hit-link collection IDs/indices,
  and no duplicate ROOT branches.
- Previous timing-only development output also matches all 32 timing fields.

20-frame descriptive totals: timing 20 clusters, 535.2473754882812 GeV;
middle 20 clusters, 561.8338623046875 GeV; final 28 clusters,
570.5353393554688 GeV. These differ by existing merging/calibration/output
logic; they are not evidence of calibrated energy closure.

## Actual default deployment

Staged only RecPFACyber with cmake --install on its build subdirectory and
prefix /tmp/pfa-three-stage-install.Uva2n6/stage. Exactly four staged files
differed from the default runtime. Backed them up under
/tmp/pfa-three-stage-install.Uva2n6/previous, then deployed each with an atomic
same-directory rename, avoiding truncation of loaded shared libraries:

```text
include/CyberPFAlg.h
include/Tools/OutputCreator.h
python/RecPFACyber/CrystalCaloRecConf.py
lib/libCrystalCaloRec.so
```

Default runtime: InstallArea/x86_64-el9-gcc11-opt. Other staged files were
identical and not replaced. No full-tree install was performed.

SHA256 libCrystalCaloRec.so:

- previous: 3ba471eecb0b3bc29b1346d67cfa6dee4a41b53c667e103e4ba0c40a214d63ab
- installed: 17e5eb775139096e718f24519b7c72e3bc0c85f698d6f9127851a19756dcfaa5

Installed libRecBreakpoint.so remained:
6704f6c2b441b4556db1f110a35ee7f4baf1921338f25b80f2cef6066db62af7.
Its separate post-fit photon projection development is still NOT installed.

Fresh shell, source setup.sh, no build-library override:

```bash
PFA_SNAPSHOT_STAGE=installed gaudirun.py /tmp/pfa_three_stages_20261025.py
python3 /tmp/check_pfa_three_stage_installed.py
```

The default-runtime smoke processed three frames (original entries 0,1,3),
exited successfully, and matched all 263 checked snapshot/final-PFA fields
against the first three verified build frames exactly. Output/log:
/tmp/pfa-three-stages-installed-20261025.root and .log.

Rollback: restore only the four listed files from the previous/ backup using
same-directory temporary copies and atomic rename. The backup is temporary
local storage, not a durable archive. The prior implementation remains in Git.

## Memory and limitations

Full outgoing AGENTS.md is preserved in
2026-10-25-agents-before-three-stage-pfa-install.md; global laws are unchanged.
This record supersedes the NOT INSTALLED status in the earlier timing-export
record for PFA only. Stage-export validation is mechanical, not physics
validation. Tuple-size population impact, endcap behavior and energy response
were not measured here. No existing tuple was overwritten or deleted.
