# EDM4hep 0.x → 1.0 Migration Summary

This document summarizes the changes needed to migrate CEPCSW from the old Key4hep/EDM4hep stack (v0.x) to the new stack (v1.0) under LCG 109, while maintaining source compatibility with both versions.

The migration was carried out in two phases:

- **Commit `f66d5c8`** — Migrate to LCG 109: compile core software and simulation (81 files)
- **Commit `fc0c2cb`** — Migrate to LCG 109: compile Reconstruction (61 files)

## Collection type renaming

EDM4hep 1.0 introduced a polymorphic `TrackerHit` hierarchy and renamed association/link collections:

| Old (EDM4hep 0.x) | New (EDM4hep ≥1.0) |
|---|---|
| `edm4hep::TrackerHitCollection` | `edm4hep::TrackerHit3DCollection` |
| `edm4hep::MCRecoTrackerAssociationCollection` | `edm4hep::TrackerHitSimTrackerHitLinkCollection` |
| `edm4hep::MCRecoTrackParticleAssociationCollection` | `edm4hep::TrackMCParticleLinkCollection` |

Each file uses `#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)` preprocessor guards with compatibility aliases:

```cpp
#include "edm4hep/EDM4hepVersion.h"
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
#include "edm4hep/TrackerHit3DCollection.h"
#include "edm4hep/TrackerHitSimTrackerHitLinkCollection.h"
using CEPCSWTrackerHit3DCollection = edm4hep::TrackerHit3DCollection;
using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::TrackerHitSimTrackerHitLinkCollection;
#else
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
using CEPCSWTrackerHit3DCollection = edm4hep::TrackerHitCollection;
using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::MCRecoTrackerAssociationCollection;
#endif
```

Code then uses the aliased types (`CEPCSWTrackerHit3DCollection`, etc.) in `DataHandle` declarations and function signatures.

## API changes

### TrackState covariance matrix

`TrackState::covMatrix` changed from a bare `std::array` to a struct with a `.values` member:

```cpp
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
covMatrix.values[0] = ...;
#else
covMatrix[0] = ...;
#endif
```

### TrackerHit is now polymorphic

`TrackerHit` became a polymorphic base class with `TrackerHit3D` and `TrackerHitPlane` subtypes. Accessing type-specific data requires an `as<T>()` cast:

```cpp
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
auto covmat = hit.as<edm4hep::TrackerHit3D>().getCovMatrix();
auto du = hit.as<edm4hep::TrackerHitPlane>().getDu();
auto dv = hit.as<edm4hep::TrackerHitPlane>().getDv();
auto uVec = hit.as<edm4hep::TrackerHitPlane>().getU();
auto vVec = hit.as<edm4hep::TrackerHitPlane>().getV();
#else
auto covmat = hit.getCovMatrix();
auto du = hit.getCovMatrix()[2];
auto dv = hit.getCovMatrix()[5];
// old gear::Vector3D construction from covMatrix values
#endif
```

### MutableTrackerHit → MutableTrackerHit3D

Functions that create tracker hits now return `MutableTrackerHit3D` instead of `MutableTrackerHit`:

```cpp
#if edm4hep_VERSION >= EDM4HEP_VERSION(0, 99, 0)
edm4hep::MutableTrackerHit3D createSpacePoint(...);
#else
edm4hep::MutableTrackerHit createSpacePoint(...);
#endif
```

### Association/Link interface

Association methods were renamed to match the new link semantics:

| Old (EDM4hep 0.x) | New (EDM4hep ≥0.99) |
|---|---|
| `ass.getRec()` | `ass.getFrom()` |
| `ass.getSim()` | `ass.getTo()` |
| `ass.setRec(hit)` | `ass.setFrom(hit)` |
| `ass.setSim(sim)` | `ass.setTo(sim)` |

### SimTrackerHit → MCParticle access

```cpp
// Old
simHit.getMCParticle()
// New
simHit.getParticle()
```

### Removed APIs

- `TrackerHit::addToRawHits()` and `TrackerHit::getRawHits()` are removed in EDM4hep 1.0. Code using them is guarded with a `#if`/`#else` that throws in the new path.

### Return type changes

`Navigation::GetTrackerHit()` now returns `std::optional<edm4hep::TrackerHit>` instead of `edm4hep::TrackerHit` directly. Similarly, `TrackerHitHelper::getAssoTrackerHit()` returns `std::optional<edm4hep::TrackerHit>`.

## C++ standard

The C++ standard was raised from C++17 to C++20 (required by the LCG 109 toolchain):

```cmake
set(CMAKE_CXX_STANDARD 20 CACHE STRING "")
```

## Dependencies

Two dependencies were temporarily disabled pending porting:

```cmake
# find_package(LCContent REQUIRED)
# find_package(PandoraSDK REQUIRED)
```

## Summary of common migration recipe per file

1. Include `edm4hep/EDM4hepVersion.h` early.
2. Guard type aliases with `#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)`.
3. Replace old collection types with the new aliases in `DataHandle` declarations and function signatures.
4. Guard TrackerHit subtype casts (`.as<T>()`) and covMatrix access with version checks.
5. Guard association method calls (`getRec`/`getSim` vs `getFrom`/`getTo`) with version checks.
6. Add `#include <optional>` and use `std::optional` for functions that may not find a hit.
