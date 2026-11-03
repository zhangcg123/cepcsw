# Automatic resolved sum110; remove the selector

User requested removal of ResolvedEcalEnergyMode: when a collected neutral is
available,110 constrains the sum. This supersedes the optional routing in
`2026-11-04-resolved-sum-in-110.md` (commit b9bba7c); that record retains the old
option/default, configuration details, validation and deployment evidence.

## Current contract

- 000: tracker only. 100: tracker + beam. Neither receives ECAL observations.
- 110 with one selected breakpoint and valid charged/collected neutral energies:
  ALWAYS use C+N (charged plus collected neutral energy), with
  sigma=hypot(sigmaC,sigmaN) against p_ECAL*exp(b).
  Passive neutral collection still works with NeutralEcalMeasurement off.
- 110 without a collected neutral: ChargedEcalEnergyMode still chooses
  ElectronOnly or ElectronPhoton. No breakpoint: electron-only charged energy.
- 111 always attempts separate charged-electron and neutral-photon observations,
  from raw inputs/original seed, not the Sum110 posterior. Existing early/delayed
  neutral placement is unchanged. Missing/failed stages retain accepted earlier
  results with statuses/masks and actual sum flags. No selectable111 sum model.
- Shared ordinary/free-loss RTS and HitScanAll routing. No diffuse, backward,
  PFA, shared-KF, loss-map, ownership, optimization-bound or acceptance changes.
- ChargedEcalMeasurement requires ElossOn=False, since resolved tracks now always
  use the one-loss/no-other-energy-change sum model. The maintained card already
  has ElossOn=False. Neutral-only/beam-only workflows keep their existing gates.

Removed the Gaudi property, enum/parser, Python assignment, shell export, frozen
control/default/validation and obsolete requested-mode tuple scalar
resolved_ecal_energy_mode. Stale BP_RESOLVED_ECAL_ENERGY_MODE requests are rejected
by preparation/the maintained card instead of silently ignored. Old cards with
the deleted property must be regenerated; existing frozen cards/data are not
rewritten. Sum-applied endpoint/scan flags remain for actual-treatment/fallback
diagnostics. An internal fitter routing flag is not exposed as a user control.

## Validation/deployment

Temporary runtime and evidence: `/tmp/bp-automatic-sum110-ah5wYs/`.
15 CTests and59 Python batch-helper tests pass. Generated Gaudi properties no
longer contain the selector. Shell/Python syntax and diff whitespace checks pass.
Focused normal and110-only tests passed, comparing against the previous explicit
Sum110 / separate111 workflow. Seed1 events5,11,16,17,31,38 match every remaining
branch exactly (6,024 branch-row values after removing the requested-mode label).
Resolved110-only events5/31 match2,008 values. An extra event5 with ElectronOnly
and the neutral switch off still reproduces the sum, apart from the requested
unresolved-mode label. One full HitScanAll event5 matches1,004 branch-row values
exactly:40 entries, including nine Sum110 sites, unchanged000/100/separate111,
all IP/covariance/likelihood/status arrays and actual-treatment flags.
Ten local event passes in total. These are regressions, not population validation.
Evidence: validate.py, validation.log, verbose Gaudi logs in the temporary directory.
The user confirmed no active jobs and authorized installation after validation.
Package-only installation is complete. A default-environment EvtMax0 probe
confirms the Gaudi selector is absent, the obsolete requested-mode branch is
absent, and endpoint/scan sum-applied flags are retained. It exits successfully.
Installed and staged libraries match SHA256
`dca4904a50daad1e997da8f2ead868c9fd8afea38b4f2c2261eb3b6a595e0ce8`.
Prior installed library is saved as previous_RecBreakpoint.so in the temporary
directory. No PFA/shared package was installed or modified.
No Condor jobs submitted; no existing tuple/card overwrite or remote operation.

## Memory preservation

The live focus is corrected in place; prior selector/default/stage-placement
details and numerical evidence remain in the two resolved-sum records and
earlier complete AGENTS snapshot. Global project laws and the ongoing scan-only
contract are unchanged. No historical evidence removed.
