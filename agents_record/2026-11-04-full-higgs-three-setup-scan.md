# Full Higgs REC scan preparation: 000/100/110

## Authority and scope

The user selected Higgs only, all available REC files/events, location scans
only, setups000/100/110 only. They confirmed no active jobs and authorized
installation after validation, including the pending ElectronOnly fix.
Prepare only; no submission authorized. Remain on breakpoint-delayed-neutral;
no remote/branch, PFA/shared-KF, truth-ownership, loss-map, optimizer bound or
acceptance change. Do not overwrite the removed campaign's retained cards/logs.

## Small implementation change

HitScanAll now follows existing NeutralEcalMeasurement: OFF enumerates
000/100/110, ON retains000/100/110/111. Beam, charged and FreeLossFit must
remain ON; TruthOverride OFF. No new property. A pure setup-list helper is
unit-tested, and initialization, caller, README and maintained card agree.
Never assume four entries per point: use saved point/setup labels.

Passive neutral collection is NOT disabled:110 ElectronPhoton uses C+N when
available, otherwise C; ElectronOnly always uses C for electron only. This
includes the pending75b3ba0 correction.111 stays separate when enabled.
No-breakpoint controls remain electron-only. Existing fallback/source/mask
flags, full IP state recording and original-seed independent fits are intact.
No online ranking/selection or NLL-gain cut. Bound[0,1] unchanged.

## Input inventory and frozen campaign

All249 REC trees in higgs_full_20261030 were opened read-only. Each contains
200 events and all required tracking, G4 provenance and pre-reclustering
PFO/cluster/hit collections. No bad file. Total49,800 input events (not a
claim of49,800 selected electron tracks). IDs1..250 except absent123.

Output: higgs_location_scan_20261104.249 jobs x200 events, STAGES=breakpoint,
group cms, memory5000 MB. Input REC files retained; no sim/trk/calodigi/rec rerun.
All manifests, input paths, event counts, controls, card syntax and SHA256
checksums audited; no ROOT output, submitted/started/completed markers.
Inventory copied to the output directory as an uncommitted campaign artifact.

Controls: HitScanAll, FreeLossFit1, beam/charged1, neutral0, ElectronPhoton,
PreReclustering, TruthOverride0, diffuse0, all backward0, Selected empty,
mean0, sigma0.001, MaxLogLoss1, MaxCalls180, tolerance0.001. Delayed neutral1
is frozen but inactive with111 off. Null, VXD/ITK, first exact TPC row0 and
eligible passive truth-owner reference are included; keep all measurements.
Truth never changes reconstructed candidates or fit loss amounts.

Commands from the repository root:

```bash
./subbreakpointhiggsscanjobs.sh                  # prepare only (already done)
./subbreakpointhiggsscanjobs.sh submit-prepared  # submit frozen cards on request
```

No jobs were submitted by this work.200-event full scans are substantially
heavier per job than the previous50-event campaign. No timing guarantee or
population physics performance claim follows from focused tests.

## Validation and installation

Evidence directory: /tmp/bp-higgs-full-prep-Rt3s8V.
15 CTests passed (6.16s),59 Python batch-helper tests passed (3.818s).
The launcher fixture checks249 x200 and verifies preparation does not submit.
Two verbose local scans on barrel seed1 event5, ElectronPhoton and ElectronOnly,
finish successfully:10 points,30 entries each, no111. All34 scan fields match
exactly the corresponding000/100/110 entries from the prior40-entry scans,
including NLL/pT/status/source/full IP arrays. Photon applies resolved sum at
nine sites; Only at zero despite usable neutrals. Main endpoints stay inactive.
Previous both-mode normal/scan evidence including events11/16/17 is preserved
in2026-11-04-honor-electron-only.md; it is not a new population claim.

Installed only RecBreakpoint through the105 package cmake_install.cmake;
no full-project/PFA/shared-KF installation. Old module backed up in the evidence
directory. Project-default and staged libRecBreakpoint.so have identical SHA256:
3f35d34f46cbb39fec1edf119b127e31a77d83b21eaf3e4931b6ff1ae3eb4dbc.
Workers use project setup.sh, not a temporary runtime. A default-runtime probe
uses the actual frozen Higgs card with EvtMax0 and an output override under
/tmp, without altering that card or creating its campaign output. The probe
passed initialization, finalization and the full-IP schema checks; its tree
contains zero rows as requested. No temporary library override was used.

## Knowledge migration

Complete pre-edit AGENTS is preserved in
2026-11-04-agents-before-full-higgs-scan.md. Introduction, project laws and
essential commands are byte-for-byte retained. Only Current focus is replaced;
its complete old contents, pending installation wording and historical evidence
remain in that snapshot. Current focus now records this deployment and campaign.
No history directory was moved, so no history-move manifest was required.
The original200 scan ROOTs remain deleted by user request; old numerical plots
refer to that old sample, not the new unsubmitted batch. The acceptance audit
remains open; no acceptance repair or physics claim was added.
