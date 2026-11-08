# Compact electron/photon truth and generator FSR

## Authority and scope

The user requested a smaller electron/photon truth recorder, proof that the
supported old outputs can be reproduced, and FSR photon truth in the existing
eBrem photon format, identified from physics-generator information. The user
explicitly excluded a many-to-many hit-association study and confirmed that no
jobs were active before implementation/build/install. The scheduler query was
unavailable locally and SSH failed host-key verification; the user confirmation
resolved the idle check. Subsequent verification jobs belong to this task.

Work stays in the reference workspace on `breakpoint-delayed-neutral`. No branch,
remote, Hive migration, PFA/shared-KF algorithm, optimizer acceptance/bound,
truth ownership, loss-map or fit-objective change is authorized by this task.
The outgoing AGENTS is preserved byte-for-byte in
[the snapshot](2026-11-09-agents-before-truth-recorder.md); its entire first
section remains unchanged. Only its obsolete current focus was replaced.
The previous migration pause and open issues remain in
[the compatibility handoff](2026-11-08-session-handoff.md).

## Implemented contract

- Default `Edm4hepWriterAnaElemTool.CompactTruthRecords=true` keeps exact first/
  last/hook steps requested by every simulated tracker hit, subtype-3 steps,
  every eBrem photon's emitting parent step (the limiter can differ), track
  boundaries, first ECAL entrance and invalid t/X0 values. Omitted contiguous
  non-eBrem ranges are stored in new `GsfG4MaterialStepSpans`, with original
  count and a double sum of the original float step-t/X0 values. Gaps remain
  gaps. This preserves arbitrary later reconstructed-hit subsets and fractional
  hook intervals without inventing hit provenance. Invalid existing links are
  still rejected.
- Photon birth records are unchanged. Compact photon-step output retains the
  last recorded step, frozen at first ECAL entry; it therefore preserves all
  existing birth/entry/last-step tuple fields, including converted/terminated
  originals. `CompactTruthRecords=false` writes the previous full histories
  and an empty span collection for same-code A/B/debugging.
- `GsfG4FsrPhotons` and `GsfG4FsrPhotonSteps` reuse the material-photon datatypes.
  `GsfGeneratorFsrLinks` records photon/emitter/final-electron MCParticleGen
  indices, unique/ambiguous status and exact photon relation. Original datatype
  layouts remain unchanged. Generator birth information and G4 entry/last-step
  information have the same units as eBrem. FSR has no material `parentStep`.
- Classification follows same-signed electron copies backwards from each
  stable generator electron, stopping at a non-electron production node.
  Stable generator photons with direct parents on that outgoing chain are
  tagged; incoming beam radiation across the production node and G4 status-0
  photons are excluded. No opening-angle/energy matching or G4 process defines
  FSR. The primary converter's explicit index map supplies the photon G4 ID.
  Ambiguity is recorded rather than resolved by guessing. This covers all final
  generator electrons; production-leg selection remains downstream.
- The breakpoint event tree adds `fsr_photon_*` fields parallel to
  `ebrem_photon_*`, generator provenance indices/status, and
  `fsr_truth_available`. Old files without all three FSR collections have
  availability 0; an empty array then does not prove zero FSR.
- The shared embedded-truth reader and breakpoint identification consume optional
  spans. The maintained tracker/GSF input cards, reverse template and dedicated
  breakpoint cards discover optional companions from metadata. This is input
  compatibility only: no GSF configurable property or fit steering changes.
  Thus no RecGsfTracking property-surface audit is triggered.

Persisted output shrinks; transient full electron histories are still buffered.
The compact format reproduces the supported checkpoints/integrals/tuple outputs,
not arbitrary discarded per-step trajectories. Old readers cannot interpret
compact gaps without the span-aware reader. New readers support old full-step
files. Conversion-descendant shower attribution and photon-to-cluster truth
assignment remain outside this existing eBrem-format contract.

## Reproduction evidence

Artifacts are under `/tmp/truth-recorder-20261109/` and adjacent dated logs.
They are not committed ROOT/log/card outputs. The frozen source is
`higgs_eeHX_100jobs_20261106/rec-higgs-1.root` (200 events, radiation on).

Before rebuilding, the old installed RecBreakpoint produced
`/tmp/truth-recorder-20261109-baseline.root` from the first 18 input events,
selected events 0,11,16,17, production-electron selection on, verbose dumps on,
with the maintained fit/scanning defaults. The updated code on that identical
legacy REC source produced `/tmp/truth-recorder-20261109-updated.root`:

| Tree | Rows | Existing branches exactly reproduced |
|---|---:|---:|
| breakpoint | 89 (including skipped/control rows) | 953 |
| neutral_pfos | 4 | 62 |

No old branch differs, including NaN patterns; 52 new FSR/availability branches
are added to neutral_pfos. These rows are a mechanical regression, not a new
physics-efficiency or population-resolution measurement.

A native PODIO test tool compacted the SAME 18 frozen REC events in memory,
rewrote material/photon/link relations, and preserved all other collections.
Its full/compact focused fit rerun again used events 0,11,16,17 and verbose
component dumps. All 953 per-track and all 114 event-level branches reproduced
EXACTLY, with no t/X0 rounding differences on this fixture. Raw counts:

| Records | Full | Compact |
|---|---:|---:|
| material steps | 30,095 | 20,478 checkpoints + 1,199 spans |
| material-photon steps | 25,062 | 112 |

Synthetic tests additionally cover skipped intermediate hits, interval sums,
invalid gaps/overlap rejection, generator production boundaries/ambiguity/cycles,
same-format FSR tuple values and missing provenance, and optional-card inputs.
Floating-point regrouping of material sums is allowed only within 1e-13 in
comparison tests; fits/losses have no such allowance.

## Final build and clean generator simulation gate

The definitive simulation test starts from a generator-only ROOT containing
MCParticleGen; using PodioInput
on an existing REC with output `keep *` also carries original raw REC collections
through the frame. That preliminary harness is not the clean detector gate.
Threaded uproot local reads stalled in this environment; comparisons use
`handler=uproot.source.file.MemmapSource`. Preliminary readers were stopped.
The source REC has no EventHeader; attempting to select a nonexistent collection
with ROOTFrameWriter caused a test-tool crash, corrected by selecting only the
actual MCParticleGen collection. No source data was changed.

The final build/install completed successfully. All 18 focused C++ CTests pass;
65 existing working-tree batch-planning tests and two new optional-card tests
pass. Core checkpoint: `c79f24b`. No configurable GSF fit property changed.

Clean simulation gate: first two radiation-on MCParticleGen events, extracted
alone with ROOTFrameWriter, full detector TDR_o1_v01, sim seed 1. Both modes use
that exact generator-only input and the final installed code. Output lives in
`/tmp/truth-recorder-20261109/clean/{full,compact}/sim.root`. The clean comparison
script checks every non-truth leaf, complete checkpoint/span coverage, exact
material sums, birth/last/ECAL photon fields, generator assignment, first G4
photon pre-momentum and parent-step/hit-step relation identities.

| Two-event clean simulation | Full | Compact |
|---|---:|---:|
| material steps | 4,342 | 3,043 checkpoints + 171 spans |
| original photon steps (eBrem + FSR) | 5,481 | 15 |
| compressed truth leaf payload (bytes) | 754,750 | 324,583 |
| uncompressed truth leaf payload (bytes) | 1,616,309 | 642,020 |
| complete SIM file (bytes) | 27,718,198 | 27,287,596 |

All **324 detector-data leaf branches are exactly equal**. Six generator FSR
photons (three in each event) match the independent generator-chain check and
the primary IDs; the first recorded G4 pre-momenta match their generator birth
momenta. All retained physical truth values agree, including all tested volume
copy numbers (zero drift here). The global hit-link collection has the same
1,549 single-SimTrackerHit links but different HC concatenation order: 1,498
row positions move. Joining by the persisted `(SimHit collectionID,index)`
reproduces every link scalar/status and exact material checkpoint identity.
This is verification of the existing single-link contract, not a new
many-to-many association study or a provenance repair.

For equal recorded truth content, compact mode reduces compressed truth payload
by 57.0% and original-photon step rows by 99.7% on this small fixture. Total
file reduction is only 1.55%; calorimeter/detector data dominate. Do not present
these two-event storage figures as a population physics validation or a universal
compression ratio. Full histories remain a debug option.

## Native chain and completed scope

The two clean compact SIM events ran through the unchanged full tracker,
calorimeter-digitization and PFA/REC algorithms, then the final installed
RecBreakpoint tupliser. The dedicated cards add only optional truth inputs.
For this serialization gate, expensive free-loss/beam/charged fits and location
scans were disabled (`BP_FREE_LOSS_FIT=0`, `BP_BEAM_SPOT_CONSTRAINT=0`,
`BP_CHARGED_ECAL_MEASUREMENT=0`, `BP_LOCATION_SCAN_SETUPS=''`); verbose output
and production-electron selection were enabled. This does not replace the
maintained-default fit regression above.

All **154 GSF-truth leaf branches and 31 MCParticleGen leaf branches** survive
every stage exactly. The final `neutral_pfos` tree has availability `[1,1]`,
FSR counts `[3,3]`, and ECAL-entry counts `[2,2]`. Birth identity/kinematics,
generator/emitter/final-electron indices, origin status, last-step status,
step/energy and ECAL entry positions match the corresponding SIM objects.
The two photons without a recorded entrance retain their own last-step data.
These checks include material summaries, original material-photon provenance
and all new FSR relations. Output is
`/tmp/truth-recorder-20261109/clean/compact/native-flat.root`; the comparison
reports are `clean/simulation-comparison.json` and
`clean/compact/native-comparison.json` under the same temporary directory.

The pre-existing bad provenance/owner issues are not repaired or hidden. No
fit objective, ECAL collection window, loss map or physics selector changed.
No large campaign, production-performance validation, ISR recorder,
conversion-descendant shower attribution, or photon-to-cluster truth matching
was added. Existing REC/flat samples were not rewritten; newly recorded FSR
transport truth requires newly simulated data.

## Build/run and repository state

From the reference root, `source setup.sh`, then:

```bash
cmake --build build.105.0.0.x86_64-el9-gcc11-opt \
  --target DetSimAna RecBreakpoint RecGsfTracking GsfTruthRecordsTest \
  RecBreakpointCompactTruthReaderTest RecBreakpointTruthEcalTupleTest -j4
ctest --test-dir build.105.0.0.x86_64-el9-gcc11-opt \
  -R 'GsfTruthRecords|RecBreakpoint' --output-on-failure
python -m unittest discover -s Reconstruction/RecBreakpoint/tests -p test_batch.py
python -m unittest discover -s Reconstruction/RecBreakpoint/tests -p test_compact_truth_cards.py
```

The four affected packages were installed using their package-local
`cmake --install build.../{Simulation,Reconstruction}/<package>` directories.
Build logs contain existing ROOT C++-standard/PCM and clock-skew warnings;
the affected source files were explicitly compiled and all targets/tests
completed. The two clean native simulations and native chain also completed
successfully after the final install. These warnings do not imply an extra
physics-validation claim.

Implementation/header changes and maintained-card compatibility are committed
in `c79f24b`, with unrelated user edits preserved. Only this task's hunks in
already-dirty `DumpGsfTrks/gsf.py.bk` and `batch_breakpoint.py` were staged.
As required by the project rule for other run cards, auxiliary span-discovery
edits to `options/run_interval_identification.py` and
`RecGsfTracking/options/run_gsf_reverse_template.py` remain uncommitted.
All C/C++ and header work is checkpointed; generated ROOT files/logs/temporary
cards and comparison scripts are not committed. No branch or remote changed,
and no batch jobs were submitted. No plot-server change was needed.

Memory-curation coverage: only the outgoing Current focus was replaced; its
complete text is in the dated snapshot, and the global-status/laws/build
section is byte-identical to that snapshot. No history directory was moved,
so a directory-migration manifest was unnecessary. Earlier handoff/snapshot
files are preserved unchanged. Hive migration remains paused.

## Storage attribution follow-up

The user questioned the small whole-file reduction. A read-only branch-size
audit confirms SIM -1.5535% and REC -1.4739% for the two-event A/B. In an existing
200-event 3.639 GB production REC, raw ECAL simulation is 83.603%, HCAL 7.162%,
and all Gsf truth only 1.555%. In the corresponding 268.46 MB flat file,
covariance branches occupy 65.605%. Details, per-collection sizes and current
campaign totals are in [the storage audit](2026-11-09-product-disk-breakdown.md).
No retention/merge policy was changed.
