# DD4hep forward/reverse TPC path closure

Date: 2026-08-19

## Question and steering

The follow-up validation asked whether outward and inward
`DD4hepBetweenSurfaces` paths agree on the same bounded measurement intervals
and agree with the material-recorder extraction. No GSF algorithm source was
changed. The same three seed-107, 2.00764 GeV, theta-85-degree reconstructed
tracks used by the recorder validation were rerun with the production
five-component BH model, `MaxComponents=12`, split/cutoff 1e-4,
`SymmetricKL`, independent reverse filtering, and verbose component material
dumps. ECAL remained off.

## Runtime populations and audit closure

The forward `MaterialTransitionCSV` contained 5,971 component rows over 693
non-seed interval groups. All were valid. On every row the active
`path_t_over_x0` was exactly equal to `geometry_path_t_over_x0`, so the CSV
records the actual selected DD4hep runtime path. All components at a fixed
interval had the same path.

The reverse runtime log contained 7,127 component rows over all 696 interval
groups, including the three hit-0 seed intervals. All were valid and all
components at a fixed interval again had the same path.

Forward interval `i -> i+1` was paired with reverse interval `i+1 -> i` by
event and `hit=i`. The 693 shared groups divide as follows:

| Interval class | Groups | Forward/reverse result |
|---|---:|---|
| common silicon intervals, hits 1--7 | 21 | equal to logged precision |
| ITK to first TPC row, hit 8 | 3 | reverse sum 0.213% lower |
| internal TPC rows, hits 9--230 | 666 | median reverse/forward 0.500042 |
| last TPC row to OTK, hit 231 | 3 | equal to logged precision |

For the 666 internal TPC groups the reverse/forward ratio ranged only from
0.500004 to 0.500081. Their integrated reverse thickness was 49.9958% lower.
The complete shared-interval sums were lower in reverse by 7.9928%, 6.2686%,
and 7.7384% in events 0, 1, and 2 respectively.

The reverse hit-0 seed paths agree with the corresponding material-recorder
intervals within +0.0101%, -0.0039%, and +0.0118%. The full 232-interval
reverse totals, including the seed, were lower than the recorder totals by
7.6612%, 6.0387%, and 7.3961%, driven by the internal TPC intervals.

The internal-TPC paths are about 4.4e-5 X0 outward and 2.2e-5 X0 inward. Both
remain below the committed production `BHSplitThreshold=1e-4`, so this defect
does not itself change whether those intervals execute a BH split under the
production baseline and cannot yet be claimed as the cause of its branch
selection failures. It does invalidate direction closure, biases accumulated
material diagnostics, and directly affects the live experimental `1e-8`
split/cutoff steering.

This means the earlier correction is valid only at the dispatch level:
`MaterialPathMode` does govern both directions, but it does not currently
produce direction-symmetric path values in the TPC.

## Identical-endpoint DD4hep control

The default-off material recorder was extended to evaluate each truth
midpoint pair through the same `MaterialManager::materialsBetween` primitive
in both endpoint orders. In a final-code one-event smoke, all 232 forward and
232 reverse queries were valid. Of those, 224 were exactly equal as stored
floats. Eight were direction-sensitive boundary cases:

- the ITK-to-first-TPC query omitted one terminal T2KGas1 segment in reverse,
  changing the total by 0.213%;
- seven internal TPC queries returned two T2KGas1 half-segments outward but
  only one of those half-segments inward, changing those intervals by about
  50%.

The identical-endpoint event sum changed only from 0.0635540329 to
0.0633796734 X0 (-0.2743%) because most truth midpoint coordinates do not sit
exactly on the reconstructed cylindrical measurement boundary. In contrast,
the GSF reconstructed TPC hits lie on the exact pad-row surfaces, and the
half-segment loss occurs systematically inward.

## Interpretation and correction boundary

The evidence isolates a DD4hep/TGeo boundary-orientation defect before the BH
model: outward navigation includes both TPC gas half-segments between pad-row
surfaces, whereas inward navigation commonly omits one. The fact that the
same primitive, geometry, and fixed component-independent interval yields a
stable factor of one half rules out KL reduction or final branch selection as
the cause.

The exact reverse start/end coordinates and material list are not presently
written by the GSF reverse audit, so the endpoint-level mechanism remains an
inference from the runtime ratios, exact surface radii, source call chain, and
recorder control. It must be confirmed in a temporary diagnostic build before
editing production GSF source.

The narrow correction candidate is to keep the existing finite-point,
matched-surface, and propagation-direction guards, but evaluate the scalar
DD4hep thickness in one canonical spatial orientation for both propagation
directions: inner matched endpoint to outer matched endpoint. The BH splitter
already receives the separate reverse flag, so scalar material integration
does not need to inherit the propagation orientation. A tiny symmetric
endpoint envelope may be evaluated only if canonical ordering alone does not
remove residual boundary cases.

Acceptance requires, before any momentum claim:

1. exact endpoint and material-list evidence on representative silicon, first
   TPC, internal TPC, and TPC-to-OTK intervals;
2. interval-wise forward/reverse path closure on the same three events,
   including the seed interval;
3. agreement with the recorder and Geant4-bounded interval material;
4. focused verbose stability, hard events 11/16/17, and held-out clean-track
   validation after the material-only correction is reviewed.
