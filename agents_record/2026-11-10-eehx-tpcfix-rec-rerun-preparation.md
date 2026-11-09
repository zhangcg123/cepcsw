# Overnight normal-ISR/FSR REC regeneration with corrected TPC provenance

The user requested rerunning REC production tonight and deferring BH-model
discussion until tomorrow. The TPC provenance fix is produced during simulation,
so this campaign starts from the same STDHEP generator inputs and runs
sim/trk/calodigi/rec in one Gaudi process. No GSF or breakpoint fits are included.

## Prepared campaign

`higgs_eeHX_ISRFSRON_REC_TPCFIX_500jobs_200evt_20261110/`

- 500 jobs, 200 events/job, target 100,000 events; seeds/files 1--500.
- Normal ISR/FSR, original E240.Pe1e1h_X.e0.p0.whizard195 input files.
- Memory 8000 MB, group cms, twelve original raw-calorimeter output drops.
- No intermediate SIM/TRK/calodigi ROOT files; final REC retained.
- All 2,500 frozen cards compile and have verified SHA-256 checksums. Every
  card equals its previous-campaign counterpart after replacing only the
  campaign output-directory path. All input files exist.
- Current installed DetSimSDLib, DetSimSD, RecGsfTracking and RecBreakpoint
  hashes exactly match the successful producer-fix validation's libraries_after.
  The tested producer fix is commit 872d760. No builds/installations were needed.
- Prepared-submission dry run validates the complete campaign and prints
  exactly 500 hep_sub commands. No submitted/started/completed markers exist
  in the new campaign. Preparation, checksums, library hashes and submission
  command are in campaign-summary.json; prepare.log and submission-dry-run.log
  are uncommitted campaign artifacts.

The previous campaign and all its 500 ROOT files are preserved. At inspection
498 old completion markers exist; jobs 69 and 442 lack completion markers.
This is not an authoritative live scheduler status. The old campaign occupies
about 142 GB; the mounted project filesystem reports 1.8 TB available, which
is not an independent user-quota query. A similar additional REC size is expected.

## Submission blocked in this session

The local hep_q/hep_sub Python scripts import htcondor, which is unavailable.
An authenticated SSH attempt to lxlogin.ihep.ac.cn fails with network unreachable.
No jobs were submitted, and no SSH configuration or batch tooling was changed.
The existing hep wrapper can mask Python failure with shell exit status zero,
so do not treat a local wrapper return code alone as successful submission.

Run from the usual working batch-login shell, in the repository root:

```bash
SAMPLE_REGION=higgs DRY_RUN=0 SUBMIT_GROUP=cms \
./subbreakpointjobs.sh submit higgs_eeHX_ISRFSRON_REC_TPCFIX_500jobs_200evt_20261110
```

The previous 200-event campaign's observed median worker duration was about
5.3 hours, maximum completed duration about 6.7 hours, before queue delay;
this is a planning reference, not a guarantee for this rerun.

## Scope and follow-up

No reconstruction, PFA, BH, GSF, breakpoint, maintained card or installed
library was modified. Fresh production completion/provenance and physics
checks have not yet run. Once submitted, freeze the normal installed runtime
until those jobs finish; use isolated development paths if needed.

The fix addresses malformed TPC first/last-step ownership ranges. The separate
merged-hit distance failure against the strict 5 mm hook cut remains unresolved.
GSF also has a separate TPC-only three-hit seed failure, documented in
2026-11-10-tpc-truth-downstream-check.md; it is not a provenance failure.

Tomorrow's BH discussion can resume from
2026-11-10-acts-default-bh-source-audit.md: the default mixture matches ACTS,
but local covariance application differs; upstream default thin-step moments
and the 0.2-X0 cap remain limitations. No BH-model change is made tonight.
