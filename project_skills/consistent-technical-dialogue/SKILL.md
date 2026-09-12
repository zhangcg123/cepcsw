---
name: consistent-technical-dialogue
description: Maintain stable terminology, symbols, assumptions and code-backed claims during multi-turn technical discussions, especially tracking states, chi2 definitions, likelihood objectives and algorithm comparisons. Use for derivations, apparent contradictions, or recovering a question lost through repeated explanations. Not needed for routine execution or cosmetic plot changes without conceptual discussion.
---

# Consistent technical dialogue

Help the user follow one line of reasoning without silently changing the words,
mathematical objects or assumptions. Continuity matters more than stylistic
variation. This skill governs explanations; it does not authorize code changes.

## One name per concept

- Recover the original goal and exact current question. Answer that question
  before proposing a different experiment.
- Reuse the user's established terms when their meaning is clear. Keep a small
  terminology ledger: canonical name, definition, symbol if used, and scope
  (before/after update, one hit/all hits). Show only entries needed now; do not
  restart a glossary every turn.
- Do not substitute synonyms for variety. Map a source-code alias once to the
  canonical name, then keep that name. If an earlier label conflated concepts,
  explicitly separate them once; consistency must not preserve an error.
- Keep a state, its residual vector, the covariance-weighted scalar calculated
  from that residual, and the full optimization objective distinct.
- Keep symbols, subscripts and units stable. Do not reuse a symbol for local
  and full-track covariances without distinction. Define every new symbol
  immediately. Use terminal-readable formulas for this user.

For CEPCSW tracking/objective discussions, read
[references/tracking-terms.md](references/tracking-terms.md) before explaining
those concepts. It is a vocabulary contract, not proof of an algorithm property.

## Compare expressions in the same arrangement

- Before explaining an equality or difference, display both complete quantities
  with the same notation, term order and level of expansion. Do not compare one
  per-hit contribution with a whole-track sum, or give an explicit formula for
  one side and only verbal labels for the other.
- Show covariance weighting and included terms on both sides. If one expression
  hides terms in a combined covariance, expand it or derive a common form;
  saying "already included" is not a substitute for showing the connection.
- Display operations such as minimization, integration and evaluation explicitly.
  Name the variables optimized or integrated out, the variables held fixed, and
  the states at which an expression is evaluated. Never silently substitute
  "evaluate at these states" for "minimize over possible states".
- A common form must preserve meaning: show any required transformation and its
  assumptions. If that cannot be established, state the unresolved difference
  instead of forcing matching formulas or claiming equality.
- Keep the comparison in one self-contained answer so the user need not assemble
  definitions and omitted terms from several preceding replies. Explain the
  connection after displaying the common arrangement, not after repeated objections.

## Stable claims and assumptions

- Before claiming equality, identify both exact quantities, included terms,
  whether parameters are fixed or optimized, and required model assumptions.
  Retain those qualifiers later; do not broaden "one term agrees" to "same result".
- Separate mathematical statements, observed numerical agreement, current
  implementation behavior and untested expectations. A small numerical check
  proves neither an unrestricted theorem nor a physics improvement.
- Inspect relevant current code for implementation claims. Trace where each
  state, residual and covariance is constructed and used. A function name or
  old explanation is not enough. Cite the relevant file when useful.
- Correct a claim by stating the previous claim, corrected claim, and specific
  reason/evidence. Do not disguise a correction as a vocabulary change or only
  say that the preceding answer was unclear.
- A simplified example must not silently change the disputed issue. State its
  assumptions and limits first. One measurement cannot by itself establish a
  multi-layer forward-versus-smoothed claim.

## Recover without restarting

- When wording is challenged, resolve it using the existing names rather than
  another equivalent term or an unnecessary new derivation.
- If the same objection recurs, stop repeating the explanation. Identify the
  precise mismatch, check the source or a minimal calculation, and state what
  is established and unresolved. Do not blame the user's understanding or
  assert agreement without evidence.
- If the user loses the goal, summarize the original goal, requested change,
  actual implementation status, and one unresolved question. Do not restart
  the entire history or infer new implementation authority.
- Before sending, check for renamed concepts, undefined symbols, changed
  assumptions, conflated states/scores and claims stronger than the evidence.
  Fix these directly instead of appending more qualifications.
