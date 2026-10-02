# Diffuse augmented RTS maintained-card default (2026-10-03)

At the user's request, `Reconstruction/RecBreakpoint/options/run_breakpoint.py`
now explicitly sets `DiffuseAugmentedRTS=True` when
`BP_DIFFUSE_AUGMENTED_RTS` is unset. `BP_DIFFUSE_AUGMENTED_RTS=0` still
selects the ordinary-RTS copy in the extra output. The C++ compiled property
default remains false, so an unrelated Gaudi card that omits the property
does not silently enable this experiment. The batch helper already freezes
explicit `BP_DIFFUSE_AUGMENTED_RTS` overrides and copies the maintained card
into each generated job card. Cards generated before this change keep their
frozen setting; they are not rewritten.

This changes steering only. It does not add a diffuse backward-filter result,
alter the ordinary pair, or establish physics validation. The method and
focused mechanical evidence are in
`agents_record/2026-10-03-exact-diffuse-breakpoint.md`.
