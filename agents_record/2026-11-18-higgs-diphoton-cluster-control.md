# Higgs diphoton cluster-control preparation — 2026-11-18

The user requests approximately 5,000 Higgs-to-diphoton events to validate
cluster performance after questioning the BP HCAL factor-four response.

Selected a clean 240GeV e+e- -> mu+mu-H, H -> gamma gamma control from
`/cefs/higgs/zhangkl/stdhep/E240/higgs/E240.Pe2e2h_aa.e0.p0.whizard195/`.
Here `aa` is the photon channel; `gg` is a separate gluon channel. The first
200 events in files0001–0025 give 5,000 events. A direct generator-only LCIO
`UTIL::LCStdHepRdrNew` check reads all 5,000 and verifies two stable Higgs-origin
photons and at least two stable muons in every event. The first checker attempt
used the unavailable `lcio` alias without its umbrella header and failed at
Cling compilation; the corrected UTIL-qualified reader passes. It did not
simulate any events or modify the generator inputs.

Prepared `higgs_mumuHgg_CLUSTER_25jobs_200evt_20261118/`: 25jobs x200events,
4,000MB, mid walltime, cms. Existing batch helper and unchanged DumpGsfTrks
templates freeze sim,trk,calodigi,rec into one in-memory event loop, retaining
REC only. No BP/GSF fits. All25 manifests, input paths, stage lists, syntax,
frozen-card hashes and prepared-submission dry-run checks pass.

A separate two-event full-chain pilot completes successfully at
`TrackingPerformanceStudies/higgs_diphoton_clusters_20261118/smoke/`, writing
`rec-higgs-1.root` and its verified completed.json marker. This demonstrates
execution/output integrity, not cluster-response validation. The installed
native code, original tracking/PFA, default cards and other running jobs remain
unchanged. There was no build or installation.

The local hep_q/hep_sub client cannot import htcondor. SSH fallback to
lxlogin.ihep.ac.cn fails hostname resolution. **The 25 production jobs have not
been submitted**, and no 5,000-event reconstruction/physics result exists yet.
Do not substitute the two-event pilot for population validation.

Submit from the usual working batch-login shell:

```bash
bash /aifs/user/data/zhangcg/gsfdev/CEPCSW/higgs_mumuHgg_CLUSTER_25jobs_200evt_20261118/submit.sh
```

Generated campaign README, generator-input-check.json, submission-dry-run.log,
frozen cards, analysis scripts and pilot ROOT/log files remain uncommitted.
Planned comparisons use common truth-Higgs-photon pairs for ECAL-only,
ECAL+HCAL and saved PFO energy response and diphoton mass; categorize barrel/
endcap and conversions, and count unmatched photons explicitly. Native ECAL
cluster weights retain the inherited-map limitation documented in AGENTS.
This clean photon control does not by itself validate electron eBrem recovery.
