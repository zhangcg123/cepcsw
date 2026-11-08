# Complete-event reconstruction check after compact truth

## Request and event counts

The user asked how many events were actually tested, and whether their
reconstruction could be reproduced in total. The earlier record established
truth/tuple preservation; it did not compare both complete detector-to-REC
outputs. This follow-up addresses that gap using the already installed code.

- Four stored REC events: 0,11,16,17, for old-executable versus new-executable
  and full-record versus compact-record breakpoint regression. The exact
  earlier 1,015/1,067-branch claims remain limited to that fixed-REC test.
- Eighteen input REC events were converted to compact truth to preserve source
  event indices. That was NOT eighteen fully validated events.
- Two generator events: 0,1, re-simulated with full and compact truth, then
  independently reconstructed end to end in this follow-up. Event 0 overlaps
  the stored-event study; do not add these as six independent generator events.
- A third reconstruction of those SAME two events starts from the identical
  compact SIM file, same card/seed, as an ordinary repeatability control. No
  additional generator events or population validation were added.

All work remains on `breakpoint-delayed-neutral`. No source, shared package,
PFA/PID algorithm, fit objective, bounds, loss map, event selector or installed
library changed. No build/install or batch submission was performed. Temporary
native comparison tools were compiled outside the repository. No migration or
branch/remote change occurred.

## Execution and comparison scope

Artifacts: `/tmp/truth-recorder-20261109/clean/`.

The two clean SIM files from the implementation gate are still the inputs:
`full/sim.root` and `compact/sim.root`. Same generator events, full detector,
seed 1; their 324 non-truth detector leaves already agree exactly. The full
case now ran tracker -> calorimeter digitization -> PFA/REC -> RecBreakpoint.
The compact case's existing tracker/calo/REC chain was retained, and its
RecBreakpoint run was repeated with the DEFAULT free-loss, beam, charged ECAL
and 110 location-scan steering, production-electron selection and verbose
dumps. Thus the two default-fit files have identical physics steering:

- `full/default-flat.root`, log `full/default-fit.log`;
- `compact/default-flat.root`, log `compact/default-fit.log`.

The ordinary repeat uses `repeat-compact/trk.py`, `calodigi.py`, `rec.py`, reading
THE SAME `compact/sim.root`, with identical settings and new output paths.
Only run-card input/output paths differ between the corresponding cases.

The native comparison reads PODIO Frames and compares collection inventories,
IDs, types, subset flags, event parameters, every persisted Data member,
relations and vector members. It serializes the actual typed buffers with the
ROOT dictionary, so fixed-array objects (including DndxTracks.hypotheses) are
covered even where uproot cannot decode them. Positive self-comparison gives
zero differences across the 80 REC collections; a one-ULP float negative
control is detected. Intentional Gsf-truth layout changes are excluded from
this physical-reconstruction comparison and remain covered by the separate
truth/checkpoint gate.

## Results: exact values, different ordering, residual PID variation

The native raw collection comparisons are NOT fully equal:

| Stage | Non-truth collections | Raw difference groups |
|---|---:|---:|
| Tracker | 49 | 31 |
| Calorimeter digitization | 61 | 37 |
| REC | 80 | 59 |

A group is a collection's data buffer, relation vector or vector-member
buffer, not an event or physics failure count. These raw differences require
object matching before interpreting physical effects. Output files are not
claimed to be identical.

Matching tracks by their actual `(TrackerHit collectionID,index)` membership
establishes:

| Collection | Event 0 | Event 1 |
|---|---:|---:|
| ClupatraSegmentTracks | 49/49 identical fit data | 17/17 |
| ClupatraTracks | 19/19 | 27/27 |
| CompleteTracks | 15/15 | 25/25 |

All matched track scalar fit data, helix states, reference positions and complete
state covariances agree EXACTLY, with no unmatched tracks or changed same-hit
fits. CompleteTracks event 0 retains its ordering; three CompleteTracks indices
move in event 1. Clupatra segment/track ordering changes more broadly. The
same checks on REC confirm the same fit-state equality after PFA.

The identical-input tracker repeat also produces ordering differences while
preserving all these matched fit values. This establishes that byte-order
repeatability is already limited when the truth format and SIM file are held
fixed. A specific Clupatra sorting/container root cause was NOT established
and no ordering was changed to force a match.

For PFOs, compare full four-momentum, mass/charge/type, covariance/reference,
PID, and linked physical content. Track links are resolved by hit membership;
clusters include all scalar/covariance/shape/subdetector-energy data, nested
clusters and calorimeter hits with their cell IDs, energies, errors, times,
positions and types. Collections and unordered constituent lists are matched
by this physical content, not their row indices. Non-finite values receive
explicit distinct NaN/+Inf/-Inf labels before comparison.

| PFO collection | Event 0 | Event 1 |
|---|---|---|
| CyberPFO | exact physical objects and relations after permutation | exact |
| CyberPFOPreReclustering | exact | exact |
| CyberPFOPID | exact | one PID-hypothesis vector differs |

All PFO kinematics, calorimeter contents, physical track assignments, assigned
PDG codes and used-PID references agree. The remaining final PID difference is
one electron-labelled PFO at event 1, row 9, energy 2.021437883377075 GeV:

| Reconstruction | Electron probability |
|---|---:|
| Full-history truth | 0.999550998210907 |
| Compact truth, first run | 0.9860338568687439 |
| Identical compact SIM, repeat reconstruction | 0.999550998210907 |

Other hypotheses in this same PID vector change too; the assigned PDG remains
11 and the four-momentum is unchanged. This PFO is outside the four selected
production-electron rows. The same score variation occurs in the identical-
input, identical-format repeat, so it cannot be uniquely attributed to compact
truth. The PID repeatability root cause remains unestablished. No PID/PFA code
was modified or tuned.

## Default breakpoint reproduction

Both full and compact default tuples have 40 track rows and two event rows.
Unmatched row-by-row comparisons see the preceding ordering effects: 900/953
track branches and 108/114 event branches are unchanged without matching.
This is not the relevant definition of a matched production-electron fit.

For the four selected production electrons, **952/953 branches agree exactly**,
including the matching NaN positions/statuses. The sole difference is
`collected_neutral_ecal_cluster_indices` for the event-1 electron: `[17]` versus
`[19]`. Its collected cluster's physical coordinates and energy are unchanged.
All 96 checked pT/error/pull/NLL/objective fields match. The complete
selected fitted means and covariances also agree; no t/X0 rounding difference
is needed. PID likelihoods for these selected four rows agree.

The comparison packs masked Awkward arrays before checking buffers; otherwise
unselected backing storage can falsely appear as a selected-row mismatch.
The unmasked whole-file comparisons and native typed-buffer checks do not
have this masking issue.

| Event / truth MC | KF pT | Free-loss RTS pT | Charged-ECAL free-loss pT |
|---|---:|---:|---:|
| 0 / 17 (e-) | 43.337554292540425 | NaN | NaN |
| 0 / 12 (e+) | 17.751328437629667 | 17.74819725448927 | 18.766706751708888 |
| 1 / 17 (e-) | 12.12146948192888 | 12.11107997059032 | 22.867392630744472 |
| 1 / 15 (e+) | 43.05696121085121 | 47.659342455241564 | 44.0386762651105 |

Every value above is identical in both modes. Event-0 e- retains the known
invalid-provenance failure (`interval_selection_status=-2`) in both cases;
this must not be counted as a successful finite breakpoint reconstruction.
The other selection statuses are 4,2,2. No provenance/owner guard was relaxed.

## Conclusion and evidence

On only TWO end-to-end events, the stored simulated detector data, matched
tracking fit results, PFO kinematics/constituents, and selected-electron
breakpoint numerical outputs reproduce. Complete event data is NOT yet
strictly reproducible because collection ordering and one final PID vector
vary, including in an ordinary identical-input repeat. This finding qualifies
any broad reading of the previous focused-verification statement. It does not
invalidate its exact fixed-REC comparisons.

This is not a population validation, an old-source versus new-source complete
simulation campaign, or a proof of bitwise complete-event reproduction. The
end-to-end A/B compares full-history and compact-history modes of the same
installed code. The old-executable A/B remains the earlier four fixed-REC
breakpoint comparison. Broader PID/order investigation needs a separate task;
no automatic fix, source migration or production campaign is authorized here.

Reports under the artifact directory:

- `tracker-comparison.json`, `calodigi-comparison.json`, `rec-comparison.json`;
- `track-identity-comparison.json`, `rec-track-identity.json`;
- `tracker-repeat-comparison.json`, `track-repeat-identity.json`;
- `pfo-identity-comparison.json`, `pfo-repeat-identity-comparison.json`;
- `default-flat-summary.json`, `selected-electron-flat-summary.json`;
- `selected-numeric-differences.json`.

These ROOT files, logs, temporary cards and analysis tools remain uncommitted.
The outgoing AGENTS is preserved in
[the snapshot](2026-11-09-agents-before-total-reconstruction-check.md).
All prior global laws remain unchanged.
