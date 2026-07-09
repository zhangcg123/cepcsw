---
name: project-status-curator
description: Curate long-running project knowledge by keeping AGENTS.md limited to (1) the project introduction/global status, active project laws, work scope, and essential compile/run instructions and (2) the current focus, while migrating only superseded detail into agents_record/ without information loss. Use when project status is messy, AGENTS.md has accumulated history, the active focus changes, records contradict one another, or the user asks to consolidate, archive, hand off, or refresh project memory.
---

# Project Status Curator

Maintain a two-layer project memory:

```text
AGENTS.md       concise present state
agents_record/  complete historical evidence
```

Do not require source or tests as part of this documentation workflow unless
the user separately asks for technical verification.

## Required AGENTS.md structure

Keep only two substantive sections:

1. **Introduction and global status** — purpose, overall maturity, validated
   capabilities, major limitations, essential compile/run commands needed to
   resume work, active project laws and work-scope boundaries, and the authority
   rule pointing to `agents_record/` for history.
2. **Current focus** — the active blocker/question, current evidence, immediate
   actions, success criteria, and explicit non-goals.

Do not turn `AGENTS.md` into a chronology, code map, runbook, incident report,
or append-only log.

## Protect active laws and scope

Before migrating anything, classify each statement as one of:

- current global status;
- active project law or work-scope constraint;
- current focus;
- historical evidence or superseded detail.

Keep the first three categories in `AGENTS.md`. Move only the fourth category
to `agents_record/`.

Active laws include constraints such as allowed package boundaries, forbidden
dependencies or shared-package edits, truth/validation standards, safety and
repository-hygiene rules, required build/run procedures, and documentation
retention requirements. They remain current even when their original rationale
comes from a resolved historical incident.

Never weaken a current rule merely to shorten `AGENTS.md`. Preserve the rule in
direct, explicit language and move only its detailed historical rationale.

## Lossless migration workflow

Treat “DO NOT MISS anything” as a hard invariant.

1. Inventory every candidate status/history document and every heading in the
   current `AGENTS.md`.
2. Identify and freeze all active project laws and work-scope constraints before
   removing any text.
3. Run `scripts/history_manifest.py create <history-dir> <manifest>` before
   renaming or moving the history directory.
4. Preserve the complete pre-edit `AGENTS.md` as a dated snapshot under
   `agents_record/` before replacing it.
5. Rename `agent_record/` or another legacy history directory to
   `agents_record/` when requested. Do not resurrect files already absent from
   the working tree unless the user explicitly asks.
6. Run `scripts/history_manifest.py check <new-history-dir> <manifest>`
   immediately after the move, before editing migrated files.
7. Build a migration map for every removed AGENTS heading:

   ```text
   old heading -> retained in new AGENTS.md | historical snapshot/record
   ```

8. Rewrite `AGENTS.md` to the two-section structure. Retain the global status,
   active laws/scope, and information needed to resume current work.
9. Update live links to `agents_record/`. Historical snapshots may retain old
   paths when exact preservation is more important; label them snapshots.
10. Search the repository for legacy directory references and classify every
   remaining occurrence as intentional history or a link requiring repair.
11. Report counts, manifest verification, migration-map coverage, protected
    active-law coverage, and any intentionally unresolved stale links.

Never delete unique evidence merely because it is outdated. Move it to a dated
record or preserve it in the pre-edit snapshot.

## Updating an existing project

When the current focus changes:

1. Capture the outgoing focus and its evidence in a dated
   `agents_record/YYYY-MM-DD-<topic>.md` record.
2. Confirm the dated record contains every detail being removed from
   `AGENTS.md`.
3. Replace the current-focus section instead of appending another focus.
4. Adjust the global status only when project maturity or validated capability
   actually changes.
5. Keep historical records unloaded during ordinary status retrieval; open
   them only for regression analysis, design rationale, comparison, or explicit
   provenance requests.

## Quality checks

- `AGENTS.md` has exactly the two intended substantive sections.
- The introduction explains what the project is and its global maturity.
- Active project laws and work-scope constraints remain explicit and complete.
- Essential compile/run instructions remain available without loading history.
- The current focus states one coherent concentration and ordered next actions.
- The outgoing AGENTS content exists in a dated snapshot or mapped record.
- The history manifest passes after a directory migration.
- No unique historical file disappeared.
- All live references use `agents_record/`.
- Current statements do not coexist with superseded “current” statements.
- The final report distinguishes current status from preserved history.
