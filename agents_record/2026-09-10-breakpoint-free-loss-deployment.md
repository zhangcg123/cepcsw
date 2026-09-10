# RecBreakpoint optional free-fit deployment completed

The user confirmed that no batch jobs were running, authorizing replacement of
the shared library after the integration checkpoint `9ea5958`.

## Build and installation

Used the normal CEPCSW environment and configured Release build:

```bash
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target RecBreakpoint -j4
cmake --install build.105.0.0.x86_64-el9-gcc11-opt/Reconstruction/RecBreakpoint
cmake --build build.105.0.0.x86_64-el9-gcc11-opt \
  --target RecBreakpointLikelihoodTest RecBreakpointTransportTest -j4
ctest --test-dir build.105.0.0.x86_64-el9-gcc11-opt/Reconstruction/RecBreakpoint --output-on-failure
```

The package-only installation replaced libRecBreakpoint.so, generated Python
Configurable files and the package components/confdb files. No whole-project
install or GSF source/card change was made. Dependency targets were checked by
the ordinary build. Both compiled tests passed (2/2).

Build and installed library SHA256:
`b191a8a0235fa2f971a8e248bd08045b0effb42011f8250e7cc51bc5fd072957`.
Build and installed RecBreakpointConf.py SHA256:
`39e183f01f663c81632fd3c7427d071c33af8321d62ea2c35a93e8315ff376b7`.

The normal run environment exposes FreeLossFit=false, FreeLossMaxLogLoss=1,
FreeLossMaxCallsPerStart=180, FreeLossTolerance=0.001,
FreeLossCheckLikelihoods=false and BackwardSeedScale=100.
The wrapper resolves the build-tree Configurable; its bytes match the installed
one. The event runner explicitly prepends installed lib/python paths after
environment setup and never prepends the private prototype paths.

CMake/Make reported filesystem clock skew (some generated dependency files had
future timestamps). The new likelihood helper and plugin sources compiled,
configuration regenerated, hashes matched and the runtime regression passed.
No attempt was made to change filesystem clocks or shared tracking code.

## Installed-card regression

Artifact directory:
`TrackingPerformanceStudies/breakpoint_free_loss_integration_20260910/`.
See production_build.log, production_install.log, production_ctest.log,
installed_driver_retry.log, installed_check.log and installed_verification.json.

Four local Gaudi runs used the maintained card with FreeLossFit off/on:
seed2:event68 and seed12:events11,16,17. Settings match the prior private gate:
LocalMarginal, Truth interval selection, truth-prior pair enabled, sigma0.001,
BackwardSeedScale100, FirstMiddleLast and verbose full dumps. Free-on runs also
enable all three equivalent likelihood checks. These are not Condor jobs.

All four runs completed, yielding eight rows. All132 flat fields agree exactly
for the four free-off rows. Free-on selected b, normalized likelihood and both
primary endpoint pT values agree exactly with the verified private build.
All13,034 verbose state/covariance records across the four runs agree exactly:
1,624 in each seed2 job and4,893 in each seed12 job. Extra truth-prior endpoint
pT agrees as well. In this input12:16 remains a no-interval control;12:17 remains
a secondary-activity control and its lower-bound scan-winner status is retained.

The original runner initially stopped before launching events because its
provenance guard correctly detected replacement of the old installed plugin.
The installed phase now exempts only that intentionally replaced old-library
hash; source/card and private-reference hashes are still checked. The original
manifest is preserved, and the new installed hashes are recorded above.

## Current use and scope

The deployment blocker is cleared. Use FreeLossFit=True in the maintained
card, or BP_FREE_LOSS_FIT=1 with subbreakpointjobs.sh, to enable the optional
mode. FreeLossFit=False remains the default. One selected LocalMarginal
interval is the supported free-fit scope; existing explicit fallback/status,
positive-sigma truth-pair and conditional-covariance contracts are unchanged.
No new batch campaign was submitted and no physics validation is claimed.

No C++ changes were required during deployment. User-owned edits to
DumpGsfTrks/gsf.py.bk, DumpGsfTrks/sim.py.bk and subtrkjobs.sh were preserved.
No ROOT files or binaries are committed, and no remote operation is performed.

The project-status-curator workflow preserved the entire outgoing AGENTS.md in
`2026-09-10-agents-before-free-loss-deployment.md`. Only the pending-deployment
paragraphs in Current focus were replaced. Global status, laws and compile/run
instructions remain byte-identical; both substantive headings remain. No
history directory moved, so no history-move manifest was needed. The previous
pending status remains historical evidence rather than a current blocker.
