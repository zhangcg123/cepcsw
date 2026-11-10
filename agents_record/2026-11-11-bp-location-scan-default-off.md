# BP location scanning disabled in maintained steering

The user requested no location scan for current BP flat-tuple production.
The standalone `Reconstruction/RecBreakpoint/options/run_breakpoint.py`,
`options/batch_breakpoint.py`, `subbreakpointjobs.sh` and
`subbreakpointhiggsscanjobs.sh` now default BP_LOCATION_SCAN_SETUPS to the
empty string, producing LocationScanSetups=[]. Truth-location ordinary,
free-loss, beam and charged-ECAL fit settings remain unchanged.

Explicit nonempty environment overrides still enable selected scans. The
compiled RecBreakpoint property default remains [110]; the maintained card
overrides it. Direct unsteered algorithm use and previously prepared/frozen
cards therefore do not adopt the new setting automatically. No existing
campaign/card is edited or submitted. No library build/install is required.

The existing batch tests and authoritative README were synchronized.
`python3 -m unittest discover -s Reconstruction/RecBreakpoint/tests -p test_batch.py`
passes all64 tests, including standalone AST steering, direct-helper and shell
launcher manifests, and explicit scan override/validation. Shell syntax checks
and Python compilation pass. No native fitting rerun: this changes card/helper
steering only; the earlier native writer A/B already exercised the explicit
empty scan list on four REC events, as recorded in
[the snapshot implementation record](2026-11-11-bp-pre-ecal-tuple.md).

Pre-existing unrelated changes in the helper, wrapper and batch tests are
preserved and excluded from this focused checkpoint. No C++ property changes,
GSF edits, shared-package changes or branch operations.
