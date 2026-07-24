# Proposed long-term GSF development skills (2026-07-23)

## Context

The user reported that all old tuples have been removed and intends to restart
the exploration from scratch. Before defining that exploration, we discussed
project-local Codex skills that could make the long-running work more
reproducible and prevent experimental history from accumulating in
`AGENTS.md`.

This is a planning record only. No skill has been created, no workflow has been
selected, and the live project focus has not been changed. Revisit these
proposals with the user before implementation.

## Recommended skills

### 1. `gsf-experiment-runner`

Highest priority. Enforce the complete experiment lifecycle:

- establish the baseline and hypothesis;
- create unique steering and output locations;
- build and install `RecGsfTracking`;
- run focused comprehensive component dumps first;
- verify hit completeness, rejection counts, finite states, and covariance
  health;
- run the required control ladder;
- capture code, configuration, input, and output provenance;
- prevent accidental pairing of stale and current results.

Use deterministic scripts for output naming, metadata capture, and mechanical
validation.

### 2. `gsf-tuple-auditor`

Make tuple integrity a mandatory gate:

- inventory files, trees, schemas, and required branches;
- identify missing, corrupt, duplicate, short, or mismatched events;
- create exact-pair tables;
- record denominators and exclusions;
- reject population comparisons with ambiguous provenance or pairing.

This should guard against previously encountered missing files/trees, topology
mismatches, stale tuples, and changing denominators.

### 3. `gsf-physics-validator`

Encode the scientific comparison protocol:

- use Geant4 pre/post-step truth;
- assign reconstruction-aligned loss surfaces;
- separate no-eBrem, light-eBrem, hard-eBrem, and secondary-topology
  populations;
- report median, central-68 width, fixed-threshold counts, RMS, and explicit
  tail metrics;
- require eventwise exact pairing;
- require clean, light, hard, muon, energy, angle, and held-out controls before
  promotion;
- distinguish mechanical validation from physics validation.

### 4. `gsf-component-trace-analyzer`

Standardize interpretation of verbose component dumps:

- reconstruct component ancestry;
- identify decisive hits and selected surface/mode;
- separate prior odds from innovation-likelihood odds;
- track cutoff and KL merges;
- compare truth-compatible competitors;
- generate a compact per-event state table.

A reusable parser should replace repeated manual reconstruction of this logic.

### 5. `gsf-experiment-recorder`

Create consistent dated records containing:

- question and hypothesis;
- exact baseline and variant;
- input identity;
- commands and configuration;
- focused mechanical results;
- population results;
- interpretation and limitations;
- accept/reject decision;
- precise resume point.

It should replace the current focus in `AGENTS.md` when appropriate rather
than append experiment history. The existing `project-status-curator` remains
the cleanup and migration tool; this proposed skill would prevent renewed
accumulation.

## Design decision

Do not create one large `CEPC GSF` skill. Keep skills small and composable,
with concise `SKILL.md` instructions and deterministic scripts for fragile or
repeated operations. This avoids reproducing the former AGENTS.md context
problem inside a skill.

## Proposed implementation order

Start with `gsf-experiment-runner` and `gsf-tuple-auditor` so the fresh baseline
is reproducible and its inputs are auditable. Build the physics validator and
component-trace analyzer from the first fresh experiment, once the new tuple
schema and output layout are known. Add the experiment recorder alongside the
runner or immediately afterward.

## Decision required later

Before creating anything, agree on concrete example requests, the fresh tuple
schema and storage layout, the required focused and population gates, and
whether these skills should be project-local under `.agents/skills/` or
installed as personal Codex skills.
