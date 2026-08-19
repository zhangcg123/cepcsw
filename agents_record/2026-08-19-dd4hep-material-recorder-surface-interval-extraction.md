# DD4hep material-recorder surface-interval extraction

Date: 2026-08-19

## Purpose and scope

The user authorized a narrow change in the material-recording codebase,
`Simulation/DetSimAna`, so future BH-model samples can be conditioned directly
on the DD4hep midpoint-to-midpoint thickness used by the GSF. This does not
change `RecGsfTracking`, its production defaults, or the existing Geant4 raw
step tuple.

## Implementation

`GsfMaterialStepRecorderAnaElemTool` has a new default-off property:

```python
steprec.RecordDD4hepSurfaceIntervals = True
```

When enabled, it adds one event-level vector tree named
`dd4hep_surface_tuple`. The original `g4step_tuple` is unchanged. The option
requires `RecordZeroLoss=True`, `MinStepLengthMm=0`, and `MinAbsLossGeV=0`;
initialization fails if the recorded step stream would be incomplete.

For every recorded track, the extractor:

1. orders the complete Geant4 records by true track-step number;
2. finds each sensitive-volume traversal and uses its track-length midpoint as
   the measurement anchor;
3. maps the adjacent CEPC TPC lower/upper sensitive half-volumes to one pad-row
   anchor;
4. calls DD4hep `MaterialManager::materialsBetween(p0, p1)` between consecutive
   anchors and sums `segmentLength/material.radLength()`, matching the primitive
   and sum used by GSF `DD4hepBetweenSurfaces`;
5. clips the authoritative Geant4 step t/X0 to the same track-length bounds and
   records discrete eBrem losses whose post-step point lies inside them.

The new tree records event/track provenance, primary status, interval and step
indices, midpoint coordinates and track-length bounds, DD4hep path length and
t/X0, DD4hep material composition, clipped Geant4 t/X0, boundary momenta, and
the count and summed loss of eBrem steps. A follow-up direction-closure
extension records the identical endpoint query in reverse order through
`dd4hep_reverse_path_tX0`, `reverse_segment_count`, `reverse_valid`, and
`dd4hep_reverse_materials`. The forward value remains the material definition;
the reverse fields are diagnostics. Generated ROOT files remain outputs and
are not committed.

## Mechanical validation

`DetSimAna` rebuilt and installed successfully in the EL9/LCG 105 build. A
final-code seed-107 one-event smoke produced 232/232 valid intervals, all
primary with parent ID zero, and verified the new provenance and ordered
track-length branches.

The quantitative comparison used three generated primary electrons at
2.00764 GeV and theta 85 degrees with seed 107. Each event produced 232 valid
truth-midpoint intervals. The standard tracker reconstruction then produced
one complete track per event. GSF audited 231 outgoing intervals per event;
the first truth interval is consumed by the separately handled GSF
seed-material split, which `MaterialTransitionCSV` does not currently emit.
The comparison therefore excludes those three unaudited seed paths. The
remaining 693 intervals match by order and bounding radii.

| Event index | recorder DD4hep sum [X0] | GSF DD4hep sum [X0] | relative difference |
|---:|---:|---:|---:|
| 0 | 0.0628641297 | 0.0630331926 | +0.2689% |
| 1 | 0.0801456265 | 0.0802977828 | +0.1898% |
| 2 | 0.0648826954 | 0.0650680700 | +0.2857% |

Summed by the target detector region across the three events:

| Region | intervals | recorder sum [X0] | GSF sum [X0] | relative difference |
|---|---:|---:|---:|---:|
| VXD | 12 | 0.01657840 | 0.01657802 | -0.0023% |
| ITK | 9 | 0.08212156 | 0.08212437 | +0.0034% |
| TPC | 669 | 0.05989240 | 0.06038045 | +0.8149% |
| OTK | 3 | 0.04930009 | 0.04931620 | +0.0327% |

The typical recorder-to-GSF endpoint separation was 0.47 mm and its 95th
percentile was 1.38 mm. This has negligible integrated impact in silicon, but
individual TPC intervals are only about 4.5e-5 X0, so small digitization-driven
endpoint displacements produce large relative interval differences. Across
all intervals, the absolute t/X0 difference had median 4.54e-7 and 95th
percentile 3.26e-6.

An electron-hypothesis pass with the production five-component BH model,
`MaxComponents=12`, split/cutoff 1e-4, `SymmetricKL`, and
`DD4hepBetweenSurfaces` produced 5,971 valid component-path evaluations over
the same 693 intervals. There were 274 actual parent calls above the split
threshold. No call exceeded the last CEPC BH knot; the maximum was
0.0165892 X0. At a fixed interval all live components received bitwise-equal
DD4hep path thickness, and the weighted electron-GSF thickness agreed with the
single-component geometry control to floating-point precision.

## Interpretation and boundary

This validates that the new material recorder reconstructs the same
midpoint-to-midpoint interval semantics and DD4hep material integration used by
the GSF, within the expected difference between Geant4 truth midpoints and
digitized reconstructed hit endpoints. It does not validate the current BH
response, the collapsed-interval approximation, or momentum performance.

The next use is an unbiased material sample in which `dd4hep_path_tX0` is the
BH conditioning variable and the Geant4 eBrem records inside the identical
track-length bounds define the retained-energy truth. Energy, angle, detector
region, sparse last-knot populations, and held-out closure must be kept
explicit before proposing any replacement BH model.
