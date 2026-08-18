# Runtime material-path and Bethe-Heitler input consistency

Date: 2026-08-18

## Active question

The active investigation is no longer a global count of detector material. It
is whether the exact material thickness supplied to the Bethe-Heitler (BH)
splitter at the first wrong branch decision is consistent with the simulated
energy loss and with the domain of the configured BH mixture.

The observed performance ordering does not answer that question by itself.
`DD4hepBetweenSurfaces` counts a more complete geometry interval than
`CurrentSurface`, but its focused track performance was worse. That can arise
from a path/ownership defect, collapsing a heterogeneous interval into one BH
convolution, a BH response mismatch, or a later measurement/selection effect.
These possibilities must be separated at the branch-crossover location.

## Definitions that must not be mixed

- A Geant4 material step is a simulated particle's pre/post step. Its energy
  change is the authoritative energy-loss truth.
- A Geant4 transition assembled from those steps is a truth-side spatial
  summary. It is not automatically one GSF propagation interval or one BH
  call.
- In `CurrentSurface`, runtime `pathTX0` is the material owned by the selected
  measurement surface, projected by the component incidence. It is a
  surface-local approximation.
- In `DD4hepBetweenSurfaces`, runtime `pathTX0` is the DD4hep material integrated
  along one extrapolated component trajectory between two bounding measurement
  surfaces. It is component- and direction-dependent.
- A BH parent call is one executed
  `split(parent, pathTX0, bz[, reverse])` after that component has a valid path
  above `BHSplitThreshold`. Candidate paths below the gate, event-hit interval
  medians, the hit-level `nSplits` counter, and Geant4 transitions are different
  populations.

For the five-component `CEPC2GeV85StepConditioned` model, inputs above the last
tabulated knot (`0.024494897427831779 t/X0`) receive the last-knot mixture; the
model does not extrapolate a stronger loss distribution beyond that point.

## Direction correction and comparison boundary

Commit `f7b2e87` corrected a dispatch defect: `MaterialPathMode` now controls
both outward and inward propagation. Before that correction, reverse
propagation always used DD4hep interval material even when the card selected
`CurrentSurface`. Pre-correction reverse tuples are therefore not same-code
comparisons. Mechanical evidence for the correction is preserved in
`2026-08-18-material-path-mode-direction-symmetry.md`.

The focused seed-216 dispatch check included DD4hep intervals near `0.0249`,
`0.183`, and `0.032--0.0459 t/X0`. It demonstrates that a large interval can
occur, but it is not part of the uniform 30-event audit below and must not be
used as that audit's population rate.

## Uniform 30-event call-boundary audit

The selected diagnostic set contains 30 events: five previously good and five
previously bad examples in each of no-eBrem, light-eBrem, and hard-eBrem. It is
mechanism-oriented, not random or held-out. Temporary cards explicitly used:

- `MaterialPathMode="DD4hepBetweenSurfaces"` in both directions;
- `BHSplitThreshold=1e-4`;
- `ComponentWeightCutoff=1e-4`;
- `EcalComponentConstraint=false`.

The exact call population was 4,297 BH parent calls: 1,953 forward calls
(including ten initial hit-0-to-hit-1 seed-material calls) and 2,344 reverse
calls. Of these, 43 were above the last CEPC BH knot, 19 were at least
`0.03 t/X0`, and the maximum was `0.0319038626 t/X0`. Thus about 1.00% were
above the last knot and about 0.44% were at least 0.03 in this selected sample.
No call reached 0.05.

The ordinary forward/reverse rows were obtained directly at the pre-split
boundary. The ten seed calls are a separate code path and were counted from
executed seed-material splits; their approximate paths in the earlier verbose
audit were inferred from printed mixture weights, and none changes the 43/19
out-of-range counts.

This corrects the earlier ambiguous statement that there were generally “too
many” out-of-range surface-to-surface inputs. They are uncommon in this
selected 30-event set. However, rarity does not establish harmlessness: the
same interval is evaluated for multiple surviving parents, the last-knot
mixture is saturated, and a rare call can still create the lineage that later
wins. A broad exact-call audit and branch-local correlation are still required.

Raw tables and the plot currently live under
`/tmp/gsf_dd4hep_uniform_x0_audit_2026-08-18/`; `/tmp` is not durable project
memory.

## Steering warning

The committed production contract remains `CurrentSurface`, split threshold
`1e-4`, component cutoff `1e-4`, and ECAL off. The live working copy of
`DumpGsfTrks/gsf.py.bk` is locally modified to DD4hep, `1e-8`, `1e-8`, and ECAL
on. Those are experimental steering changes, not new defaults. They must not be
used silently in a baseline comparison, and this record intentionally does not
overwrite them.

## Ordered investigation

1. Freeze and print the exact steering for every diagnostic run. Keep the
   production baseline unchanged and use temporary cards for material controls.
2. Add or use exact BH-call capture on an unbiased sample, including the seed
   path. Record event, direction, bounding surfaces, parent identity/weight,
   path composition, `pathTX0`, and the mixture returned by the BH model.
3. For bad events, locate the first surface where a truth-compatible lineage
   loses posterior rank or is removed. Compare matched good controls; do not
   infer causality from an aggregate `t/X0` histogram.
4. At that crossover, compare `CurrentSurface`, DD4hep interval composition,
   and the Geant4 pre/post-step truth spatially between the same boundaries.
   Check forward/reverse consistency only on equivalent component states.
5. Test BH energy-loss closure at the exact input thickness: compare the
   predicted retained-energy mixture and moments with Geant4 fractional loss,
   categorized by energy, angle, and detector region. Treat last-knot
   saturation explicitly.
6. Classify the failure as path/ownership, interval-collapse granularity, BH
   response, or downstream measurement/selection before proposing a source
   change. Validate any proposal on focused verbose events, required hard-loss
   events 11, 16, and 17, and then an independent population with clean-track
   safety.

Success requires a truth-blind discrepancy at or before the first bad branch
decision that separates bad events from matched good controls and predicts a
same-code correction without sacrificing the no-eBrem core. If the exact path
and BH response close correctly at the crossover, the investigation must say
so and move downstream rather than retuning material.

Current non-goals are changing source before this diagnosis, promoting
`DD4hepBetweenSurfaces`, tuning split/cutoff thresholds, component capacity,
KL ranking, reverse seed covariance, final publication heuristics, or the ECAL
selector; fitting SimHit momentum; truth-dependent runtime logic; and changing
shared tracking packages.
