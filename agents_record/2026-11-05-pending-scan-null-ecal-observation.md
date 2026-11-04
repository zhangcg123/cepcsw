# Deferred: consistent ECAL observation for the scan null

Status: identified by code inspection, NOT implemented. The user reports
breakpoint batch jobs running and explicitly deferred changes until later
today or tomorrow. Do not modify fitter/scan code, run cards, frozen jobs or
the installed runtime while these jobs are active. Reconfirm a quiet window
before any future installation. No job interruption is authorized.

## Exact issue

For each track, neutral clusters are collected once from the original input
track's AtCalorimeter reference and the configured theta/phi window. This
collection does not require a selected or truth breakpoint and does not vary
with the scanned location. RecBreakpoint.cpp passes the fixed raw C/N
observations to LocationFitScan.

LocationFitScan includes one null point (-1) per requested setup/track, not
one null per candidate location. fitLocation runs a fresh5D KF/RTS fit for
the null; breakpoint candidates run the free-loss wrapper. With setup110,
ElectronPhoton and a collected neutral:

- Null: uses charged energy C, with sigma_C; electron-only prediction.
- Each non-null candidate: uses C+N, with hypot(sigma_C,sigma_N), to constrain
  the electron+photon prediction. The same C/N is used at every location.

Source: HitLocationScan.cpp lines230-252 and fitLocation lines165-173;
BreakpointFitter.h combineResolvedEcalMeasurements currently requires one
interval. NeutralEcalMeasurement=False disables separate111, not passive
collection or use of N in110's sum.

Therefore the current resolved null-minus-candidate NLL difference is NOT a
same-observation likelihood ratio. NLL comparisons between successful non-null
locations remain usable when observations/masks/dimensions/sum flags agree.
Without collected N this particular C-versus-C+N mismatch does not occur.

## Agreed direction, deferred

For a fair null/candidate comparison, use the same measured C+N and the same
measurement uncertainty in both hypotheses whenever candidates use that sum.
The5D null predicts electron energy only; the augmented candidate predicts
electron+photon energy. Do not invent a loss coordinate in the null or require
a truth breakpoint to collect/use the observation. Same measurements do NOT
require equal fitted-parameter counts. Threshold calibration must still account
for the optimized loss center, its boundary and the location search; NLL gain
is not automatically a one-degree-of-freedom chi-square or eBrem probability.

This is a pending scan-reference correction, not authorization to change all
ordinary no-breakpoint endpoint families. Review that scope before coding.
Future focused tests must cover no-neutral, collected-neutral, failed/missing
ECAL and no-truth-breakpoint tracks; preserve unaffected scan candidates,
baseline outputs and existing failure flags. Existing running outputs/cards
must remain interpretable as the old null behavior, not silently relabeled.

## Reminder / handoff

Bring this issue up at the next interaction later on2026-11-05 (Asia/Shanghai)
or the following day. No standalone timed-reminder tool is exposed in this
session, so no scheduled notification has been created. The persistent
AGENTS current-focus reminder is the handoff mechanism, not a timer guarantee.

Historical status being superseded: the live focus said, "No real campaign
prepared/submitted; only one dry-run test card under /tmp." That described the
earlier agent validation. The user now reports running jobs; no queue inventory,
job count, performance test, source/run-card edit or installation was performed
as part of this record-only request. Active project laws, build instructions
and previous evidence files are preserved; no history directory was migrated.
