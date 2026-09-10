# Breakpoint maintained-card free-loss default on

The user requested that the free-loss controller in the maintained run card
default to ON. `options/run_breakpoint.py` now uses
`BP_FREE_LOSS_FIT` fallback `1`; explicit `0` still disables the extra fit.
Newly prepared dedicated batch jobs freeze this updated card. Existing prepared
cards and outputs are unchanged. The C++ property default remains false: this
is a run-card steering change, not a fitter or compiled-default change.

Ordinary and truth-prior pairs remain separate. Free-loss fitting supports one
selected LocalMarginal interval; empty/unsupported/failed cases retain the
documented copied-result status behavior. No scope or physics claim changes.

Updated README and free-loss guide distinguish compiled and card defaults.
All 23 batch tests passed, including default-on and explicit false steering;
both shell helpers passed bash syntax checks and git diff --check passed.
No reconstruction runs, rebuild, batch submissions, or remote changes were
needed or performed. Shared GSF/tracker files were not edited.
