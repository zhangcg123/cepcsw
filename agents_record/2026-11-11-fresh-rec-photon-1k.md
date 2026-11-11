# Fresh photon/ECAL study from current REC: user reduced to1000events

Status: COMPLETE. Canonical study: TrackingPerformanceStudies/photon_ecal_fresh_1k_20261111/.

The original running-state controls below are retained as history. All five
retained native writers finished200events; excess jobs and the stopped queue
controller have been terminated. No photon-study PFA process remains.

The user requested fresh analyses on5000current REC events, questioned why
PFA was needed, then explicitly reduced the sample to1000. Keep five already
running, preselected REC seeds1,22,43,63,84, all200events each. This choice
precedes any fresh truth/purity result. Other initially started sources
105/126/147 are stopped and excluded; the remaining original25-source plan
must not run. The original plan is preserved, not used as an analysis input.

Original working directory (now renamed after every native writer finished):
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

## Process controls at the reduction (historical, no active controls)

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

## Completed fresh results

Canonical directory is now
TrackingPerformanceStudies/photon_ecal_fresh_1k_20261111/.
All1000retained events finished native PFA successfully; no study PFA process
remains. The stopped25-source controller was killed only after its four
retained writers finished, then canceled-source partial ROOT files were
removed. Their logs and original plan remain as provenance. No source REC,
existing tuple, production job, installed library or original card was changed.

All1000full ordered MC events match current REC at debug precision across
13fields, and all1000event fingerprints are unique. Independent checks also
match all1895raw FSR payload roots/owners/momenta against CURRENT REC
GsfGeneratorFsrLinks and MCParticle; all have generator-origin status1.
Those raw records precede selected-production-track ownership cuts.

| REC seed | Input events | Selected tracks | Exact cluster events | Excluded |
| --- | ---: | ---: | ---: | ---: |
|1|200|399|187|13|
|22|200|398|188|12|
|43|200|399|184|16|
|63|200|393|184|16|
|84|200|399|186|14|
|Total|1000|1988|929|71|

No fit-success, recoil, FSR/eBrem or purity cut chose these sources. The
cluster gate compares energy, position, complete hit multisets and PFO
charge/track ownership. The71failed events are not relabelled using the
replay's changed clusters. Photon rows retain unresolved status; purity uses
only929exactly matched events.

### Truth-photon population and neutral association

| Birth energy | Type | Tagged | Any neutral | Dominated neutral | Dominated in theta10 |
| --- | --- | ---: | ---: | ---: | ---: |
|All|FSR|1837|448|219|74|
|All|eBrem|3710|843|143|119|
|>=10MeV|FSR|913|408|219|74|
|>=10MeV|eBrem|2866|843|143|119|
|>=1GeV|FSR|409|291|194|73|
|>=1GeV|eBrem|1153|450|137|119|

Any own-charged contribution:790FSR/2423eBrem at all energies. Status totals:
FSR1711resolved/126unresolved replay; eBrem2587resolved/779missing exact MC
roots/344unresolved replay. At>=1GeV: FSR384resolved/25unresolved; eBrem
1047resolved/92unresolved/14missingMC. These unresolved rows are not counted
as confirmed absence. Photon-neutral counts are lower bounds, not overall
photon detection efficiencies.

For plotted birthE>=1GeV and>50%photon-plus-descendant dominance:

| Type | Photons with dominated neutrals | theta10 accepted | theta50 accepted |
| --- | ---: | ---: | ---: |
|FSR|194|73|107|
|eBrem|137|119|123|

Phi remains200mrad in both. FSR acceptance in this conditional population
is37.63%->55.15%,34additional photons(+46.58%); eBrem86.86%->89.78%.
There are219FSR/164eBrem cluster associations in the plots,75/135inside
the current window; one photon can create several points. Full view includes
all points. Zoom is |deltaPhi|<=300 and|deltaTheta|<=75mrad, with85FSR and
14eBrem points beyond the displayed view. Black dashed/current theta10 and
gray dotted/proposed theta50 boxes share phi200. Blue/orange point colors
refer to the CURRENT10mrad decision. No production-window change.

### All-energy neutral-window purity

This denominator includes ALL usable neutral clusters in the selected
windows, not only photon-dominated objects or>=1GeV photons. Union each
cluster once per event over both selected electrons. No extra energy cut.

| Quantity | theta10 | theta50 | Added10--50 |
| --- | ---: | ---: | ---: |
|Unique neutral clusters|433|561|128|
|Events with neutrals|328|378|99|
|Summed cluster E[GeV]|6009.712|6430.695|420.983|
|FSR energy fraction|18.876%|21.385%|57.204%|
|Tagged eBrem fraction|38.327%|36.372%|8.451%|
|Other selected-electron descendants|41.802%|40.186%|17.113%|
|Other-origin contamination|0.994%|2.057%|17.231%|
|Unknown fraction|0|0|0|
|Other-origin dominated clusters|19|62|43|

Other-origin dominated fractions by COUNT are4.39%,11.05%and33.59%, distinct
from energy fractions. The added128clusters include40FSR-dominated,
13eBrem-dominated,25other-electron-dominated,43other-origin-dominated and
7mixed clusters. Energy proxies from other origins are59.752/132.293/
72.541GeV; FSR proxies1134.413/1375.234/240.821GeV. FSR energy rises21.23%.
These are reconstructed cluster energy times normalized native PFA MC weights,
not exact deposited-energy fractions. Other selected-electron descendants
include shower content without an individually retained brems root; broad
electron-origin purity must not be described as FSR purity. Associated
material/shower energy can double count recovery already present in GSF.
This conditional929-event estimate has no held-out detector/recovery
validation, and no recoil spectrum was recalculated.

### Final verification and artifacts

Every native window decision agrees with saved BP collection indices after
exact current/replayed cluster remapping. All photon keys and accepted MC
roots are unique. Unclassified photon flags are not interpreted as observed
absence. The10mrad union is a subset of50mrad; cluster counts, total energy
and every ancestry-category energy close between original+added and widened.
Location-plot photon counts agree with purity photon acceptance for both
windows/types. Native library hashes remain unchanged. Rendered files match
their recorded hashes; both images are visually inspected and renderer
assertions exclude legend/x-axis-title overlap. Analysis uses fresh data only;
older code helpers supply constants/calculations, not cached records.

Artifacts: input_manifest.json, archived input_manifest_planned_5k.json,
runtime_libraries.json, analysis_code_hashes.json, truth_input_identity.json,
fsr_origin_status_counts.json, per-source fresh native ROOT files/validation/
fingerprint CSVs, survey/{summary.json,photons.csv,neutral_cluster_locations.csv,
window_purity_candidates.csv,location_window_summary.json,
window_purity_summary.json}, staged plots/ and gallery_before_fresh_1k.json.
The full study occupies about0.565GiB; no REC file was copied.

### Verified gallery publication

The dedicated plot-gallery agent updated the existing third section after
archiving the two old photon PNGs/two PDFs plus gallery.json/README.md
(six files) to TrackingPerformanceStudies/.analysis_archive/
photon_ecal_before_fresh_1k_20261111/. All six archived hashes match the
pre-update manifest. All six recoil PNGs/six PDFs remain byte-identical;
all four served fresh photon files match the staged final summary hashes.
The gallery validates eight PNGs across three sections, each matched once.
Homepage and both fresh PNG URLs return HTTP200, with PNG bytes verified.
Existing managed server remains healthy on ailogin002.ihep.ac.cn,
localhost8000, PID2122174; no restart was needed. Main-agent review confirms
the manifest, preserved figures, archive and final plot hashes.

## Proposed compact REC cluster truth persistence (2026-11-12, not implemented)

The user asked what the truth links contain, how trustworthy/heavy they are,
and whether the next generation can persist them directly. Read-only code
review confirms digitization assigns hit MC weights from Geant4 contribution
energies; PFA propagates energy-weighted maps through bars/half-clusters and
recomputes the merged map in TrackClusterConnectingAlg before pre-reclustering
export. This map exists in ordinary PFA without enabling full WriteAna.

Measured on all1000fresh native events:42,173pre-ECAL clusters and245,786
cluster-MC map entries, mean42.173clusters/245.786links per event. All weights
are finite and positive; nonempty maps sum to unity within5.88e-6. Median/
95th-percentile/max links per event:243/426/3270. These are consistency checks,
not independent physical validation of every shower split or exact digitized
energy contribution. Native MC retention/ancestry limits still apply.

The installed EDM4hep already supplies
MCRecoClusterParticleAssociationCollection: one float weight and references
to Cluster/MCParticle. Proposed collection name:
EcalClusterPreReclusteringParticleAssociation. Save the complete original
native map for every charged AND neutral pre-cluster in our passive snapshot
exporter. Do not save only a leading MC or selected photon/electron subset;
purity needs the other contributors. Do not duplicate MC momentum/PDG/endpoints
or enable the complete debug ntuple. Preserve raw native weights; downstream
ancestry classifies generator FSR/material eBrem. Direct ObjectID relations
remove debug-fingerprint matching and the replay-object mismatch requirement
for new files, but do not restore missing MC photon roots.

Two8byte ObjectIDs plus a4byte weight give20bytes/link raw payload, about
4.916kB/event or491.6MB/100,000events before compression/I/O overhead. This
payload is0.332%of the current compressed REC bytes/event(1.483MB). The full
PFA debug output was433.3MB/1000events and is not the proposed output.
Actual added compressed PODIO size and serialization time are unmeasured.
Before next production, verify a passive exporter leaves original clusters,
hits, PFOs and tracking outputs unchanged and reproduces native maps/purity
with direct IDs. No production implementation or library/card change has
been made for this proposal. Artifact: cluster_truth_storage_estimate.json.
