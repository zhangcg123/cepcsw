# BP flat cleanup and H0_F/theta25 rerun preparation — 2026-11-13

User explicitly requested deletion of all BP flat tuples and rerunning from REC
with the new window and H0_F included. Deleted 1995 production/targeted
flat ROOT files, 383.948 GiB apparent size, from four superseded campaigns:

- BP_NOSCAN_PREECAL_500jobs_200evt_20261111: 500.
- BP_FSRFLOAT_NOSCAN_500jobs_200evt_20261112: 500.
- BP_H0_FSRFLOAT_NOSCAN_500jobs_200evt_20261113: 500.
- BP110_FSRFLOAT_9458evt_20261112/products: 495, named higgs-N.root.

The three full campaigns each had all500 completion markers before deletion.
All495 targeted products have finished native-run records (exit0),326 with
post-validation completion;169 retain old scientific-QA failures. The first
name-based inventory missed these differently named targeted files; a recursive
BP-campaign audit caught them and they were included before completion. Only
flat products were removed: old cards, manifests, logs, fit payloads, plots and
cached analysis data remain. No whole campaign directory was removed.
Before/after identities of all500 retained REC_ECALTRUTH files and all500 GSF
flat files agree exactly in size/mtime. All1995 deleted paths are confirmed
absent. Existing plot images stay available but native rereads of deleted BP
parents now fail; do not claim they have been regenerated with the new settings.

## Replacement campaign

`higgs_eeHX_ISRFSRON_BP_H0F_THETA25_NOSCAN_500jobs_200evt_20261113/`
contains500 frozen jobs x200 events =100000 inputs from the SAME retained
REC_ECALTRUTH500x200_20261112. STAGES=breakpoint only; no upstream rerun/cleanup.
Theta HALF-window25mrad (50mrad total), phi HALF-window200mrad. Only the selected
production electron pair is refit. Truth-selected material location/null,
NO location scans, free-loss/beam/charged ECAL/FSR ON, ElectronPhoton C+N,
separate111/diffuse/truth override/backward filters OFF. Other controls match
the previous full H0 campaign. Memory4000MB, cms group, scheduler-default
walltime with no explicit short cap. No automatic hypothesis selector.

All four hypotheses are available: H0=`null_110_*`; independent no-material
H0_F (also named H_F)=`fsr_only_110_*`; material H_beta=`charged_ecal_free_loss_*`;
material+FSR H_betaF=`charged_ecal_free_loss_fsr_*`. H0 always runs, H0_F is
booked/attempted by the existing FSR switch. Installed plugin exactly matches
the previously native-validated H_F version SHA256
`ba82a23202fa11e75928a8e245a5396b03e8678ae1e5734b45831a6d260d2bcd`.
No source, fitter, run-card template, shared library or install changed here.

All500 card syntax/hashes/controls and prepared-only marker states pass.
The prior ROOT-verified input inventory is copied, all500 source paths,
sizes/mtimes and REC completion markers match. No redundant full ROOT scan.
Submission dry run emits exactly500 cms/4000MB commands, no walltime override.
Zero new submitted/started markers or outputs. Actual submission is unavailable
in this session: hep_q/system Python raise ModuleNotFoundError: htcondor.
No queue-wide idle claim is made; finished-file deletion uses per-run records.

Submit from the normal batch-login shell (any directory):

```bash
bash /aifs/user/data/zhangcg/gsfdev/CEPCSW/higgs_eeHX_ISRFSRON_BP_H0F_THETA25_NOSCAN_500jobs_200evt_20261113/submit.sh
```

Artifacts are uncommitted in the campaign and
`TrackingPerformanceStudies/bp_theta25_h0f_batch_20261113/`: preparation and
submission logs, frozen controls/plugin hash, all file/retained identity
inventories, deleted path lists, final deletion/submission checks. Runtime uses
project-default installation as before, not a new private snapshot. Native
performance is not tested for the new window; no completion/physics claim.
Complete outgoing AGENTS focus is preserved in
`2026-11-13-agents-before-bp-h0f-theta25-production.md`.
