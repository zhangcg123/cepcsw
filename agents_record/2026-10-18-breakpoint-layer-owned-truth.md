# Breakpoint layer-owned truth implementation and validation

## Scope and contract

User authorized implementing layer-owned eBrem truth for RecBreakpoint only.
GSF and identification workflows are explicitly deferred. The shared
TruthBHLossEventData reader now retains step radiation length and sim-hit cell
ID and declares matchBreakpoints; its implementation and geometry ownership
helpers live under RecBreakpoint. Legacy matchTrack calculations are unchanged.

Both halves of a sensitive traversal use its exact SimTrackerHit contributing
step range, not the hook's before/after division. Reviewed TDR_o1_v01 barrel
support/cooling follows its sensor assembly. Shared or unknown material is
saved but excluded. Ownership, missing accepted hits, ambiguous accepted hits,
outermost-hit exclusion and invalid losses are distinct statuses. There is no
nearest-radius fallback. TPC high-pT sim-hit IDs are raw one-based row numbers,
not silicon-style bitfields; owner metadata converts them to zero-based rows.

The common index i is the SAME accepted hit for both directions:

```
forward:  update hit[i] -> loss at hit[i] -> propagate to hit[i+1]
backward: propagate hit[i+1] to hit[i] -> inverse loss -> update hit[i]
RTS:      reuse the corresponding forward transition
```

All fit families receive the same selected index. Existing largest-positive-
loss-only selection remains. Ordinary/diffuse/free-loss fitting receives no
truth loss. Truth override uses the same owner group and
z=1-sum(step momentumLoss)/preMomentum(first emission), b=-log(z), retaining
the configured SigmaLogLoss. This is recorded subtype-3 step momentum loss,
including continuous loss within those steps, not emitted photon energy.
Hook-to-hook t/X0 and hook bounds remain separate physical diagnostics.

No propagation, measurement, smoother or optimizer algebra was changed.
No new configurable property was added. The maintained card comments and
README were updated; the installed runtime was NOT changed. Full schema:
Reconstruction/RecBreakpoint/docs/truth-layer-ownership.md.

## Mechanical checks

- Built RecBreakpoint, RecBreakpointTruthOwnershipTest and
  RecBreakpointTruthMatchTest in build.105.0.0.x86_64-el9-gcc11-opt.
- Six CTests passed: transport, likelihood, diffuse, neutral loss candidate,
  geometry ownership, and actual EDM truth-to-breakpoint association.
- 36 batch-helper unit tests passed. Card Python compilation and git diff
  whitespace checks passed.
- Synthetic association test covers emissions on BOTH sides of one hook,
  grouping, missing accepted hit, unknown gap, outermost hit, TPC row metadata,
  and exact preservation of legacy hook material calculations.
- Private ROOT navigator requires BuildCache plus cache BuildInfoBranch;
  it is initialized once per algorithm, reused across tracks, and owned by
  unique_ptr. The initial trial exposed and fixed missing cache initialization.

## Local event test

Input: sim_large_barrel_20261001/rec-barrel-1.root, event indices 0--49.
Maintained run_breakpoint.py defaults: Persistent6D, Truth location selection,
SigmaLogLoss=0.001, free-loss/diffuse/ECAL/truth-override enabled. No batch
submission and no installation. Temporary evidence:
/tmp/breakpoint-ownership-9qaTUf/final-50.root and final-50.log.

Completed successfully: 56 track rows, 53 successfully associated rows.
Event23 tracks0/1 and event40 track0 have no provenance-bearing association for
accepted hit0 and are flagged, not labelled truth-clean. Each of the 50 events
has at least one successfully associated row. These are structural counts,
not topology-clean optimization/performance denominators.

Deduplicate by (event, primary G4 track, G4 step), NOT flat-tree row:

| Owner | Unique recorded emissions |
|---|---:|
| Sensitive provenance | 16 |
| Uniquely associated passive assembly | 17 |
| Shared structure | 4 |
| Unassigned material | 12 |
| Geometry/X0 mismatch | 0 |
| Total | 49 |

14/50 events contain at least one shared/unassigned emission:
2,3,6,9,13,17,24,26,29,31,39,40,48,49. Unique physical owner does NOT guarantee
an eligible accepted hit. Across all 56 rows (including repeated emissions),
eligibility counts are: 21 eligible, 10 missing accepted hit, 7 outermost hit,
18 without unique owner. These row counts must not be presented as unique
emission counts.

The persisted checker verifies vector alignment, candidate offsets, ownership
counts, owner system/layer against accepted-hit CellIDs, largest-group
selection and truth-override grouped loss/z. Command:

```
python Reconstruction/RecBreakpoint/tests/check_truth_ownership_tuple.py /tmp/breakpoint-ownership-9qaTUf/final-50.root
```

## Source-matched regression

The installed library is older than the current source in its free-loss
Persistent6D support. It is NOT a valid baseline for attributing changes to
this patch. Built a temporary baseline using HEAD's RecBreakpoint.cpp/.h
and the SAME current fitter/optimizer/helper objects. Build script and baseline
sources: /tmp/breakpoint-ownership-9qaTUf/build_baseline.py and baseline-src/.

Manual breakpoint8, event16, truth override off: 472 common branches exactly
equal (NaN-aware) between pre-change and updated main algorithm. Evidence:
samecode-manual.root and new-manual.root. No fitter changes are hidden by an
installed-runtime mismatch.

Verbose focused gate, seed1 events11,16,17 (zero-based), same-source baseline:

| Event | Selected index old -> new | RTS pT old -> new [GeV] | Free RTS pT old -> new [GeV] | Truth RTS pT old -> new [GeV] |
|---|---|---|---|---|
| 11 | [] -> [] | 26.737668561 -> identical | 26.737668561 -> identical | 26.737668561 -> identical |
| 16 | [8] -> [8] | 11.298067033 -> identical | 11.308964847 -> identical | 11.308440303 -> 11.308434096 |
| 17 | [8] -> [] | 35.460906220 -> 35.460232274 | 35.514193363 -> 35.460232274 | 35.580333403 -> 35.460232274 |

Event11 changes only selection status (no emissions now explicitly status3).
Event16 keeps its loss/index; oracle changes slightly because its denominator
is now pre-emission momentum rather than the earlier interval-hook momentum.
Event17's non-sensitive TPC gas emission has no unique layer owner: it is
retained but excluded as requested, and all empty-breakpoint extras copy the
reference. This can worsen pT; exclusion is NOT a physics improvement claim.

Final TPC metadata correction changes only owner system/layer/key relative
to the earlier 50-event run; all other branches are exactly equal. Analysis:
/tmp/breakpoint-ownership-9qaTUf/summarize_validation.py.

## Remaining limits / next action

No endcap passive ownership validation: unknown structures fail closed.
No broad efficiency or momentum-resolution claim; this is a 50-event
mechanical test, not held-out physics validation. More events should quantify
loss-dependent ownership coverage before promotion. Installation awaits review.
No ROOT files, batch products or generated tables are part of the source commit.
Outgoing identification status was preserved completely in
2026-10-18-AGENTS-before-layer-ownership.md before updating AGENTS.md focus.
