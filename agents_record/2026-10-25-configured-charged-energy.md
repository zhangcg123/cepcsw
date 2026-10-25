# Configurable charged ECAL source and electron/photon sum — 2026-10-25

## Request and installed controls

User requested TrkMergedECAL as a configurable charged-constraint input and a
sub-option choosing electron-only versus electron+photon energy. After local
validation the user explicitly approved installation, confirming no breakpoint
jobs were active and acknowledging deployment of the previously tested passive
photon projection fields in the same build.

| Property | Compiled default | Maintained card |
|---|---|---|
| ChargedEcalMeasurement | false | BP_CHARGED_ECAL_MEASUREMENT, fallback 0 |
| ChargedEcalInputClusters | EcalCluster | BP_CHARGED_ECAL_INPUT_CLUSTERS, fallback TrkMergedECAL |
| ChargedEcalEnergyMode | ElectronOnly | BP_CHARGED_ECAL_ENERGY_MODE, fallback ElectronOnly |

Old frozen cards omit the two new properties and retain their final-PFO,
electron-only behavior. New maintained cards explicitly select TrkMergedECAL.
To enable the sum use ChargedEcalEnergyMode="ElectronPhoton" and disable
NeutralEcalMeasurement; ElossOn must remain false. BeamSpotConstraint remains
independent and cumulative. These are sub-options of the existing charged
families, not new endpoint families. Original rts_* and free_loss_rts_* remain
unconstrained. Diffuse, truth-centered and backward workflows are unchanged.

The card adds the configured input to PodioInput only when charged measurement
is enabled. The batch preparer freezes both BP_* sub-options. Invalid energy
mode, empty enabled source, or sum+neutral steering is rejected before submission;
the algorithm independently rejects invalid mode and sum+Eloss/neutral at init.

## Observation and source selection

Shared ChargedEnergyMeasurement.h uses six-dimensional state
(drho, phi0, kappa, dz, tanl, b), with b=log(p_before/p_after), electron mass
neglected. At the propagated ECAL face:

```text
p = sqrt(1+tanl^2)/abs(kappa)
ElectronOnly:   h = p
ElectronPhoton: h = p*exp(b)
H = (0, 0, -h/kappa, 0, h*tanl/(1+tanl^2), h)  [sum mode]
```

Electron-only sets the last derivative to zero. The existing native KalTest
scalar update, full 6x6 covariance/cross-terms, native tracker-to-ECAL F/Q,
terminal RTS edge and normalized likelihood code are reused. Ordinary and
every free-loss trial use the same projection; no extra objective penalty,
photon fit parameter, spatial measurement or second filter is introduced.

The sum model assumes one loss and no other energy change to ECAL. A selected
cluster must physically contain the electron and modeled photon; the code does
not infer containment or automatically add separately resolved photon clusters.
The separate neutral-energy constraint is prohibited to avoid observing the
same modeled photon twice in this initial configuration. Coefficients and
energy-error formula remain a*sqrt(E)+c*E (GeV convention), independent variances
summed; a=.011, c=.004 by default.

No new PFA modification, track-link export or rec rerun was needed. Shared
ChargedEnergyCandidate selects exactly one final charged PFO linked to the
current input track. For EcalCluster it preserves the previous selection. For
an alternate input, every constituent hit key (display cell ID and copied
float32 x,y,z) of each final charged shower must be contained in exactly one
candidate. Ignore recalibrated hit energies and collection-local hit indices;
the source can have extra constituents absent from the final cached hit list.
Missing/ambiguous matches leave the constraint unapplied, preserving the
preceding accepted stage. No nearest-cluster fallback or whole-event energy
sum is used. Deduplicate source indices before summing energies. Other real
neutral clusters in TrkMergedECAL are not automatically selected as charged.

New per-track fields: charged_ecal_input_clusters, charged_ecal_energy_mode,
charged_ecal_source_cluster_indices, charged_ecal_selection_error. Existing
free_loss_charged_ecal_* fields retain the shared observation/status contract:
0 off, 1 unavailable, 2 selected, 3 ambiguous, -1 selection error. Selection is
not fit application; inspect the cumulative family constraint_status/mask.

## Validation

All 11 focused RecBreakpoint CTests pass, including new analytic/numerical
charged-energy gradients, native scalar KF gain, calibrated-hit-independent
source lookup, duplicates, unavailable and ambiguous source/PFO cases.
The original nine helper tests also remain passing. Existing build clock-skew
warnings remain; no dependency or shared KF implementation was changed.

Local fixtures/logs: /tmp/bp-charged-shower.bWh8yU. Input is the new
rec_large_barrel_20261025/rec-barrel-1.root. Unchanged frozen reference card:
bp_flat_barrel_20261024/runcards/barrel-1/breakpoint.py. The wrapper overrides
only input/output, selected entries and verbose dumps before initialization.

- Old installed versus new compatibility code: selected entries
  0,1,3,4,8,10,11,16,17,18; all 817 prior flat branches match exactly on 10 rows.
- Maintained card with merged source, both ElectronOnly and ElectronPhoton:
  all 10 rows select a source; the two eligible-breakpoint rows (3,16) apply
  both ordinary/free constraints; eight ineligible rows preserve prior results.
- Extra sum-mode entries 5,7,13,25,31: six rows because entry 13 contains two
  reconstructed tracks; all six source selections succeed, five eligible rows
  apply ordinary/free constraints, the other row copies the previous stage.
  Entry 13 is a split-track/topology control, not a single-track performance
  representative. No population improvement is claimed from these tests.
- Invalid ElectronPhoton + neutral configuration is rejected at algorithm
  initialization; the same invalid batch steering fails before card creation.
- DRY_RUN one-job preparation in /tmp/bp-charged-shower-batch.HipE6Q freezes
  source=TrkMergedECAL, mode=ElectronPhoton, neutral=0. No job submitted.
- Fresh default-runtime sum-mode smoke repeats the 10 selected entries and
  compares all 851 branches exactly against the isolated build.
- Fresh default-runtime beam+charged sum smoke on entries 5,7 applies both
  ordinary/free charged stages with masks 3 (beam+charged), verifying accumulation.

## Scoped installation and remaining limits

Staged only RecBreakpoint, then atomically replaced these two default-runtime
files, with backups under /tmp/bp-charged-shower.bWh8yU/previous:

```text
python/RecBreakpoint/RecBreakpointConf.py
lib/libRecBreakpoint.so
```

The staged RecBreakpoint.confdb differs only in its generated timestamp and
was left unchanged; component identities are unchanged. No full-tree install.

SHA256 libRecBreakpoint.so:

- prior: 6704f6c2b441b4556db1f110a35ee7f4baf1921338f25b80f2cef6066db62af7
- installed: 2d868baf1b3f2280d43a727ad2e7839608d17d5a1ac97b3bbb0bd73659d34a69

Installed PFA remains unchanged at
686747f7273dcc1c07d10be98ca121f43875edf2aef8858f81bf794c7ae78254.
The passive rts_photon_calo_* / free_loss_rts_photon_calo_* projection fields
are now installed too, by explicit user approval. Earlier NOT INSTALLED records
remain historically correct but no longer describe the current runtime.

No batch submission, tuple deletion or gallery change occurred. No source/PFA
track-link changes were made, and no GSF/shared KF sources were changed. Do not
interpret execution or smaller objective as physics validation. Further checks
must separate truly unresolved showers, independently resolved photons,
containment/calibration effects and correctly selected breakpoints.
Full outgoing AGENTS.md is preserved in
2026-10-25-agents-before-configured-charged-energy.md; global laws unchanged.

Follow-up: paired tests on ten additional clean >1%-loss, event-wide no-real-
neutral tracks are recorded in
[the no-neutral check](2026-10-25-charged-sum-no-neutral-check.md).
