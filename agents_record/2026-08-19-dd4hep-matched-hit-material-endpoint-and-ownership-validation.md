# DD4hep matched-hit material endpoint and ownership validation

Date: 2026-08-19

## Defect and correction

`DD4hepBetweenSurfaces` previously obtained the destination of each material
segment by independently intersecting the component helix with the bounded
target measurement surface. In inner VXD intervals the analytic intersection
was geometrically present and correctly directed, but the strict finite-sensor
`insideBounds` test rejected crossings only microns from the accepted hit.
The material manager was therefore never called, the runtime path was marked
invalid, and the BH convolution was skipped even though the measurement update
could be accepted through DDKalTest's different surface rules.

The corrected helper uses the already matched target `TrackerHit` global point
as the segment endpoint. It still requires a finite point on the matched
KalTest surface and requires the displacement from the component pivot to have
the configured forward or reverse sign along the component tangent. This
retains explicit layer and propagation-direction guards without repeating the
fragile bounded-surface intersection.

The global hit position is intentional. A temporary implementation used
`DDVMeasLayer::HitToXv`, but the 30-event audit showed why that is not a valid
general endpoint: a one-dimensional hit does not constrain both local
coordinates, so `HitToXv` can supply an arbitrary unmeasured coordinate. That
temporary implementation was replaced before the source checkpoint. No
source outside `Reconstruction/RecGsfTracking` changed, and no configurable
property or default changed.

## Mechanical and invalid-path validation

The EL9/LCG 105 `RecGsfTracking` target built and installed successfully.

A comprehensive verbose `DD4hepBetweenSurfaces` run of seed 107, entry 0
completed with:

- 105/105 valid displayed forward component paths;
- 2,146/2,146 valid displayed reverse component paths;
- zero invalid paths in either direction;
- restored inner reverse paths of `0.000748438858`, `0.000641475376`,
  `0.000632445904`, and `0.000714070074 t/X0` at hits 0--3.

Before the correction, the uniform selected 30-event audit contained 141
invalid displayed forward evaluations out of 7,453 and 1,034 invalid reverse
evaluations out of 67,911. A same-card rerun of all 30 events after the final
global-hit endpoint correction completed every job and contained:

- 8,144/8,144 valid displayed forward evaluations;
- 67,918/67,918 valid displayed reverse evaluations;
- zero invalid displayed paths.

The forward evaluation population changes when restored material causes
additional BH evolution, so the old/new totals are not a binomial efficiency
comparison. They establish that the previously missing intervals are now
evaluated rather than silently skipped.

For the 715 displayed intervals where a first forward and first reverse
component could be paired within the same event, track, and hit index, the
mean absolute path difference was `1.76e-5 t/X0`, the maximum was
`3.68e-5 t/X0`, and none exceeded `1e-4 t/X0`. Small differences remain
expected because the forward and reverse component states are not generally
identical.

The corrected reverse candidate-path population retained the earlier scale:
24 of 67,918 evaluations were above the last CEPC BH knot, none exceeded
`0.05 t/X0`, and the maximum was `0.0319110059 t/X0`. These are candidate
evaluations, not the distinct executed-BH-call population recorded in the
2026-08-18 call-boundary audit.

## Surface-to-surface ownership check

The seed-107 entry-0 inner VXD intervals permit a direct comparison with the
matching Geant4 pre/post-step file. The corrected DD4hep material composition
between two measurement points is physically the surface-to-surface
ownership expected by a collapsed interval:

```text
half current sensor + current support/gap + half target sensor
```

Using half of each Geant4 silicon step at the bounding measurement points gave:

| Interval | DD4hep path t/X0 | Geant4 owned t/X0 | Relative difference |
|---|---:|---:|---:|
| hit 0 -> 1 | 0.000748438858 | 0.000748209706 | +0.0306% |
| hit 1 -> 2 | 0.000641475376 | 0.000641635181 | -0.0249% |
| hit 2 -> 3 | 0.000632445904 | 0.000633054536 | -0.0961% |
| sum | 0.002022360138 | 0.002022899422 | -0.0267% |

The runtime CSV independently decomposed the hit-1-to-hit-2 path into half
silicon (`0.000232406`), support (`0.000157237`), air, and half target silicon
(`0.000232406`). This closes the ownership question for the representative
inner VXD chain at the sub-per-mille level. It does not validate every detector
region or the BH response to the collapsed interval.

## Baseline and interpretation boundary

`MaterialPathMode=CurrentSurface` does not execute the changed helper during
normal operation. A direct stored-output regression on seed 140, entry 2 gave
identical old/new flat-tuple values, including `gsf_pT=32.059781473061292 GeV`
and `res_pT_gsf=0.0005281473087270`.

This correction validates endpoint construction and representative material
ownership. It does not promote `DD4hepBetweenSurfaces`, establish momentum
performance, or show that collapsing the full interval into one BH convolution
has the correct energy-loss response. The selected 30-event sample is not
held-out, and the deleted canonical hard-event 11/16/17 input remains
unavailable for that historical validation gate.

The next active check is branch-local: at the first wrong lineage decision,
compare this now-valid DD4hep interval and its BH retained-energy mixture with
the matching Geant4 loss, then distinguish interval-collapse/BH mismatch from
later posterior selection.
