# Outgoing RecBreakpoint focus before objective-only beam test

This is the complete outgoing `AGENTS.md` current-focus text, preserved before
the beam-guided free-loss experiment changed the active question.

> Active work remains RecBreakpoint on local `breakpoint`. Its shared-width
> Gaussian LocalMarginal and Persistent6D workflows, the separate free-loss
> prior-centre fit, and the truth-centred diagnostic are mechanically available.
> The free-loss fit is not a safe overall replacement: the completed 2,000-event
> study found central hard-loss recovery but light-loss degradation and large
> positive tails. In the later 50-event diagnostic, free-loss had lower complete
> RTS chi2 than truth-centred fits in 47 events, yet worse absolute IP pT in 32
> of those. This is selected mechanism evidence, not production validation.
>
> The only retained tuple inputs are `sim_large_barrel_20260823/` (251,000
> events) and `sim_large_endcap_20260823/` (100,000 events). All other generated
> tuple ROOT files, including tracker, GSF, breakpoint and temporary outputs,
> were removed at the user's request. Historical analysis tables, plots, logs
> and dated records remain; do not refer to their raw ROOT tuples as available.
> The exact cleanup inventory is in
> `agents_record/2026-10-01-sim-only-tuple-cleanup.md`.
>
> The source and installed RecBreakpoint module now serialize measurement,
> process and seed contributions by default for ordinary, free-loss and
> truth-centred RTS results. Direct tests showed unchanged endpoints and score
> closure; the package-only deployment is recorded in
> `agents_record/2026-10-01-score-breakdown-deployment.md`. Before a new batch,
> regenerate tracker inputs from the retained simulation sample. Then investigate
> light-loss failures and extreme tails with the saved per-hit score breakdown
> before proposing model changes.
> No new objective, prior or default physics change has been authorized.
>
> Detailed current evidence and retained outputs:
> `agents_record/2026-09-30-free-loss-shared-sigma.md`,
> `agents_record/2026-09-30-free-loss-population.md`,
> `agents_record/2026-10-01-breakpoint-default-score-breakdown.md`, and
> `agents_record/2026-10-01-breakpoint-score-more50.md`. The complete outgoing
> focus is preserved verbatim in
> `agents_record/2026-10-01-before-sim-only-tuple-cleanup.md`.

The tuple inventory above was true at the earlier cleanup; on 2026-10-02 the
workspace instead contains new `sim_large_barrel_20261001/` and
`breakpoint_barrel/` ROOT campaigns (100 files each). The earlier two simulation
directories are not present in this workspace now.
