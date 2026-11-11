# Fresh photon/ECAL study from current REC: user reduced to1000events

Status: RUNNING, no fresh aggregate purity result yet.

The user requested fresh analyses on5000current REC events, questioned why
PFA was needed, then explicitly reduced the sample to1000. Keep five already
running, preselected REC seeds1,22,43,63,84, all200events each. This choice
precedes any fresh truth/purity result. Other initially started sources
105/126/147 are stopped and excluded; the remaining original25-source plan
must not run. The original plan is preserved, not used as an analysis input.

Current working directory:
TrackingPerformanceStudies/photon_ecal_fresh_5k_20261111/.
Its name reflects the initial request; do not rename while native writers
are active. input_manifest.json has five sources and input_events=1000;
input_manifest_planned_5k.json preserves the initial plan. Source REC and BP
paths, sizes/mtimes, current REC-card hashes and code/library hashes are saved.
All selected REC files and BP side trees have200entries. Current REC parents
match current GSF production manifests. No fit-success cut or recoil cut.

## Why a PFA-only replay

Current REC has ECALBarrelParticleAssoCol/ECALEndcapsParticleAssoCol links
from raw digitized hits to MCParticle, pre-reclustering clusters and copied
3D shower hits. OutputCreator::CreatePFOSnapshot does not persist the cluster
MC weight map. Its copied ECAL points carry display IDs(module<<5)+layer,
not source raw-bar object links. Thus no direct exact cluster->MC map can be
read from those outputs. Reuse native PFA logic to rebuild its internal map
from the saved inputs and export it with WriteAna. No geometric truth match.

Native chain: PodioInput -> CyberPFAlg stopping after
TrackClusterConnectingAlg -> PodioOutput. No simulation, digitization,
tracking or BP/GSF fit. Original cards, reconstruction sources and installed
libraries remain unchanged. A first environment-only attempt failed before
ROOT outputs were produced; retain failed_environment_run.log. Corrected
runs source setup.sh and set PYTHONNOUSERSITE=1; ROOT6.30/02 is the native
environment. Offline readers remain separate single-threaded uproot readers.

## Process controls at the reduction

Separate pilot controller3447526 runs seed1, native child3447571, and will
validate/extract200events. Former queued controller3459026 was SIGSTOPped
to prevent launching later planned sources. Its retained native children:
seed22 PID3459215, seed43 PID3459245, seed63 PID3459172, seed84 PID3459151.
Its excess children seed105 PID3459174, seed126 PID3459207, seed147 PID3459150
received SIGTERM. Verify command paths and PHOTON_STUDY_JOB before any process
operation; PIDs may expire/reuse. reduction_to_1k.json stores checked identities.

Keep the stopped queue controller until its four retained native children
finish, so its sandbox/session does not kill those writers. Then terminate
that stopped controller, verify no unselected replay survives, and run run.py
on the five-source manifest. It skips completed per-source reports and may
reuse ONLY this study's fresh native outputs after checking native successful
termination and Processed200events. It will independently validate/extract
the four finished native outputs whose original controller was stopped.
Never resume the former25-source controller. Do not start duplicate writers.

## Verification and pending completion

For every fresh native event, compare the current REC's complete ordered MC
lists across13fields: PDG, generator status, momentum xyz, G4endpoint xyz,
vertex xyz, mass and charge at debug float32 precision. Hash complete events
and require1000unique contents. For cluster attribution additionally require
exact cluster energies/positions, all constituent-hit multisets, and PFO
charge/track ownership. Failures remain unclassified, excluded from purity.
Never silently replace them with replayed objects. Every angular10/200mrad
decision must equal saved native collection indices after exact remapping.

Reuse generator FSR IDs and exact G4-material eBrem birth/parent identities;
follow saved descendants. Root IDs, photon keys, cluster associations and
energy-category closure are asserted. Compare10/50mrad theta at phi200mrad,
without a new energy cut. Deduplicate each neutral cluster across electron
windows per event. Energy partitions use reconstructed energy times normalized
PFA MC weights. Distinguish FSR-only purity, tagged eBrem, other selected-
electron descendants and other origins; associated energy can double count
GSF material recovery. Photon location plots separately use birthE>=1GeV
and photon-plus-descendant dominance>50% of cluster MC-labelled content.

After aggregation run render.py; it reuses calculation/plot CODE only, with
all data inputs in this fresh study. Figures include current theta10 and
proposed theta50 boxes, phi200, plus a full view and window zoom. Keep current
production window unchanged. Preserve/hash-check all six recoil PNGs/six PDFs.
Archive the previous two photon PNG/PDFs plus gallery metadata before update.
Delegate gallery organization/verification to one plot-gallery sub-agent.
Current localhost8000 page still shows the earlier1k sample, not fresh results.

Global AGENTS introduction/laws/compile section is preserved byte-for-byte;
the complete outgoing focus is snapshotted in the companion before-fresh record.
Analysis code, native files, CSVs, logs and gallery manifests remain uncommitted.
