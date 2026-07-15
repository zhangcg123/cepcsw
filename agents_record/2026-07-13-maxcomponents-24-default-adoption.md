# Adoption of `MaxComponents=24` as the active default

Date: 2026-07-13

## Decision

After reviewing the focused overshoot and reproducible random 100-event
capacity comparisons, the user explicitly selected `MaxComponents=24` as the
new project default. This is an intentional operating-point decision. It does
not erase the measured tradeoff or constitute production validation.

The outgoing `MaxComponents=12` decision and all 12/24/36 evidence remain in:

- `2026-07-13-maxcomponents-24-overshoot-focused-test.md`
- `2026-07-13-random-light100-maxcomponents-24-test.md`
- `2026-07-13-random-light100-maxcomponents-36-test.md`

## Implementation

The C++ Gaudi property default is now 24. The documented default, standard
electron steering files, focused event-11 steering, reverse-template
environment fallback, and reverse-sample helper default were changed to 24.
The intentional single-component smoothing diagnostic remains at 1. Historical
plot defaults and result labels remain 12/24 so existing comparisons retain
their meaning.

## Verification

`RecGsfTracking` built successfully and the configured CEPCSW install
completed. The generated Gaudi configuration reports `MaxComponents: 24`.

A default-helper, aggregate-weight, comprehensive-verbose run was performed on
light events 284/1 and 404/8 without passing `--max-components`. Both jobs
terminated successfully, retained complete output, and logged:

```text
GSF configuration: maxComponents=24 ... verbose=1/1/1
```

A direct comparison against the earlier explicitly configured 24-component
outputs found zero changes in both events. Thus the installed default path
reproduces the already-audited explicit-24 behavior.

## Known tradeoff and forward baseline

In the fixed random 100-event topology-clean light sample, 24 improves the
central-68 half-width from 0.4144% to 0.3463% relative to 12, while events
inside +/-1% fall from 85 to 83 and RMS changes from 1.2260% to 1.2281%.
There are nine material absolute-residual improvements and four worsenings.
Event 404/8 is a strong capacity-enabled recovery, while late-loss event 284/1
is a capacity-enabled false inner-radiation correction. Increasing further to
36 is worse than 24 on the same sample.

Use 24, not 12, as the baseline for subsequent optimization and regression
comparisons. Continue to track clean-core degradation, false inner-radiation
selection, hard-loss recovery, and memory growth explicitly.
