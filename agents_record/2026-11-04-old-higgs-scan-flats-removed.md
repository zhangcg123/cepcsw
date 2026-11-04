# Old Higgs location-scan flat tuples removed

At the user's request, removed all249 regular files matching
`higgs_location_scan_20261104/breakpoint_flat-higgs-*.root`, totaling0.61 GiB.
The directory contained no other ROOT filenames. Verified zero ROOT files
remain at its top level. This is permanent deletion, not a move to trash;
the data can be regenerated from the retained REC inputs.

Preserved all249 `higgs_full_20261030/rec-higgs-*.root` inputs, the old scan
`runcards/` and `outlog/`, and all28 analysis PNGs under
`TrackingPerformanceStudies/breakpoint_higgs_location_scan_20261104`.
The new default output `higgs_location_scan_truth_20261104` does not yet exist.
No new campaign was prepared/submitted and no implementation/runtime changed.

Previous live status, preserved as historical evidence:
Earlier campaign `higgs_location_scan_20261104` had249 files but only80
verified complete jobs (16,000 events);169 were killed at the10-hour limit.
Partial files were not usable as completed outputs. Its completed80-file
analysis remains, but its source ROOT files are now deleted. Old job completion
markers/logs are retained history, not evidence that their ROOT outputs still
exist. The prior200 scan ROOTs in `higgs_silicon_scan_20261103` were already
absent; that earlier deletion was not repeated.

Current next step: regenerate flat tuples from the existing REC sample,
retaining enabled truth-location endpoint families and scanning only110.
No code, physics, project-law or historical-directory migration was performed.
