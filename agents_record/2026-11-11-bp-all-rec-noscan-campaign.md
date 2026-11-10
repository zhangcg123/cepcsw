# Prepared full current-REC BP campaign, no location scan

User request: rerun BP flat tuples using all current REC files and events;
provide the submission command. Preparation only; no jobs submitted.

- Input: `higgs_eeHX_ISRFSRON_REC_TPCFIX_500jobs_200evt_20261110/`.
- Output: `higgs_eeHX_ISRFSRON_BP_NOSCAN_PREECAL_500jobs_200evt_20261111/`.
- Native ROOT metadata inventory confirms all500 IDs1..500, exactly200 events
  each:100,000 events. All500 REC completion markers also exist.
- Prepared500 BP-only manifests/cards,200 events/job; no event-index filter.
- Truth-production-electron selector ON; truth-location000/100/110 families
  remain available. Free-loss/beam/charged ON; separate-neutral OFF.
- PreReclustering ElectronPhoton ECAL input, sigma_log_loss0.001, delayed-neutral
  campaign control1. Oracle, diffuse and additional backward refits OFF,
  matching the dedicated Higgs launcher's steering. No FSR-fit changes.
- LocationScanSetups=[] explicitly frozen. New pre_ecal cluster snapshot uses
  the tested installed BP module from25dc2b2; no build/install in this task.
- 4000MB/job, group cms. Existing helper supplies no explicit short-walltime
  request. No new scheduler/resource options or helper implementation changes.

All500 cards parse and match manifest hashes; inputs/seeds/counts and controls
pass. A helper submission dry run emits exactly500 commands and creates no
submitted/started markers. `submit.sh` passes shell syntax checking and changes
into the correct workspace before invoking the helper. The installed BP module
SHA256 matches the earlier native four-event writer regression; see
[the snapshot record](2026-11-11-bp-pre-ecal-tuple.md). Scans-off defaults:
[the steering record](2026-11-11-bp-location-scan-default-off.md).

Campaign directory contains README, submit.sh, campaign-summary.json,
prepare-settings.json, input-event-inventory.json, preparation-validation.json,
frozen cards/manifests and preparation/submission-dry-run logs. All are
uncommitted batch artifacts. Source REC and other campaigns are untouched.
The initial slow Python metadata reader was stopped and replaced with native
ROOT metadata access; the complete accepted inventory is the native result.

Submission command, from any directory:

```bash
bash /aifs/user/data/zhangcg/gsfdev/CEPCSW/higgs_eeHX_ISRFSRON_BP_NOSCAN_PREECAL_500jobs_200evt_20261111/submit.sh
```
