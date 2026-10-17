# Breakpoint batch uses the project-default installation

The user requested that breakpoint batch jobs follow the standard project
runtime rather than explicitly selecting the LCG 105 build directory.

The existing dump_breakpoint.sh already sources project setup.sh. The Python
worker now resolves gaudirun.py from that environment and calls it directly
for every selected stage. The hard-coded build.105.../run launcher and its
existence check were removed. A missing gaudirun.py fails before creating the
started marker or running any stage.

This matches the standard manual setup.sh + run.sh runtime selection:
InstallArea/$CEPCSW_LCG_PLATFORM libraries/configuration take precedence.
Compile changes must be installed before batch use. Cards are frozen, but
runtime binaries are shared and not frozen; do not install over active jobs.
Prepared manifests do not contain the runner, so not-yet-started jobs also use
this updated launcher. The separate identification batch helper is unchanged.

Validation: all 38 batch-helper tests pass, including exact launcher arguments,
no build-directory requirement, missing-launcher failure, intermediate cleanup
and downstream failure retention. Shell syntax and git diff checks pass.
No physics job, scheduler submission, installation or fitter change was made.
