# Counterfactual loss-scanner retirement (2026-08-31)

The default-off, truth-assisted counterfactual loss scanner was removed from
the active `RecGsfTracking` implementation and configuration surface to keep
the production candidate focused on live GSF mechanisms. It was a log-only
diagnostic that cloned manually specified loss hypotheses at truth-selected
transitions; it never entered the live mixture or persisted an EDM/flat-tuple
schema.

Removed properties:

- `CounterfactualLossScan`
- `CounterfactualTruthTransitionMap`
- `CounterfactualLossFractions`
- `CounterfactualLossVariance`

The associated reverse-loop branch construction, propagation, result logging,
reverse-template environment controls, maintained-card assignments, sample
runner controls, and dedicated log-analysis scripts were removed. The generic
fixed-loss state transformation remains required by
`RecGsfGlobalLossRefitter`; it was renamed internally from
`applyCounterfactualReverseLoss` to `applyReverseLossHypothesis` without a
behavioral change.

The active property inventory changes from 43 to 39. The maintained
`DumpGsfTrks/gsf.py.bk` card explicitly steers 38 of 39 properties and still
inherits only `RecordTruthMaterialIntervals=true`. Generated historical cards
that assign any removed property are incompatible with the current
Configurable and must be regenerated; their existing ROOT outputs remain
interpretable.

The original mechanism and evidence remain in
`agents_record/2026-07-14-counterfactual-truth-surface-loss-scan.md` and other
dated records. This retirement does not implement the proposed identity-
backbone, single-radiative-hypothesis bank; that is a separate design requiring
its own review and validation.

Validation after removal:

- `RecGsfTracking` and `RecGsfFlatTuple` built and installed successfully in
  the EL9/LCG 105 development build.
- The installed Configurable exposes none of the four retired property names;
  a source/card audit finds 39 algorithm properties and 38 explicit maintained-
  card assignments, with only `RecordTruthMaterialIntervals` inherited.
- The active reverse template fitted focused events 11, 16, and 17
  successfully (four input tracks total). A separate comprehensive event-11
  component dump completed with 10 final components, 2,189 accepted updates,
  zero rejected updates, 11 splits, and 11 reductions.
- A one-event `RecGsfGlobalLossRefitter` smoke test completed after the neutral
  helper rename, preserving the separate global-loss mechanism.
