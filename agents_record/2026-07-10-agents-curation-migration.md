---
name: agents-curation-migration
description: Lossless migration from the long-form AGENTS guide and agent_record directory to a two-section AGENTS.md plus agents_record history
metadata:
  type: documentation-migration
  date: 2026-07-10
---

# AGENTS status-curation migration

## Purpose

Keep `AGENTS.md` limited to the project introduction/global status and the
current focus. Preserve all other material under `agents_record/` and avoid
loading it during ordinary status retrieval.

## Lossless directory migration

- Before rename: `agent_record/`, 21 files.
- After rename: `agents_record/`, the same 21 relative paths, sizes, and SHA-256
  digests.
- Verification was performed before any migrated record was edited.
- The original manifest is stored as
  `2026-07-10-agent-record-migration.sha256`.
- The complete outgoing `AGENTS.md` was moved without modification to
  `2026-07-10-pre-curation-agents-snapshot.md`.
- The root-level legacy index `MEMORY.md` and chronological log
  `DEVELOPMENT.md` were also moved into `agents_record/`; their contents were
  retained and their relative cross-link remains valid.

The first manifest guaranteed preservation of every file present in the
working tree. A later staged-diff audit found four additional historical
artifacts that were already deleted locally but still existed in `HEAD`. To
honor the stronger “do not miss anything” rule, they were recovered from Git
and flattened into `agents_record/`:

- `bh-parameterization.png`
- `2026-07-05-gsf-development-handoff.md`
- `2026-07-05-measure-cepc-electron-energy-loss-plan.md`
- `2026-07-05-optimize-bh-for-cepc-plan.md`

Thus both the migration-time working files and the recoverable tracked
plan/handoff evidence are preserved.

## AGENTS heading migration map

| Outgoing heading | Destination |
|---|---|
| Project objective | Condensed into new section 1; full text in pre-curation snapshot |
| Current status | Condensed into new section 1; full text in snapshot and current status records |
| Immediate next work | Condensed into new section 2; full text in snapshot and `2026-07-10-gsf-topn-energy-loss-status.md` |
| Source-of-truth hierarchy | Condensed into section 1 retrieval paragraph; full policy in snapshot |
| Status lifecycle | Condensed into section 1 maintenance paragraph; full policy in snapshot |
| Code and data map | Pre-curation snapshot and existing code/data reference records |
| Active configuration policy | Pre-curation snapshot and `Reconstruction/RecGsfTracking/README.md` |
| Build and focused validation | Pre-curation snapshot and build/operational records |
| Record organization | This migration record and pre-curation snapshot |
| Repository hygiene | Pre-curation snapshot |
| Scope constraints | Current non-goals in section 2; full constraints in snapshot |

Follow-up correction: the scope constraints remain active project law, not
merely historical rationale. Their complete operational form was restored to
the `Project laws and work scope` subsection of the new AGENTS section 1.

Root historical documents were mapped as follows:

| Outgoing root file | Destination |
|---|---|
| `MEMORY.md` | `agents_record/MEMORY.md` |
| `DEVELOPMENT.md` | `agents_record/DEVELOPMENT.md` |

## Later plan/handoff audit

The repository was searched for directories named `plan`, `plans`, `handoff`,
or `handoffs`.

- `agents_record/plans/` and `agents_record/handoffs/` existed after the initial
  migration. Their formerly tracked files had already been deleted before this
  curation.
- `.claude/plans/wise-scribbling-grove.md` was the only remaining plan file.
- A one-file SHA-256 manifest was created, the file was moved unchanged, and
  the manifest check passed.
- After verification, its internal legacy path was updated from
  `agent_record/DEVELOPMENT.md` to `agents_record/DEVELOPMENT.md`.
- The plan was flattened to
  `agents_record/2026-06-28-rollback-prefit-qp-refinement-plan.md`.
- The now-empty `.claude/plans/`, `agents_record/plans/`, and
  `agents_record/handoffs/` directories were removed.

The migrated plan is a historical rollback plan for the removed analytical
prefit and q/p-refinement experiments. It is not part of the current focus.

Every outgoing heading is therefore retained either in the concise current
guide or in the exact historical snapshot. No AGENTS content was discarded.
