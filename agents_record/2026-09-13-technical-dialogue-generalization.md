# Generalize the technical-dialogue skill

On 2026-09-13 the user requested a domain-independent skill rather than rules
specialized to the tracking-objective discussion. The skill now covers stable
language, comparable presentation, explicit reasoning, evidence, corrections
and recovery of the original question. No domain glossary or prescribed
scientific conclusion remains in the active skill. AGENTS.md still requires it.

The removed material is preserved below, not discarded. It is historical
project context, not required input to the generalized skill. Its old path was
project_skills/consistent-technical-dialogue/references/tracking-terms.md.
The revised skill was authored using skill-creator and checked with its validator.
No tracking source, run cards, outputs or remote state were changed.

## Previous project instruction

For multi-turn algorithm explanations, derivations and disputed technical
claims, use the skill at project_skills/consistent-technical-dialogue/SKILL.md.
Keep one canonical name per concept, define symbols before use, and explicitly
label corrections and changed assumptions. Do not rename the same quantity
across replies or conflate a state, residual, chi2 contribution and full
objective. Read its linked terminology reference for tracking discussions.

## Previous skill, verbatim

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

## Previous tracking reference, verbatim

# Stable names for CEPCSW tracking

These names separate concepts; they do not establish equality between scores.
Current source determines what a field contains. Preserve agreed symbols;
do not introduce symbols merely to restate these definitions.

| Canonical name | Meaning |
|---|---|
| Forward-predicted state | State at a hit before using that hit, obtained by propagation from the preceding forward-updated state. |
| Forward-updated state | State after using that hit. Forward-filtered state is an alias; map once, then use forward-updated state. |
| RTS-smoothed state | State at a hit after RTS incorporates later-hit information through the buffered forward model; not an independent backward-filter state. |
| Residual | Difference between two explicitly identified quantities at specified locations. A residual is not a chi2 value. |
| Forward-predicted measurement residual | Measured coordinates minus those predicted from the forward-predicted state. Innovation is an alias; do not alternate names. |
| Smoothed measurement residual | Measured coordinates minus those predicted from the RTS-smoothed state. Specify captured linearized versus native measurement evaluation. |
| Smoothed propagation residual | Destination RTS-smoothed state minus the preceding RTS-smoothed state transported with the specified model and fixed trial loss. |
| Seed residual | First RTS-smoothed state minus the initial seed mean, with matching parameterization and pivot. |
| Chi2 contribution | Scalar residual^T * inverse covariance * residual. Identify that covariance. Penalty and quadratic contribution are aliases, not additional quantities. |
| Forward innovation chi2 sum | Sum of chi2 contributions from forward-predicted measurement residuals. Prediction covariance contains propagated seed/state uncertainty and process noise. Not a direct complete residual score of the stored forward-updated trajectory. |
| Complete smoothed chi2 | Sum of smoothed measurement, smoothed propagation and seed chi2 contributions. Measurement-only smoothed chi2 is different. |
| Global measurement chi2 | Measurement residuals relative to the joint Gaussian model mean, weighted with their full joint covariance. Specify the model; do not silently identify it with a native forward field. |
| Minuit objective | Complete scalar actually returned to Minuit at one trial loss. List its terms; do not abbreviate it to chi2 if other terms are included. |
| Trial loss | Loss parameter currently evaluated. In maintained free fitting, b is a log momentum-loss parameter, not a percentage; verify its mapping when needed. |

## Guardrails for this discussion

- Use chi2 consistently for covariance-weighted squared-residual quantities.
  If code calls a field quadratic, map it once to the relevant chi2 name.
  Do not call a residual vector and its chi2 contribution the same thing.
- Never say "forward and smoothed are the same". Identify whether comparing
  states, residuals, per-hit chi2, complete chi2, Minuit objectives, fitted
  losses or final IP momenta.
- Distinguish measurement/process noise, predicted/updated/smoothed state
  covariances, and residual covariances. Do not substitute them by analogy.
  Singular noise requires constraints on unsupported directions.
- Score comparisons retain the same loss, hits, seed treatment, noise and
  model unless a difference is the explicit subject. Comparing at fixed loss
  does not establish equal minimizing losses for different objectives.
- This glossary prescribes no answer to the disputed equality. Inspect code
  and establish assumptions before claiming a mathematical or numerical match.

## Arrange a tracking-chi2 comparison

When a common complete-trajectory expression is justified, display measurement,
propagation and seed contributions in the same order on both sides, each with
its residual and inverse covariance. Keep any determinant/normalization terms
explicitly separate on both sides if comparing full Minuit objectives.

If an equality involves minimizing over all trajectory states at fixed trial
loss, show that minimization on the relevant side. Distinguish it from evaluating
the same expression on stored forward-updated states or RTS-smoothed states,
and from Minuit varying the trial loss. Do not omit these operations just to make
two expressions look identical. This is a presentation requirement; the common
expression and any claimed equality still need mathematical/code verification.
