# Deploy cumulative breakpoint constraints — 2026-10-24

The user explicitly confirmed no batch jobs were running and authorized direct
local installation. Deployed verified source revision 66d8477 into the project
default InstallArea/x86_64-el9-gcc11-opt. No source/card changes or batch
submissions were performed as part of deployment.

Only RecBreakpoint files were replaced:
- lib/libRecBreakpoint.so
- lib/RecBreakpoint.components
- lib/RecBreakpoint.confdb
- python/RecBreakpoint/RecBreakpointConf.py
- python/RecBreakpoint/__init__.py

The package-specific CMake install script staged these files first. Each
installed file was replaced by rename, avoiding in-place rewriting of a mapped
library. No GSF/shared tracking libraries changed. Previous files, including
the Python package, are backed up under:
`/tmp/recbreakpoint-deploy-20261024.OCrOxk/previous/`.
Staged files are under the sibling staged/ directory.

Installed and built plugin SHA256 now agree:
072f2b0173e8140b5aeca79f97b30a02e4db650a6cb139d39ecb565dd341fde5

Previous installed hash:
72164a985e9fb7fc8df77601991bc4d9735d4450a27e84c68f01595d7e6e2df5

## Default-runtime verification

Sourced setup.sh and invoked gaudirun.py directly, NOT build.../run.
The Python configuration resolves inside InstallArea; getDefaultProperties()
confirms all three switches and the new output properties. An initial direct
getattr probe hit Gaudi's unset-instance-property behavior; the default-property
API and actual card assignment succeed.

Installed-runtime smoke: rec-barrel-2.root, zero-based event 1, switches 1/0/1.
Output /tmp/bp-installed-101-20261024.root and matching .log.
- 838 exact NaN-aware field comparisons against the same build-tree event pass.
- Cumulative schema/masks and 287 paired constrained-field checks pass.
- Charged stage stays NaN; neutral stage contains beam+neutral for both methods.
- Four native beam-boundary covariance checks pass, max error 1.67e-16.

No local tracking processes were found. Before the user confirmation, hep_q
could not query the queue because the login-node Python lacked htcondor.
The user then explicitly confirmed no batch jobs; no queue state was altered.

All three constraint defaults remain OFF. Current cards support cumulative
stages beam -> charged -> neutral. New generated batch cards use the installed
runtime via the existing project setup; regenerate historical cards containing
retired controls. No additional physics/population validation is implied.

## Preserved pre-deployment focus

## 2. Current focus

Active work on local `breakpoint` is cumulative optional-constraint publication.
Original ordinary/free-loss endpoints remain unconstrained. Three default-OFF
switches enable six RTS stage families (ordinary/free for each). Enabled
measurements accumulate in saved-stage order beam -> charged -> neutral:
all ON gives beam, beam+charged, then beam+charged+neutral. OFF stages are
NaN/empty. Each stage refits from the original seed/prior with its accumulated
measurements; it does not recycle a prior stage's posterior as a new prior.

Missing/failed additions retain the preceding accepted constraints for each
method. Applied/requested masks (beam=1, charged=2, neutral=4) expose exactly
which measurements are in each saved endpoint. Physical KF site order is
unchanged: beam, tracker/neutral breakpoint, charged ECAL. All stages reuse the
existing fitter and native transport/measurement/RTS machinery. Diffuse/truth
and original backward outputs remain unchanged; all backward defaults OFF.

Built/tested, NOT installed: eight CTests pass, all-OFF matches 6608 existing
field values on eight tracks, three hard-loss tracks yield masks 1,3,7, and
the all-three final endpoints reproduce the former combined fit exactly.
Two-switch/no-interval/free-fit-OFF tests, native covariance closure and real
batch tuple validation pass. One pre-existing batch-wrapper-default test still
fails; the user's wrapper was not changed.

No population/endcap validation claimed. Neutral linearization, charged-PFA
independence/calibration and extra per-stage CPU cost remain. Next: install
when authorized, then compare categorized performance. Contract, provenance,
test details and superseded focus:
`agents_record/2026-10-24-cumulative-constraint-stages.md`.
