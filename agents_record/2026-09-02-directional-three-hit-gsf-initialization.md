# Direction-local three-hit GSF initialization

Date: 2026-09-02

## Decision and implementation

The common `GsfTrackInitializer` no longer passes the complete ordered hit
list to `MarlinTrk::createPrefit`. The complete-list call selected the first,
middle, and last available two-dimensional hits in both directions, so the
nominal forward and backward messages were geometrically seeded with
opposite-side information.

The initializer now builds the geometric prefit from exactly three available
two-dimensional hits nearest the requested starting boundary:

- outward: the first three innermost hits;
- inward: the last three outermost hits.

The selected hits retain their physical inner-to-outer order when passed to
`createPrefit`, preserving the helix orientation convention. The loose
`FullLDCTracking` covariance, `KappaSeedCov` behavior, pivot transport, and
explicit boundary-hit MarlinTrk measurement update are unchanged. The forward
change applies to every GSF workflow. The inward change applies only to the
fresh seed selected by `InwardSeedCovarianceScale<=0`; a positive scale still
copies the final forward population. No configurable property or EDM schema
changed.

This removes opposite-side geometric seed information, but it is not a
strict likelihood-message construction. Each three-hit geometric prefit uses
same-side hits that the loose-covariance filter subsequently measures again.
At the first two intervals there cannot be three strictly inner hits; the
analogous limitation applies to the last two intervals outward of a tested
loss.

## Mechanical validation

The EL9/LCG-105 `RecGsfTracking` and `RecGsfFlatTuple` targets built and
installed. A verbose event-11 double-off run reported:

```text
forward prefitHits = 0,1,2
inward  prefitHits = 231,232,233
forward BH splits = 0
inward BH splits = 0
inward accepted updates = 233
```

Focused double-off fresh-inward runs completed for events 11, 16, and 17.
Their one-component endpoint views were identical within each event:

| Event | Truth pT [GeV] | CompleteTracks [GeV] | Directional double-off pT [GeV] |
|---:|---:|---:|---:|
| 11 | 40.731567 | 40.895454 | 40.892444 |
| 16 | 37.894016 | 18.292832 | 18.287323 |
| 17 | 18.796978 | 14.806669 | 14.817893 |

Event 16 retains its known secondary reconstructed track and is not a
topology-clear performance claim.

## Same-sample F/B score gate

The compiled-double-off survey reran the exact 199 topology-clear tracks from
the preceding global-prefit gate: 100 no-brem controls and 99 brem events.
All 199 selected tracks fitted successfully, with zero BH children. The
interval decision retained the 0.2% truth-loss floor and accepted a score at
the truth interval or either immediately adjacent recorded interval. The
Type-I denominator excludes those three-interval truth windows.

| Threshold | Prefit | Type I | Type II, loss >=0.2% | Type II, loss >1% | Type II, loss >2% | Type II, loss >5% | Type II, loss >10% |
|---:|:---|---:|---:|---:|---:|---:|---:|
| 0.95 | old first/middle/last | 5,308/45,415 = 11.69% | 70/96 = 72.92% | 33/54 = 61.11% | 17/32 = 53.12% | 8/16 = 50.00% | 5/11 = 45.45% |
| 0.95 | directional three-hit | 5,244/45,415 = 11.55% | 56/96 = 58.33% | 24/54 = 44.44% | 10/32 = 31.25% | 4/16 = 25.00% | 2/11 = 18.18% |
| 0.977249868, one-sided 2 sigma | old first/middle/last | 3,794/45,415 = 8.35% | 74/96 = 77.08% | 35/54 = 64.81% | 18/32 = 56.25% | 8/16 = 50.00% | 5/11 = 45.45% |
| 0.977249868, one-sided 2 sigma | directional three-hit | 3,782/45,415 = 8.33% | 61/96 = 63.54% | 27/54 = 50.00% | 12/32 = 37.50% | 4/16 = 25.00% | 2/11 = 18.18% |
| 0.998650102, one-sided 3 sigma | old first/middle/last | 1,985/45,415 = 4.37% | 81/96 = 84.38% | 41/54 = 75.93% | 21/32 = 65.62% | 10/16 = 62.50% | 7/11 = 63.64% |
| 0.998650102, one-sided 3 sigma | directional three-hit | 2,143/45,415 = 4.72% | 68/96 = 70.83% | 33/54 = 61.11% | 15/32 = 46.88% | 6/16 = 37.50% | 4/11 = 36.36% |

At the loose 0.95 cut, the location breakdown is:

| Truth interval group | Eligible loss intervals | Old misses | Directional misses |
|:---|---:|---:|---:|
| hits 0--4 | 8 | 8 | 8 |
| hits 5--11 | 53 | 31 | 31 |
| hits 12--219 | 13 | 9 | 9 |
| hits 220 and later | 22 | 22 | 8 |

All 14 newly detected intervals are in the outer boundary group. The
directional inward seed therefore removes a real outer-boundary pathology,
but the unchanged 40/66 miss count over hits 5--219 shows that global prefit
contamination was not the main interior brem-identification failure. The
identity-only Gaussian score remains passive and unsuitable as a hit-0
trigger.

As a limited clean-track safety check, the 100 topology-clear no-brem controls
gave the following FullMixtureMode residual summaries. In this double-off
one-component run all three endpoint views coincide.

| Prefit | Median residual | width68 | RMS | Maximum absolute residual |
|:---|---:|---:|---:|---:|
| old first/middle/last | -0.0061% | 0.2323% | 9.3912% | 65.5630% |
| directional three-hit | -0.0036% | 0.2185% | 9.3915% | 65.5657% |

The eventwise directional-minus-old endpoint shift has median
`-1.99e-5 GeV` and maximum absolute value `0.0249 GeV`. Thus this focused
sample shows no new clean-core degradation, but the unchanged pre-existing
tails prevent a clean-track validation claim.

The stable 133-event secondary-tracker-activity population was excluded from
the optimization sample and was not rerun as a separate control in this
focused initialization gate. Generated ROOT files, logs, and CSV tables are
under `/tmp/gsf-directional-prefit-brem-detector/` and are intentionally not
tracked.
