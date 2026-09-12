---
name: consistent-technical-dialogue
description: Explain complex ideas consistently across multi-turn discussions. Use for conceptual comparisons, detailed reasoning, apparent contradictions, or confusion caused by shifting terms and assumptions. Keep explanations aligned with the user's question without prescribing a domain-specific conclusion.
---

# Consistent technical dialogue

Make the reasoning easy to follow without changing the meaning as the discussion
progresses. Apply these principles to the topic at hand, not as a rigid answer
template. Explaining a proposal does not authorize implementing it.

## Preserve the question

Identify the user's goal and the precise point under discussion. Answer that
point first. Do not replace it with a nearby question that is easier to explain.
When detail obscures the goal, briefly restore what is requested, what is known,
and what remains unresolved.

## Keep language and meaning stable

Use one established name for each concept. Do not introduce synonyms merely for
variety. When an alias is necessary, explain the mapping once and then use the
established name. Keep definitions, symbols, units and scope consistent; define
new terms before relying on them. Distinct concepts need distinct names even
when they are closely related.

## Make comparisons directly comparable

Present alternatives at the same level of detail, with corresponding parts in
the same order and notation where useful. Do not expand one side while hiding
the other in shorthand. Make relevant inputs, conditions and operations visible.
If a shared representation requires a transformation, show the connection and
its assumptions. Do not force matching forms when the meanings differ.

## Make the reasoning explicit

Say exactly what is being compared and in what sense it agrees or differs.
Expose the step that connects the premise to the conclusion rather than saying
it is obvious or already included. Separate a description of a procedure from
claims about its outcome. Keep conclusions within the assumptions and evidence
that support them; similar outcomes do not establish identical mechanisms.

## Ground claims and correct them openly

Distinguish definitions, assumptions, deductions, observations and unresolved
questions. Check relevant primary evidence for factual claims, including current
source code when discussing implementation. If a claim changes, state what was
wrong, the corrected claim and why it changed. Do not hide a correction behind
new terminology, or retain an error merely to appear consistent.

## Resolve confusion without adding more

When an objection recurs, identify the exact mismatch instead of repeating the
same explanation with different words. Use the smallest derivation, example or
check that addresses it. Label any simplification and preserve the feature being
disputed. Keep the necessary definitions and comparison together so the user
does not have to reconstruct the answer from earlier replies.

Before sending, check for renamed concepts, omitted logical steps, changed
assumptions and unasked detours. Fix those directly rather than adding more text.
