# Retired neutral-cluster breakpoint constraints (2026-10-20)

## Outgoing focus, preserved verbatim

Active work on local `breakpoint` is the optional charged-ECAL measurement in
the free-loss Persistent6D fit. Its new terminal ECAL-face state receives an
energy-only Kalman update; RTS traverses that ECAL-to-last-tracker edge before
continuing inward. `FreeLossChargedEcalMeasurement` remains default-off, fits
no ECAL cluster position, and leaves the neutral-ECAL branch separate. All
backward filters remain default-off. The OFF path reproduced all 609 focused
pre-existing flat-tuple branches, six package CTests passed, and the fixed
100-event barrel hard-loss/no-collected-neutral ON cohort produced 100 rows
with 98 converged fits. Its median absolute IP pT error remained worse than
the earlier tracker-only output; this is a mechanical gate, not physics
validation. Native propagation does not expose the terminal transition cross
covariance, so the ECAL transport is numerically differentiated. Charged
cluster independence/calibration and unresolved-photon contamination remain
the leading physics risks.

Next: verify this terminal covariance model and charged-cluster truth content,
then perform same-final-code, held-out clean/light/hard-loss A/B comparisons,
including endcap events, before considering default-on steering. Do not add
ECAL position constraints in this phase. Exact behavior, outgoing focus and
tests are in `agents_record/2026-10-19-explicit-charged-ecal-face-rts.md`.

## Change

At the user's request, both neutral-energy-constrained fit paths were removed:
the `FreeLossEcal` parallel likelihood search and the absolute-energy RTS
controlled by `EcalLossReferenceMode`. Their C++ fitter branches, obsolete
absolute-loss mapping, Gaudi controls, EDM collections, flat-tuple endpoints,
run-card steering, and batch controls are no longer active. Old controls are
rejected explicitly. Historical implementation and results remain in prior
dated records and Git history.

The per-track neutral-cluster selection fields and the event-level
`neutral_pfos` tree remain diagnostic-only. They do not supply any breakpoint
prior or likelihood. The charged-ECAL terminal measurement is separate and
unchanged, still default-off. New tuples omit the retired endpoint branches;
old tuples remain readable but have a different schema.

## Checks and remaining work

The package builds, all six package CTests pass in the CEPCSW environment,
and focused batch-card tests verify removal and rejection of retired controls.
The installed plugin processed a real rec event with one fitted track row.
A second local smoke with Manual interval 5 completed ordinary, free-loss,
diffuse, and truth-assisted fits (`status=1`, `free_loss_status=2`,
`diffuse_augmented_status=2`, `truth_override_status=1`). Both new tuples
contain the diagnostic neutral fields and no `free_loss_ecal_*` or
`absolute_neutral_*` branches. The full batch-helper suite has one unrelated
failure in its full-chain submission-wrapper test because the locally modified
wrapper selects an absent external `rec` input. No batch jobs or population
comparison were run for this removal.
