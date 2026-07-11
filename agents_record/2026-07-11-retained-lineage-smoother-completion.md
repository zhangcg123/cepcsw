# Retained-lineage smoother and material-model audit completion — 2026-07-11

This record preserves the outgoing project focus before concentration moved to
the transition-matched CEPC Bethe-Heitler training dataset. Full implementation
details, intermediate failures, branch tables, and provenance are in
`agents_record/2026-07-11-retained-lineage-smoother-start.md`.

## Completed implementation

The user authorized a narrow TrackSystemSvc interface extension after the
audit showed that `RecGsfTracking` could not construct an RTS smoother without
the exact inter-surface transport Jacobian and process noise. `MeasurementUpdate`
now exposes the existing KalTest `F` and `Q`; all smoothing logic remains in
`Reconstruction/RecGsfTracking`.

The opt-in `RetainedLineageSmoothing` path composes the exact propagator with
the selected surface-local BH process Jacobian and runs an RTS recursion on
each retained forward lineage. It deliberately requires `TopN`, disables the
experimental reverse refit, and currently supports geometric IP extrapolation
only. A failed branch smoother suppresses track publication rather than
silently returning a filtered fallback.

Focused events 11, 16, and 17 retain 234/234 hits and smooth 7/7, 7/7, and
12/12 global-model branches. Their IP pT values are 1.7933, 1.8118, and 1.5789
GeV versus truth 2.0004 GeV and LCIO 1.7934, 1.8118, and 1.5790 GeV. This is a
mechanically stable negative result, not hard-loss recovery or full smoother
validation.

## Localized loss of backward information

Exact MarlinTrk covariance closure is typically at the `1e-6` level and no
worse than about `1e-4` in the first checked transitions. The fixed global BH
model instead adds curvature variance of about `7.6e-6` even for its
near-no-loss component and about `4.4e-3` for its `z=0.975` component, versus
incoming variance near `1.0e-7`. The first backward curvature gain falls to
about `0.013` or `2.3e-5`, preventing later measurements from constraining the
pre-loss state.

The no-BH one-component control has curvature gain near one. Event 11 publishes
1.7932 GeV, within 0.0002 GeV of LCIO. Large-sample covariance-pull validation
on the running muon sample remains useful but is not the active blocker.

A diagnostic three-hit delayed-TopN run retained 12 rather than 7 event-11
histories. The selected result stayed 1.7933 GeV and the largest smoothed branch
reached only 1.793415 GeV. Pruning is therefore not the immediate blocker and
delayed TopN is not a validated final policy.

## ACTS reference and corrected material ownership

`BHModel="ActsAtlas"` faithfully implements ACTS's default ATLAS regime
selection: no change below `0.0001 X0`, one analytic Gaussian below `0.002 X0`,
the six-component low polynomial model below `0.1 X0`, and the high model up to
a `0.2 X0` cap. It is a comparison model, not CEPC validation.

Component-local incidence correction is implemented with explicit
outgoing-current-surface ownership:

```text
pathTX0 = normalTX0 / abs(unitTangent dot unitSurfaceNormal)
```

The tangent comes from the filtered component and the DD4hep normal is
evaluated at its pivot. The final forward surface has no outgoing transition;
invalid near-tangent projections are rejected. Forward and reverse workflows
use the same current-surface convention in their respective directions.

Event 11's first two corrected values are `0.000573798` and `0.000566019 X0`.
The largest corrected paths for events 11, 16, and 17 are `0.00163546`,
`0.00163468`, and `0.00164820 X0`. All stay below ACTS's `0.002 X0` mixture
threshold, so ActsAtlas remains single-component and produces 1.7934, 1.8118,
and 1.5790 GeV.

## Decision handed to the next focus

Stop further reverse-refit investigation, global-model tuning, and pruning
optimization until a physically matched process model exists. The next model
must be trained on primary-electron Geant4 pre/post-step truth aggregated over
the same owned reconstruction intervals used by the filter. In the actual
`1e-4` to approximately `1.7e-3 X0` range it must retain an explicit near-loss
core plus low-weight hard-loss hypotheses, with narrow variance conditional on
each hypothesis. Its tail weights must scale with `t/X0`.
