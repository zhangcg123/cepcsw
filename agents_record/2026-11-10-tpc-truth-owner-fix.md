# TPC truth owner reset, preserving older hit behavior (2026-11-10)

## Authorization and exact scope

The user requested a fix and a few tests, without regenerating current REC
production. They then explicitly restricted the work to code added by this
development: no changes to older simulation hit-building, tracking or PFA code;
report any necessary broader modification before editing it.

The b633efb August22 extension added the first/last/hook step-number fields
in TimeProjectionChamberSensitiveDetector. Their original event/row-exit resets
missed changes of G4 particle. This could join a previous particle's first
step to the next particle's last step, producing a reversed provenance range.

The fix replaces only those added bookkeeping lines with TpcPadRowTruth,
a small private state containing owner ID and first/last/hook numbers.
addStep clears this metadata on owner change, then starts the new range.
The existing row-centre hook assignment and event/ordinary-row-exit clears
use the same new state. A fresh particle cannot inherit an old hook; if it
never provides its own centre crossing, the hook stays unavailable.

No old energy/path/time/position/momentum calculation, hit-creation branch,
threshold, low-pT accumulation, GSF/breakpoint fitter, PFA, geometry, material
model or reader guard changed. The header/class layout and two simulation
libraries are rebuilt together. CMakeLists.txt is untouched.

The scope audit checked git blame for every deleted/replaced existing line:
all17 originate in b633efb, with zero pre-existing lines changed. New header
and C++ test files belong entirely to this fix. This repairs our truth
bookkeeping; it does not claim to repair older physical accumulation behavior.

## Unit and build checks

The standalone test exercises same-particle accumulation; resets; the two
observed reversed-range patterns459->170 and490->132; owner changes with
smaller, equal and larger counters; stale-hook invalidation; resuming a
previous owner; and reused track IDs after an event reset.

Run from the repository root:

```bash
g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I Simulation/DetSimSD/src \
  Simulation/DetSimSD/tests/TpcPadRowTruthTest.cpp \
  -o build.gsf-selection-dev/validation/tpc-provenance-owner-fix-20261110/tpc-truth-test
build.gsf-selection-dev/validation/tpc-provenance-owner-fix-20261110/tpc-truth-test
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target DetSimSD -j4
cmake -P build.105.0.0.x86_64-el9-gcc11-opt/Simulation/DetSimSD/cmake_install.cmake
```

Unit test, DetSimSDLib/DetSimSD build and package-only installation passed.
Baseline jobs finished before installation. Baseline installed-library hashes
were unchanged through their runs; baseline module copies are retained with
the test artifacts. GSF and breakpoint installed hashes are unchanged afterward.
No new Gaudi configurable property was added or changed.

## Native simulation A/B

Inputs are source events0,1,17 of normal-ISR/FSR generator job4:
/cefs/higgs/zhangkl/stdhep/E240/higgs/E240.Pe1e1h_X.e0.p0.whizard195/e1e1h_X.e0.p0.0004.stdhep.
Temporary cards use StdHepRdr.StartIndex, one event each and the same seed4,
full detector/physics and otherwise identical baseline/fixed steering.
This standalone simulation is an A/B regression, not a bit-identical replay of
the old combined SIM/TRK/REC execution's complete random-number history.

Only EventHeader, MCParticle, eight tracker SimTrackerHit collections and truth
collections are retained. No digitization, track reconstruction, calorimeter
reconstruction or PFA is run. All six native jobs complete successfully.

| Generator source event | Type3 TPC links | Reversed ranges before -> after | Shared-reader electron paths before -> after | Exactly equal raw tracker hits | Exactly equal MCParticles |
| ---: | ---: | --- | --- | ---: | ---: |
| 0 |892|0 -> 0|2/2 -> 2/2|17759|380|
| 1 |274|1 -> 0|1/2 -> 2/2|8388|476|
| 17 |1119|1 -> 0|3/4 -> 4/4|11552|439|

The two fresh baseline bad links were614->164 (hook163, status28) and488->134
(hook133, status30). Afterward all2285 pad-row links have complete status31,
ordered bounds and matching saved hook/SimHit positions (maximum distance0).

The consumer check supplies all stored type3 TPC links and synthetic tracker
measurements exactly at their SimTrackerHit positions to the unchanged
TruthBHLossEventData reader shared by GSF and breakpoint. All eight primary-
electron TPC paths then pass strict interval matching. These are not actual
reconstructed tracks or tracking-efficiency counts. The TPC-only test avoids
unrelated references to omitted coil/muon collections in these trimmed SIM
files; it retains every tested TPC link and all saved material steps/spans.

A native PODIO comparison checks every persisted data field and MC relation
of all37699 simulated tracker hits across the eight retained collections,
every scalar/vector and parent/daughter relation of1295 MCParticles, and
EventHeader fields. Every comparison is exact. This demonstrates preservation
of the original physical hit data in these three cases, not universal physics
validation. ROOT-native readers are used for these PODIO data; uproot's split
leaf reading did not complete and was not used as verification evidence.

## Limits and current data

Existing REC files were not rewritten or regenerated. They retain their old
provenance defects, so the established strict Truth-before-scan workflow can
still fail on those stored records. New simulation receives corrected truth
bookkeeping. The separate merged-TPC-hit/reconstructed-position5mm mismatch
remains unchanged. No GSF or breakpoint component fit was rerun in this task.

The user assumed there is no TPC splitting/scanning: the unchanged GSF actually
uses thickness gating without a blanket TPC veto, while breakpoint retains
the first exact TPC row0 as a reconstructed boundary candidate. This audit
changes neither behavior. Endcap BH range limitations remain deferred.

Artifacts are uncommitted under
build.gsf-selection-dev/validation/tpc-provenance-owner-fix-20261110/:
unit executable, six cards/logs/SIM outputs/run results, baseline libraries,
scope-audit.json, baseline-truth.json, fixed-truth.json,
physical-payload-comparison.json and validation.json, with their inspector scripts.

## Subsequent downstream check and header clarification

Actual reconstructed-track GSF/breakpoint checks were subsequently run; see
2026-11-10-tpc-truth-downstream-check.md. The initial SIM files omit EventHeader.
The initial physical comparator accepted both missing header collections as
empty, so its header-equality check was vacuous. The complete SimTrackerHit and
MCParticle field/relation comparisons remain valid. Downstream standalone
tracking received a new EventHeader only in isolated test fixtures; no simulation
or original reconstruction source was changed to supply it.
