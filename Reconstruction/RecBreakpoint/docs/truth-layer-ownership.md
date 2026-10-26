# Layer-owned truth breakpoints

`TruthBHLossEventData::matchBreakpoints` is the only truth-to-breakpoint mapper
for RecBreakpoint. The ordered hit array is outward in both filter directions.
Index i denotes the point at hit[i]: outward loss then propagation i->i+1;
inward propagation i+1->i then inverse loss then measurement i. RTS reuses the
forward transitions. Free-loss varies its prior center, not this location.

Sensitive ownership uses the complete SimTrackerHit contributing-step range,
not its interior hook. Passive ownership uses reviewed TDR_o1_v01 geometry:
ITK layer-specific ladders, OTK inner/outer modules, VXD bent staves and
side-specific planar flex/sensor envelopes. VXD common support retains
candidate layers {4,5}; the global shell is shared without a unique layer.
Other passive structures remain unassigned except the two explicit effective
assignments below. Geometry is queried just inside
the emitting volume at the post-step endpoint and its radiation length checked
against G4. No nearest-layer rule is used. Exact sensitive provenance also
works outside this passive geometry whitelist. Endcap passive ownership is
not implemented/validated; it is not silently assigned to a barrel layer.

Sensitive provenance identifies the actual sim hit. Passive assemblies are
matched to same-primary sim hits and then exact reco associations. Returning
tracks are not matched across an intervening other sensor assembly. Multiple
accepted candidate hits are ambiguous; a missing accepted hit and an outermost
hit are separate ineligible statuses. None is interpreted as physical no-loss.

## Effective barrel boundary ownership

Two explicit, default rules extend the assignment without changing simulation,
reconstruction, the G4 step record, or the fitter's loss-placement algebra:

| Emitting volume | Effective target | Rule code |
|---|---|---:|
| `BeamPipe_IPInnerTube_*` | VXD layer 0 | 1 |
| `TPCinnerWallLog_*` and its material slices | TPC row 0 | 2 |

Exact sensitive-step provenance takes precedence. Otherwise the emitting
volume must pass the existing G4/geometry radiation-length check. Match the
same primary to the actual target layer's subsequent simulated traversal,
without crossing an intervening different sensor assembly, then use the exact
reco association. A missing layer-0 hit is not replaced by the first available
hit. Ambiguous/missing/outermost hits remain ineligible. Beam-pipe returns after
the target traversal do not attach backwards to that visit. The whitelist does
not cover endcap pipe sections, TPC outer walls, or unknown passive structures.

These are effective associations, not claims of physical sensor ownership.
The loss still acts at the owner hit before outward propagation to the next
hit: the real pre-layer loss has therefore been relocated outward in the
fitted model. Preserve the true post-step position/material path and explicitly
check IP momentum performance before making a physics claim. Existing rec
files suffice; rebuild the package and rerun only the breakpoint consumer to
regenerate results. The shared package-level tuple helper also exposes the
rule tag to the identification consumer; GSF's workflow is not changed.

Eligible emissions at one breakpoint are summed. Truth selection retains only
the largest positive group; ties retain the innermost. Truth override uses
z=1-sum(step momentumLoss)/preMomentum(first emission), b=-log(z), with the
configured unchanged SigmaLogLoss. This groups subtype3 step momentum changes,
not photon energies; continuous losses within those steps are still included.
The old hook-to-hook t/X0, first/last steps and fractions remain physical
interval diagnostics, NOT the membership definition for this loss group.

## Flat tuple: one vector entry per recorded primary eBrem emission

- `truth_layer_association_status`: 0 not attempted, 1 success, -1 failed match.
- `truth_ebrem_step`: G4 step number; same primary as interval_selection_truth_g4_track_id
  in Truth mode (truth_override_g4_track_id for a Manual oracle).
- `truth_ebrem_owner_status`: 1 sensitive provenance, 2 passive assembly or
  explicit effective target, 3 shared structure, 4 unassigned material,
  5 geometry/X0 mismatch.
- `truth_ebrem_effective_owner_rule`: 0 no effective reassignment, 1 beam pipe
  to VXD0, 2 TPC inner wall to row0. This tag can be nonzero even if the target
  hit is unavailable; inspect breakpoint status separately.
- `truth_ebrem_breakpoint_status`: 1 eligible, 2 missing accepted hit, 3 multiple
  candidate accepted hits, 4 outermost hit, 5 no unique owner, 6 invalid loss.
- `truth_ebrem_owner_system`, `_owner_layer`: decoded detector identifiers, -1 unknown.
  OTK layer=0 alone is insufficient; use the assembly key as well.
  TPC high-pT sim-hit cell IDs are one-based row numbers, converted to zero-based
  owner layers; they are not decoded as the silicon CellID bitfield. Low-pT
  accumulation provenance retains layer=-1 when its copy number is not a row.
- `truth_ebrem_owner_key`, `_material_path`: owner assembly/cell and exact navigated
  material placement. The latter is endpoint-near inside the emitting volume.
  Effective keys are `effective:beam-pipe-to-vxd0` and
  `effective:tpc-inner-wall-to-row0`; they are not geometry placement paths.
- `truth_ebrem_breakpoint_hit`: accepted hit index, -1 if unresolved. Only status1
  is eligible; status4 still retains the outermost hit index for diagnosis.
- `truth_ebrem_candidate_layer_offset`, `_candidate_layer`: flattened candidate
  layer lists; offsets length N+1 on success, including [0] for no emissions.
- `truth_ebrem_x/y/z`: actual post-step emission position in mm.
- `truth_ebrem_momentum_before`, `_momentum_loss`: pre-step magnitude and recorded
  step loss in GeV.
- `truth_ebrem_no_unique_owner_count`: number with owner status3/4/5.
- `truth_ebrem_ineligible_count`: number with breakpoint status !=1.

Emission records include unselected, missing-hit, shared and unassigned cases.
These vectors are independent of the selected interval vector (at most one in
Truth mode). Count events after deduplicating event/primary IDs across tracks;
report failed associations separately. A no-eligible-breakpoint refit is a
zero-breakpoint control, not evidence of a truth-clean track.
