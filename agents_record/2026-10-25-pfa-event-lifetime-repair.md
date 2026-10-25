# PFA event-lifetime crash repair — 2026-10-25

## Batch incident and scope

The completed rec_large_barrel_20261025 campaign has 376 prepared jobs:
257 completed and 119 failed, all during rec with exit code 139. Every failing
stack contains TruthClusteringAlg::Initialize and podio::ObjBase::release.
104 name MCParticle destruction; the remaining 15 name Track destruction.
This is not a scheduler-group rejection or a recorded memory-limit kill.

The user authorized fixing this problem after the batch finished. Changes are
limited to RecPFACyber cleanup, its regression test/documentation, and durable
project status. No batch cards, physics settings, clustering/calibration logic,
breakpoint fitter or shared tracking libraries are changed.

## Root cause and minimal source repair

AlgorithmManager runs Initialize -> RunAlgorithm -> ClearAlgorithm for each
algorithm inside the current event. TruthClusteringAlg::Initialize copies
CyberDataCol::TrackCol, taking shared ownership of Cyber::Track objects with
embedded edm4hep::Track and MCParticle handles. ClearAlgorithm omitted both
m_TrackCol and m_1dclusterVCol, so these lists survived event teardown.
At the next Initialize, clearing m_TrackCol destroyed stale EDM handles.

PODIO only reference-counts untracked objects in ObjBase::acquire/release.
The input collection owns tracked objects: MCParticleCollectionData::clear
explicitly deletes them. Keeping a C++ handle does not keep those tracked
objects alive beyond their event. Reading a stale handle during destruction
therefore depends on whether freed memory is still mapped/readable.

Repair: add m_TrackCol.clear() and m_1dclusterVCol.clear() to the existing
ClearAlgorithm. The current CyberDataCol still owns tracks and published
clusters for downstream processing. This neither removes outputs nor changes
the reconstruction calculation. Git blame attributes the pre-existing cleanup
omissions to upstream commit 877bf094 (2024-08-08), not the new exporter.
The timing of batch failures after the export update does not prove a new
exporter physics bug; extra allocations can expose a latent lifetime defect.

## Validation and important limits

Focused test: Reconstruction/RecPFACyber/tests/check_truth_clustering_lifetime.py.
It loads the selected PFA module and exercises the real Initialize/ClearAlgorithm
methods across three track-containing/empty-event cycles. After algorithm
cleanup, only CyberDataCol may own the track; after data.Clear(), its weak
observer must expire. Repeated cleanup must remain safe.

- Unfixed installed module: deterministic assertion failure (track retained).
- Repaired build: passes.
- Repaired default installation: passes.

The fresh project-default runtime (no build-library override) additionally
completed all 100 seed-16 events. Log/output:
/tmp/pfa-event-lifetime.AkcEyQ/installed-16.log and installed-16.root.
Compared against the repaired build on all 100 frames: 231 final-PFA/timing
fields are exact; middle-cluster fields and linked hits agree after reordering
(events 21 and 36 only). This fresh installed run used no build-tree override.

The ordinary unfixed seed-16 local rerun completed 50 events, and an allocator-
perturbed unfixed rerun (MALLOC_PERTURB_=165) completed 100. Thus the actual
batch segmentation fault was not deterministically reproduced locally. Do not
describe these as a crashing-before/passing-after event A/B pair: the controlled
before/after failure is the ownership regression, supported by all 119 batch
stack traces and PODIO collection destruction semantics.

Repaired build ran 100 sequential events each for failed seeds 16, 20 and 143;
all completed successfully. Their batch crashes occurred at zero-based events
27, 82 and 72 respectively. Seed 143 was tested with MALLOC_PERTURB_=165.
All runs use retained calodigi files and frozen rec cards, with separate /tmp
outputs. No simulation, tracker or digitization rerun was needed.

Exact output checks:

- Seed 16, first 50 normal unfixed/fixed frames: 231 final-PFA/timing fields
  equal exactly. Middle clusters differ in vector ordering only at events 21
  and 36; every cluster field and linked constituent-hit record agrees after
  relation-preserving reordering. The connecting code iterates a map keyed
  by Track pointers when publishing remaining matched clusters. Vector index
  order across changed allocation layouts is not a physics identity.
- The established 20-frame gate again passes: 32 timing fields match the
  original timing-only run, 32 middle fields match the original connected-only
  run, 199 final-PFA fields match the retained earlier full-chain output;
  energies are finite/nonnegative and snapshot hit links are valid.
- The old sim_large_barrel_20261018/rec-barrel-1.root reference disappeared
  outside this task. Used the retained /tmp/ecal-timing-full-20261025.root
  20-frame reference (entries 0..19); its original reference equivalence was
  already established by the earlier export gate. Nothing was restored or
  deleted to recreate that original rec file.

The first progress message about 263 exact fields was premature: the comparison
was still running and later found the two ordering differences. Corrected in
the conversation immediately and resolved with the relation-aware check above.

Local validation directory: /tmp/pfa-event-lifetime.AkcEyQ. It contains frozen-
card wrapper run_rec.py, before/after ROOT/log files, compare_outputs.py and
build/staging logs. The 20-frame repaired output is
/tmp/pfa-three-stages-full-20261025.root; original stopped-chain references were
retained. Local scripts and generated ROOT/log artifacts are not committed.

## Scoped installation

Only the PFA package was staged from build.105.0.0.x86_64-el9-gcc11-opt into
/tmp/pfa-event-lifetime.AkcEyQ/stage. The generated configurable differs only
in its timestamp, so it was not replaced. The sole runtime replacement is
InstallArea/x86_64-el9-gcc11-opt/lib/libCrystalCaloRec.so, copied to a same-
directory temporary file and atomically renamed after backing up the old file.

SHA256:

- prior module: 17e5eb775139096e718f24519b7c72e3bc0c85f698d6f9127851a19756dcfaa5
- repaired module: 686747f7273dcc1c07d10be98ca121f43875edf2aef8858f81bf794c7ae78254
- unchanged installed RecBreakpoint:
  6704f6c2b441b4556db1f110a35ee7f4baf1921338f25b80f2cef6066db62af7

Rollback copy: /tmp/pfa-event-lifetime.AkcEyQ/previous/libCrystalCaloRec.so.
Use an atomic replacement if rollback is requested. This /tmp copy is not a
durable archive. RecBreakpoint photon projection remains built/tested only,
not installed. No full-tree install was performed.

## Recovery inputs and next action

No batch was submitted/resubmitted and no existing ROOT/log/card file was
modified or deleted. Exactly 119 failed-job calodigi files remain available;
successful jobs already cleaned up their own intermediate files. Failed rec
outputs may be incomplete and require a rec rerun; installation does not
repair existing files. Do not delete those retained calodigi inputs first.
The existing helper can prepare STAGES=rec against rec_large_barrel_20261025
into a fresh output directory; missing successful-job calodigi inputs are skipped.

Failed seeds:

```text
9,12,14,16,19,20,25,41,56,58,60,64,67,69,73,74,75,79,81,88,90,95,
108,114,115,117,120,123,134,138,141,142,143,145,151,152,155,156,165,
167,170,171,178,179,180,183,185,190,192,194,200,206,207,208,216,219,
221,222,224,226,229,237,238,242,247,250,263,264,266,272,274,277,280,
286,297,300,305,313,321,327,328,337,343,350,352,354,355,357,363,373,
374,378,391,398,407,409,411,421,427,428,430,436,438,439,446,447,451,
453,458,460,462,463,468,481,484,486,491,492,499
```

Outgoing full AGENTS.md is preserved in
2026-10-25-agents-before-pfa-lifetime-repair.md. Global laws remain unchanged.
No full 119-job batch regression or endcap campaign was performed here.
