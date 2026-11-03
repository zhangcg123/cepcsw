# Move resolved Sum to110; keep one separate111 workflow

## User-directed correction

The user requested: resolved110 may use the sum previously implemented as111Sum;
remove the111Sum workflow and retain separate111 only. This supersedes the
stage placement in `2026-11-04-resolved-ecal-sum-option.md` (commit2eeacad),
not its measurement algebra or focused historical evidence. No new selector,
output family, photon collection, PFA/shared KF change, optimization change,
branch operation, remote operation or batch submission.

The same `ResolvedEcalEnergyMode` property controls resolved110 only:

| Case | 110 | 111 |
|---|---|---|
| Resolved, Separate default | Charged electron only | Charged electron and neutral photon separately |
| Resolved, Sum | Charged+neutral total constrains electron+photon total | Charged electron and neutral photon separately |
| No collected neutral | Existing charged mode (unresolved sum with ElectronPhoton) | Copy preceding accepted stage |
| No selected breakpoint | Charged electron only | Copy preceding accepted stage |

Sum110 needs a valid charged observation, collected neutral energy and one
breakpoint; the separate-neutral switch need NOT be on. Neutral collection is
already passive. `ElossOn=False` is required for Sum even if that switch is off.
The measured total is C+N (charged plus collected neutral energy), with
sigma=hypot(sigmaC,sigmaN) for their assigned errors, assuming independent
errors. The shared terminal projection is p_ECAL*exp(b), unchanged.

The controllers retain the raw parts, never the fitted posterior. For111 they
force Separate and refit with the original charged-electron and neutral-photon
observations. Sum is not an additional observation in111; no total+neutral
double counting. Ordinary111 uses its existing early neutral measurement;
free-loss111 retains its early/delayed selector. The same routing is used by
HitScanAll. 000/100 and uncollected-neutral/no-breakpoint controls are unchanged.
Missing/failed new fits retain the preceding accepted result as before; a
failed111 can therefore retain Sum110, explicitly marked as a fallback, not a
selectable111Sum fit.

`charged_ecal_effective_energy_mode` now reports ElectronPhoton when resolved110
is routed to Sum. Endpoint and scan `resolved_ecal_sum_applied` flags remain
the authoritative actual-treatment markers. A successful Sum110 has mask3 with
beam; successful separate111 has mask7. The global requested selector alone does
not say which treatment a fallback actually retained. Compare NLL only for
matching observation models, dimensions and treatment flags.

The compiled/card/batch default stays Separate. The same maintained environment
control is `BP_RESOLVED_ECAL_ENERGY_MODE=Sum`. Regenerate prepared cards to change
steering. Existing ROOT outputs and frozen cards are untouched. Historical
111Sum files remain distinguishable by their applied flag placement.

## Validation/deployment

Temporary runtime and evidence: `/tmp/bp-resolved-sum110-x7DcDF/`.
15 CTests and60 Python batch-helper tests pass. The scalar measurement algebra
is unchanged; the new unit check verifies separate111 retains the raw C/N.
Same-code normal checks passed on seed1 events5,11,16,17,31,38 in both modes,
plus110-only Sum runs on events5/31:14 event passes. All 6,030 existing branch-row
values in default Separate agree exactly with the previous installation output.
000/100 and no-neutral/no-breakpoint controls are unchanged. Resolved Sum110
IP parameters/covariance, per-hit states and NLL reproduce the previous Sum111
fit exactly, for ordinary and free-loss. Successful separate111 retains every
previous separate111 field, including delayed-neutral diagnostics.
Sum110 also matches with NeutralEcalMeasurement off; its111 output is inactive.

| Seed:event | Charged-only110 IP residual [%] | Sum110 [%] | Separate111 [%] |
|---|---:|---:|---:|
| 1:5 | +13.554865 | +2.645584 | -0.467135 |
| 1:31 | +3.299683 | +0.017440 | +0.126992 |

Residual =100*(reconstructed IP pT - matched truth pT)/matched truth pT.
Mechanism checks only, not an inclusive performance claim.

One full HitScanAll Sum run on event5 also passed:40 entries, nine Sum110 sites.
Every000/100/111 entry matches the previous Separate scan, and the110 sum states,
covariances, losses and likelihoods match the previous111Sum entries at each site.
Null111 still inherits110. Applied flags/masks and raw-total prediction+residual
checks pass. Evidence: `validate.py`, `validation.log`, verbose Gaudi logs.

The user explicitly reconfirmed no active jobs and authorized installation after
validation. Package-only installation is complete. An additional resolved event5
through the normal setup, without a temporary runtime override, reproduces all
948 staged track fields exactly and verifies Sum110 / separate111. Total16 local
event passes. No Condor jobs submitted, no existing tuples/cards overwritten.
Installed and staged libraries match SHA256
`008129b9f67e29aaa16eb1352d17865f538d00350ecd2141f7010bdee284a3a8`.
The prior library is preserved at `previous_RecBreakpoint.so` in the temporary
directory. The library installation does not modify PFA or shared KF sources.

## Live-memory update

The previous focus's complete resolved-sum contract and deployment evidence are
retained in `2026-11-04-resolved-ecal-sum-option.md`; the earlier full AGENTS
snapshot and scan records remain unchanged. Only the active stage-placement
description is replaced, without weakening project laws or dropping evidence.
