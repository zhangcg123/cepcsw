# Retired LocalMarginal breakpoint fit (2026-10-19)

`RecBreakpoint` now fits a selected breakpoint only with `Persistent6D`.
The retired `LocalMarginal` loss map, covariance marginalization, loss recovery,
and multi-interval route were removed from the fitter. Empty interval lists
still use the unchanged five-dimensional reference KF/RTS path. Truth-prior,
diffuse, free-loss, ECAL-guided, and optional backward outputs retain their
existing interfaces. The detailed former mathematics was moved to
[the historical walkthrough](2026-10-19-retired-local-marginal-walkthrough.md).

`LossStateMode="Persistent6D"` remains as a fixed compatibility property so
previously generated cards with that explicit value still run. `LocalMarginal`
fails clearly in standalone cards, batch preparation, and Gaudi initialization;
manual multiple-breakpoint lists are rejected. The `loss_state_mode` flat-tuple
branch is retained with its existing value and schema.

Focused before/after run on five events from
`sim_large_barrel_20261018/rec-barrel-1.root` produced six track rows,
including one selected-breakpoint row and five empty-interval rows. All 609
flat-tuple branches matched exactly. All six `RecBreakpoint` C++ tests passed
under `setup.sh`. The Python batch suite passed 39/40 tests; its existing
`test_submission_wrapper_full_chain_default_and_rec_override` case failed
because its external `rec-barrel-12.root` fixture was absent, not because of
this removal.
