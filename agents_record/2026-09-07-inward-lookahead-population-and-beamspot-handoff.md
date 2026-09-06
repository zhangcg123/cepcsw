# Inward look-ahead population result and beam-spot handoff

Date: 2026-09-07

## Archived inward look-ahead status

The numeric `InwardLookaheadDepth` implementation remains mechanically
available.  Depth zero exactly reproduced the stored `LocalMeasurement`
endpoints on focused indices 11, 16, and 17.  Depth one was mechanically
stable but did not recover event 16 and changed recovered event 17 from a
-0.311% to a -0.732% pT residual in the first current-control comparison.
Depth two verified cumulative independent probes, separate per-probe
normalization, averaging, and the final prior/local plus look-ahead/local
channel combination.  Those probes reuse hits that are subsequently visited
by the live recursion, so the method remains an uncalibrated evidence-reuse
diagnostic.

The latest protected-depth-two and no-identity-protection samples had 6,270
common topology-clear events.  Their summary was:

| Category | N | Protected median / width68 (%) | No protection median / width68 (%) | Protected -> no-protection within 1% | `|delta residual| > 0.1 percentage point`: improve / worsen |
|---|---:|---:|---:|---:|---:|
| inclusive | 6,270 | -0.0782 / 0.5195 | -0.0676 / 0.5315 | 5,040 -> 5,031 | 103 / 93 |
| no eBrem | 2,609 | -0.0245 / 0.2059 | -0.00918 / 0.2076 | 2,396 -> 2,395 | 9 / 18 |
| 0--0.1% loss | 1,061 | -0.0487 / 0.1867 | -0.0339 / 0.1892 | 997 -> 995 | 3 / 6 |
| 0.1--1% loss | 894 | -0.2446 / 0.3552 | -0.2248 / 0.3622 | 820 -> 820 | 12 / 10 |
| at least 1% loss | 1,563 | -0.4685 / 2.6119 | -0.4572 / 2.5859 | 742 -> 747 | 72 / 43 |

The inclusive catastrophic count above 100% absolute residual changed from
24 to 25; in the at-least-1% category it changed from 20 to 18.  Disabling
identity protection slightly reduced central bias and helped some lossy
tracks, but broadened both clean subcategories and the inclusive population.
It therefore did not pass the clean-track and tail-safety gate.  A complete
same-code depth 0/1/2 population scan remains useful regression evidence but
is no longer the immediate experiment.

The maintained `DumpGsfTrks/gsf.py.bk` currently retains the last campaign
steering (`InwardLookaheadDepth=2`, `ProtectIdentityLineage=false`).  This is
deliberately different from the compiled and active reverse-template defaults
of depth zero and identity protection enabled.

## Beam-spot endpoint experiment

`IMarlinTrack` exposes measurement updates only for detector `TrackerHit`
objects.  Its point propagation/extrapolation methods do not impose a vertex
measurement, and fabricating a tracker hit would require a detector layer that
does not exist.  The first beam-spot implementation therefore lives entirely
inside `RecGsfTracking`, outside the MarlinTrk classes.

This is a default-off terminal Gaussian constraint.  It acts independently on
copies of the already formed smoother/reverse BestBranch, WeightedMean, and
FullMixtureMode IP endpoints.  It does not alter the component recursion,
lineage weights, pruning, KL reduction, final branch selection, or the three
ordinary endpoint collections.  The state and covariance are moved to the
configured `(x,y,0)` beam pivot, constrained in local `drho`, and moved back to
the standard origin pivot.  For uncorrelated transverse beam widths the scalar
measurement variance is

```text
R = cos(phi0)^2 sigma_x^2 + sin(phi0)^2 sigma_y^2.
```

The nominal comparison configuration is `(x,y)=(0,0)` mm,
`sigma_x=0.0145` mm and `sigma_y=3.6e-5` mm.  There is no longitudinal
constraint or x-y beam covariance in this first version.  Successful updates
add the beam delta-chi-square and one NDF while preserving the source track's
real hit list.  Failure publishes an exact copy of the relevant source
endpoint.

The paired collections are `GSFTracksBeamSpotBestBranch`,
`GSFTracksBeamSpotWeightedMean`, and
`GSFTracksBeamSpotFullMixtureMode`.  `GSFBeamSpotConstraintStatus` is a bitmask:
bit 0 means attempted, and bits 1, 2, and 3 mean successful BestBranch,
WeightedMean, and FullMixtureMode updates respectively.  The flat tuple has
parallel `beamspot_*_gsf_*` scalar families and residuals; they are
unavailable/zero when the feature is off.

Focused same-code validation on selected indices 11, 16, and 17 passed for
all endpoints (`status=15`).  A paired default-off rerun reproduced all 46
ordinary scalar endpoint fields exactly.  The focused pT values were:

| Event number | Truth | LCIO | Best | Beam Best | Weighted | Beam Weighted | Full mode | Beam Full mode |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 12 | 9.151077 | 9.097590 | 9.094142 | 9.094179 | 9.318403 | 9.318325 | 9.094221 | 9.094221 |
| 17 | 38.360703 | 38.261775 | 38.273821 | 38.273992 | 39.092994 | 39.099096 | 38.273948 | 38.273948 |
| 18 | 31.755604 | 18.220515 | 18.219263 | 18.220123 | 18.254617 | 18.266424 | 18.219281 | 18.219281 |

These values are from reverse.  A separate smoother smoke test completed and
exercised the documented endpoint-local fallback: event 12 constrained all
three views, while non-positive-definite endpoint covariance prevented some or
all constraints on events 17 and 18.  The output tracks remained available as
exact source fallbacks and the missing success bits exposed the condition.
Do not treat an available constrained collection as proof that its success bit
is set.

These are mechanical checks, not evidence of physics improvement.  The next
gate is a same-code topology-clear on/off population comparison, separately
reporting no-eBrem, light-eBrem, hard-eBrem, transition location, clean core,
and catastrophic tails for all three endpoint definitions.  The feature can
advance only if clean tracks remain safe and lossy-event recovery improves
without increasing extreme tails.
