# Live beam-boundary reverse GSF

## Decision

The first terminal endpoint-copy beam prototype was superseded before
population validation. `BeamSpotConstraint` now enables a live reverse-only
boundary measurement that participates in component evolution. It remains
compiled default-off and is not a production-physics setting.

The five properties and their compiled defaults are unchanged:

```text
BeamSpotConstraint = false
BeamSpotX = 0.0 mm
BeamSpotY = 0.0 mm
BeamSpotSigmaX = 0.0145 mm
BeamSpotSigmaY = 3.6e-5 mm
```

There is no longitudinal constraint and no configured x-y correlation. The
transverse observation is the helix `drho=0` measurement at
`(BeamSpotX, BeamSpotY, 0)`, with

```text
R = cos(phi0)^2 BeamSpotSigmaX^2 + sin(phi0)^2 BeamSpotSigmaY^2.
```

The update changes the full five-dimensional mean and covariance through the
ordinary Gaussian gain. Its component score is

```text
log w' = log w - 0.5 * (deltaChi2 + log S),
```

where `S` is the scalar innovation variance.

## Live boundary sequence

Forward:

1. Build the existing direction-local three-hit prefit.
2. Move that prefit Gaussian to the beam pivot and apply the scalar beam
   update. The pre-existing initializer's hit-0 update is discarded.
3. Evaluate one canonical DD4hep t/X0 from the beam pivot to matched hit 0.
4. If `ForwardBHSplitting`, `ElectronHypothesis`, and `BHSplitThreshold`
   permit it, split the beam state over that complete interval.
5. Transport each outward component from the beam through the innermost
   material surface to hit 0 with the package's KalTest cradle, retaining the
   complete Jacobian and process noise, then perform the ordinary MarlinTrk
   hit-0 update before continuing the outward recursion.

Reverse:

1. Run the independent reverse filter normally through the hit-0 update.
2. Reuse the exact canonical beam-to-hit-0 material object computed for the
   outward boundary.
3. If `InwardBHSplitting`, `ElectronHypothesis`, and `BHSplitThreshold`
   permit it, split each hit-0 state for the final inward interval.
4. Propagate each child from hit 0 to the beam with the MarlinTrk point
   propagator, then apply the scalar beam update and likelihood.
5. Normalize, apply the ordinary component-weight cutoff and configured KL
   reduction, and publish the surviving live bank through the ordinary
   BestBranch, WeightedMean, FullMixtureMode, final-component, and lineage
   outputs.

The forward boundary uses a package-local synthetic source site on the
innermost cylindrical material surface solely to call the existing KalTest
cradle transport. It does not fabricate an EDM measurement or modify shared
tracking code. The returned beam-to-hit-0 transport is composed with the
standard MarlinTrk hit-0 update so the stored smoothing transition closes from
the beam state rather than from a geometrically repivoted hit-0 state.

The MarlinTrk point-propagator overload without an LCIO reference hit is
required for reverse step 4. `initialise()` creates a live dummy site at hit 0
but does not put that dummy in the LCIO-hit-to-fitted-site map; consequently
the reference-hit overload rejects the otherwise valid boundary propagation.

The beam and split evaluations use lineage hit/surface `-1`. Beam measurement
nodes use generic accepted status `4`. Separate
`GSFTracksBeamSpot*`, `GSFBeamSpotConstraintStatus`, and `beamspot_*` flat
fields were retired: a beam-on job changes the ordinary reverse endpoints.
Beam-off/on comparisons therefore require separate jobs and output paths.

## Compatibility gates

Beam mode currently requires all of the following:

- `ReverseFiltering=true`;
- `GaussianSumSmoothing=false`;
- `MaterialPathMode=DD4hepBetweenSurfaces`;
- `MaterialIPExtrapolation=false`;
- `TruthBHLossOverride=false`;
- `EcalComponentConstraint=false`;
- `InwardSeedCovarianceScale<=0`.

The fresh reverse seed avoids applying the forward beam observation once via
a copied forward mixture and then a second time at the inward boundary.
Neither directional BH gate is forced on by beam mode.

## Focused mechanical gate

The EL9 build and install completed for `RecGsfTracking` and
`RecGsfFlatTuple`. A verbose same-code run used barrel seed 12, selected event
indices 11, 16, and 17, fresh inward initialization, LocalMeasurement depth
zero, both directional BH gates on, and deterministic energy loss off.

The forward prefit started at the beam, the KalTest boundary transport reached
hit 0, and the beam-to-hit-0 and hit-0-to-beam splits both ran. The cached
boundary t/X0 was respectively `0.00631769688`, `0.00477007227`, and
`0.00689154367` for the three events. All 300 post-split reverse children
propagated and accepted the beam update. Cutoff/KL retained ten components per
event and all three ordinary endpoints were published.

The endpoint pT comparison in GeV is:

| Event index | Beam | BestBranch | WeightedMean | FullMixtureMode |
|---:|:---:|---:|---:|---:|
| 11 | off | 9.09415 | 9.18175 | 9.0942461 |
| 11 | on  | 9.09419 | 9.19747 | 9.0944667 |
| 16 | off | 38.2563 | 38.6125 | 38.2572829 |
| 16 | on  | 38.2564 | 38.6806 | 38.2574508 |
| 17 | off | 18.2158 | 18.2610 | 18.2166950 |
| 17 | on  | 18.2164 | 18.3503 | 18.2174036 |

The beam-off values reproduce the previously stored current-code results for
these focused events. These checks establish mechanics and default-off
regression only; they do not establish a resolution improvement.

## Next gate

Run separate same-code beam-off and beam-on topology-clear populations. Report
no-eBrem, light-eBrem, hard-eBrem, transition-location, clean-core, and
catastrophic-tail behavior independently for all three endpoint definitions.
Do not advance the experiment on a selected core improvement alone.
