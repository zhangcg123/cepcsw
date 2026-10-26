# Effective beam-pipe and TPC-inner-wall breakpoint ownership

User requested beam-pipe eBrem -> VXD layer 0 and TPC-inner-wall eBrem ->
first TPC layer, following the no-eligible-owner population audit. Implemented
inside RecBreakpoint only. No simulation/rec producer, GSF implementation,
shared KF, run card, or installation was changed.

## Contract

`TruthLayerOwnership.cpp` recognizes the audited barrel `BeamPipe_IPInnerTube_*`
and `TPCinnerWallLog_*` placements/material slices. The effective targets are
system=1/layer=0 and system=4/layer=0 (TPC row 0), respectively. Exact sensitive
provenance wins; otherwise retain the existing geometry/G4 radiation-length
check. Match a subsequent same-primary simulated traversal of that exact layer,
without an intervening different sensor assembly, then its exact accepted reco
hit. Do not substitute another layer if row/layer 0 is absent. Missing,
ambiguous and outermost targets remain ineligible. Original physical positions,
material paths, steps, pre-momenta and losses remain unchanged.

The owner key explicitly encodes the rule. New vector
`truth_ebrem_effective_owner_rule`: 0 unchanged/physical, 1 beam pipe -> VXD0,
2 TPC inner wall -> TPC row0. Owner status 2 now includes these effective
targets; rule status and breakpoint eligibility must be read separately.
No shared TruthBHLossEventData header/layout change was necessary. The
package's identification consumer shares the mapper and tuple helper and thus
will see these rules after rebuild; no independent identification algorithm or
training change was made. Unknown/endcap pipe sections and TPC outer wall
remain unassigned.

The fitter still applies loss at the owner hit, before outward propagation to
the next hit, with the existing inverse-direction convention. These rules
relocate a pre-layer physical loss outward to an effective breakpoint. They
are NOT a claim of exact loss-position equivalence or validated pT recovery.

## Validation (not installed)

Built RecBreakpoint and its two ownership tests in the project LCG105 build.
All 11 RecBreakpoint CTests pass. Expanded tests cover exact rule recognition,
unchanged physical ownership, retained truth, missing target/no substitution,
outermost target, returning emission, intervening sensor and geometry mismatch.
The first fixture revision compared a stored float to double literal 0.1;
corrected the assertion to compare the stored G4 value directly. No numerical
production change was needed for that test correction.

Fresh installed-before/new-library-after runs on seed1 events
11,16,17,26,38,57,79,90,161, using the same frozen barrel-1 production card,
111 constraints, PreReclustering/ElectronPhoton, verbose dumps, EvtMax=162.
Only selected events and output paths overridden. New runtime used a temporary
LD_LIBRARY_PATH directory containing just a symlink to new libRecBreakpoint.so;
all other installed runtime components were retained. New persisted branch
verified that the new library executed. No Condor jobs submitted.

Outputs/cards/logs and comparison script:
`/tmp/bp-effective-owner-5Ap8b5/` (`before.root`, `after.root`, `comparison.json`,
`compare.py`, `before.py`, `after.py`, `ctest.log`). Temporary outputs may expire;
the numerical summary and source fixtures below are the durable evidence.

Installed library SHA256 before AND after:
`5094b852748d01cf84bda31f18bba26900b3fe93d68a46d9894d14b782e01fbf`.
New build library SHA256:
`5cd304c27020a9c4390b2cf9ac48b2dcd566f2b41679ee13127821d5925ccca1`.
Build emitted existing ROOT/C++/PCM warnings and filesystem clock-skew warnings;
changed translation units compiled, targets linked, tests and new-schema runtime
checks completed successfully.

All 9 rows preserve physical truth vectors and truth/KF pT exactly. Four
unaffected rows (11,16,38,79) reproduce all 11 compared pT endpoint fields
exactly (44 field comparisons, NaN-aware). All non-effective per-emission owner
keys/statuses/hit indices also reproduce exactly. Tuple ownership validator
passes all 9 rows. Event 17 is affected by TPC-inner-wall ownership, not an
unchanged control; event 11 has no recorded eBrem in this particular seed.

Residual = 100*(reco pT/truth pT - 1), in percent:

| Seed1 event | Old -> new breakpoint | Free000 before | Free000 after | Free111 before | Free111 after |
|---|---|---:|---:|---:|---:|
| 11 | none -> none | -0.206015 | -0.206015 | -0.401622 | -0.401622 |
| 16 | 8 -> 8 | -0.211041 | -0.211041 | -0.131861 | -0.131861 |
| 17 | none -> 9 (TPC0) | -0.193091 | +0.086781 | -0.206790 | -0.192497 |
| 26 | none -> 0 (VXD0) | -1.493418 | -1.493418 | -1.560019 | -1.597144 |
| 38 | 5 -> 5 | -2.695607 | -2.695607 | -0.494561 | -0.494561 |
| 57 | none -> 0 (VXD0) | -2.801654 | -2.801654 | -2.630770 | -0.854592 |
| 79 | 5 -> 5 | +22.169638 | +22.169638 | +2.899366 | +2.899366 |
| 90 | none -> 9 (TPC0) | -1.565073 | +0.238160 | -1.782631 | +1.513172 |
| 161 | none -> 10 (TPC0) | -0.883239 | +0.154300 | -0.825927 | +0.228559 |

Both beam-pipe cases acquire eligible hit0 but unconstrained free-loss pT
does not move. Do not interpret increased eligibility as guaranteed recovery.
This small targeted test is not population/endcap validation.

## Deployment / continuation

New build exists, project-default InstallArea is unchanged. Confirm safe timing
before installing for batch use. Existing rec files already contain the needed
G4 provenance; after installation, rerun only STAGES=breakpoint with those rec
files as input. No sim/trk/calodigi/rec regeneration is required. Recompute owner
population tables after rerunning; prior tables describe the old ownership rules.
The independent neutral/free-loss acceptance issues recorded in the separate
2026-10-26 audit handoff remain unresolved and unchanged by this patch.

## Installed after explicit approval

User confirmed no active jobs and authorized immediate installation. Source
checkpoint c37574a was pushed to origin/breakpoint. Package-specific CMake
installation was staged under `/tmp/bp-effective-owner-5Ap8b5/staged`, then
three changed files were replaced by same-directory rename:
`lib/libRecBreakpoint.so`, `lib/RecBreakpoint.confdb`, and
`python/RecBreakpoint/RecBreakpointConf.py`. The latter two differ only in their
generated timestamps; property defaults/allowed values are unchanged. Previous
files are under `/tmp/bp-effective-owner-5Ap8b5/previous`, with `install.sh` and
`install.log` recording the operation. No other package was installed.

Installed library hash now equals the new build hash above. Direct
`source setup.sh; gaudirun.py installed.py`, with no temporary library override,
reran seed1 events 26,38,90. All 856 breakpoint branches and all 57 neutral_pfos
branches reproduce the new-library reference exactly on all three rows
(NaN-aware). The persisted ownership validator also passes. Cards/output/checker
are in the same temporary directory. Installation is complete; the preceding
"not installed" sections describe the pre-approval validation stage only.

Outgoing AGENTS snapshot:
`agents_record/2026-10-26-agents-before-effective-boundary-owners.md`, verified
byte-identical before editing. Global status/laws/commands retained unchanged;
only current focus replaced. No history directory migration was needed.
