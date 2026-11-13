# BP run-card and batch theta-window baseline — 2026-11-13

User selected a theta half-window of 25 mrad, total width 50 mrad. Phi remains
at its 200 mrad half-window (400 mrad total width). The collector compares
absolute angular differences and includes its exact boundary.

The maintained `options/run_breakpoint.py` defaults `BP_NEUTRAL_THETA_MRAD`
to 25. `subbreakpointjobs.sh` exports this default, inherited by both Higgs
wrappers. Direct `options/batch_breakpoint.py` preparation also freezes theta
25 and phi 200 in each new manifest and generated card. Explicit environment
overrides are preserved by every path. No fitter, collection algorithm, error
model or compiled property default changed; the unsteered compiled theta default
remains 10 mrad. No rebuild/install is required for this card-only change.

Existing prepared cards/manifests, REC/flat files and gallery figures are not
rewritten. In particular, the existing recoil samples still use their frozen
prior window. New preparation is required to use the new baseline in fits.
No job submission or reconstruction/performance rerun was performed.

Validation: all 66 standard-library batch tests pass. The window test evaluates
the actual card assignments, direct helper preparation, general shell launcher
and Higgs REC launcher with default theta 25 and explicit override 50. The
existing full-chain Higgs wrapper test now also checks frozen theta 25/phi 200.
Shell syntax checks pass for all three launchers and the worker. Tests prepare
temporary planning fixtures only; no Condor/Gaudi jobs run.

Unrelated pre-existing edits in the batch planner, test suite and common shell
launcher are preserved and excluded from this checkpoint.
