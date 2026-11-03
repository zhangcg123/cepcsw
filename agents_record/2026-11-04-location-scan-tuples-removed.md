# Location-scan ROOT outputs removed

At the user's request, permanently deleted the200 top-level files matching
`higgs_silicon_scan_20261103/breakpoint_flat-higgs-<integer>.root`.
Validated the exact directory, count, filename family and one representative
location-scan TTree before deletion. Total logical size169,753,171 bytes
(161.89 MiB). Post-deletion check: zero top-level ROOT files; directory now15 MiB.

Retained all REC inputs, analysis plots/tables, logs, frozen cards and completion
markers. Temporary development outputs under /tmp were outside this removal.
No source/configuration/installation/remote change or batch submission.

The prior live note described this as the completed original200-file x50 Higgs
silicon-only campaign, without the later TPC row0/full-IP extensions. Preserve
that as historical provenance, not an assertion that those ROOT files remain.
Their numerical studies and dated records are retained. Deleted binaries are
not recoverable from Git or trash; data must be regenerated from retained inputs
if needed. Existing completion markers describe the former runs, not current
output availability; no worker state was reset by this cleanup.
