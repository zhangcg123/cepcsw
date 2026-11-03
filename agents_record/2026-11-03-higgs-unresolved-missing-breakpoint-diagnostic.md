# Higgs hard-eBrem unresolved tracks: missing-breakpoint diagnostic

Recorded 2026-11-03 on `breakpoint-delayed-neutral`, at the user's request.
Read-only analysis of saved tuples; no refits, C++ changes, run-card changes,
ownership changes, installation changes or remote operations were performed.

## Selection and denominators

Input: all **249 completed Higgs jobs**, 49,800 generated events, under
`higgs_full_20261030/`. Counts below are **electron tracks, not Higgs events**.
The existing analysis selects a unique best reconstructed representative per
stable truth electron using associated-hit count, purity, total hits and track
index. It does not require breakpoint-fit success.

Every selected track must have:

- Total dedicated same-parent tracker-eBrem photon **birth energy**, divided by
  that electron's initial truth energy, **strictly greater than 1%**. This is
  a summed-loss condition, not an individual-photon threshold. Do not replace
  it with the legacy `truth_ebrem_step` multiplicity.
- Successful hit-supported neutral collection, status 1 and cluster count 0,
  using the maintained track-reference window (theta +/-10 mrad, wrapped phi
  +/-200 mrad). This is **not** an event-wide neutral veto.
- A valid truth electron ECAL entrance and usable charged-ECAL input.

Barrel/endcap are defined by the recorded truth electron ECAL entrance.
The sample is the **unresolved-neutral subset**, not all hard-eBrem Higgs
electrons. The six displayed events were selected by additional truth photon
geometry/energy criteria and are not a representative subsample.

| Region | Selected tracks | No truth-selected breakpoint | Fraction |
|---|---:|---:|---:|
| Barrel | 21,409 | 7,408 | 34.60% |
| Endcap | 12,131 | 6,163 | 50.80% |
| Combined | 33,540 | 13,571 | 40.46% |

For the barrel subset with exactly one recorded same-parent photon, 4,409 of
8,602 tracks (51.26%) have no truth-selected breakpoint. This still is not the
exact selection of the six event displays.

The saved beam-only free-loss (`100`) family additionally lacks breakpoint
output on **12 tracks that did have a truth-selected breakpoint** (8 barrel,
4 endcap): 7 input-KF copies (`result_status=3`) and 5 unavailable outputs
(`result_status=0`). These must not be counted as ownership-selection failures.
Thus the `100` output has no breakpoint on 13,583/33,540 tracks (40.50%). The
unconstrained `000` family has 45 selected-but-no-breakpoint outputs (40 KF
copies, 5 unavailable). An empty family output is not proof that truth selected
no breakpoint.

## Why the 13,571 truth selections are empty

10,351 tracks have successful truth association but no eligible positive
layer-owned loss (`interval_selection_status=4`, association status 1).
Another 3,220 have rejected truth association (selection status -2,
association status -1). These are not Minuit failures.

For the successful-association tracks, assign one reason per track using its
largest positive recorded eBrem-process momentum loss. This is an eligibility
diagnostic, not a redefinition of photon multiplicity or the >1% selection.
Other emissions on a track can have other reasons.

Percentages in the next table use each region's **missing-selection** count.

| Reason | Barrel, N=7,408 | Endcap, N=6,163 | Combined count |
|---|---:|---:|---:|
| Outermost accepted hit | 2,641 (35.65%) | 86 (1.40%) | 2,727 |
| Unassigned material | 1,517 (20.48%) | 4,230 (68.64%) | 5,747 |
| Owner layer has no accepted hit | 932 (12.58%) | 214 (3.47%) | 1,146 |
| Shared material, no unique owner | 476 (6.43%) | 225 (3.65%) | 701 |
| Truth association rejected | 1,842 (24.87%) | 1,378 (22.36%) | 3,220 |
| Geometry/material consistency failure | 0 | 30 (0.49%) | 30 |
| Total | 7,408 | 6,163 | 13,571 |

All 2,641 barrel dominant-outermost cases have OTK ownership (system 5).
The endcap dominant-outermost cases have system 6 (84), TPC/system 4 (1),
or system 3 (1).

### Association rejections

| Recorded validation failure | Barrel | Endcap | Total |
|---|---:|---:|---:|
| Incomplete hit provenance | 1,091 | 737 | 1,828 |
| No provenance-bearing associated SimTrackerHit | 612 | 459 | 1,071 |
| Reco hit too far from its associated exact G4 hook | 120 | 48 | 168 |
| Nonmonotonic G4 hooks in ordered track hits | 9 | 123 | 132 |
| Nonphysical interval-loss check | 10 | 11 | 21 |
| Total | 1,842 | 1,378 | 3,220 |

These are rejection messages, not an independent diagnosis of their underlying
cause. Ownership cannot be physically classified from the unavailable ownership
vectors of these failed matches. Some interval-loss checks report a loss larger
than the starting momentum; one reports a tiny negative loss near roundoff.
No repair or relaxation of these checks was attempted.

## What the barrel 20.5% unassigned category contains

This is **1,517/7,408 missing-selection tracks**, not 20.5% of all hard tracks.
Relative to all 21,409 selected barrel tracks it is **7.09%**.
Material refers to the largest recorded positive eBrem-process loss.

| Recorded material | Tracks | Fraction of 1,517 |
|---|---:|---:|
| TPC outer wall | 1,125 | 74.16% |
| World volume (`/world_volume_1`) | 211 | 13.91% |
| TPC endplate | 84 | 5.54% |
| Other TPC volumes | 81 | 5.34% |
| TPC readout | 8 | 0.53% |
| OTK envelope/outside recognized module | 8 | 0.53% |

The 81 other TPC volumes are TPCLog_0 (54), outer grip (14), inner grip (12)
and TPCSensitiveVolLog_11 (1). The physical interpretation of the world-volume
records has not been investigated; do not call them detector material without
further evidence.

For comparison, the 4,230 endcap unassigned cases are TPC endplate (3,081),
TPC readout (427), ITK endcap (345), OTK endcap (251), world volume (117),
other TPC volumes (7), one beam-pipe section and one TPC outer-wall case.
The endcap excess is predominantly outside the current audited ownership-map
coverage, not evidence of an optimizer missing a known breakpoint.

## Ownership is separate from breakpoint eligibility

Checked against `Reconstruction/RecBreakpoint/src/TruthLayerOwnership.cpp`,
`TruthBreakpointOwnership.cpp` and truth selection in `RecBreakpoint.cpp`.

- Implemented effective rules are **beam pipe -> VXD0** (rule 1) and
  **TPC inner wall -> TPC row0** (rule 2). They recognize specific audited barrel
  paths, not arbitrary passive material. Exact sensitive provenance wins.
- **TPC outer wall -> OTK is not implemented.** It was discussed as reasonable
  truth bookkeeping, potentially without activating a fit breakpoint, but no
  such change was authorized or made in this diagnostic.
- An existing owner label does not guarantee an eligible breakpoint: an exact
  accepted hit must be available and unambiguous, and the **outermost accepted
  hit is excluded** by the current guard.
- Existing eligible ownership results are actually consumed by tracking:
  truth mode chooses the largest positive eligible layer-owned loss and passes
  its hit index to the fitter. They are not all diagnostic-only labels.
- No selected breakpoint therefore does **not** mean no identifiable layer,
  no physical eBrem, or a failed loss minimizer. Conversely, an unassigned
  material label does not prove the material can have no effective owner.

## Comparison with the earlier barrel-gun diagnostic

Earlier study: 500 frozen barrel-gun flat tuples and **5,323 tracks with no
selected breakpoint, all with successful truth association**. The remembered
59% is **3,165/5,323 = 59.46% with ANY outermost-hit emission**, not a combined
TPC-wall/outermost/VXD percentage.

| Any positive recorded emission has this property | Earlier barrel-gun | Current Higgs barrel | Current Higgs endcap |
|---|---:|---:|---:|
| Outermost accepted hit | 3,165 | 3,082 | 257 |
| TPC outer-wall material | 1,467 | 1,356 | 3 |
| VXD0 owner | 28 | 59 | 62 |
| Union, removing overlaps | 4,318 | 4,137 | 319 |

These rows overlap; never add them. The union in the old sample is 81.12%.
For a comparable denominator excluding rejected truth associations, the current
Higgs barrel sample has **5,566** missing selections: outermost-hit cases are
**55.37%**, and the boundary union is **74.33%**. Including all 7,408 barrel
missing selections, the union is **55.85%**. In the endcap the union is only
319/4,785 = 6.67% of successful-association missing selections, or 5.18% of
all 6,163 missing selections. Do not transfer the old barrel fraction to the
combined Higgs population.

Neither ownership eligibility nor these categories establish whether a loss
affects IP performance. No claim that all boundary cases are harmless follows.

## Evidence and reproducibility

Analysis directory (uncommitted generated artifacts):
`TrackingPerformanceStudies/breakpoint_charged_ecal_no_ebrem_20261103/`.

- `hard_unresolved_tracks.csv`, `hard_unresolved_summary.json`: original selected
  track keys, truth photon-energy selection and 249 source-file paths.
- `hard_unresolved_breakpoint_availability.csv` / `.json`: per-track truth
  selection and actual `000`/`100` output availability.
- `hard_unresolved_missing_breakpoint_reasons.csv` / `.json`: exclusive
  largest-loss reason, exact association rejection messages and boundary flags.
- `hard_unresolved_missing_breakpoint_emissions.csv`: positive-loss emission
  eligibility, owner, material path, system/layer and hit index.
- Earlier comparison: `TrackingPerformanceStudies/ecal_refit_vs_simple_comparisons_20261029/`
  files `electrononly_reason_summary.json`, `electrononly_reasons.md`,
  `electrononly_reason_rows.csv` and `electrononly_emission_reasons.csv`.
- Existing effective-rule implementation/deployment record:
  `agents_record/2026-10-26-effective-boundary-truth-owners.md`.

Fresh ROOT branch reads matched cached event/track/MC keys. Source file size
and modification time were checked unchanged across each read. Row uniqueness,
track-count totals, vector alignment and absence of eligible positive losses
on successful-association missing selections were asserted. No original tuple
was modified. Counts are a selected-sample diagnostic, not general detector or
breakpoint-finding efficiency.
