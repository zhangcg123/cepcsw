# Surface-consistency runtime removal

On 2026-09-06 the rejected reverse-selection experiment
`ReverseSelectionMode=SurfaceConsistency` and its sole tuning property
`SurfaceConsistencyUninformativeFloor` were removed from the active
`RecGsfTracking` implementation and maintained workflow. The original design,
numerical studies, and rejection evidence remain in
`agents_record/2026-07-13-bounded-surface-consistency-rejection.md`.

The removal covers the Gaudi property, initialization validation, bounded
surface-coincidence likelihood, final branch-score path, verbose reporting,
active reverse template environment control, maintained `gsf.py.bk` steering,
and reverse-selection sample-script choice. `ReverseSelectionMode` now accepts
only `AggregateWeight` and the default-off `DominantLineage` diagnostic.
Passive `SurfaceLineageMassDump` recording remains independent and unchanged.

The resulting public surface has 40 `RecGsfTracking` properties. The
maintained card explicitly steers 39 and deliberately inherits only
`RecordTruthMaterialIntervals=true`. An independent option audit synchronized
the package README and workflow README. Historical or generated cards that
assign the removed property or select the removed mode must be regenerated;
186 untracked campaign cards were found and intentionally left untouched.

## Mechanical and regression gates

- `RecGsfTracking` and `RecGsfFlatTuple` built and installed successfully in
  the EL9/LCG-105 development build.
- Assigning `SurfaceConsistencyUninformativeFloor` now fails during Python
  configuration because the property does not exist.
- Selecting `ReverseSelectionMode=SurfaceConsistency` now fails algorithm
  initialization with the allowed-value diagnostic.
- A comprehensive `AggregateWeight` reverse run completed over 18 entries,
  with verbose component dumps selected for zero-based source events 11, 16,
  and 17. It processed all 18 events and wrote both EDM and flat-tuple output.
- The BestBranch, WeightedMean, and FullMixtureMode availability, status, and
  pT arrays were bit-identical to the immediately preceding same-configuration
  run for all 18 rows.
- Python/shell syntax checks and `git diff --check` passed.

These are removal and no-regression gates only; they do not add physics
validation.
