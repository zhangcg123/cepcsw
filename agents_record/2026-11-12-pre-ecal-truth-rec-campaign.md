# Pre-reclustering ECAL truth and REC campaign, 2026-11-12

## Authorization and implementation

User requested rerunning500 jobs without removing raw digitized ECAL banks,
after the proposal to persist missing pre-cluster MC maps. Scope is our passive
RecPFACyber snapshot exporter, output registration, callers and focused test.
No original PFA, tracking or simulation algorithm changed. No shared library
was installed or reference-build output overwritten.

The new automatic `EcalClusterPreReclusteringParticleAssociation` copies every
native `getLinkedMCP()` entry with direct cluster/MC ObjectIDs and its original
weight. Shared ECAL clusters export once. No threshold, new matching or
renormalization; both charged and neutral contributions survive. Current
EDM uses MCRecoClusterParticleAssociation; >=1 compatibility uses
ClusterMCParticleLink but that version was not built here. HCAL truth is not
added. Existing raw hit contribution banks remain independent inputs.

## Prepared production

Directory: `higgs_eeHX_ISRFSRON_REC_ECALTRUTH_500jobs_200evt_20261112/`.
500 frozen REC-only cards, seeds1..500,200events each =100000events.
Inputs: corresponding current
`higgs_eeHX_ISRFSRON_REC_TPCFIX_500jobs_200evt_20261110/rec-higgs-N.root`.
All500 headers were checked: each contains200events and raw digitized banks.
Only reader -> existing PFA/PID -> output executes. Manifests reuse the
calodigi-input slot for external REC. No stdhep/simulation/digi/tracking or
GSF/BP rerun. Original stdhep path is provenance used by the existing verifier.
Cleanup retains external sources; source size/mtime checked before/after.

Keep ECALBarrel, ECALEndcaps, ECALBarrelParticleAssoCol,
ECALEndcapsParticleAssoCol, MCParticle, existing tracker/G4 truth and snapshots.
The prior12 simulated-calorimeter-hit / hit-to-simhit drop rules remain.
Outputs use new filenames; no existing REC deletion or overwrite requested.
Cards otherwise preserve previous seeds and PFA/PID steering. Memory4000MB,
walltime short. Native18-event RSS was about3.0GB, elapsed2m33s.

Runtime is campaign-local libCrystalCaloRec.so, SHA256
00929285cba2e516494f3f92922bcd7236837b391b9de774c85ad9976ddd7296;
two unchanged support libraries are copied beside it. Local build recompiles
only exporter and CyberPFAlg objects and links the unchanged remaining objects.
The frozen worker/helper/library hashes and card hashes are checked. Output
verification requires200events plus raw4banks and new association, and writes
`ecal_truth_verified.json`. Failure suppresses generic completion marker.
`ready.json` is backed by the qualified native gate below. All500cards, frozen
hashes and script syntax verified. No submission performed.

On the usual batch login host, from repository root:

```bash
bash higgs_eeHX_ISRFSRON_REC_ECALTRUTH_500jobs_200evt_20261112/submit.sh
```

The submitter requests4000MB/short, records confirmed clusterIDs, skips recorded
submissions and stops on any unconfirmed submission. This host lacks working
htcondor Python imports; actual scheduler acceptance has not been tested.
Do not delete the campaign libraries while its jobs are active.

## Verification and limits

Artifacts: `TrackingPerformanceStudies/pre_ecal_truth_output_20261112/`.
Snapshot ownership and new truth fixture pass (`unit.log`). Old/new native runs
process seed1 events0..17 including11,16,17. All4149 saved associations have
valid unique cluster/MC links, positive finite weights, near-unit cluster sums,
and match the native debug map. Raw digitized banks, MC and tracking contents
match. All existing pre/final cluster energies, positions and hit memberships
match after order-independent comparison. Pre-ECAL charged/neutral/track
ownership matches. Final CyberPFO and CyberPFOPID physical contents match.

Raw PFA arrays reorder. Event2 has a pre-PFO HCAL ownership difference between
two low-energy neutral ECAL clusters; ECAL inputs and final PFO/PID physics
remain equal. Independent OLD/OLD replay differs at event8 in pre-ECAL and
PFO output. Thus original PFA replay variation persists; no claim of universal
bitwise or event-by-event PFA reproduction. No old PFA code altered to fix it.
`validation.json` and `control.json` preserve comparisons and qualified pass.
No GSF/BP fits,500-job population test or physics validation performed here.

## Memory update

Outgoing complete AGENTS is preserved in
`2026-11-12-agents-before-ecal-truth-rec.md`; only Current focus is replaced,
and the explicit passive-export scope exception is added. Other laws remain.
No history directory was moved; no manifest migration required.
