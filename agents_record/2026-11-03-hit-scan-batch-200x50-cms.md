# Higgs scan-only batch: 200 jobs x50 events, group cms

User requested 200 jobs with50 events/job and scheduler group cms, then
explicitly chose "Leave prepared; show command". No jobs were submitted.

The maintained subbreakpointhiggsscanjobs.sh now defaults to NEVT=50,
SEED_FIRST=1, SEED_LAST=201 and SUBMIT_GROUP=cms (still overridable).
rec-higgs-123.root is absent. Existing missing-input skip behavior produces
exactly200 jobs from IDs1..201 excluding123, totaling10,000 requested events.
Each job reads the FIRST50 events of its own existing rec file, no sim/rec
rerun and no event-offset/splitting machinery. This is a different10k-event
subset from the former50-file x200 preparation, not an identical repartition.
The input-selection choice was stated to the user before preparation.

Input: higgs_full_20261030/rec-higgs-<ID>.root.
Output: higgs_silicon_scan_20261103/breakpoint_flat-higgs-<ID>.root.
Current manifests/cards: output/runcards/higgs-<ID>/; logs: output/outlog/.
External rec files are retained. Only breakpoint runs. Scan mode remains
HitScanAll: all locations x000/100/110/delayed111, no online location ranking,
selection or gain cut. No C++ or installed-runtime change was needed.

Before refreshing, verified all50 previous cards were unstarted/unsubmitted,
checksum-valid, with no scheduler logs or flat outputs. Moved those cards
recoverably to runcards_before_200x50_cms_20261103/ inside the output directory.
An archive README warns that manifests contain historical absolute paths.
The earlier ranked-card archive also remains. No ROOT data was deleted.

Validation: all56 batch/card tests passed, including the missing123 case,
200 manifests,50 events/job, exact10k total and cms dry-run commands. All200
actual cards/manifests pass frozen environment/checksum checks and have no
submission/start markers. All200 dry-run commands explicitly use -g cms.
The read-only ROOT-header audit passed for all200 selected rec files, each
containing at least50 events: all10,000 requested events are available.
The source library remains the previously validated scan-only installation;
no new fit/physics validation is claimed by this steering change.

Use this command from the project root; the explicit assignment avoids an
older exported SUBMIT_GROUP overriding the new default:

```bash
SUBMIT_GROUP=cms ./subbreakpointhiggsscanjobs.sh submit-prepared
```

The helper refuses duplicate submissions through its existing submitted/start
markers and output/log checks. Group is selected at SUBMISSION time, not encoded
in the per-job manifest. No actual Condor execution was tested in this change.

Temporary evidence: /tmp/bp-scan-only-9xx5YM/test_200x50.log,
prepare_200x50.log, submit_200x50_dry.log and audit_200x50.py/log.
Implementation/deployment contract remains in
2026-11-03-four-setup-scan-only.md. Existing unrelated worktree edits are
preserved; generated cards, ROOT files and logs are not committed.
