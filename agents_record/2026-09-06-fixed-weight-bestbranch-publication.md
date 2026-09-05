# Fixed-weight reverse BestBranch publication

Later on 2026-09-06, the remaining `ReverseSelectionMode` configurable and
its rejected `DominantLineage` alternative were removed. Reverse BestBranch
publication is now fixed to the component with the largest normalized terminal
inward weight, exactly matching the former `AggregateWeight` behavior.
WeightedMean and FullMixtureMode remain independent automatic endpoint views.

The removal covers the Gaudi property, allowed-value validation, maintained
and active-template steering, environment control, alternative selection
score, selection-specific verbose labels, and the two obsolete reverse-
selection campaign scripts. The paused ECAL re-ranker now explicitly uses
`component weight * ECAL likelihood`, which is the same tracker prior it used
under `AggregateWeight`. The persisted dominant-lineage fraction remains a
passive lineage/KL diagnostic and cannot steer endpoint publication.

The resulting public surface has 39 `RecGsfTracking` properties. The
maintained card explicitly steers 38 and deliberately inherits only
`RecordTruthMaterialIntervals=true`. An independent option audit found 186
untracked generated campaign cards still assigning the retired property (372
assignments in total); these outputs were not edited and must be regenerated
before reuse.

The earlier implementation and rejection evidence remain in
`agents_record/2026-07-13-bounded-surface-consistency-rejection.md` and
`agents_record/2026-09-06-surface-consistency-runtime-removal.md`.

## Mechanical and regression gates

- `RecGsfTracking` and `RecGsfFlatTuple` built and installed successfully in
  the EL9/LCG-105 development build.
- Assigning the retired `ReverseSelectionMode` property now fails during
  Python configuration because the property no longer exists.
- A comprehensive reverse run processed 18 entries and selected zero-based
  source events 11, 16, and 17 for verbose component dumps.
- BestBranch, WeightedMean, and FullMixtureMode availability, status, and pT,
  together with the published hit count, were bit-identical across all 18 rows
  to the immediately preceding same-configuration `AggregateWeight` run.

These checks establish mechanical equivalence to the former default selector;
they do not add physics validation.
