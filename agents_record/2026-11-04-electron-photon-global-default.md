# ElectronPhoton is the global breakpoint charged-energy default

The user explicitly requested ChargedEcalEnergyMode=ElectronPhoton globally,
after the five shared fit/scan switches had been aligned separately.

Changed the Gaudi property, FitSettings and FreeLossSettings defaults, standard
run card fallback and direct batch preparation. Batch preparation freezes the
mode in job.json rather than leaving it implicit. The generic wrapper already
defaults to ElectronPhoton; the Higgs wrapper now also preserves an explicit
ElectronOnly override. README and the maintained free-loss guide agree.

This is default steering only. With a breakpoint,110 uses C for the total
electron+photon energy without a collected neutral, or C+N for that total when
a neutral is collected. Passive collection remains available with111 OFF.
No-breakpoint fits still explicitly route to electron-only. Explicit111 still
uses separate charged-electron and neutral-photon observations, regardless of
this option. No loss-map, optimizer, objective, truth ownership, PFA/shared-KF,
branch or remote changes. ElectronPhoton still requires ElossOn=False.

Existing frozen cards/results are not rewritten. Explicit ElectronOnly
remains supported. New defaults require regeneration of old batch cards;
an explicitly frozen old mode continues to win over the compiled default.

Verification evidence: /tmp/bp-electronphoton-default-fXNb5Q.
62 Python batch tests pass, including compiled/card/direct-preparer/both-wrapper
default agreement and explicit ElectronOnly wrapper overrides.15 CTests pass;
the charged-energy test also checks both internal settings defaults and retains
the explicit ElectronOnly rejection/routing tests. Shell syntax and diff checks
pass. Only the RecBreakpoint module and charged-energy test are rebuilt; tests
retain the existing full package suite. No batch is submitted by this change.

Verbose staged runs on barrel seed1 events11/16/17 left all six shared
controls unset, including charged-energy mode. Assertions confirmed the effective
card values and generated configurable defaults. Every947 branches per event
exactly reproduces the previous explicitly steered ElectronPhoton run:2,841
branch-row values, including all scan vectors and preserved truth endpoints.
This is a regression check, not new population performance evidence.

The project-default module is updated with the package-only CMake install,
not a full-project install. Previous module and generated Python configuration
are backed up in the evidence directory. Old installed module SHA256:
a1c71b9529f7cf3ace728d93beaa8e5d0c7d9e61235085513bc5def87d15b18e.
New built/staged/installed module SHA256:
565b276140c1a9cee33a4d2c46090b56e945bc5af9f4e200840be631ec77752d.
Use the project-default direct gaudirun, without build/run or a temporary
library pin. Existing batch cards/logs and unrelated dirty files are preserved.
No production cards are regenerated or submitted, and no remote operation is
performed. The default-off separate neutral constraint is not enabled here.
