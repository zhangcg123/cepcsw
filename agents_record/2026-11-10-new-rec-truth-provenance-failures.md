# New REC truth-provenance failures (2026-11-10)

Read-only investigation of the three blocked tracks in the new-production
GSF/breakpoint comparison. Input is exclusively
`higgs_eeHX_ISRFSRON_REC_500jobs_200evt_20261109/rec-higgs-4.root`.
No source, card, guard, build, installation or production input was changed.

## Exact recorded failures

The original warnings were reproduced by reading native PODIO relations,
sorting each selected track's hits by radius as RecBreakpoint does, and
following reconstructed-hit associations into embedded Geant4 links.
Across events 0,1,11,16,17, all ten selected tracks, 2086 provenance-bearing
hit associations were inspected; precisely these three fail the existing
status/distance criteria. This is a focused diagnosis, not a population count.
All three affected SimTrackerHit positions equal their hook positions exactly.

| Event / input track / ordered hit | G4 track | Type | First / last / hook step | Status | Reco-to-hook distance (mm) |
| --- | ---: | ---: | --- | ---: | ---: |
| 1 / 23 / 85 | 3 | 3 | 322 / 323 / 322 | 31 | 7.789763809 |
| 1 / 30 / 9 | 5 | 3 | 459 / 170 / 169 | 28 | 0.035517507 |
| 17 / 22 / 9 | 3 | 3 | 490 / 132 / 131 | 30 | 0.615216668 |

Type 3 is the TPC pad-row producer. Status bits are set by
`Simulation/DetSimAna/src/Edm4hepWriterAnaElemTool.cpp`:
bit 0 valid first/last range; bit 1 first-step relation exists; bit 2 last-step
relation exists; bit 3 hook number exists; bit 4 hook-step relation exists.
Complete is 31. Thus 28 lacks a valid range and first-step relation; 30 lacks
a valid range although all three referenced step objects exist. Neither code
means that the hook step itself is missing.

## Invalid TPC ranges: stale accumulation across particles

`Simulation/DetSimSD/src/TimeProjectionChamberSensitiveDetector.cpp`
initializes PadRowFirstStepNumber at event start and resets it after ordinary
pad-row exit processing. The high-pT path only assigns a new first step when
that variable is negative; it does not detect a change in Geant4 track ID.
Unfinished high-pT pad-row state can therefore carry into another particle.

Native event-17 links show the boundary explicitly:

- link 855: G4 track 4, pad row 153, first/last 488/489;
- link 856: G4 track 3, pad row 1, first/last 490/132, hook 131;
- link 857: G4 track 3, pad row 2, first/last 133/134, hook 133.

The inherited first-step value precedes the track change. The later Geant4
track cannot have a contributing range running backwards from 490 to 132.
Event 1/track 30 has the same first-row pattern, 459/170, followed by the
valid second row 171/172. The records and code identify a sensitive-detector
accumulation-state defect. The exact preceding particle for the event-1
boundary was not separately traced.

This is not explained by compacting away valid hook records: both hooks
survive, their positions agree exactly with SimTrackerHit, and the range
numbers themselves are already inconsistent. The compactor requests first,
last and resolved hook steps as checkpoints; it cannot manufacture a valid
first-step number from this malformed producer state.

## Distance failure: merged electron/muon measurement

Event 1/track 23's reconstructed TPC hit index 3846 has two native
TPCTrackerHitAss relations, each weight 0.5:

- TPC SimTrackerHit 7906: MC index 12, PDG -11, the selected positron,
  z=766.826097005099 mm; its Geant4 hook matches the simulated position exactly.
- TPC SimTrackerHit 6690: MC index 377, PDG 13, nearby muon,
  z approximately 775.058 mm.

The reconstructed z is 774.5872133574912 mm, 7.7611163523922 mm from the
positron's simulated z. Transverse displacement is approximately 0.667 mm;
total distance is 7.789763808583814 mm. The hit covariance is
approximately [0.126469,-0.124991,0.123531,0,0,25] mm^2, hence sigma_z=5 mm.
This agrees with `TPCDigiAlg::writeMergedVoxelsToHit`, which averages the
constituent positions and smears with tpcZRes=_binningZ (default 5 mm), while
saving associations to both constituents. It is a merged measurement, not
an absent or incorrectly positioned electron hook.

TruthBHLossEventData keeps associations only to SimTrackerHits carrying the
embedded selected-electron provenance. The muon has no such link, so the
matcher sees one provenance-bearing association and reaches its distance
check. That check compares the digitized reconstructed hit directly with
the electron hook using an absolute 5 mm cut. It does not account for this
merged hit's covariance or its second constituent. Relaxing the distance cut
alone would not remove the physical mixed-particle measurement.

## Why all breakpoint outputs disappear

In the maintained Truth interval-selection workflow, RecBreakpoint requires
every ordered input hit to pass matchBreakpoints/matchTrack before selecting
the eligible eBrem location. Any failure throws before the ordinary/free-loss
fit and the subsequent optional 110 location scan. These are pre-fit truth
selection failures, not optimizer failures or evidence that tracker-only
fits cannot run. HitScanAll already treats truth matching as optional
diagnostic context, unlike this Truth workflow. The ordinary GSF run does
not require the same complete exact-hook match.

Potential remedies require separate agreement: correctly reset TPC
accumulation at particle boundaries; distinguish exact SimHit/hook integrity
from reconstructed measurement smearing/merging; permit scans independent
of optional truth-reference availability. No remedy is implemented here,
and merely dropping status checks or increasing the cut is not a complete fix.

## Artifacts

`build.gsf-selection-dev/validation/provenance-investigation-20261110/`:
`check_hit_provenance.py`, `hits.json`, `bad-hits.json`, `inspect.log`,
`inspect_tpc_context.py`, `tpc-context.log`. These are uncommitted analysis
artifacts. They read the completed new REC file; no simulation/refit was rerun.
