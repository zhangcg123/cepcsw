# Install pre-PFO exports and conditional charged constraints

User requested installation before regenerating rec and breakpoint flat tuples,
and explicitly confirmed no active jobs. No local reconstruction workers were
found. Queue tools were not on the checking shell's PATH; user confirmation
provided job coordination. No batch jobs were submitted or old tuples removed.

Deployed verified source through breakpoint commit 5ad01f2 into the project
default InstallArea/x86_64-el9-gcc11-opt. Both RecBreakpoint and CrystalCaloRec
were rebuilt. All 11 breakpoint CTests passed again. No fitter/PFA source,
run-card or user submission settings changed during deployment.

Package-specific CMake install scripts staged into
/tmp/bp-pfa-deploy-20261026.Zflv9n/staged. Only eight changed files were replaced,
by same-directory rename rather than overwriting a mapped library:

```
lib/libRecBreakpoint.so
lib/libCrystalCaloRec.so
lib/RecBreakpoint.confdb
python/RecBreakpoint/RecBreakpointConf.py
python/RecPFACyber/CrystalCaloRecConf.py
include/CyberPFAlg.h
include/Tools/AlgorithmManager.h
include/Tools/OutputCreator.h
```

Previous versions are under /tmp/bp-pfa-deploy-20261026.Zflv9n/previous;
install.sh and install.log identify exact targets. CrystalCaloRecLib, component
identities, GSF/shared tracking libraries and unrelated headers were unchanged.

Installed SHA256:

```
libRecBreakpoint.so
5094b852748d01cf84bda31f18bba26900b3fe93d68a46d9894d14b782e01fbf
libCrystalCaloRec.so
4e033497189ea8d02f072292843e25556f030fff93ee0e22c738be705c14f6fd
```

## Default-runtime gate

Sourced setup.sh, then invoked gaudirun.py directly, without build/run.
Python configuration modules and ROOT dynamic-library resolution point into
InstallArea. EcalConstraintInput defaults to PreReclustering;
ChargedEcalEnergyMode remains ElectronOnly. To request conditional sum routing,
explicitly select ElectronPhoton; all backward defaults remain OFF.

Installed-runtime rec smoke regenerated seed1 entries0..17 from the saved
pre-PFA input, then breakpoint selected entries0,3,5,11,12,16,17 with 111,
ElectronPhoton, pre-PFO input, sigma_b=.001 and free fitting enabled.
Both applications completed successfully. check.py reports:

- 447 final/intermediate/pre-PFO fields match the build-tree rec exactly over
  all 18 events. Includes pre-PFO ECAL/HCAL/hit/track links and timing/merged output.
- All 855 breakpoint fields match the build-tree reference on seven rows.
- All 57 final-PFO side-tree fields match on seven event rows.

The comparison covers the affected PFA outputs, not every unrelated rec branch.
An initial all-rec-field Uproot check could not decode the existing custom
DndxTracks.hypotheses[5] representation in the reference file; it was narrowed
to the affected output graph. No corruption claim or Dndx change was made.
Cards, logs, rec.root, bp_flat.root and checker are in the deployment directory.
No population performance validation or batch smoke is claimed.

## Rerun contract

The batch worker already sources project setup and therefore uses this runtime.
Regenerate rec to obtain the new pre-PFO graph; old rec cannot gain collections
from the install alone. The rec template keeps all output collections.
Regenerate/freeze new breakpoint cards rather than reusing historical job cards.
The user submit script still has STAGES=trk,calodigi,rec and existing campaign
paths; this deployment did not append breakpoint or overwrite existing files.
For automatic charged interpretation use BP_CHARGED_ECAL_ENERGY_MODE=ElectronPhoton
alongside the desired 111 gates. Default energy mode itself was not changed.

Current AGENTS deployment status is updated; the complete outgoing version is
preserved in 2026-10-26-agents-before-pre-pfo-install.md. Earlier NOT INSTALLED
records remain historically correct but no longer describe the default runtime.
