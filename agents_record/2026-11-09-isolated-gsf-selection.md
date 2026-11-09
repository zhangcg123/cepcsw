# Isolated GSF production-electron selection (2026-11-09)

## Authority and frozen production

The user requested the same recoil-electron selection used by breakpoint,
implemented as a configurable class in GSF. The latest idle-job answer is
**jobs are active**, superseding the earlier no-jobs reply. The user then
explicitly said long production jobs must not block further development.
Development therefore uses a separate source worktree, build, and install;
production source, workflow cards, build cache, and libraries remain unchanged.
Only this record, the preserved snapshot, and current-focus documentation are
updated in the primary workspace. No normal build/install or batch submission
was performed. Primary branch remains `breakpoint-delayed-neutral`.

Private area, relative to the primary repository:

- source: `build.gsf-selection-dev/source` (detached worktree from `e07c749`);
- inherited current GSF card checkpoint: `1c1fb09`;
- package-only build: `build.gsf-selection-dev/package-build`;
- private install: `build.gsf-selection-dev/install`;
- environment: `source build.gsf-selection-dev/setup-dev.sh` in a fresh shell.

The package-only harness at `build.gsf-selection-dev/package/CMakeLists.txt`
imports existing dependencies read-only. A first full-project configuration
could not fetch CKF over the restricted network and was abandoned without any
production changes. Private build commands are:

```bash
source setup.sh
cmake -S build.gsf-selection-dev/package -B build.gsf-selection-dev/package-build \
  -DCMAKE_INSTALL_PREFIX="$PWD/build.gsf-selection-dev/install"
cmake --build build.gsf-selection-dev/package-build -j4
ctest --test-dir build.gsf-selection-dev/package-build --output-on-failure
cmake --install build.gsf-selection-dev/package-build
```

Keep the generated private per-module `.components`/`.confdb` files next to its
libraries (the initial setup copied those too). The generated private Python
configuration was verified to expose the new bool with compiled default false.
Do not use normal `build.sh` or install to the production prefix while jobs are
active. A future batch should freeze its chosen install; do not replace a
library, dictionary or configuration path already used by a job. This overlay
still depends on the unchanged existing installation.

## Implementation and configuration

All new C++ changes are in the private worktree's `RecGsfTracking` package.
`SelectTruthProductionElectrons=False` keeps all-track behavior. The maintained
generic card sets false explicitly; the eeHX test card sets true before its
conditional `CompleteTracksParticleAssociation` input request. The standalone
breakpoint `TruthElectronSelector` class is compiled privately into GSF;
there is no duplicated physics selector or link to the breakpoint Gaudi plugin.
Its unique stable generator e-/e+ outside Higgs ancestry, association aggregation,
duplicate ranking, ambiguity and one-reconstructed-partner contracts are shared.
No explicit generator Z parent is required.

The event selector runs once, and its gate precedes per-track truth matching,
hit preparation, initialization and all fit workflows. Nothing is cloned,
filtered out of REC, or renumbered. Missing truth/associations fail closed.
Truth oracle/passive recorder index 0 is never silently reassigned.

Five automatic integer EDM outputs record selection status, e-/e+ MC indices,
original selected input ordinals, truth candidate counts, and the actual
published-track-to-input mapping. They are mirrored by `production_electron_*`
and `gsf_input_track_indices` flat branches. Existing final-component/lineage
mappings retain original input ordinals. Status -5 denotes an event excluded
by `SelectedEventIndices`; other codes match breakpoint's selector.

The dedicated property audit passed: 44 custom properties documented, 43
explicitly steered in the maintained card, and only passive
`RecordTruthMaterialIntervals` deliberately inherited. Current maintained
look-ahead is 2 despite the retired disabled comment; documentation and that
comment are synchronized only in the private worktree. Original fit settings
are preserved. Scalar legacy tuple fields still use MC[0]/track[0]; they do
not establish two-electron momentum or recoil performance. Use EDM endpoints
and the original-index maps.

## Focused evidence

Artifacts live under `build.gsf-selection-dev/validation/`, not in Git.

| Case | Selected original inputs | Published original inputs |
|---|---|---|
| Normal ISR/FSR REC seed 4, event 0 (41 inputs) | 9, 18 | 9, 18 |
| Compact REC event 0 (15 inputs) | 2, 13 | 2 |
| Compact REC event 11 (17 inputs) | 4, 16 | 4, 16 |
| Compact REC event 16 (26 inputs) | 6, 8 | 6, 8 |
| Compact REC event 17 (31 inputs) | 7, 13 | 7, 13 |

The first case completed in 135.86 s with comprehensive verbose component
dumps, peak RSS 1,877,436 KiB. The compact regression completed in 408.91 s,
peak RSS 2,263,636 KiB. Both used unchanged maintained physics settings.
Every saved lineage/component input stayed inside the selected pair; published
row maps and component normalization checks passed. Regression event 0 input
13 was rejected at its first hit. The legacy warning says `track 2` because
it prints `nFit+1`, not the original input index; saved maps are authoritative.
This failure has not been diagnosed or claimed equivalent to an old fit.

The reused selector unit test passed its ancestry/copy, missing-Higgs,
nonunique-pair, duplicate/tie, association-aggregation and missing-match cases.
A native enabled-selector run without requested track associations produced
status -4 and no output tracks or fit lineage, rather than an all-track fallback.
The default-off native A/B compared the private and installed code on normal
REC event 8 (four input tracks). All **250 common legacy flat branches across
nine rows** agree exactly, treating paired NaNs as equal; only event 8 fits.
That control uses MaxComponents=1, both BH gates off, beam off, look-ahead 0;
it is an isolation/default-off compatibility test, not a radiative benchmark.
Both controls published original inputs 0,1,2,3.

The integration checker is maintained in the private worktree at
`Reconstruction/RecGsfTracking/tests/check_production_electron_selection.py`.
It checks selected/published/lineage provenance, row mapping, vector lengths,
finite component means and positive normalized mixture weights. Tests covered
five selected physics events and ten selected tracks, with nine fit outputs.
There is no population, resolution, tail-safety or calibrated-efficiency claim.

19 fingerprints (SHA256 plus nanosecond mtimes) of production sources/cards,
build cache, GSF/truth libraries and Python configuration remain identical to
`build.gsf-selection-dev/production-before.json`. The private area occupied
about 615 MiB including source, builds, ROOT outputs and verbose logs at check.
No remote refs were changed; implementation is checkpointed in the detached
private worktree until an explicit production/publication transition.

## Knowledge migration

The complete pre-edit AGENTS.md is preserved in
`2026-11-09-agents-before-isolated-gsf-selection.md`. Section 1 (global status,
active laws, scope and essential commands) is retained byte-for-byte. The old
REC-preparation focus and all of its figures, commands and evidence links are
retained in that snapshot. Section 2 is replaced with concurrent frozen
production/private GSF development status. No history directory was renamed,
no existing history file removed, and no new physics validation claim added.
