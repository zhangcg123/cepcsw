# Selected-particle first ECAL entrance in GsfG4MaterialSteps

User authorized adding truth ECAL entrance location/momentum to the existing
simulation collection, after clarifying that the recorder supports selected
primary species and multiple generator particles. This is a simulation-truth
extension, not a tracking-algorithm or calorimeter-resolution change.

## Contract

Schema 2 adds two int16 fields to G4MaterialStep:

- firstEcalEntry: first outside-to-ECAL volume crossing of this selected track.
  Read its existing postPosition (mm) and postMomentum (GeV), with trackID,
  parentID, pdg, stepNumber and postGlobalTime retaining normal meanings.
- ecalEntryOnly: the boundary step failed the old material-step selection and
  was saved only to retain entrance truth. Exclude it from tracker material,
  eBrem loss and ownership calculations.

Uses the existing photon recorder's ECAL touchable-ancestry test. A track born
inside ECAL has no crossing record; a track that never reaches ECAL has none.
IDs are event-local. At most one extra record per selected particle. If the
boundary already passed material selection it is flagged in place, not copied.
Material selection and photon birth/step selection are unchanged. Extra step
indices can shift: follow persisted relations, never assume old indices.

WriteGsfTruthEventData, GsfTruthPDGs and GsfTruthPrimaryOnly retain control. No
new configurable or maintained card change. Existing keep-all sim -> rec
propagates the fields. Existing files cannot acquire omitted true entrance
information without re-simulation. No electron-entry flat-tuple branches were
added in this change; those remain a separate downstream export task.

Shared TruthBHLossEventData::prepare skips entry-only records before indexing
material steps. This is the only numerical-reader change, shared by breakpoint
and GSF. No fitter, ownership rule, material calculator or KF algebra changed.
The sim hook-midpoint search also excludes entry-only records. Existing full
step/provenance relations remain exact.

## Validation

EL9/LCG105 builds passed for DetSimAna, RecBreakpoint, RecBreakpointTruthMatchTest
and RecGsfTracking. Generated-header precedence was fixed in the two consumers
and the match test so an old installed schema cannot shadow the current build.
The build reports the existing ROOT standard and filesystem clock-skew warnings;
the new APIs/records were explicitly exercised at runtime.

All 11 RecBreakpoint CTests pass. The match test adds a synthetic ECAL-only
step with large material/energy loss and verifies unchanged emissions, selected
breakpoint loss and hook retained fractions.

Local tests in /tmp/primary-ecal-entry-xMoA6T (no batch submission):

| Sample | Events | ECAL entries | Extra step records |
|---|---:|---:|---:|
| e-, 10 GeV, theta85 | 8 | 8 | 6 |
| mu-, 10 GeV, theta20 | 2 | 2 | 0 |
| e- plus e+, 10 GeV each, theta85 | 2 | 4 | 4 |
| stopping e-, momentum 0.001 GeV | 1 | 0 | 0 |

Barrel single-electron entry radii: 1830.1203--1838.5739 mm. Muon endcap entries
are z=2930 mm. Both primary IDs (1 and 2) appear separately in each pair event;
the entries need not be ordered by ID. This tests multi-primary bookkeeping,
not a full StdHep campaign.

Same-seed old-installed versus new-build electron runs: the old 5,116 material
steps match exactly after excluding the six extra entry-only steps. Every old
material field, including both volume copy numbers, matches in this sample.
67 photon birth/path and MCParticle branches also match exactly. Photon-parent
relations resolve to the recorded parent track/step after indices shift.
Compressed file sizes: 8,197,733 -> 8,199,486 bytes (+1,753 bytes).

Native PODIO readback with new libraries reads both the old schema-1 file
(8 events, 5,116 steps, flags default zero) and new schema-2 file
(8 events, 5,122 steps, 8 entries), including valid photon-parent relations.
Old zero flags mean unavailable recording, not physical non-arrival.

An initial Python validation error was branch-name normalization in the checker,
not an old runtime being loaded; explicit full-path branch selection fixed it.
The validator excludes PARAMETERS objects which uproot cannot interpret.

## Deployment / limits

Built and locally tested, NOT installed. InstallArea and user batch controls
remain unchanged. New simulation was run with the build lib directory and
generated ROOT includes explicitly prepended. No existing ROOT input was
overwritten. Existing tracking flat tuples were not regenerated.

No full rec/refit population rerun or real StdHep test; current-geometry volume
naming only. Known neutral-linearization and minimizer-acceptance problems are
unrelated and remain unresolved. The calorimeter error formula was not changed.
