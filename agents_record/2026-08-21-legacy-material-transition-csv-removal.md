# Legacy material-transition CSV removal

Date: 2026-08-21

## Decision

Remove the superseded forward-only `MaterialTransitionCSV` recorder from the
active `RecGsfTracking` interface. Its only remaining purpose was backward
compatibility. `MaterialBHAuditCSV` already records the exact seed, forward,
and reverse runtime candidates, executed BH calls, returned mixtures, parent
identity/weight/lineage, path composition, and child state, so retaining two
overlapping recorders made the workflow and option surface harder to maintain.

The comprehensive recorder remains default-off. The compiled default, reverse
template, and maintained `DumpGsfTrks/gsf.py.bk` all set
`MaterialBHAuditCSV` to empty. It should be enabled only in temporary focused
diagnostic cards; normal maintained-card batch jobs produce no material CSV.

## Removed interface and implementation

- Removed the `MaterialTransitionCSV` Gaudi property and output stream from
  `GsfAlgorithm`.
- Removed the legacy forward-only CSV initialization, row writer, finalization,
  and its diagnostic-only `componentTransitionMaterialPath` helper.
- Removed `GSF_MATERIAL_TRANSITION_CSV` handling from the maintained reverse
  template.
- Removed the explicit legacy assignment from `DumpGsfTrks/gsf.py.bk` and the
  active property tables.

Historical records and existing CSV files remain valid evidence. Already
generated `DumpGsfTrks/rungsf-*` cards that assign the removed property are
stale and must be regenerated from `gsf.py.bk`; generated cards and historical
records were deliberately not bulk-rewritten.

## Option-surface audit

The dedicated configurable-property audit found exactly 45 Gaudi properties in
`GsfAlgorithm.h`. `Reconstruction/RecGsfTracking/README.md` documents exactly
those 45, and `DumpGsfTrks/gsf.py.bk` explicitly assigns all 45, with no
missing, extra, or duplicate property. The card retains the user-selected
`BHModel="CEPCRuntimeGenericGrid5Clear"` experiment.

## Validation

- The focused EL9 build and install of `RecGsfTracking` completed successfully.
- A same-input event-11 run completed with the comprehensive audit explicitly
  enabled and returned the unchanged GSF interaction-point momentum
  `40.935105 GeV` (relative residual `0.0049970`).
- Its comprehensive audit was byte-for-byte identical to the pre-removal
  reference. Both files had SHA-256
  `928cc2c136c7c3282a1ffe14b909ab55e2b463c8da003fe1999db303c01462a5`.
- `git diff --check` and Python syntax checks passed.

This is an interface/workflow simplification, not a physics-algorithm change.
