# RecGsfTracking cloned-state memory leak fix

Date: 2026-08-08

## Symptom and isolation

Production-style reverse-GSF runs showed strong event-count-dependent memory
growth.  With the current five-component model and `MaxComponents=12`, GNU
`time -v` measured:

- one event: 1,768,340 kB peak RSS;
- five events: 3,057,088 kB peak RSS;
- five events without the flat tuple and EDM output algorithms: 2,995,284 kB.

The output services therefore accounted for little of the growth.  Inspection
of `GsfComponent::clone()` found that cloned `TKalTrackSite` objects owned their
cloned hits, but were not made owners of the newly allocated
`TKalTrackState` objects added to them.  Component splitting repeatedly
deep-clones the accumulated site/state history, amplifying this leak.

## Fix

All implementation changes remain inside `Reconstruction/RecGsfTracking`:

- call `SetOwner()` on every cloned site before adding cloned states;
- delete a component when the inactive direct-KalTest fallback encounters an
  unsupported hit type;
- clear the internal `TrackSummary` vector at the start of each event, making
  its documented view event-local instead of retaining every track for the
  full job.

No configurable property or physics behavior changed.

## Verification

- `RecGsfTracking` built and installed successfully in the EL9/LCG 105 build.
- A comprehensive verbose component dump for event 0 completed with finite
  forward and reverse results and no measurement rejection.
- Comprehensive verbose runs for events 11, 16, and 17 completed; all four
  fitted tracks (event 16 contains two) produced finite reverse outputs with
  zero reverse measurement rejections.
- The same five-event production-style run peaked at 1,661,292 kB after the
  fix, down from 3,057,088 kB before it, with essentially unchanged runtime
  and all tracks completed.
- A 20-event production-style run completed all events and peaked at
  1,741,988 kB.  The small increase relative to five events is consistent with
  bounded high-water/output buffering, not the former per-event leak.

The generated ROOT files and logs used for the checks are temporary outputs
under `/tmp` and are not project records.
