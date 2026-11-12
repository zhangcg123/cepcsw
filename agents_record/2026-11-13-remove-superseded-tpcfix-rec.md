# Superseded TPC-fix REC directory removed — 2026-11-13

After confirming the latest BP production uses the newer REC_ECALTRUTH files,
the user explicitly requested removing the old REC directory in its entirety.
Deleted:
`higgs_eeHX_ISRFSRON_REC_TPCFIX_500jobs_200evt_20261110/`.

The whole directory, including500REC ROOT files and all logs/cards/campaign
metadata inside it, is absent. Before deletion the ROOT files totaled
151181382902bytes;whole-directory apparent size was152261026033bytes
(141.8GiB). No Git-tracked file belonged to that directory.

A host-context scan found no current-user open file handles under this path.
All500new BP jobs had completed;existing GSF jobs had498completion markers,
and55/421had explicit failed.json records with exit139. This is a known-job
and host-handle check, not a global scheduler audit. No job was killed.

Protected and verified unchanged by exact size/mtime comparison:

-500newREC ROOT files under REC_ECALTRUTH_500jobs_200evt_20261112;
-500new BP_FSRFLOAT_NOSCAN_500jobs_200evt_20261112 outputs;
-500existing GSF_MATCHBP_500jobs_200evt_20261111 outputs, including retained
partial files from the two failed jobs.

Existing plots,flat outputs,analysis caches and saved provenance remain.
Historical manifests retain their original input paths as provenance. Reruns
requiring the removed REC files need new input cards or appropriate saved
provenance/caches. In particular,the parent category extractor's supplemental
GSF40/429 initialization still stats the original REC paths; it cannot be run
unchanged after this deletion. Current resolved/unresolved classification uses
newer REC_ECALTRUTH inputs,which remain available. No reconstruction or
analysis validation was relaxed to accommodate deleted data.

Deletion ran once against the exact explicitly checked directory,without
following a root symlink. The before/after protected-file inventory check and
absence assertion passed. Generated machine-readable execution evidence:
`/tmp/old_rec_deletion_20261113.json`.
