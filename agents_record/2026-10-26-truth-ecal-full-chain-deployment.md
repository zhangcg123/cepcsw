# Truth ECAL entrance: installation and full-chain batch preparation

User authorized updating the submission script and installing the verified
new development. No batch jobs were submitted. Queue inspection was attempted
but hep_q cannot import htcondor on this host; installation follows the user's
explicit authorization after deletion of the old tuples.

## Maintained campaign

- `subbreakpointjobs.sh` now defaults to sim,trk,calodigi,rec,breakpoint.
- Output remains the user's `sim_large_barrel_20261026`.
- Input defaults to `sim_large_barrel_20261018`, is unused by the full chain,
  and differs from output as required by the helper. For partial chains set
  the actual existing predecessor input path explicitly.
- Retained settings: barrel e-, seeds 1--500, 200 events/job, 5000 MB, group
  higgs; beam/charged/neutral 111; ElectronPhoton charged mode and
  PreReclustering inputs. The gun's hard-coded 10--50 GeV range remains in
  sim.py.bk; barrel theta is 40--85 degrees.
- The worker retains sim/rec/flat, removes its own trk only after verified
  calodigi, and its own calodigi only after verified rec. External inputs and
  outputs of failed downstream stages are not blindly deleted.
- Flat verification now requires the nine new `truth_ecal_entry_*` branches.
  It does not require status 1, since a particle may have no recorded entrance.
- README/default tests synchronized; all 44 batch tests pass, including the
  new missing-entry-branch rejection and full-chain default/reuse-SIM override.

## Installation

Built DetSimAna, RecBreakpoint and RecGsfTracking. Installed the four package
directories GsfTruthEventData, DetSimAna, RecGsfTracking, RecBreakpoint using
their CMake install scripts into the project-default
`InstallArea/x86_64-el9-gcc11-opt`. The GSF install includes the shared-reader
schema adaptation/entry-only exclusion from the earlier recorder change;
no new GSF physics change or PFA source edit was made here.

Installed/build hashes agree:

- libRecBreakpoint.so:
  `a7f75cd2c64b91a846df1823d69b206babe7a968b07aa55f9de9189c7ba4089b`
- libGsfTruthEventData.so:
  `64bf396fbc79ecd6a26e64949b3ebb97784f3c43eb9a3c4b65bb1ec4c4a11408`
- libDetSimAna.so:
  `aea67107cfd8b8f78fa35978b7dd2bd3639808143c7f482711d141cd294a2674`

Previous main libraries, schema headers/dictionary and package configurations
were backed up in `/tmp/bp-entry-install-bsscYa/previous-runtime.tar.gz`.
Logs and focused local worker artifacts are in that directory. All 12 C++
tests pass with plain `source setup.sh`, without build-library overrides.
This supersedes the uninstalled status of the preceding two truth-entry records.

## Installed worker smoke test

Prepared one two-event job with the updated default STAGES and ran
`dump_breakpoint.sh .../smoke/runcards/barrel-1/job.json` locally. No build
library/include overrides. All five stages finished; the flat tuple has two
successful ordinary rows, zero invalid truth-override rows, and two valid
truth ECAL entrances. Native PODIO comparison confirms all entrance coordinates,
momenta and step IDs match SIM, REC and flat exactly. `completed.json` confirms
both intermediate cleanup operations succeeded: TRK/calodigi absent, SIM/REC/flat
retained. The production output directory was not created during validation.
The existing HcalDigi tile-response-map warning still falls back to a uniform
map; this work does not alter calorimeter calibration.

## Use

From the project root, with no unintended environment overrides:

```bash
./subbreakpointjobs.sh
```

This prepares and submits 500 full-chain jobs. For inspection only use DRY_RUN=1
and a separate output directory; prepared cards are exclusive and are not
silently overwritten on a subsequent preparation.
