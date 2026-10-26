# Track-matched truth ECAL entrance in the breakpoint flat tuple

## Scope and contract

User approved exporting the simulation entrance fields into the downstream
breakpoint flat tuple. This extends the recorder in commit bc6a577; it changes
no fit, material ownership, constraint, energy-error formula, or photon array.

`RecoAssociationTuple::TrackRecoTuple` always books nine scalar branches:
`truth_ecal_entry_status`, `truth_ecal_entry_g4_track_id`,
`truth_ecal_entry_step_number`, `truth_ecal_entry_{x,y,z}` (mm), and
`truth_ecal_entry_{px,py,pz}` (GeV). They follow the row's unique best matched
MCParticle through SimTrackerHit provenance to a unique Geant4 track ID, then
the flagged first ECAL entry. No gun index or G4 ID 1 assumption. The parent ID
in the existing event-level photon arrays can be joined to this ID within the
same event, but this does not imply any reconstructed cluster truth match.

Statuses: 1 valid; 0 known ID without a recorded entry; -1 no unique matched
MCParticle; -2 no provenance identity; -3 ambiguous IDs/duplicate entries/PDG
inconsistency; -4 nonfinite kinematics; -5 unavailable input collections.
Invalid/unavailable rows keep NaN position/momentum and step -1. A uniquely
resolved ID is retained. Status 0 cannot distinguish legacy simulation,
unrecorded species, and no physical ECAL arrival.

The standard card now reads steps, links, and their SimTrackerHit collections
even without truth breakpoint selection. The batch helper already preserves
them through trk/calodigi/rec; no batch-helper change is needed.

## Validation

Local artifacts: `/tmp/bp-truth-ecal-kh51bF/`. No batch jobs submitted.
Build-directory libraries and generated headers were selected explicitly;
the installed runtime was not changed.

- Built RecBreakpoint and the new RecBreakpointTruthEcalTupleTest.
- All 12 RecBreakpoint CTests pass with the build-directory libraries.
  The new test covers two particles/IDs, shuffled steps, missing input and
  identity, duplicate entries, ambiguous IDs, tied MC matches, invalid values,
  legacy no-flag data, and reset between rows.
- The test caught a default PODIO object being available even when empty;
  the implementation now uses an explicit null step sentinel.
- An initial CTest invocation accidentally used installed schema-1 libraries
  and failed missing schema-2 symbols. Repeating with build lib first passes;
  deployment must include the matching GsfTruthEventData library/dictionary.
- Eight new-recorder electron events from
  `/tmp/primary-ecal-entry-xMoA6T/new-electron.root` were processed locally
  through trk, calodigi, rec, and breakpoint using maintained templates.
  Eight track rows have status 1. All six entrance coordinates/momenta and
  step/track IDs match simulation and rec exactly (`check_entries.py`).
- One further local run with Manual selection, empty intervals and TruthOverride
  off still saves a valid entry (status 1), testing independence from truth-based
  breakpoint selection and override.
- Final-source events 11,16,17 were rerun against the prior effective-owner
  reference `/tmp/bp-effective-owner-5Ap8b5/after.root`: all 856 existing main
  branches and 57 photon/neutral-tree branches are exactly unchanged, with NaNs
  compared as missing values. New entrance status is 0 in each legacy event.
  Details are in `compare-final.log`. `installed.root` in that directory only
  contains events 26,38,90, so it is not the reference for 11,16,17.
- Repeated the final-source regression on all nine events
  11,16,17,26,38,57,79,90,161: all 856 existing main branches and all 57
  event-tree branches are exactly unchanged against `after.root`; new entry
  statuses are all 0. See `regression-nine-final.py`, verbose log, and
  `compare-nine-final.log`. No tracking-result changes observed.
- Batch tests: 43/44 pass. The existing submission-default test expects
  trk,calodigi,rec,breakpoint, while the user's edited subbreakpointjobs.sh
  currently defaults to breakpoint-only. That unrelated setting was preserved.

## Deployment / data limits

Not installed. Installed libRecBreakpoint.so SHA256 remains
`5cd304c27020a9c4390b2cf9ac48b2dcd566f2b41679ee13127821d5925ccca1`.
Verified build libRecBreakpoint.so SHA256 is
`a7f75cd2c64b91a846df1823d69b206babe7a968b07aa55f9de9189c7ba4089b`;
build libGsfTruthEventData.so is
`64bf396fbc79ecd6a26e64949b3ebb97784f3c43eb9a3c4b65bb1ec4c4a11408`.
Both the upstream recorder/schema update and this downstream update need a
coordinated installation before production use. Old simulations cannot gain
ECAL entrance truth by rerunning rec or breakpoint alone. Fresh simulation is
required; then ordinary downstream passthrough and re-tuplising suffice.
No StdHep population validation or production batch was performed.
