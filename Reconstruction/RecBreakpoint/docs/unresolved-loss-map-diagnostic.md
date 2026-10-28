# Unresolved charged-sum constraint: breakpoint-map diagnostic

Date: 2026-10-28. Branch: `breakpoint-delayed-neutral`.

## Question and scope

Does the charged electron-plus-photon ECAL measurement move the smoothed
breakpoint state far enough from its forward reference to invalidate the
stored loss-map Jacobian, even without a separate neutral measurement?

This is a diagnostic, not an algorithm change. Production sources, cards,
installed libraries, PFA, and shared KF were unchanged. A copy of
`RecBreakpoint.cpp` in `/tmp/bp-unresolved-mapping-IWDgyq` adds passive logging
after accepted fits. Only that translation unit was rebuilt; the production
fit objects and link flags were reused. The resulting library was not installed.

Artifacts, copied diagnostic source, selection, scripts, local cards, logs,
tuples, and numerical tables:
`TrackingPerformanceStudies/breakpoint_unresolved_mapping_20261028/`.

## Selection and comparisons

From the first 30 seeds of the current `sim_large_barrel_20261026` campaign,
select topology-clear primary electrons with one recorded eBrem step, loss
above 1%, one selected breakpoint matching that emission's owner, no collected
hit-supported neutral, and an accepted charged ElectronPhoton constraint.

The fixed selection has eight events each in VXD, ITK, and TPC: four previously
improved and four worsened by the charged constraint. Within each stratum,
sample across the observed loss range. Consequently this sample must NOT be
used to estimate population improvement rates. Include seed 1 events 0, 1, 4
as no-eBrem controls and 11, 16, 17 as the established regression checks.

Each fresh local run saves parallel tracker-only free loss (000), beam-only
free loss (100), and beam+charged free loss (110). The neutral switch is OFF.
The 100 result distinguishes beam effects from the additional charged update.
Other controls retain the frozen campaign settings, including SigmaLogLoss
0.001, Persistent6D, PreReclustering, and backward filters OFF. No Condor jobs.

## Same-surface measurement

Do NOT compare momentum before hit i with momentum at hit i+1: that includes
geometric propagation and material. Instead, reconstruct the virtual state
immediately after the loss at the owner surface, before propagation.

Let k0 be the forward reference curvature before the loss, b0 the accepted
outer-trial prior center, ks the final smoothed pre-loss curvature, and bs the
final fitted log loss. The full map is:

```text
k_after = k_before * exp(b)
```

Its frozen affine map predicts:

```text
k_after_affine = exp(b0) * [ks + k0 * (bs - b0)]
```

Independently recover the smoothed post-loss marginal from the next-hit RTS
state using the conditional Gaussian gain:

```text
G = P_post_ref * F_geometry^T * inverse(P_next_predicted)
x_post_smoothed = x_post_ref + G * (x_next_smoothed - x_next_predicted)
```

Here F_geometry is the stored joint transition multiplied by the inverse
loss Jacobian; P_post_ref is the mapped birth covariance. This introduces no
measurement, changes no live state, and does not rerun the fit. Check that its
post-loss curvature agrees with the affine expression and its b agrees with
the saved fitted b. Only then compare with `ks * exp(bs)`.

The reported relative curvature defect is:

```text
100 * [k_after_smoothed / (ks * exp(bs)) - 1]  [%]
```

For delta_b = bs-b0, the exact relative Taylor remainder is:

```text
[1 + (k0/ks)*delta_b] * exp(-delta_b) - 1
```

This is a crucial distinction: the loss map is linear in curvature when b is
fixed. A large curvature-reference change alone does not imply a large loss-map
error. The mixed curvature/b change and exponential remainder matter.

Also compare the same-surface energy closure
`p_pre_smoothed - p_post_smoothed * exp(bs)` with the charged energy error,
and separately compare the terminal charged full/affine energy expressions.
Neither comparison tests the accuracy of the intervening geometric transport
Jacobians or proves that the reconstructed charged cluster contains the photon.

## Input-energy check

Use each track's own primary truth ECAL entrance and only photons with the
matching parent Geant4 track ID. Compare charged-cluster energy with the sum
of their ECAL-entry energies, separately retaining photons without an entry.
This is an energy-consistency check, not hit-level cluster truth association.
No collected neutral is not proof that the photon was merged into this cluster.

## Mechanical gate

Seed 3 event 48 rerun with the unmodified installed library reproduces all
869 main-tree and 57 neutral-side-tree branches of the diagnostic build exactly.
The installed library remains SHA256
`911962a6b7e9e91d533018fc699c522a7e3c3d68e50dd58dbbdea774872ec986`.
Stored campaign outputs are not the final A/B evidence; all comparisons use
parallel results from the fresh runs, and stored endpoint drift is reported
separately in `summary.json`.

## Completed results

All 30 selected rows completed in 16 local jobs. All 24 signal rows retained
the single correct selected owner, no collected neutral, charged sum mode,
and accepted 110 mask. All their recorded parent-matched photons have truth
ECAL entrances. The independently reconstructed post-loss state satisfies the
stored affine loss map to a maximum relative curvature discrepancy of
8.28e-11 over the 000/100/110 diagnostics. This checks that the comparison is
at the same surface, not across a material/geometric interval.

| Owner | N | Maximum absolute full-map curvature defect, 110 [%] | Maximum energy closure / charged sigma, 110 |
|---|---:|---:|---:|
| VXD | 8 | 0.005954 | 0.010156 |
| ITK | 8 | 0.00005950 | 0.0001027 |
| TPC | 8 | 0.00003232 | 0.00004279 |

Across the 24 signal tracks, median absolute map defect is 3.25e-6 percent.
Median absolute `b_final-b_ref` is 1.47e-6; maximum is 3.98e-4. The terminal
ECAL full-minus-affine energy discrepancy is also small: maximum 0.000494
charged-energy sigma. These scales do not support a large breakpoint-map
linearization failure as the explanation for the observed percent-level pT
errors in this sample. They do not validate the other transport Jacobians.

The explanation is structural: for fixed b the loss map is exactly linear
in curvature. Even large upstream curvature corrections need not break the
map. Minuit rebuilds the mapping for each trial center; the accepted internal
fit usually changes b only slightly relative to that center. This does NOT
mean the total loss estimate remains near zero or that ECAL cannot move it;
the outer trial center is itself optimized with the charged measurement.

Fresh same-run pT results (residual = 100*(pT_reco/pT_truth - 1)):

| Owner | Median absolute residual 000 [%] | Median absolute residual 110 [%] | Within 1%, 000 -> 110 |
|---|---:|---:|---:|
| VXD | 1.5681 | 1.2490 | 1/8 -> 4/8 |
| ITK | 0.5222 | 0.5486 | 6/8 -> 5/8 |
| TPC | 0.2438 | 0.2864 | 8/8 -> 7/8 |
| All 24 | 0.5379 | 0.4592 | 15/24 -> 16/24 |

Exactly 12 improve and 12 worsen, as expected from the deliberately balanced
selection. This is not an efficiency or representative performance estimate.
All three no-eBrem controls worsen slightly but remain within 1% (110 residuals
-0.17005%, +0.16460%, -0.14030%); they have no selected loss map to blame.

Six signal tracks have a charged-cluster energy discrepancy above five assumed
sigmas relative to the matched electron-plus-photon truth ECAL-entry sum.
This is separate from the fit's Taylor remainder. Illustrative cases:

| Seed:event | Owner | 000 pT residual [%] | 110 pT residual [%] | Charged energy vs truth entry sum [%] | Absolute map defect [%] |
|---|---|---:|---:|---:|---:|
| 1:38 | VXD | -2.69561 | -0.49456 | -0.41293 | 0.00001216 |
| 1:79 | VXD | +22.16964 | +2.89700 | +2.89662 | 0.00021746 |
| 25:126 | VXD | -1.62604 | +4.55531 | +4.58848 | 0.00000671 |
| 12:153 | VXD | -64.86274 | -64.86816 | -65.01228 | 0.00000339 |

For 12:153 the charged cluster is 6.02595 GeV, the truth electron at ECAL is
6.05819 GeV, and its photon enters with 11.16484 GeV. The cluster energy is
consistent with essentially the electron energy, not their 17.22303 GeV sum.
This strongly suggests missing photon energy in the selected measurement; it
does not establish where the shower went or whether another cluster exists
outside the collection window. No PFA modification or hit-level association
audit was performed.

Stored production endpoint drift remains in four fields: 1:79/110
-0.0007326953 GeV, 16:54/000 -8.5555e-6 GeV, 23:100/110 +0.0001434295 GeV,
25:126/110 -1.2450e-5 GeV. Its cause is unestablished. The fresh comparisons
and installed-versus-instrumented exact regression, not stored endpoints,
support the conclusions here.

Conclusion: do not promote a loss-map relinearization repair from this test.
The measured loss-map defect is too small to explain these failures. First
distinguish a genuinely unresolved photon contained in the charged cluster
from an absent collected neutral whose energy is missing elsewhere, and audit
the assumed measurement uncertainty/calibration on that selected input.
