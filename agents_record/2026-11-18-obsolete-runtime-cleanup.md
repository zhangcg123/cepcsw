# Obsolete private runtimes removed — 2026-11-18

The user asked whether the extra installations were still active and whether
they could be removed. Removed the two obsolete partial installation trees:

- `build.gsf-ecal-tuple-dev/install/`
- `build.gsf-selection-dev/install/`

Also removed obsolete shared-library files/symlinks from historical analysis,
regression, backup and private-build output directories. This is binary cleanup,
not removal of the surrounding source, studies or their results. In total,
66 files, 69,280,353 apparent bytes, were removed: 29 files in the two private
install trees and 37 other library files/symlinks. No Git-tracked file was
removed. Source copies, headers, logs, cards, ROOT data, plots and notes remain.

Preserved unchanged:

- Default `InstallArea/x86_64-el9-gcc11-opt/`, selected by root `setup.sh`.
- Main configured build `build.105.0.0.x86_64-el9-gcc11-opt/`.
- Three frozen libraries in
  `higgs_eeHX_ISRFSRON_REC_ECALTRUTH_500jobs_200evt_20261112/lib/`.
  The completed campaign's worker still prepends this directory and verifies
  its hashes, so it remains a dependency of that historical worker's reruns.
  Its PFA/ECAL-correction library hashes match the current default. New
  production continues to use the default installation under the active law.

## Activity checks and limits

All five known campaigns have terminal markers on every job:

| Campaign | Completed | Failed |
|---|---:|---:|
| BP_ECAL_HCAL_PAIRED_20261117 | 500 | 0 |
| BP_H0F_THETA25_20261113 | 500 | 0 |
| GSF_BPNEUTRALS_20261118 | 499 | 1 |
| REC_ECALTRUTH_20261112 | 500 | 0 |
| mumuHgg_CLUSTER_8GB_20261118 | 25 | 0 |

The GSF failure is job149, gaudirun exit139, recorded before cleanup. It was
not retried or diagnosed in this task. Current campaign launchers do not
select the removed development runtimes; the previous-GSF provenance JSON
names one as a historical input only.

Host-context maps on ailogin002 showed no loaded obsolete libraries in the
18 inspected same-user processes; four process map reads were unavailable.
`hep_q` is unavailable even after sourcing the host shell configuration.
These are known-campaign and local-host checks, not a global scheduler audit.

## Verification and future use

Every removed file/link was inventoried with size, modification time, content
hash or symlink target and checked again before deletion. All209 protected
library/setup-file checks passed afterward, including unchanged hashes and
size/mtime. Both obsolete install trees are absent. No build, install, native
test, reconstruction or batch submission ran during cleanup.

Machine-readable inventories and result:
`TrackingPerformanceStudies/runtime_cleanup_20261118/inventory.json` and
`result.json` (uncommitted analysis artifacts).

Historical A/B scripts can still mention deleted binary paths; rebuild an
explicit isolated regression runtime before rerunning those controls. Do not
silently substitute current libraries for a historically different control,
and do not treat a saved old-runtime path as an installation still present.
