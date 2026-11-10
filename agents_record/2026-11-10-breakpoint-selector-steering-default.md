# Breakpoint production-electron selector on by default in maintained steering

The user requested that production-electron selection default on in the
breakpoint run card and batch helper. The existing canonical environment name
is `BP_SELECT_TRUTH_PRODUCTION_ELECTRONS` (plural); no alias or rename is added.

Updated defaults:

- `options/run_breakpoint.py`: absent environment control selects true.
- `subbreakpointjobs.sh`: defaults the control to1, retaining explicit0.
- `options/batch_breakpoint.py`: direct batch preparation also freezes1 when
  the environment control is absent.
- The dedicated Higgs launcher already defaults1 and is unchanged.

Set `BP_SELECT_TRUTH_PRODUCTION_ELECTRONS=0` for particle-gun samples or
all-track refits. The compiled C++ property default stays false for custom cards
that omit the property. The selector implementation, fit, truth recording and
event-wide tuple storage do not change. Existing frozen cards and running jobs
retain their original steering; no build, install or submission is needed.

The README now distinguishes maintained steering from the compiled default.
The existing batch default/override/freeze test checks default1, explicit0,
normalization of true and rejection of invalid boolean input. All64 batch
planning/worker tests pass. Direct evaluation of the real standalone-card
assignment passes absent-control/on/off checks; shell syntax and diff checks
pass. No native physics rerun is required for this default-only steering change.

Pre-existing edits in the batch helper, shell launcher and batch tests are
preserved and excluded from this task's checkpoint.
