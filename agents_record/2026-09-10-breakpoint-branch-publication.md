# Breakpoint branch publication

The user requested renaming local test_breakpoint to breakpoint and publishing
it to origin, including its maintained run card and batch submission scripts.
They explicitly excluded ROOT files from staging, commits and the push.

Read-only remote inspection found origin/dev but no origin/breakpoint or
origin/test_breakpoint. This publication creates origin/breakpoint with a
normal non-force push; it does not delete or overwrite another remote branch.

Included maintained workflow files:

- Reconstruction/RecBreakpoint/options/run_breakpoint.py (newly tracked).
- subbreakpointjobs.sh, retaining the user's shared sigma default0.001.
- dump_breakpoint.sh, preserving the user's commented set -eu.
- Existing tracked options/batch_breakpoint.py and its latest implementation.
- RecBreakpoint/CMakeLists.txt and Reconstruction/CMakeLists.txt registration,
  required to build the package from the published tree.
- DumpGsfTrks/trk.py.bk: tuplepath handling and embedded G4 collection inputs
  are required by the breakpoint worker/template replacements and truth reader.

No generated ROOT files, logs, plots, analysis tables or prepared campaign
cards are staged. The pre-publication tracked-file and unpublished-history
checks found no .root/.ROOT paths. Unrelated GSF card, simulation gun-angle
and subtrkjobs.sh changes remain local. No fitter physics changes in this
publication; the verified iteration-removal code is included in branch history.

The dedicated run card and build files are now explicit tracking exceptions
to the default project policy. The existing documentation snapshots retain
the former branch name and untracked-card policy as historical facts.
