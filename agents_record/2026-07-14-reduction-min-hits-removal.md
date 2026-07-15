# Removal of ineffective delayed-reduction control

Date: 2026-07-14

## Decision

The user requested removal of `ReductionMinHitsAfterSplit` after source review
showed that it did not provide post-measurement reduction in the active
workflow. The removal is confined to `Reconstruction/RecGsfTracking` plus the
copy-friendly explanatory note. Historical records describing the original
experiment remain unchanged.

## Source finding

The forward reducer computed an age gate and then used:

```text
mayReduceNow = oldEnoughToReduce || justSplit
```

Every newly created over-budget forward mixture therefore reduced immediately
because `justSplit` was true, independently of the configured minimum age. The
reverse filter never consulted the property and always reduced immediately
after process convolution and before its measurement update. The setting was
therefore not a switch for the posterior-reduction experiment under discussion.

The original delayed-reduction experiment remains preserved in
`agents_record/2026-07-10-acts-gsf-review-and-lifetime-experiment.md`. Its zero
default was compatibility-preserving rather than a validated physics choice.

## Removed machinery

- the `ReductionMinHitsAfterSplit` Gaudi property, initialization validation,
  and configuration log field;
- `GsfComponent::hitsSinceSplit`, its clone propagation, split resets,
  successful-update increments, and verbose `age` column;
- the forward `oldEnoughToReduce`, `mayReduceNow`, and `defer-reduce` path;
- the property row in `RecGsfTracking/README.md` and assignment in the active
  reverse template;
- the obsolete `run_gsf_lineage_smoothing_delayed_event11.py` option.

Current reduction ordering and physics are otherwise unchanged: an
over-budget forward mixture is reduced after its same-surface outgoing split,
and the reverse mixture is reduced after its inward process split and before
the target measurement update.

## Validation

The configured EL9/LCG-105 `RecGsfTracking` target rebuilt and linked
successfully, and the complete install finished successfully. Compiler output
contained the existing external KalTest/ROOT warnings and no removal-related
error.

The legacy event-11 option first failed before event processing because its
default `tuples/trk-e--2.0-85-1.root` input is absent. A replacement
comprehensive-verbose run used the current local
`tuples285/trk-e--2.0-85-284.root`, seed/event 284/1. It completed with:

- 232/232 retained hits;
- 2,539 accepted forward updates, zero recovery, and zero rejection;
- 2,772 accepted reverse updates and zero reverse rejection;
- seven reverse splits and seven reverse reductions;
- 12 finite final reverse components;
- reverse best-branch IP pT `1.99265 GeV` versus truth `2.0004 GeV` and LCIO
  `1.9837 GeV`.

Gaudi returned scheduled-stop code 4 after successful finalization because the
selected event had been processed. The diagnostic log is
`/tmp/gsf-remove-reduction-min-hits-284-1.log`; ROOT outputs remain generated
artifacts under `/tmp`.

## Scope caveat

Generated `DumpGsfTrks/rungsf-*.py` cards outside the authorized
`RecGsfTracking` implementation scope still contain explicit zero assignments.
They were not modified because they are generated batch cards and unrelated
working-tree outputs. They must be regenerated or have the stale assignment
removed before reuse with the new installed component configuration.

