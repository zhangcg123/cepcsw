# Stable terminology for technical discussions

The user requested a reusable project skill after repeated explanations changed
terminology and obscured the original question about the free-loss objective.

Created project_skills/consistent-technical-dialogue/ with SKILL.md, optional
agents/openai.yaml UI metadata, and references/tracking-terms.md. AGENTS.md now
requires this skill for multi-turn algorithm explanations, derivations and
disputed technical claims. The new rule does not change the implementation
scope or authorize further objective changes.

The standard .agents directory could not be written through apply_patch, even
using a scoped escalated request. No permission or ownership was changed. The
skill therefore lives in the writable project_skills directory and is reached
through the explicit AGENTS.md rule. Do not assume this nonstandard location is
automatically indexed in the skill picker; the project instruction supplies the
entry point. Existing skills and their directory were not modified.

The skill-creator instructions and validator were used. The new skill and its
required reference were read completely after creation and applied to the
current reply. Key requirements: one name per concept, stable symbols, explicit
corrections and assumptions, current-code checks for implementation claims,
and recovery of the original question without new implementation authority.
The vocabulary reference does not prescribe the answer to disputed mathematics.

Validation: quick_validate.py passed. Manual behavior review checked that the
instructions prohibit residual/chi2 conflation, silent expansion of a fixed-loss
score equality into equal fitted losses, and an unlabeled single-measurement
example substituted for the multi-layer question. This was a manual review,
not an independent behavioral evaluation or guarantee of future compliance.
The local link and UI prompt point to the correct skill; git diff --check passed.

Only the added law, new skill files and this record are part of the checkpoint.
No source, run cards, ROOT files, existing experiment results or remote state
are changed. The ongoing objective question is not resolved by adding a skill.
