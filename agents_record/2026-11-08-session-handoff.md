# Fresh-session handoff — migration PAUSED

Updated 2026-11-08. Read this before acting on earlier migration/campaign plans.

Original long-session ID: `01a0f8c1-b52f-7e53-8c6d-8ca660fc5a8d`.
After archiving, restore with `codex unarchive 01a0f8c1-b52f-7e53-8c6d-8ca660fc5a8d`;
reopen explicitly with `codex resume 01a0f8c1-b52f-7e53-8c6d-8ca660fc5a8d`.
The new session should read this handoff first and consult only relevant old
excerpts if needed, not reload the entire enormous history automatically.

## Latest user decision and authority

The user wants eventually to port RecBreakpoint into the newer Hive-based
CEPCSW, but first understand the compatibility issues. **Further migration is
paused. Investigation/discussion only until the user requests a concrete fix.**
Do not move/copy more package code, data or Git history. Do not change source,
run cards, installations, branches/remotes or submit jobs. Do not undo/remove
the port already present. Existing quiet-window permissions were for earlier
tasks; they are not authorization for future changes. Documentation changes
for this session handoff were explicitly requested.

## Workspaces — do not mix them

| Role | Workspace | Observed branch |
|---|---|---|
| Current session/reference | `/aifs/user/data/zhangcg/gsfdev/CEPCSW` | `breakpoint-delayed-neutral` |
| Candidate Hive destination | `/aifs/user/data/zhangcg/ebrem/CEPCSW` | `workflow_hive` at the previous inspection |

Current reference HEAD, rechecked for this handoff:
`9323859016e2ece23f11d3a3715046a74ab4e1f5`.
The reference uses LCG105/GCC11-era dependencies. The candidate uses Key4hep
2026-04-08/GCC14/C++20/EDM4hep1.0. Never source both environments in one shell.
Use fresh shells and verify resolved library paths before future execution.

The destination already has a port installed as UNCOMMITTED working-tree
changes. Its remote/HEAD alone does not contain that port. Preserve both dirty
worktrees and the reference history. No history import/branch operation was
performed. Migration paused does NOT mean rollback.

The upstream destination detector chain is multithreaded; RecBreakpoint is
still a separate sequential REC-to-flat process. Do not clone its ROOT-writing
algorithm into Hive or assume a complete multithread breakpoint implementation.

## What the breakpoint package does

- Reads CompleteTracks, retaining the baseline KF and track-matched MC truth.
- Ordinary Persistent6D augmented KF/RTS carries five helix coordinates plus
  b=log(p_before/p_after), including covariance/correlations. Its Gaussian
  prior defaults to MeanLogLoss=0 and SigmaLogLoss=0.001.
- Free-loss is an outer Minuit2 wrapper of the SAME augmented KF/RTS. It varies
  the Gaussian prior CENTER, retaining that sigma. It is not a prior-free b fit.
  Its objective is normalized full-track NLL, not smoothed hit-only chi2.
- Separate diffuse RTS has no narrow finite b prior. The outer free-loss and
  additional constraints have not been extended to diffuse.
- Truth-centered oracle, ordinary RTS and free-loss endpoints are separate.
  All backward-filter variants default OFF.
- Defaults: free-loss/beam/charged ON, separate neutral OFF; ElectronPhoton;
  PreReclustering PFO/cluster inputs; largest eligible truth-layer breakpoint;
  additional location scans for110 only. Truth-site endpoints remain saved.
- 000=tracker;100=+beam;110=+charged-stage ECAL. ElectronPhoton110 uses combined
  charged+collected-neutral energy when resolved and charged energy when not.
  111 is the separate charged/neutral-measurement experiment, disabled by default.
- Scan includes silicon VXD/ITK and reconstructed TPC row0, plus a5Dnull and
  parallel truth reference. Null/candidate likelihoods use the same observations,
  including C+N where applicable. No calibrated no/light-eBrem test exists.

These are the last recorded defaults, not a new source audit. For exact current
steering inspect the relevant workspace's card; do not execute it just to read it.

## Known issues to understand, not silently fix

The latest completed comparison started from the SAME first two new generator
events, sim seed1, and ran both full detector chains plus default110scans.
Different Geant4/physics-list/default settings produced DIFFERENT detector
samples. Do not call old/new pT differences a controlled port regression.

1. Invalid hit-step provenance exists in BOTH projects. Old event0 e−, track0:
   accepted hit9 status28, firstStep1469 > lastStep136, hook135. New event1 e−,
   track18: hit9 status30, firstStep549 > lastStep152, hook151. The reader
   correctly rejects these rows; refits/scans are NaN/empty. Counter lifecycle
   in SD aggregation is a hypothesis, not an established root cause. Do not
   clamp/swap counters or disable validation.
2. Both new positrons have approximately85%/84% tracker-brems loss but no
   eligible truth breakpoint. Finite ordinary/free outputs do not mean recovered
   loss. Missing owner hits/track segmentation needs a separate diagnosis.
3. Initial two-slot A/B showed different digitized hits despite equal MCtruth.
   Scheduling/shared random streams are plausible, not proven sole causes.
   Parallel bitwise reproducibility has not been established.
4. Native compatibility must preserve the original KF path and PFA algorithms.
   Passive snapshots/diagnostics were added, not permission to tune CyberPFA.
   SD class-layout changes require rebuilding factories AND the corresponding
   libraries; a private overlay previously mixed incompatible builds.
5. Legacy LCG batch helpers are not a supported integrated Hive-to-breakpoint
   chain. Git-history preservation and new batch integration remain pending.
   The independent optimizer-acceptance audit is also still open.

Existing checks distinguish: all16 numerical/schema CTests;166 original
new-fork physical fields preserved in a serial baseline/port comparison;
172 diagnostic fields across4control tracks;1,015 branches reproduced between
staging and native runtime on the SAME fixed REC sample. These are separate
mechanical checks, not cross-stack or population physics validation.

## Durable evidence and dependencies

Detailed previous port/runbook/code map/two-event pT table:
[2026-11-07 destination handoff](/aifs/user/data/zhangcg/ebrem/CEPCSW/agents_record/2026-11-07-multithread-breakpoint-handoff.md).
It is historical evidence: THIS record's pause instruction overrides its
proposed migration/implementation actions. Do not load older months of GSF
history unless needed for a specific question.

Latest raw local test artifacts currently at:
`/tmp/breakpoint-two-projects-20261107-BqATXw/` (comparison.csv/json,
results.log, old/new logs, REC and flat files). Temporary files may disappear;
the durable table/failure details are in the linked destination handoff.
Do not claim a rerun from merely rereading that table.

New50k ISR/FSR-off eeHX input is STILL in the reference project:
`GeneratorStudies/eeHX_ISRFSROFF_50k_20261106/`.
First file: `e1e1h_X.e0.p0.ISRFSROFF.001.stdhep`.
50,000 readable events in251files; use validation_files.csv for actual counts.
Higgs-decay photons remain even with production ISR/FSR disabled. Do not delete
the reference workspace assuming those inputs were migrated.

## How the next session should begin

Read AGENTS.md and this handoff, acknowledge the migration pause, then ask which
compatibility question the user wants to discuss. Read-only checks are allowed
when relevant. Do not auto-run tests, fix provenance, rebuild/install or submit.
Keep terminology stable and distinguish measured facts from hypotheses.

Start a FRESH chat, not a fork/resume of the enormous old transcript:

```bash
cd /aifs/user/data/zhangcg/gsfdev/CEPCSW
codex "Read AGENTS.md and agents_record/2026-11-08-session-handoff.md. Migration is paused; investigation only."
```

After this reply, `/archive` in the OLD session archives it without deleting
its transcript. Confirm when prompted. Alternatively use `codex archive ID`
from a shell (get the ID from the old session's `/status`). Restore with
`codex unarchive ID`; do not use delete. Archive also attempts to archive
descendant sub-agent sessions. Commands were checked against installed
codex-cli0.167.0 and official documentation; no session was archived by this task.

## Documentation preservation

The complete pre-edit AGENTS is preserved byte-for-byte in
[the snapshot](2026-11-08-agents-before-session-handoff.md).
Its entire introduction/global laws/build instructions remain unchanged.
Old Current focus / Current implementation and controls / Evidence and
deployment / Next step and analysis context are retained in that snapshot;
the live focus now points here and marks the old campaign historical.
No history directory was moved/deleted, so no directory-migration manifest
was necessary. No code or installation was changed for this fresh handoff.
