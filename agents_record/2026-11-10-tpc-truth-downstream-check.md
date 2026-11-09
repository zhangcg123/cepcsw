# Real-track downstream checks after TPC truth fix (2026-11-10)

The user asked for actual GSF/breakpoint results after the truth-bookkeeping
fix, beyond the initial shared-reader tests at simulated-hit positions.
This is validation only: no implementation/build/install/source change and
no existing production REC file was changed. All library hashes captured
before these tests remain identical. Original simulation, tracking and PFA
code is protected by the user's explicit instruction.

## Inputs and test preparation

Use the fixed standalone SIM outputs from generator-source job4 events1,17
under tpc-provenance-owner-fix-20261110/. Each contains one event; downstream
fit event_index0 maps to source1 or17. These are genuine new simulations
from that generator source, not bit-identical replays of old combined-chain
REC events. Source1/17 are the two small cases that reproduced reversed
ranges before the producer fix and now have complete TPC links.

The SIM test files were deliberately trimmed to tracker/truth output. They
omit EventHeader and coil/muon hit collections. An initial standalone tracking
attempt failed at the missing header; those attempts are preserved under
trk/attempt-missing-header/. A new metadata EventHeader was added only to a
scratch copy (source event number, run0, timestamp0, weight1) to define seeding.
No original physics field/truth link was edited. The maintained tracker card
was copied into each test directory; unused muon digitization was omitted,
while detector digitizers and tracking algorithms/fit settings were unchanged.
EventHeader, low-pT/space-point TPC and brems-photon truth inputs were explicitly
requested. Both tracker reconstructions then completed successfully.

GSF consumes the genuine new CompleteTracks and associations. It uses the
maintained reverse/fresh inward/beam/BH settings with pair selection, depth0,
truth oracle off and passive recorder on. All component/split dumps are on.
Breakpoint runs its actual Truth-location workflow with the selected pair,
ordinary RTS/free-loss family and beam measurements. ECAL, neutral constraints,
location scans, truth-loss override and diffuse experiment are off. Its
mandatory passive tuple inputs receive empty EcalCluster and CyberPFOPID
collections in a separate tracker fixture. No PFA algorithm is run. This is
a tracker/beam test, not the full ECAL-constrained free-loss/110 workflow.

## Native results

| Generator source event | Original input track | G4 track | GSF exact-hook intervals | Max hook distance mm | GSF published | BP status |
| ---: | ---: | ---: | ---: | ---: | --- | ---: |
| 1 |2|5|60|0.532431|yes|1|
| 1 |3|3|232|1.85076|yes|1|
| 17 |26|4|162|1.29009|yes|1|
| 17 |32|3|223|1.78202|no, first-hit fit rejection|1|

Both selectors return status1 and select the same two inputs in each file.
All four GSF truth matches complete before fitting. All four breakpoint Truth
selection paths complete and publish finite ordinary RTS results. No malformed
status28/30 or endpoint-distance warning recurs in these cases. All four BP
selected interval lists are empty: no eligible owned eBrem breakpoint was
selected, so these are null/no-breakpoint fits. They establish that the prior
strict provenance block is gone and the workflow proceeds to real fitting;
they do not validate nontrivial breakpoint energy-loss recovery.

BP ordinary RTS pT values in input order are15.1044802,43.5470489,
24.6995957,18.2726657GeV. These are diagnostics, not a resolution comparison.

## Separate GSF initialization failure

Source17/input32 has224 matched measurements beginning with TPC rows at
r637.5,642.5,647.5mm, rather than inner silicon hits. Its outward prefit uses
indices0,1,2 and returns omega=-0.00769407861, corresponding to pT0.1168918GeV
at3T. Every one of the ten forward candidates is rejected at hit0; no output
endpoint is published. The log labels this as processed track2; original
input/output provenance maps identify it as CompleteTracks input32.

The passive recorder had already matched its223 exact-hook intervals. Its
final passive status remains-4 because the fit was not published, not because
the G4 provenance is invalid. No source-level seed diagnosis/fix is part of
this task; short-lever-arm TPC seeding is a candidate cause, not a validation
claim about every similar track.

A same-code recorder-off control for source17 gives exactly the same failure
and output mapping [26]. All206 non-material flat fields, including complete
component/lineage data, are exactly equal to the recorder-on run. This directly
separates the initialization/first-hit failure from truth recording.
Production-electron/final-mixture/lineage mapping checks pass for both files.

## Limits and artifacts

No claim that all GSF fits now succeed is supported:3/4 endpoints were
published. The specific malformed-range issue is absent in all four real-track
truth checks. Existing REC files remain unchanged with their old stored faults;
the separate merged-hit5mm case was neither repaired nor reproduced here.
No ECAL-constrained scan, nontrivial breakpoint loss recovery, population
resolution or momentum-safety validation is established by these two events.

Uncommitted artifacts:
build.gsf-selection-dev/validation/tpc-truth-downstream-20261110/ contains
tracker cards/logs/outputs, isolated metadata/empty-calo fixtures, GSF/BP
cards/logs/flat tuples, recorder-off control, summary.json, passivity.json,
selection-check logs, validation.json and unchanged-library hashes.
All seven successful native tracker/fit/control jobs completed.
