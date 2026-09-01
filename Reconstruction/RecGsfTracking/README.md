# RecGsfTracking

`RecGsfTracking` refits `CompleteTracks` with a Gaussian-sum electron model.
The smoother and reverse workflows always write three row-aligned endpoint
views: the selected branch to `GSFTracksBestBranch`, the moment-matched state
to `GSFTracksWeightedMean`, and the maximum of the complete five-dimensional
mixture density to `GSFTracksFullMixtureMode`. A
default-on automatic component record also persists every positive-weight
final smoother/reverse component's normalized weight, IP kappa mean, and
kappa variance so the transverse-momentum marginal can be reconstructed from
the flat tuple without rerunning verbose diagnostics. A
single outward GSF pass supplies reverse. A transient
`SharedForwardFilterResult` retains its final filtered population and the
post-update, post-cutoff, post-reduction filtered history. One common
`runGsfInwardFilter` path first revisits hit `N-2`, performs the live
backward-predicted-by-hit update, and returns the terminal backward mixture.
Inside each interior reverse-surface step it also forms and records the
two-filter product `B_smoothed[i] = F_updated[i] x B_predicted[i]` immediately
before committing the corresponding measurement results to live
`B_updated[i]`. The boundary states are the existing live mixtures:
`B_smoothed[0] = B_updated[0]` and
`B_smoothed[N-1] = F_updated[N-1]`. Product states are reduced independently
and never propagate or publish. The compiled `InwardWeightMode` default,
`LocalMeasurement`, also keeps their weights diagnostic-only. Experimental
`SmoothedMarginal` instead marginalizes the normalized unreduced direct-pair
weights over every forward partner and attaches the result to the matching
live `B_updated[i]` state before its cutoff, reduction, and next propagation.
This intentionally reuses overlapping forward evidence at successive
surfaces and is an algorithmic overlap score, not a calibrated Bayesian
posterior.
The independent boolean properties `ForwardBHSplitting` and
`InwardBHSplitting` gate only BH component creation in the shared outward
filter and reverse inward filter, respectively. Both compile to `false` so an
unsteered fit creates no BH children in either direction. Disabling either
gate leaves its material-path evaluation, passive interval recording,
propagation, and measurement updates active. `InwardBHSplitting` is inert
unless `ReverseFiltering=true`.
Reverse publishes the inner boundary state,
`B_updated[0] = measurement[0] x B_predicted[0]`.
Both use the single `InwardSeedCovarianceScale` property. Positive values copy
the final forward population into the common inward filter and scale every
component covariance; values at or below zero instead construct one fresh,
unit-weight backward seed with the standard prefit and an explicit outermost-
hit update. That mode is independent of the final forward mixture. The
geometric prefit is direction-local: the outward filter uses the three
innermost available two-dimensional hits and the fresh inward filter uses the
three outermost. Former method-specific seed-covariance
properties were removed rather than retained as duplicate controls. A
default-off ECAL experiment can additionally write a paired component-selection result to
`GSFTracksEcalConstrained`. Each component uses the baseline MarlinTrk
`addHit(reference) -> initialise(componentState) -> addAndFit(currentHit)`
update path. All forward GSF workflows use a dedicated standard-KF-style
initializer: a temporary prefit through the first three available
two-dimensional hits, followed by the loose covariance
`Var(d0)=1e6`, `Var(phi)=1e2`, `Var(omega)=1e-4`, `Var(z0)=1e6`, and
`Var(tanLambda)=1e2`, pivot transport to the first hit, and an explicit
MarlinTrk update with that first hit. `KappaSeedCov` is retained only as a
legacy diagnostic override of the prefit curvature variance; it does not
restore the former `CompleteTracks`-anchored seed. The same initializer and
curvature convention are used with the last three available two-dimensional
hits at the outermost boundary when fresh inward initialization is selected.
The old alternate KF fitter
and other initialization experiments have been removed; historical
comparisons remain under `agents_record/`.

## Complete configuration reference

Reference date: 2026-09-02. `RecGsfTracking` exposes 39 Gaudi properties in
`src/GsfAlgorithm.h`. “Compiled” below means constructing the algorithm
without a run card. “Active reverse” means the effective no-environment-
override configuration in `options/run_gsf_reverse_template.py`. The
distinction matters because that template enables `ElossOn` and
`ReverseFiltering`, whose compiled defaults are false.

### Physics and material model

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `ElectronHypothesis` | `true` | `true` | Enable electron-hypothesis BH processing. Set false for forced no-BH particle controls. |
| `BHModel` | `CEPC2GeV85StepConditioned` | same | Select the BH Gaussian-mixture parameterization. Canonical values are `CEPC2GeV85StepConditioned`, `CEPC2GeV85StepConditioned6`, `CEPCRuntimeGenericGrid5Clear`, `CEPCRuntimeCategoryAligned5Clear`, `CEPCRuntimeCategoryAligned9Clear`, `CEPCRuntimeCategoryAligned15Clear`, and `ActsAtlas`. Only the first is the active default; all others are default-off research controls. |
| `TruthBHLossOverride` | `false` | `false` | Enable the all-or-nothing, truth-dependent BH-loss oracle described below. Invalid event/track truth falls back to the configured BH model with an explicit status tag. This diagnostic is never production steering. |
| `TruthBHLossInputTrackIndex` | `0` | same | Zero-based `CompleteTracks` index used by the embedded-EventData truth oracle and passive material recorder. Other input tracks use the configured BH model and have no interval record. It must be nonnegative. |
| `TruthBHLossMaxEndpointDistance` | `5.0 mm` | same | Maximum allowed endpoint discrepancy between an accepted runtime hit and its exactly associated embedded Geant4 truth hook. It must be finite and positive. |
| `RecordTruthMaterialIntervals` | `true` | `true` | Passively record material-consistency information for the `CompleteTracks` index and endpoint guard configured by the two preceding properties. Each accepted-hit interval records Geant4 truth t/X0 between exact associated hooks, DD4hep t/X0 between those same truth positions, and forward/reverse runtime GSF material-path summaries in `GSFTruthMaterialIntervals` and the flat tuple. Per-track status is written to `GSFTruthMaterialRecordStatus`. This default-on diagnostic never supplies material or loss to the GSF and cannot change a BH call, split threshold, component, weight, or published track. |
| `BHSplitThreshold` | `1e-4` | same | Minimum component-local outgoing material thickness used to trigger a BH process split. |
| `ForwardBHSplitting` | `false` | `false` | Enable BH component creation in the shared outward filter, including the initial hit-0 to hit-1 interval and every later outgoing accepted-hit interval above `BHSplitThreshold`. When false, the outward state still propagates, updates measurements, evaluates DD4hep paths, and contributes passive material summaries, but it remains unsplit by BH. This does not disable deterministic `ElossOn` or `MSOn`. |
| `InwardBHSplitting` | `false` | `true` | Enable BH component creation in the independent reverse inward filter for every outer-to-inner interval above `BHSplitThreshold`. It is inert unless `ReverseFiltering=true` and does not govern `GaussianSumSmoothing`, which only consumes the retained forward graph. When false, the inward state still propagates, updates measurements, forms requested same-surface products, and contributes passive material summaries, but creates no new inward BH children. With positive `InwardSeedCovarianceScale`, components copied from the final forward mixture are not collapsed. This does not disable deterministic `ElossOn` or `MSOn`. The active reverse value is an explicit maintained-card campaign override, not the compiled default. |
| `MSOn` | `true` | `true` | Enable multiple-scattering process noise in the underlying track fit. |
| `ElossOn` | `false` | `true` | Enable the baseline KalTest deterministic energy-loss treatment in addition to BH splitting. |
| `MaterialPathMode` | `DD4hepBetweenSurfaces` | same | Material assignment for both outward and inward propagation. The default integrates the complete DD4hep volume interval between matched measurement endpoints in canonical inner-to-outer order; `CurrentSurface` remains an explicit comparison control. |
| `MaterialIPExtrapolation` | `false` | `false` | Include material effects during final extrapolation to the interaction point. Kept off in the active workflow. |
| `KappaSeedCov` | `-1` | same | Legacy diagnostic override for the standard-KF-style outward initializer and for the fresh inward initializer selected by `InwardSeedCovarianceScale<=0`. Any finite value `<=0` selects the exact standard-KF `Var(omega)=1e-4`; a finite positive value instead sets `Var(omega)=KappaSeedCov * alpha^2` before pivot transport, where `kappa=omega/alpha`. It changes only this curvature entry; the direction-local three-hit two-dimensional prefit, other four loose covariance entries, and explicit seed-hit update remain unchanged. Outward uses the three innermost hits; fresh inward uses the three outermost. It is irrelevant to the copied-mixture inward seed selected by a positive inward scale. |

Material between consecutive accepted measurements is owned by the outgoing
transition from the current measurement to the next one. The final
measurement has no outgoing transition, and `MaterialPathMode` governs both
propagation directions. In the default `DD4hepBetweenSurfaces` mode, the
outward component's current measurement state is the segment start and the
already matched target `TrackerHit` global point is the endpoint. Inward
filtering evaluates the same bounded interval in canonical inner-to-outer
order using the matched inner and outer hit points; the separate reverse flag
still controls the BH response. The material manager integrates the DD4hep
volume interval after finite-point, matched-surface, and propagation-direction
checks. This avoids independently re-solving a bounded target-surface
intersection after the hit was accepted. The returned material segments must
cover the requested endpoint distance. If TGeo navigation from an exact
boundary omits the leading volume, the query is retried from 1 micrometre
inside the interval and the leading cap is restored from `materialAt` at its
midpoint. A retry that still does not cover the interval is invalid rather
than silently accepting a partial material path.
In the `CurrentSurface` control, the owning surface's inner and outer normal
thicknesses are divided by the absolute dot product of the component-local
track tangent and DD4hep surface normal. Outward propagation evaluates the
current filtered state; inward propagation evaluates the same physical surface
at the reverse component's target crossing before the measurement update.

The DD4hep endpoint and representative inner-VXD ownership are mechanically
validated, but the collapsed interval's BH energy-loss response and population
momentum performance remain under study. Default status is therefore a
steering decision, not a claim of production physics validation.

### Truth BH-loss oracle

`TruthBHLossOverride` is a default-off diagnostic that asks whether exact
Geant4 eBrem loss would survive the ordinary downstream GSF measurement,
reduction, reverse-filtering, and publication workflow. Its only supported
truth source is the embedded `EventData` provenance stored in the current EDM
event. On the configured input track, every otherwise eligible BH call
receives the normal component and material path, but its configured `BHModel`
response is replaced by one child with conditional process weight one at

```text
z_truth = 1 - truth_ebrem_loss_GeV / truth_p_before_GeV.
```

The child uses only the splitter's `1e-12` numerical retained-fraction
variance floor. The input is
the Geant4 eBrem-attributed loss, not total pre/post momentum loss: using the
latter would mix other processes into the oracle and overlap the independently
configured deterministic energy-loss treatment. The rest of the GSF workflow
is unchanged. This makes the mode a truth-dependent mechanism test, not a
candidate reconstruction algorithm or a BH-model validation.

The current input event must contain `GsfG4MaterialSteps` and
`GsfSimTrackerHitG4StepLinks` together with the standard reconstructed-to-
simulated tracker-hit association collections. The algorithm maps every
accepted `CompleteTracks` hit through its `MCRecoTrackerAssociation` to the
exact `SimTrackerHit`, follows the embedded provenance link to the contributing
Geant4 step range and measurement hook, and derives the interval eBrem loss
from those ordered steps. The 5 mm endpoint guard remains an independent
integrity check. Missing or ambiguous associations, provenance, step bounds,
or required interval coverage invalidate the whole selected truth scope and
fall back to ordinary BH. Absence of either required PODIO collection from the
input file is instead an input/card contract error: truth-on cards must request
both collections explicitly. Because these collections travel with the ordinary
event through `keep *`, this source requires neither a side material ROOT file
nor an event-number join.

The override requires `ElectronHypothesis=true` and
`MaterialPathMode=DD4hepBetweenSurfaces`. The embedded collections are
validated per event. Missing coverage of any exact consecutive accepted-hit
interval—including hit 0 to hit 1—invalidates the selected track's complete
scope before filtering and falls back to the ordinary BH model for that whole
track; it never assumes zero loss. Other input tracks are outside the oracle
scope and use the configured BH model unchanged. This all-or-nothing boundary
prevents an undocumented truth/BH hybrid within one fitted track.

The per-input-track status codes are:

| Status | Meaning |
|---:|---|
| `1` | Complete truth scope validated before filtering; eligible BH calls use the truth response. |
| `2` | Input track is not `TruthBHLossInputTrackIndex`; ordinary BH used. |
| `0` | Truth override disabled. |
| `-1` | Embedded `EventData` truth is invalid or unavailable; ordinary BH used. |
| `-2` | Runtime-hit/truth-anchor endpoint guard failed; ordinary BH used. |
| `-3` | Exact consecutive runtime interval mapping failed; ordinary BH used. |
| `-4` | Selected track/event was not processed, for example because of focused `SelectedEventIndices` steering or an unusable input track. |

### Passive truth-material interval recorder

`RecordTruthMaterialIntervals` is independent of `TruthBHLossOverride`. When
enabled, it uses the embedded `EventData` provenance and the same exact
reconstructed-hit association, selected input-track index, and endpoint
integrity guard described above, but it only writes diagnostic information.
It does not replace `pathTX0` or the configured `BHModel` response, and it does
not alter propagation, split gating, measurement updates, reduction, reverse
weights, or final selection.

For each consecutive accepted-hit interval, the Geant4 reference integrates
fractional boundary steps plus every complete intervening step:

```text
truth_g4_tx0 =
    (1 - from_hook_fraction) * from_step_tx0
  + sum(complete intermediate step_tx0)
  + to_hook_fraction * to_step_tx0
```

When both hooks lie in one Geant4 step, the interval thickness is instead
`(to_hook_fraction - from_hook_fraction) * step_tx0`. The DD4hep truth-hook
value is independently integrated between those same two truth positions.
The runtime values are the actual component-path material summaries seen in
the ordinary forward and reverse workflows. They are stored separately by
direction as weighted, minimum, maximum, and leading-path values; they are not
per-BH-call child records. `GSFTruthMaterialIntervals` contains one passive
summary per accepted-hit interval for the configured input track, while
`GSFTruthMaterialRecordStatus` contains the per-input-track scope status. The
flat-tuple representation uses the `truth_material_` prefix, including scalar
scope status/validity and interval vectors. The material-record status uses
the same numeric status table as the truth oracle above, interpreted as
recording validity rather than permission to steer BH. Keeping these
quantities side by side permits a later diagnostic to separate
Geant4/DD4hep material-map
differences from reconstructed-endpoint/component-path differences without
using truth to steer the fit. Missing or invalid provenance is represented in
the recorded interval validity/status; it has no effect on an otherwise
ordinary GSF job when the truth BH-loss oracle itself is off.

Because this recorder reads `GsfG4MaterialSteps` and
`GsfSimTrackerHitG4StepLinks` from the current event, an enabled input card must
request both collections. The compiled and active reverse-template value is
true, so the maintained simulation workflow and reverse template request both
by default. A control using a historical input without embedded provenance must
set `RecordTruthMaterialIntervals=false` explicitly and omit those collection
requests. Disabling the recorder changes only diagnostic output availability.
Unrequested association collections for detector regions unused by the
selected track are tolerated. The selected track itself remains strict: every
accepted hit must resolve through one available standard truth association or
the complete recording scope is marked invalid.

### Mixture population and reduction

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `MaxComponents` | `10` | `10` | Posterior-reduction trigger/capacity for the live forward/reverse mixtures and reduction target for retained interior `B_smoothed` products. A BH split is updated before reduction, so this is not an instantaneous ceiling. Keep 12 and 24 only as explicit comparisons. |
| `ReductionTargetComponents` | `0` | `0` | Number retained after reduction; zero means use `MaxComponents`. Valid values are zero or `1..MaxComponents`. |
| `ReductionMergeCost` | `SymmetricKL` | same | Pair-ranking cost for moment merging: active `SymmetricKL` ranks pairs by their unweighted symmetric component-to-component KL distance; default-off `Runnalls` ranks the information-loss bound of the weighted mixture approximation. Both perform the same weight-aware moment merge after choosing a pair. Runnalls was tested and rejected for promotion. |
| `ComponentWeightCutoff` | `1e-4` | `1e-4` | Remove normalized target-measurement posterior components below this weight in the live forward and `LocalMeasurement` reverse mixtures; `SmoothedMarginal` instead cuts the selected forward-marginalized live reverse weights. It separately cuts normalized Gaussian-overlap weights in the retained interior `B_smoothed` product, while preserving at least the largest and, when enabled, an identity lineage. The live marginal is computed from all valid direct pairs before that product cutoff or KL reduction. This cutoff precedes live component-count reduction and is independent of `BHSplitThreshold`. |
| `ProtectIdentityLineage` | `true` | `true` | Preserve at least one exact no-radiation lineage through cutoff and reduction when the target component count exceeds one. |

Forward children from transition `i -> i+1` remain expanded through
measurement `i+1`; reverse children from `i+1 -> i` remain expanded through
measurement `i`. The exact innovation likelihood is applied and normalized
before cutoff and reduction. A transition can therefore temporarily require
roughly `MaxComponents * number-of-BH-modes` measurement updates.

### Forward and backward-workflow publication

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `GSFOutputMode` | `BestBranch` | inapplicable | Forward-only publication selector: `BestBranch` or moment-matched `WeightedMean`. It does not select smoother or reverse output. The maintained card pins it to `BestBranch` for explicit compatibility. |
| `ReverseFiltering` | `false` | `true` | Run the independent inward multi-component refit from the complete final forward mixture. This is the active production candidate. |
| `InwardSeedCovarianceScale` | `100` | `100` | For reverse, a finite positive value copies every final forward component into the inward seed and multiplies every element of its covariance by this factor. A finite value `<=0` instead constructs one fresh standard-KF-style backward seed, updates the outermost hit `N-1` exactly once, and starts the live inward recursion at `N-2`. The maintained comparison card now uses `-1` as fresh-seed campaign steering; this does not change the compiled or active-template default. |
| `InwardWeightMode` | `LocalMeasurement` | same | Select the live inward weights while always propagating the measurement-updated `B_updated` means and covariances. `LocalMeasurement` uses `prior(B_predicted) x likelihood(hit|B_predicted)`. Experimental `SmoothedMarginal` uses the normalized unreduced pair weights `weight(F_updated) x weight(B_predicted) x GaussianOverlap(F_updated,B_predicted)`, summed over all valid forward partners for each backward component. It applies only at interior surfaces; hit 0 retains the local-measurement weight because there is no explicit interior product. A missing/nonpositive marginal rejects that candidate rather than silently falling back. Reusing overlapping forward evidence at successive surfaces is intentional but not a calibrated Bayesian posterior. The maintained `DumpGsfTrks/gsf.py.bk` reverse branch selects `SmoothedMarginal`; the compiled and active reverse-template default remains `LocalMeasurement`. |
| `ReverseInitialWeightMode` | `ForwardPosterior` | same | Copied-mixture reverse-start weights: active `ForwardPosterior` or default-off `Uniform` diagnostic. It is ignored by fresh inward initialization, whose single root has unit weight. |
| `ReverseSelectionMode` | `AggregateWeight` | same | Final branch score: active `AggregateWeight`; rejected diagnostics `DominantLineage` and `SurfaceConsistency`. |
| `SurfaceConsistencyUninformativeFloor` | `0.05` | same | Lower bound used only by `SurfaceConsistency`; 0.05 caps its selection Bayes factor at 20. |

`AggregateWeight` selects the component with the largest normalized weight.
`DominantLineage` multiplies that weight by the fraction supplied by its
strongest real pre-merge lineage. `SurfaceConsistency` multiplies it by a
bounded forward/reverse radiative-surface coincidence likelihood. Both
alternatives are retained only to reproduce rejected diagnostics.
Fresh inward initialization has no inherited forward-process metadata, so
`SurfaceConsistency` supplies the same floor to every component and reduces
to ordinary weight ranking in that mode.
`ProtectIdentityLineage` is a reduction safeguard, not another selection mode.

Smoother and reverse have no output selector. Every successful track is
written row-for-row to three collections: `GSFTracksBestBranch` is
BestBranch, `GSFTracksWeightedMean` is the normalized final-mixture moment
match, and `GSFTracksFullMixtureMode` is the joint density maximum of the full
mixture at the IP. `ReverseSelectionMode` affects only BestBranch. It does not
alter the components entering either of the other two views.

For reverse, at every interior hit `0 < i < N-1`, every accepted backward
prediction is paired with all stored forward filtered components at that hit.
The resulting two-filter product is built before the live update is committed
and is persisted in the lineage record. Its direct-pair weights are normalized
before the product's own cutoff and KL reduction. `SmoothedMarginal` sums
those direct weights by backward parent and uses that marginal as the live
weight of the corresponding measurement-updated `B_updated[i]` state;
`LocalMeasurement` leaves the product fully diagnostic. At the boundaries no
product node is synthesized: `B_smoothed[0]` is the terminal `B_updated[0]`,
while `B_smoothed[N-1]` is the final `F_updated[N-1]`. Reverse derives all
three endpoint views from `B_updated[0]`; no interior smoothed state is
propagated or published.
Historical CMS-like files retain their original collections and flat fields;
they are not an active workflow and are not renamed in place.

FullMixtureMode maximizes the complete five-dimensional Gaussian-mixture PDF
in the local IP helix coordinates `(drho, phi0, kappa, dz, tanLambda)`. This is
not a one-dimensional `q/p` mode inserted into otherwise averaged parameters,
and it is not another component selector. Each surviving component is first
extrapolated to the IP. A deterministic multistart search uses the mixture
mean, every component mean, and all pairwise weight-interpolated means; a
Gaussian mean-shift iteration followed by Newton refinement locates stationary
maxima, and the highest-density valid maximum is published. Its covariance is
the local Laplace covariance, the inverse negative Hessian of `log p(x)` at
that maximum. This coordinate-dependent experimental summary is mechanically
available by default but is not yet performance-validated.

Neither WeightedMean nor FullMixtureMode has a unique branch chi-square/NDF,
so both retain the selected BestBranch fit-quality metadata. A failed mode
search publishes an exact BestBranch parameter/covariance fallback and records
a negative status in `GSFFullMixtureModeStatus`; consumers must inspect that
status rather than treating every persisted mode track as a successful
optimization. The current KL smoother assigns one common smoothed inner
mean/covariance to every survivor, so its three endpoint states are expected
to be numerically identical; all are saved to keep the output contract
uniform.

The final component record is automatic and has no configuration property.
For every successfully published smoother or reverse track, all finite,
positive-weight survivors are normalized within that track and independently
extrapolated to the same IP endpoint used by the method. The EDM stores each
component's normalized weight, kappa mean, kappa variance, method source, and
validity, together with both the input- and output-track indices. It is passive
output only: recording does not alter reduction, selection, endpoint
publication, or any component state. Ordinary forward workflows do not expose
this final multi-component endpoint and therefore produce no component rows.

### Experimental ECAL component constraint

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `EcalComponentConstraint` | `false` | `false` | Enable a default-off, two-sided ECAL likelihood that can re-rank the already fitted final reverse components. It requires reverse filtering and starts from the BestBranch publication. |
| `EcalConstraintRatioThreshold` | `1.1` | same | Activate re-ranking only when the unconstrained branch has `max(p/E,E/p)` above this value. It must be finite and greater than one. |
| `EcalConstraintLogPSigma` | `0.15` | same | Gaussian width of the component likelihood in `log(p/E)`; it must be finite and positive. |
| `EcalConstraintLikelihoodFloor` | `0.05` | same | Additive likelihood floor in `(0,1]`; 0.05 limits the ECAL re-ranking Bayes factor to 20. |
| `EcalConstraintPhiWindow` | `0.10` | same | Maximum absolute azimuth difference, in radians and in `(0,pi]`, for summing positive-energy `EcalCluster` objects around the extrapolated outer forward-GSF direction. |
| `EcalConstraintThetaWindow` | `0.10` | same | Maximum absolute polar-angle difference, in radians and in `(0,pi]`, for the same cluster-energy sum. A cluster must pass both the phi and theta windows. |

The ECAL observation uses neither truth nor LCIO/PFO momentum. It sums
`EcalCluster` energy inside both configured angular windows around the
extrapolated outer forward-GSF direction and, after the ordinary reverse fit
has finished, multiplies each final component's existing reverse selection
score by

```text
floor + (1 - floor) * exp[-0.5 * (log(p_component/E) / sigma)^2].
```

This is a selection constraint, not a track--calorimeter parameter
combination: it does not alter any fitted component state or covariance.
`GSFTracksBestBranch` always preserves the unconstrained tracker-only result. When the
experiment is enabled, `GSFTracksEcalConstrained` is created alongside it. If
the ECAL observation is unavailable or the symmetric ratio does not cross the
threshold, the paired output is an exact parameter/covariance copy of the
unconstrained result; otherwise it can publish a different existing reverse
component. The mode is an unvalidated research control and remains inactive in
the production baseline.

### Alternative backward workflows

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `GaussianSumSmoothing` | `false` | `false` | Run the retained-graph experimental Gaussian-sum smoother. It is default-off and forfeits much of the observed hard-loss recovery. |
`ReverseFiltering` and `GaussianSumSmoothing` are alternative workflows and
must not be enabled simultaneously. A positive inward scale copies the final
forward mixture; a nonpositive scale gives reverse a fresh backward root.

### Focused-event and component diagnostics

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `SelectedEventIndices` | empty | empty | Empty processes the normal event stream; otherwise process only the listed zero-based input entries. |
| `VerboseDump` | `false` | `false` | Print general filtering, track, and workflow diagnostics. |
| `VerboseSplitDump` | `false` | `false` | Dump component populations around BH splits, cutoff, and reduction. |
| `ComponentDebugDump` | `false` | `false` | Dump exact component states, innovation quantities, and lineage histories. |
| `SurfaceLineageMassDump` | `false` | `false` | Propagate and print aggregate BH-mode probability mass by surface. |
| `ComponentDebugMaxHistory` | `240` | `240` | Maximum process/lineage history retained per component for debug output. |

The reverse template connects the first three verbose properties to
`GSF_VERBOSE_COMPONENTS`. Comprehensive focused-event validation normally
enables all three together.

The former public runtime material/BH-audit CSV option and writer are retired,
and the runtime CSV and side-ROOT readers have also been removed. Historical
audit CSVs and project records remain valid as evidence, but current code does
not consume those helper files and stale cards that assign removed properties
must be regenerated. The maintained recording path is now the default-on
`GSFTruthMaterialIntervals` EDM collection and matching
`truth_material_*` final flat-tuple vectors. They preserve accepted-hit truth,
truth-hook DD4hep, and summarized forward/reverse runtime material values, not
one row per BH parent or child.

### Collection handles

The data handles are configurable separately from the 39 properties:

| Role | Default collection |
|---|---|
| input reconstructed tracks | `CompleteTracks` |
| generic forward output tracks | `GSFTracks` |
| selected smoother/reverse BestBranch output tracks | `GSFTracksBestBranch` |
| paired smoother/reverse moment-matched tracks | `GSFTracksWeightedMean` |
| paired smoother/reverse full-mixture density-mode tracks | `GSFTracksFullMixtureMode` |
| per-output-track full-mixture-mode status | `GSFFullMixtureModeStatus` |
| final-mixture component track mapping | `GSFFinalMixtureComponentInputTrackIndex`, `GSFFinalMixtureComponentOutputTrackIndex` |
| final-mixture component identity/method/status | `GSFFinalMixtureComponentIndex`, `GSFFinalMixtureComponentID`, `GSFFinalMixtureComponentSource`, `GSFFinalMixtureComponentValid` |
| final-mixture component PDF parameters | `GSFFinalMixtureComponentWeight`, `GSFFinalMixtureComponentKappa`, `GSFFinalMixtureComponentKappaVariance` |
| component-lineage node mapping and identity | `GSFLineageNodeInputTrackIndex`, `GSFLineageNodeOutputTrackIndex`, `GSFLineageNodeId`, `GSFLineageNodeSource`, `GSFLineageNodeOperation`, `GSFLineageNodeHitIndex`, `GSFLineageNodeSurfaceIndex`, `GSFLineageNodeComponentId`, `GSFLineageNodeGeneration` |
| component-lineage decision and state data | `GSFLineageNodeBHComponentIndex`, `GSFLineageNodeMeasurementStatus`, `GSFLineageNodeFate`, `GSFLineageNodeNoRadiation`, `GSFLineageNodeBestBranch`, `GSFLineageNodeFinalMixture`, `GSFLineageNodeValid`, plus the `GSFLineageNode*` weight, BH, material, innovation, kappa/covariance, lineage-fraction, and merge-cost collections described below |
| component-lineage edges | `GSFLineageEdgeInputTrackIndex`, `GSFLineageEdgeOutputTrackIndex`, `GSFLineageEdgeFromNodeId`, `GSFLineageEdgeToNodeId`, `GSFLineageEdgeOperation` |
| input ECAL clusters | `EcalCluster` |
| paired ECAL-constrained output tracks | `GSFTracksEcalConstrained` |
| truth particles | `MCParticle` |
| per-input-track truth-oracle status | `GSFTruthBHLossStatus` |
| passive truth-material interval summaries | `GSFTruthMaterialIntervals` |
| per-input-track truth-material recording status | `GSFTruthMaterialRecordStatus` |

### Flat-tuple paired-track branches

`RecGsfFlatTuple` keeps method-explicit tracker-result schemas. The
`bestbranch_gsf_*` fields come only from `GSFTracksBestBranch`, while the
generic `gsf_*` fields come from an optional `GSFTracks` collection. When
`GSFTracksWeightedMean`, `GSFTracksFullMixtureMode`, or
`GSFTracksEcalConstrained` is present in the ordinary GSF event store, the
tuple also fills the corresponding parallel scalar set:

| Branches | Meaning |
|---|---|
| `gsf_pT`, `gsf_p`, `gsf_eta`, `gsf_theta`, `gsf_phi`, `gsf_d0`, `gsf_z0`, `gsf_omega`, `gsf_tanl`, `gsf_chi2`, `gsf_ndf`, `gsf_nhits`, `gsf_type` | Generic forward result from `GSFTracks`. These are zero for smoother/reverse. |
| `bestbranch_gsf_pT`, `bestbranch_gsf_p`, `bestbranch_gsf_eta`, `bestbranch_gsf_theta`, `bestbranch_gsf_phi`, `bestbranch_gsf_d0`, `bestbranch_gsf_z0`, `bestbranch_gsf_omega`, `bestbranch_gsf_tanl`, `bestbranch_gsf_chi2`, `bestbranch_gsf_ndf`, `bestbranch_gsf_nhits`, `bestbranch_gsf_type` | Smoother/reverse BestBranch from `GSFTracksBestBranch`. |
| `bestbranch_gsf_available` | One when `GSFTracksBestBranch` is present for the row; zero for forward. |
| `weighted_gsf_pT`, `weighted_gsf_p`, `weighted_gsf_eta`, `weighted_gsf_theta`, `weighted_gsf_phi`, `weighted_gsf_d0`, `weighted_gsf_z0`, `weighted_gsf_omega`, `weighted_gsf_tanl`, `weighted_gsf_chi2`, `weighted_gsf_ndf`, `weighted_gsf_nhits`, `weighted_gsf_type` | Paired `GSFTracksWeightedMean` result for smoother/reverse. The chi-square/NDF are inherited from BestBranch. |
| `weighted_gsf_available`, `weighted_gsf_changed` | Presence tag and exact scalar comparison against the BestBranch fields. They are zero for forward output. |
| `fullmixture_gsf_pT`, `fullmixture_gsf_p`, `fullmixture_gsf_eta`, `fullmixture_gsf_theta`, `fullmixture_gsf_phi`, `fullmixture_gsf_d0`, `fullmixture_gsf_z0`, `fullmixture_gsf_omega`, `fullmixture_gsf_tanl`, `fullmixture_gsf_chi2`, `fullmixture_gsf_ndf`, `fullmixture_gsf_nhits`, `fullmixture_gsf_type` | Paired full five-dimensional mixture-density mode from `GSFTracksFullMixtureMode`. The chi-square/NDF are inherited from BestBranch. |
| `fullmixture_gsf_available`, `fullmixture_gsf_changed` | Presence tag and exact scalar comparison against BestBranch. They are zero for forward output. |
| `fullmixture_gsf_status` | `1` successful joint mode; `0` not applicable; `-1` incomplete component set; `-2` optimization failure; `-3` invalid local covariance; `-4` unavailable method endpoint. Negative values identify a persisted BestBranch fallback. |
| `final_mixture_component_available`, `final_mixture_component_n` | One when at least one final smoother/reverse component was recorded, and the common length of every `final_mixture_component_*` vector. These branches always exist. |
| `final_mixture_component_input_track_index`, `final_mixture_component_output_track_index` | Map every component to its source `CompleteTracks` index and row-aligned published GSF track index. This preserves all output tracks even though the legacy scalar endpoint fields describe only the first track. |
| `final_mixture_component_index`, `final_mixture_component_id`, `final_mixture_component_source`, `final_mixture_component_valid` | Position in the final internal component vector, event-local diagnostic component ID, source code (`1` Gaussian-sum smoother or `2` reverse terminal inward mixture), and IP-state validity. Codes `3` (historical CMS-like hit-1 smoothed endpoint) and `4` (historical terminal-backward fallback) remain reserved for interpreting older tuples. `valid=1` requires successful extrapolation, finite parameters, positive finite kappa variance, and a positive-definite full IP covariance. |
| `final_mixture_component_weight`, `final_mixture_component_kappa`, `final_mixture_component_kappa_variance`, `final_mixture_component_pT` | Per-component normalized weight, IP kappa mean, covariance element `Cov(kappa,kappa)`, and derived `1/abs(kappa)` in GeV. The pT entry is NaN when `valid!=1` or kappa is unusable. |
| `lineage_graph_available`, `lineage_node_n`, `lineage_edge_n` | Presence flag and the common lengths of the node and edge vector families. The branches always exist; smoother and reverse populate them automatically, while forward, unprocessed rows, and older EDM inputs leave them zero/empty. |
| `lineage_node_input_track_index`, `lineage_node_output_track_index`, `lineage_node_id` | Stable graph key and track mapping. Node IDs start at zero independently for each input track, are never reused within that track, and remain in the record after the live component is deleted. The unique event-local key is `(input_track_index,node_id)`. `output_track_index=-1` preserves the evaluated graph when no GSF endpoint could be published. |
| `lineage_node_source`, `lineage_node_operation`, `lineage_node_hit_index`, `lineage_node_surface_index`, `lineage_node_component_id`, `lineage_node_generation` | Workflow side, creation operation, call-site hit/surface, diagnostic component ID, and BH generation. Source is `1` forward, `2` reverse/backward, or `3` an interior two-filter smoothed product. Operation is `1` seed, `2` BH split child, `3` evaluated measurement result, `4` KL-merge output, or `5` smoothing candidate. Smoother graphs contain the forward construction used by the smoother. With positive `InwardSeedCovarianceScale`, reverse links each copied forward state to its backward seed; with a nonpositive scale, it instead contains one source-2 operation-1 root at hit `N-1` with no forward parent. Reverse records operation-5 candidates only at successfully processed interior surfaces `0 < i < N-1`; `B_smoothed[0]` is represented by the terminal source-2 nodes and `B_smoothed[N-1]` by the final source-1 nodes. Each explicit smoothing candidate has one source-1 `F_updated[i]` parent and one source-2 pre-measurement parent representing `B_predicted[i]`; the exact transported backward prediction is persisted on the source-3 candidate itself. Numeric source and operation codes are unchanged. |
| `lineage_node_bh_component_index`, `lineage_node_bh_weight`, `lineage_node_bh_mean`, `lineage_node_bh_variance`, `lineage_node_material_tx0` | Exact configured BH mode and interval thickness for a split-created child. They are NaN or `-1` when the node was not created by a successful BH split. |
| `lineage_node_measurement_status`, `lineage_node_dchi2`, `lineage_node_logdet_innovation`, `lineage_node_log_unnormalized_posterior`, `lineage_node_normalized_posterior`, `lineage_node_prior_weight` | Surface-local evidence. For source-1/source-2 operation-3 nodes, status is `0` rejected, `1` accepted through the exact measurement update, or `2` accepted through the legacy recovery path; `dchi2` and `logdet_innovation` are the measurement innovation terms, and `log_unnormalized_posterior` always preserves the local `prior(B_predicted) x likelihood(hit|B_predicted)` log score. `normalized_posterior` is the actual selected pre-pruning live weight: the normalized local score in `LocalMeasurement`, or the normalized forward-marginalized product weight in interior `SmoothedMarginal` steps. For a source-3 operation-5 candidate, status remains `-1` because no detector measurement is performed, `prior_weight=w_F*w_B`, `dchi2` is the five-dimensional `F_updated`/`B_predicted` compatibility quadratic, `logdet_innovation=log(det(C_F+C_B))`, and `log_unnormalized_posterior` is the exact pair log weight used by the algorithm. Its `normalized_posterior` is the direct pair weight before the product cutoff and KL reduction. Split, seed, and KL-output nodes retain NaN for non-applicable evidence fields. |
| `lineage_node_fate`, `lineage_node_no_radiation`, `lineage_node_best_branch`, `lineage_node_final_mixture`, `lineage_node_valid` | Fate is `0` active, `1` advanced to a child, `2` measurement rejected, `3` removed by weight cutoff, `4` consumed by KL merge, `5` final survivor, `6` abandoned because its endpoint or complete output track failed, or `7` a smoothed-mixture survivor retained for diagnostics but not included in the published endpoint. The remaining flags identify the exact identity lineage, published BestBranch, final-mixture membership, and a finite recorded state. |
| `lineage_node_weight`, `lineage_node_predicted_kappa`, `lineage_node_predicted_kappa_variance`, `lineage_node_predicted_pT`, `lineage_node_filtered_kappa`, `lineage_node_filtered_kappa_variance`, `lineage_node_filtered_pT`, `lineage_node_smoothed_kappa`, `lineage_node_smoothed_kappa_variance`, `lineage_node_smoothed_pT`, `lineage_node_dominant_lineage_fraction`, `lineage_node_merge_cost` | Node-local statistical state. For measurement nodes, predicted quantities are the pre-update state and filtered quantities are the post-update state. For source-3 operation-5 candidates, predicted quantities are the exact `B_predicted[i]` input and the explicit `smoothed_*` quantities are the `F_updated[i] x B_predicted[i]` product. Source-3 KL outputs also populate `smoothed_*`; other sources receive NaN in those aliases. The legacy generic `filtered_*` values remain populated for source 3 for schema compatibility and equal `smoothed_*`. pT is derived as `1/abs(kappa)`. Merge cost is finite only for a KL output. `weight` is the current/final node weight, while `normalized_posterior` preserves the direct candidate's pre-pruning posterior. |
| `lineage_node_fb_delta_kappa`, `lineage_node_fb_delta_kappa_variance`, `lineage_node_fb_delta_pT`, `lineage_node_fb_delta_pT_variance`, `lineage_node_fb_brem_probability` | Passive forward/backward compatibility diagnostics for each direct source-3 operation-5 pair. The signed definitions are `delta_kappa=kappa(B_predicted)-kappa(F_updated)` and `delta_pT=pT(B_predicted)-pT(F_updated)`, with `pT=1/abs(kappa)`. Because no forward/backward cross-covariance is available, the stored variances use the same independence approximation as the overlap: `Var(delta_kappa)=C_F(kappa,kappa)+C_B(kappa,kappa)` and first-order `Var(delta_pT)=C_F/kappa_F^4+C_B/kappa_B^4`. For the full state difference `delta_x=x_Bpred-x_Fupdated` (with wrapped phi), let `q=delta_x^T(C_F+C_B)^-1 delta_x` and `R=F_chi2,5(q)`. The direction-signed five-dimensional score is `P_brem=(1+R)/2` for `delta_pT<0`, `(1-R)/2` for `delta_pT>0`, and `0.5` at zero. It therefore approaches one for a large 5D incompatibility in the energy-loss direction and zero for a large opposite-direction incompatibility; in one dimension the construction reduces exactly to the former `Phi(-delta_pT/sigma_delta_pT)` score. For a brem decision it is meaningful on the exact no-radiation pair; a radiative pair can return toward `0.5` after its loss hypothesis reconciles the messages. Under ideal independent calibrated Gaussian messages, thresholds `0.97725` and `0.99865` have the nominal one-sided 2-sigma and 3-sigma tail probabilities, but the actual F/B messages do not justify that calibration. This remains an algorithmic compatibility score, not a physical posterior: it assumes zero F/B cross-covariance and contains neither a no-brem point-mass prior nor a radiative-loss prior. Non-curvature inconsistencies can also raise its magnitude. Values are NaN for other node types, reduced source-3 KL outputs, invalid inputs, and the two boundary mixtures. These fields never steer filtering, weights, reduction, or publication. The stored field used the one-dimensional definition in tuples made before the 2026-09-02 5D update; the raw delta and `dchi2` fields disambiguate or reconstruct either definition. Older EDM inputs retain the lineage graph and receive row-aligned NaN only for fields absent from that input. |
| `lineage_edge_input_track_index`, `lineage_edge_output_track_index`, `lineage_edge_from_node_id`, `lineage_edge_to_node_id`, `lineage_edge_operation` | Directed graph connectivity. Edge operation is `1` BH split, `2` measurement, `3` KL merge, `4` copied forward-to-backward seed, or `5` a forward/backward state entering a smoothed-mixture candidate. Edge operation 4 exists only for positive inward scale; a fresh nonpositive-scale inward root has no incoming edge. Every KL output has two incoming merge edges; every smoothed-mixture candidate has exactly two incoming smoothing edges. Numeric codes are unchanged. |
| `ecal_gsf_pT`, `ecal_gsf_p`, `ecal_gsf_eta`, `ecal_gsf_theta`, `ecal_gsf_phi`, `ecal_gsf_d0`, `ecal_gsf_z0`, `ecal_gsf_omega`, `ecal_gsf_tanl`, `ecal_gsf_chi2`, `ecal_gsf_ndf`, `ecal_gsf_nhits`, `ecal_gsf_type` | Paired `GSFTracksEcalConstrained` result. |
| `ecal_gsf_available` | One when a constrained track is present for the tuple row; otherwise zero. |
| `ecal_gsf_changed` | One when the constrained and ordinary AtIP track parameters or fit quality differ; otherwise zero. |
| `res_pT_gsf`, `res_pT_bestbranch_gsf`, `res_pT_weighted_gsf`, `res_pT_fullmixture_gsf`, `res_pT_ecal_gsf` | Generic method, BestBranch, WeightedMean, FullMixtureMode, and constrained fractional pT residuals relative to the first truth particle. |
| `truth_bh_scope_status`, `truth_bh_scope_valid` | Status code above and a convenience one/zero validity tag for `CompleteTracks` index 0. Older inputs without `GSFTruthBHLossStatus` receive the disabled/invalid defaults `0,0`. |
| `truth_material_scope_status`, `truth_material_scope_valid`, `truth_material_interval_n` | Passive material-record scope status/validity for the configured track and number of interval-vector entries. |
| `truth_material_input_track_index`, `truth_material_output_track_index`, `truth_material_hit_from_index`, `truth_material_hit_to_index`, `truth_material_surface_from_index`, `truth_material_surface_to_index`, `truth_material_cell_from`, `truth_material_cell_to` | Per-interval reconstructed-track, accepted-hit, matched-surface, and cell-ID bounds. |
| `truth_material_track_id`, `truth_material_first_step`, `truth_material_last_step`, `truth_material_start_hook_fraction`, `truth_material_end_hook_fraction`, `truth_material_start_x`, `truth_material_start_y`, `truth_material_start_z`, `truth_material_end_x`, `truth_material_end_y`, `truth_material_end_z`, `truth_material_step_count` | Exact matched Geant4 track, boundary steps/fractions, hook positions in mm, and positive-length step-piece count. |
| `truth_material_g4_tx0`, `truth_material_p_before`, `truth_material_ebrem_loss`, `truth_material_retained_fraction` | Fractionally integrated Geant4 t/X0, inner-hook momentum in GeV, subtype-3 eBrem loss in GeV, and its retained-momentum fraction. |
| `truth_material_dd4hep_hook_valid`, `truth_material_dd4hep_hook_layer_count`, `truth_material_dd4hep_hook_tx0` | Validity, material-segment count, and DD4hep t/X0 integrated between the same two truth hooks. |
| `truth_material_runtime_mode`, `truth_material_split_threshold` | Runtime material-mode code (`1` `CurrentSurface`, `2` `DD4hepBetweenSurfaces`) and configured BH split threshold. |
| `truth_material_forward_candidate_count`, `truth_material_forward_valid_count`, `truth_material_forward_above_threshold_count`, `truth_material_forward_weighted_tx0`, `truth_material_forward_min_tx0`, `truth_material_forward_max_tx0`, `truth_material_forward_leading_component_id`, `truth_material_forward_leading_component_weight`, `truth_material_forward_leading_tx0` | Forward component-path population and parent-weighted/minimum/maximum/leading runtime material summaries. |
| `truth_material_reverse_candidate_count`, `truth_material_reverse_valid_count`, `truth_material_reverse_above_threshold_count`, `truth_material_reverse_weighted_tx0`, `truth_material_reverse_min_tx0`, `truth_material_reverse_max_tx0`, `truth_material_reverse_leading_component_id`, `truth_material_reverse_leading_component_weight`, `truth_material_reverse_leading_tx0` | Equivalent reverse component-path summaries, or empty/zero counts when reverse filtering is inactive. |

The weighted branch set has no method/configuration switch. The flat tuple
always creates it and fills it only when `GSFTracksWeightedMean` is present;
forward does not produce that collection, so all weighted values and both
flags remain zero.

The full-mixture branch set follows the same presence-driven rule and has no
configuration switch. It is filled only from `GSFTracksFullMixtureMode` and
its status collection. Forward jobs leave it unavailable/zero with status
zero. A negative status with
`fullmixture_gsf_available=1` means the row-aligned track is the deliberate
BestBranch fallback, not a successfully found density mode.

The `final_mixture_component_*` vectors are likewise automatic/default-on and
presence-driven. Weights are normalized independently for each
`(input_track_index, output_track_index)` group and sum to one when all final
positive-weight components were captured. For `pT > 0`, the saved one-
dimensional kappa marginals reconstruct the transverse-momentum density as

```text
f(pT) = sum_i w_i / pT^2 * [
          N(+1/pT | kappa_i, variance_i)
        + N(-1/pT | kappa_i, variance_i)] .
```

Use only entries with `valid=1`. If any component for a track is invalid, the
saved valid subset is not the complete final mixture and must not silently be
presented as such. The component vectors describe the mixture underlying all
three endpoint summaries; they are not a fourth published track and do not
duplicate hit vectors.

The `lineage_node_*` and `lineage_edge_*` vectors are also automatic and
default-on for smoother/reverse jobs. They record immutable
snapshots rather than pointers to live `GsfComponent` objects. A rejected
measurement node, a posterior-cutoff node, and both inputs consumed by a KL
merge therefore remain available after their C++ components are deleted. A
split followed by a merge is represented by a diverging and reconverging
diamond: this is the requested visual loop shape, while edge direction remains
acyclic in node-creation order. Reconstruct one track's graph by selecting its
`input_track_index`, then joining every edge endpoint to `node_id` in that
same group. Do not join bare node IDs across different input tracks.

The persisted graph is diagnostic evidence, not another GSF result and not an
additional selector. Persisting it never changes propagation, covariance,
posterior normalization, cutoff, reduction, or publication. Under
`SmoothedMarginal`, however, the same transient direct-pair overlap weights
that are recorded in source-3 nodes are marginalized to steer live source-2
weights; source-3 states themselves remain non-propagated. Complete graph
persistence can materially increase tuple size; this is deliberate because
silent truncation would remove exactly the rejected/pruned alternatives
needed to explain the first wrong branch decision.

The BestBranch branch set is likewise presence-driven: it is populated only
from `GSFTracksBestBranch` and remains unavailable/zero for forward jobs. The
generic `gsf_*` fields remain available for that non-paired workflow and are
zero for smoother/reverse. This is an
intentional schema rename for newly produced backward-workflow files;
historical smoother/reverse/CMS-like EDM/flat files retain their original
`GSFTracks`/`gsf_*` names and must not be silently interpreted as the new
schema.

The constrained branches always exist in newly produced flat files. When the
experiment is off or the paired collection is absent, `ecal_gsf_available=0`
and its scalar/residual fields are zero. The constrained track deliberately
has no duplicate hit-vector branches: the experimental collection copies the
BestBranch tracker hits. Smoother/reverse hits are therefore recorded
once as `bestbranch_gsf_hit_*`; generic forward hits remain in `gsf_hit_*`.
WeightedMean and FullMixtureMode also share the BestBranch hit
list and have no duplicate hit-vector branches.

For the BH oracle, the flat tuple contains its scope status. Its LCIO and GSF
hit vectors reproduce associated output hits, not the subset that was
successfully matched and used internally. The separate passive
`truth_material_*` vectors do expose the accepted-hit interval and summarized
runtime material population, but not individual parent/child calls or a
textual invalidity reason. The retired runtime BH-audit CSV is available only
in historical outputs and readers.

All `truth_material_*` interval vectors have
`truth_material_interval_n` entries and mirror the passive
`GSFTruthMaterialIntervals` collection. These final-tuple vectors are the
current material-recording path and deliberately store population summaries
rather than one entry per parent or BH child.

### Historical `DumpGsfTrks` card compatibility

`DumpGsfTrks/gsf.py.bk` explicitly configures 38 of the 39 `RecGsfTracking`
properties. It deliberately inherits only the compiled
`RecordTruthMaterialIntervals=true` default. Its reverse material, split/cutoff, and
ECAL settings agree with the production baseline:
`BHSplitThreshold=1e-4`, `ComponentWeightCutoff=1e-4`,
`DD4hepBetweenSurfaces`, and ECAL off. For the current backward-only BH
mechanism campaign, the card deliberately sets `ForwardBHSplitting=false` in
its common steering and enables `InwardBHSplitting=true` only in the reverse
branch. The forward value agrees with the compiled default; the inward value
is a deliberate campaign override of the compiled `false`. Unsteered active
reverse templates inherit the compiled `false/false` pair. Its top-level
`bh_model` selector is the user-selected, default-off
`CEPCRuntimeGenericGrid5Clear` experiment rather than the production
`CEPC2GeV85StepConditioned` model. The retired runtime BH-audit CSV is no
longer steered. The card's `RecGsfFlatTuple` instance writes
the default-on `truth_material_*` vectors alongside BestBranch
`bestbranch_gsf_*`, paired `weighted_gsf_*` and `fullmixture_gsf_*`, generic
`gsf_*`, and default-zero `ecal_gsf_*` scalar branch sets.

The maintained template exposes only `method="smoother"` and
`method="reverse"`; it currently selects reverse.

The maintained `gsf.py.bk` template keeps the compiled and active
reverse-template `TruthBHLossOverride=false` base value. For a truth-oracle
diagnostic, the generated per-job card may set it true. Embedded EventData is
the fixed source; there is no source selector or helper-file input property.
The card reads `GsfG4MaterialSteps` plus
`GsfSimTrackerHitG4StepLinks` from the ordinary input event, keeps
`TruthBHLossInputTrackIndex=0` and
`TruthBHLossMaxEndpointDistance=5.0`, and requires no side material file. The
compiled and active reverse-template values remain false, track index 0, and
5.0 mm. This is deliberate truth-diagnostic campaign steering, not a
production-default change.
Generated truth-on cards append `truth-bh`; generated off controls append
`truth-bh-off` to their GSF EDM and flat-tuple filenames.

The maintained card inherits the compiled and active reverse-template
`RecordTruthMaterialIntervals=true` default. It unconditionally requests
`GsfG4MaterialSteps` and `GsfSimTrackerHitG4StepLinks` in its base `PodioInput`
list, including when `TruthBHLossOverride=false`. The recorded
truth/DD4hep/runtime values remain passive and do not affect the GSF workflow.
Their directional `*_above_threshold_count` values describe thickness-eligible
material paths, not executed BH splits, so they remain populated when a split
gate is false. Use lineage operation 2 or the verbose split counters to audit
actual child creation. Likewise, the truth BH-loss oracle replaces only split
calls that are enabled and executed; disabling both directional gates executes
no oracle replacement.

### Configuration-maintenance contract

The 39-property inventory above is part of the configurable interface, not a
one-time snapshot. Any change that adds, removes, or renames a
`RecGsfTracking` property, changes its compiled or active default, or changes
its accepted values must include a dedicated sub-agent configuration audit.
That audit must, in the same change:

1. update this complete property reference and recount the exposed properties;
2. update `options/run_gsf_reverse_template.py` when the active effective
   steering changes;
3. add or update an explicit setting in `DumpGsfTrks/gsf.py.bk`, including an
   intentional inactive value or a documented inapplicability for
   workflow-specific diagnostics, so the historical workflow does not
   silently acquire a future compiled default;
4. record any intentional difference between that card and the active
   baseline in `DumpGsfTrks/README.md`;
5. verify that every property declared in `src/GsfAlgorithm.h` is documented
   here and classified as active, inactive, experimental, or diagnostic.

Do not consider a configurable-property implementation complete until this
documentation and steering audit is complete.

## Current limitation

`CEPC2GeV85StepConditioned` is the constrained eight-knot, five-component
transition model for the 2 GeV pT, 85-degree primary-electron execution sample.
It consumes the component-local transition `t/X0` and returns hypotheses in
`z=p_after/p_before`. Its source artifact and diagnostics are under
`data/CEPC2GeV85StepConditioned/`. This same-sample execution model is not a
general or independently validated CEPC Bethe-Heitler parameterization.

`CEPC2GeV85StepConditioned6` uses the same events, t/X0 bins, interpolation,
and total radiative probabilities. It keeps the no-eBrem, 0--1%, and >20%
components, while replacing the former 1--5% plus 5--20% pair with three
truth-extracted 1--5%, 5--10%, and 10--20% components. Its artifact is under
`data/CEPC2GeV85StepConditioned6/`. It is selectable for comparison and is not
the default or a validated improvement.

`CEPCRuntimeGenericGrid5Clear` and `CEPCRuntimeCategoryAligned5Clear` are
parallel, default-off five-component models fitted to topology-clear training
events from exact runtime `DD4hepBetweenSurfaces` intervals. Both take only
the existing component-local `pathTX0` input and represent one exact no-eBrem
component plus four aggregate Geant4 eBrem-loss classes: 0--1%, 1--5%, 5--20%,
and greater than 20%. They use the same interpolation conventions as the
five-component default. `CEPCRuntimeGenericGrid5Clear` retains the default
model's generic logarithmic knot grid, isolating the effect of retraining on
runtime intervals. `CEPCRuntimeCategoryAligned5Clear` instead places its knots
at observed TPC, VXD, service, ITK/bridge, outer, and thick-interval t/X0
bands; detector category is not a runtime input. Their source artifacts are
under the correspondingly named `data/` directories. These models improve
held-out interval-level closure but are not validated for final GSF momentum,
clean-track safety, the separately reported secondary-activity control, or
sparse high-thickness intervals. Above the last fitted knot they retain the
existing constant-mixture extrapolation limitation.

`CEPCRuntimeCategoryAligned9Clear` is a parallel, default-off nine-component
proposal bank derived from the same topology-clear exact runtime intervals and
the same category-aligned t/X0 knots as
`CEPCRuntimeCategoryAligned5Clear`. It keeps an exact no-eBrem identity atom
and uses fixed fractional-loss proposal centers at 1%, 3%, 5%, 7%, 9%, 15%,
30%, and 70%; the component probabilities at each knot are fitted from the
corresponding Geant4 interval-loss population. It exists to provide finer
loss-magnitude coverage, not as a validated BH replacement. It remains
experimental and default-off, and requires held-out interval closure, lineage
survival, clean-track safety, and final-track population validation. Its source
artifact is under `data/CEPCRuntimeCategoryAligned9Clear/`.

`CEPCRuntimeCategoryAligned15Clear` is a parallel, default-off proposal bank
fitted from the same topology-clear exact runtime intervals and using the same
eight category-aligned t/X0 knots. It has 15 total components: one exact
no-eBrem identity atom, five radiative proposals in 0--1% loss, six in 1--6%,
one merged 6--10% proposal, and two proposals spanning 10--100%. The radiative
loss strata are `[0,0.1]`, `[0.1,0.2]`, `[0.2,0.4]`, `[0.4,0.7]`,
`[0.7,1.0]`, `[1.0,1.4]`, `[1.4,1.9]`, `[1.9,2.6]`, `[2.6,3.5]`,
`[3.5,4.7]`, `[4.7,6.0]`, `[6,10]`, `[10,30]`, and `[30,100]` percent.
Each proposal mean is its stratum midpoint and its sigma is one quarter of the
stratum width, so adjacent two-sigma bounds meet while center spacing grows
monotonically. Knot-local weights are direct topology-clear training counts;
the total radiative probability at every knot is unchanged from the nine-
component model. Nineteen knot/component cells have fewer than 25 training
entries, though none is empty. This is proposal-coverage mechanics, not BH or
GSF validation. The authoritative artifact is under
`data/CEPCRuntimeCategoryAligned15Clear/`.

`ActsAtlas` is the ACTS default ATLAS-derived parameterization retained as a
non-CEPC control. New steering should use one of the seven canonical values
listed in the property table.

## Historical Geant4 transition dataset

The maintained workflow no longer schedules
`GsfMaterialStepRecorderAnaElemTool` or produces its side material ROOT file.
The producer source and standalone test card are removed; the implementation
is available in Git history. Historical tuples remain useful as evidence, but
the current GSF truth oracle no longer reads them. Standalone historical
analysis tools may still inspect those files. The tuples recorded authoritative
Geant4 pre/post-step truth, including true track-step order, sensitive-volume
and touchable identifiers, track-length coordinates, and momentum directions.
Tracker-region DD4hep constants were explicitly converted to millimetres.

For historical BH-model dataset reproduction, the recorder could also write a
default-off `dd4hep_surface_tuple` using the same DD4hep
`MaterialManager::materialsBetween` primitive and `length/radLength` sum as
`DD4hepBetweenSurfaces` in the GSF. It
uses the midpoint of each sensitive traversal as a truth-side proxy anchor,
treats the adjacent TPC lower/upper sensitive half-volumes as one pad row, and
stores the DD4hep material composition together with the clipped Geant4 step
material and eBrem loss inside the same midpoint-to-midpoint bounds. These
anchors are not guaranteed to be the consecutive accepted/matched hit bounds
used by a runtime GSF fit; pair them eventwise before making closure claims. It
also evaluates
the identical endpoints in reverse order and stores
`dd4hep_reverse_path_tX0`, `reverse_segment_count`, `reverse_valid`, and
`dd4hep_reverse_materials`; these are direction-closure diagnostics, not a
second material definition. Both endpoint orders enforce the same complete-
coverage invariant used by the GSF. The tuple exposes `coverage_repaired`,
`reverse_coverage_repaired`, the original `initial_covered_length_mm` values,
and the final `covered_length_mm` values for auditing boundary recovery.
The retired producer required `RecordDD4hepSurfaceIntervals=True`, together
with `RecordZeroLoss=True`, `MinStepLengthMm=0`, and `MinAbsLossGeV=0`, for
that historical tree. These are no longer active run-card properties.

The former `build_g4_transition_dataset.py` helper was removed with the
helper-file workflow. Historical transition CSVs and their derivation records
remain under project history; reproducing that builder requires the historical
revision. `g4_t_over_x0` in those files was a Geant4 diagnostic, while
`reco_t_over_x0` remained empty until rows were matched to owned GSF surface
transitions.

Historical studies compared a G4 transition CSV with the retired GSF material
audit using
`scripts/compare_g4_reco_material_transitions.py`. Select
`--reco-column path_t_over_x0` for the active current-surface calculation or
`--reco-column geometry_path_t_over_x0` for DD4hep volume integration. In
matched event 11, the respective Geant4 and DD4hep totals are 0.0737544 and
0.0739544 X0; the hard-eBrem transition agrees to 0.064%. Select
`--reco-column interval_path_t_over_x0` only for the diagnostic crossed-cradle
sum, which is not authoritative material semantics.

New campaigns should use embedded `GsfG4MaterialSteps` provenance together
with `GSFTruthMaterialIntervals` and the final flat tuple's
`truth_material_*` vectors instead of regenerating either helper output.
