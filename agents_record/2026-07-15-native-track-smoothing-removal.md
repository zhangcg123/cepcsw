# NativeTrackSmoothing removal (2026-07-15)

`NativeTrackSmoothing` was a default-off diagnostic that called KalTest
`SmoothAll()` independently on each final GSF component. It neither propagated
GSF mixture weights backward nor combined components, so it was not a
Gaussian-sum smoothing implementation.

At the user's request it was removed entirely from the active package:

- the Gaudi property and member were removed;
- the `SmoothAll()` runtime branch and verbose summary were removed;
- all initialization compatibility checks were removed or simplified;
- `GSF_NATIVE_TRACK_SMOOTHING` was removed from the reverse template.

No active `Reconstruction/RecGsfTracking` or `AGENTS.md` reference remains.
Older dated records retain the name solely as historical provenance. The
package and option-file syntax checks complete successfully after removal.
