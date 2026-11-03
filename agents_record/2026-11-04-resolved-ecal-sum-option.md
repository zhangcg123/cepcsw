# Optional resolved ECAL total-energy measurement

## Scope and control

The user requested a maintained option for resolved charged+neutral showers to
constrain the total rather than the two parts separately. No PFA, shared KF,
GSF, ownership, loss-map, optimization-bound or optimizer-acceptance changes.
No batch submission or existing output overwrite. Work remains on
`breakpoint-delayed-neutral`; no remote/branch operations.

`ResolvedEcalEnergyMode` accepts `Separate` (compiled/card/batch default) and
`Sum`. Maintained card and batch environment: `BP_RESOLVED_ECAL_ENERGY_MODE`.
Both charged and neutral switches must be enabled and their observations usable,
with exactly one selected breakpoint, for Sum to apply. Missing parts and
no-breakpoint controls keep the existing routing. Failed new fits retain the
preceding accepted stage. All backward switches retain their previous defaults.

The final neutral-stage ordinary/free-loss families (111 when all switches are
on) replace their two observations with one terminal observation in Sum mode:

```text
C, sigmaC: selected charged ECAL energy and its assigned error [GeV]
N, sigmaN: collected neutral energy and quadrature-summed error [GeV]
y = C + N
sigma_y = hypot(sigmaC, sigmaN)
h(x) = p_track_at_ECAL * exp(b)
b = log(p_before / p_after)
```

This reuses the existing charged electron+photon energy projection, native
transport, scalar KF update, RTS smoothing and shared normalized likelihood.
No separate neutral update or neutral likelihood remains in that fit. The
delayed-neutral request is ignored only when the sum is actually formed.
Quadrature assumes independent measurement errors. ElossOn must be off for
this single-loss/no-other-energy-change model. No containment correction.

000/100/110 are unchanged. In resolved events, 110 continues to constrain only
the charged electron. `combineResolvedEcalMeasurements` normalizes a local copy
inside the shared BreakpointFitter; every ordinary fit and Minuit trial starts
from raw C/N, so neither errors nor neutral energy accumulate across trials.
The exact-diffuse workflow is unchanged. No new output collection family.

## Persisted interpretation

- `resolved_ecal_energy_mode`: requested Separate/Sum.
- `<family>rts_resolved_ecal_sum_applied`: actual fitted observation; false on
  inactive/default/fallback states unless the retained source really used Sum.
- `location_scan_resolved_ecal_sum_applied`: aligned with every scan entry,
  including source inheritance. No-breakpoint111 still copies110, flag false.
- Actual mask is 3 for beam+sum, not 7: there is one terminal charged-site
  observation and no independent neutral-site observation. Use the flag with
  mask and dimensions when grouping comparable likelihood models. A successful
  sum and a charged-only fallback cannot be treated as identical observations.
- Original C/N, cluster indices and errors are retained unchanged.
  `charged_ecal_effective_energy_mode` still describes the charged stage110;
  final111 treatment is identified by the new applied flag.

Do not compare NLL across Separate and Sum as equal-dimensional hypotheses.
Existing analysis readers that assume mask3 means charged-only need to inspect
the new flag before using Sum samples. Historical files without the flag used
the unchanged Separate mode.

## Validation and deployment

Temporary build/runtime/tests:
`/tmp/bp-resolved-sum-option-h8IwS7/`.
15 CTests and 60 Python batch-helper tests passed; extended ROOT tuple test
checks the marker is booked, read back and reset. Shell syntax, Python syntax
and diff whitespace checks pass. Gaudi property generation defaults Separate.

Same-code staged Separate/Sum Truth runs use existing barrel seed1 events
5,11,16,17,31,38, with full verbose state/covariance diagnostics. Default matches
the prior full-IP installation's events11/16/17 across every old branch.
All earlier-stage fields and no-neutral/no-breakpoint controls are unchanged.
Resolved Sum endpoints for events5/31 exactly reproduce the old temporary sum
experiment's IP parameters and covariance; no separate neutral update remains.

| Seed:event | Baseline KF residual [%] | Separate delayed111 [%] | Sum111 [%] |
|---|---:|---:|---:|
| 1:5 | -19.459462 | -0.467135 | +2.645584 |
| 1:31 | -8.197376 | +0.126992 | +0.017440 |

Residual = 100*(reconstructed IP pT - matched truth pT)/matched truth pT.
These are mechanism checks, not population validation or universal improvement.
The earlier selected 24/50-track experiments are preserved at
`/tmp/bp-resolved-total-energy-O50te3/`; their truth-selected performance must
not be advertised as inclusive evidence.

Full HitScanAll Separate/Sum event5 runs also passed: 40 aligned entries, Sum
applied at all nine non-null sites, no-breakpoint111 inherits110, and earlier
000/100/110 entries are exact. The Separate scan matches every old branch in
the prior full-IP output. Across the normal runs, 2,985 legacy branch-row values
and 5,408 unchanged A/B branch-row values are exact. Both actual Sum endpoints
use mask3 and no delayed/separate neutral update; optimizer prediction+residual
equals raw C+N. Validation script and output are `validate.py`, `validation.log`
in the temporary directory.

The user explicitly confirmed no active jobs and authorized installing after
validation. Package-only installation is complete. An EvtMax0 probe through the
normal setup (no temporary runtime override) verifies the compiled default is
Separate, accepts Sum and books the new scalar/endpoint/scan fields. It exits
successfully. Installed and staged libraries match SHA256
`42fcda65da4dbde569481c494eb34fb2f62ae15b09347035cc4cd58e6f136416`.
The previous installed library is preserved as `previous_RecBreakpoint.so` in
the temporary directory (SHA256
`ccb10088f63a2945d42b2fae071174cda392c21a026d0ec888db9d44fb572e48`).
Existing prepared campaign cards/data are untouched. New batch cards must be
prepared to freeze Sum; submit-prepared does not change existing steering.

## Project-memory preservation

The complete outgoing AGENTS is preserved without removal of unique evidence
in `2026-11-04-agents-before-resolved-sum-option.md`. The scan campaign details,
prepared-card runbook and statistics moved there; essential live scan contracts,
ownership/audit references and all global project laws remain in AGENTS.
