# RecGsfTracking

`RecGsfTracking` refits `CompleteTracks` with a Gaussian-sum electron model and
writes the tracker-only result to `GSFTracks`. A default-off ECAL experiment
can additionally write a paired component-selection result to
`GSFTracksEcalConstrained`. Each component uses the baseline MarlinTrk
`addHit(reference) -> initialise(componentState) -> addAndFit(currentHit)`
update path.  The old alternate KF fitter and initialization experiments have
been removed; historical comparisons remain under `agents_record/`.

## Complete configuration reference

Reference date: 2026-08-22. `RecGsfTracking` exposes 45 Gaudi properties in
`src/GsfAlgorithm.h`. “Compiled” below means constructing the algorithm
without a run card. “Active reverse” means the effective no-environment-
override configuration in `options/run_gsf_reverse_template.py`. The
distinction matters because that template enables `ElossOn` and
`ReverseFiltering`, whose compiled defaults are false.

### Physics and material model

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `ElectronHypothesis` | `true` | `true` | Enable electron-hypothesis BH processing. Set false for forced no-BH particle controls. |
| `BHModel` | `CEPC2GeV85StepConditioned` | same | Select the BH Gaussian-mixture parameterization. Canonical values are `CEPC2GeV85StepConditioned`, `CEPC2GeV85StepConditioned6`, `CEPCRuntimeGenericGrid5Clear`, `CEPCRuntimeCategoryAligned5Clear`, and `ActsAtlas`. Only the first is the active default; all others are default-off research controls. |
| `TruthBHLossOverride` | `false` | `false` | Enable the all-or-nothing, truth-dependent BH-loss oracle described below. Invalid event/track truth falls back to the configured BH model with an explicit status tag. This diagnostic is never production steering. |
| `TruthBHLossSource` | `CSV` | same | Oracle input interpretation: a prejoined strict `CSV`, or a per-event `G4StepTuple` read and matched inside the algorithm. |
| `TruthBHLossInput` | empty | empty | Source path for `TruthBHLossOverride`: the strict runtime-interval CSV or the material recorder ROOT file selected by `TruthBHLossSource`. Empty is valid only while the override is off. |
| `TruthBHLossInputTrackIndex` | `0` | same | With `G4StepTuple`, the zero-based `CompleteTracks` index that receives the unique primary-electron truth oracle. Other input tracks use the configured BH model. It must be nonnegative. It does not affect CSV selection. |
| `TruthBHLossMaxEndpointDistance` | `5.0 mm` | same | With `G4StepTuple`, maximum allowed distance from any accepted runtime hit to its nearest Geant4 sensitive-midpoint anchor. It must be finite and positive. It does not affect CSV input. |
| `BHSplitThreshold` | `1e-4` | same | Minimum component-local outgoing material thickness used to trigger a BH process split. |
| `MSOn` | `true` | `true` | Enable multiple-scattering process noise in the underlying track fit. |
| `ElossOn` | `false` | `true` | Enable the baseline KalTest deterministic energy-loss treatment in addition to BH splitting. |
| `MaterialPathMode` | `DD4hepBetweenSurfaces` | same | Material assignment for both outward and inward propagation. The default integrates the complete DD4hep volume interval between matched measurement endpoints in canonical inner-to-outer order; `CurrentSurface` remains an explicit comparison control. |
| `MaterialIPExtrapolation` | `false` | `false` | Include material effects during final extrapolation to the interaction point. Kept off in the active workflow. |
| `KappaSeedCov` | `1e-7` | same | Forward GSF seed-covariance scale; the small baseline value tightly anchors the start to `CompleteTracks`. |

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
reduction, reverse-filtering, and publication workflow. `TruthBHLossSource`
chooses either the original prejoined `CSV` contract or direct eventwise
matching from the material recorder's `G4StepTuple`. On a selected track,
every otherwise eligible BH call
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

For `TruthBHLossSource="CSV"`, `TruthBHLossInput` is a CSV with this exact
header:

```text
event_index,input_track_index,hit_from_index,hit_to_index,cell_from,cell_to,truth_p_before_GeV,truth_ebrem_loss_GeV
```

Every accepted-hit interval uses `hit_from_index >= 0`,
`hit_to_index = hit_from_index + 1`, and the exact bounding hit cell IDs. This
includes `hit_from_index=0`, `hit_to_index=1`: the GSF seed already contains
filtered hit 0, so its specially executed seed-material call consumes the
ordinary hit-0-to-hit-1 interval rather than an IP-to-hit-0 interval. The
normal forward loop then starts at hit 1 and consumes the later consecutive
intervals. Preprocessing must derive these rows from the authoritative Geant4
pre/post-step record and match them to the exact runtime accepted-hit
intervals; neither `MCParticle` nor SimTrackerHit momentum is a substitute.
CSV presence selects the oracle at whole-track granularity: a particular
`(event_index,input_track_index)` is selected when the input contains at least
one row for that pair.

For `TruthBHLossSource="G4StepTuple"`, `TruthBHLossInput` is the ROOT file
written by `GsfMaterialStepRecorderAnaElemTool` and must contain the required
`g4step_tuple` pre/post-step branches. The algorithm indexes it by `event_id`,
requires exactly one primary electron or positron Geant4 track in each
processed event, reconstructs sensitive-traversal midpoint anchors in track
step order, and treats each TPC lower/upper sensitive half-volume pair as one
pad-row anchor. It nearest-matches every accepted runtime hit to those anchors,
requires a strictly increasing anchor sequence and a maximum endpoint distance
no larger than `TruthBHLossMaxEndpointDistance`, and sums Geant4 process
subtype-3 eBrem loss across all truth intervals spanned by each runtime hit
pair. This source selects only `TruthBHLossInputTrackIndex`; every other input
track uses the configured BH model. A missing event, ambiguous primary,
nonphysical loss, malformed event, nonmonotonic match, excessive endpoint
distance, or incomplete runtime interval coverage invalidates the whole truth
scope before filtering starts. The selected track then completes with the
ordinary configured BH model; it never mixes accepted truth intervals with BH
responses. The fallback is tagged in `GSFTruthBHLossStatus`, the flat tuple,
and every runtime material/BH audit row. The material tuple and reconstructed
input must come from the same simulation events. The reader loads one indexed
event at a time; no external CSV join is needed for batch jobs.

The override requires `ElectronHypothesis=true` and
`MaterialPathMode=DD4hepBetweenSurfaces`. Enabling it with an empty input or a
source that cannot be opened or whose CSV schema cannot be parsed still fails
at initialization. In CSV mode, once a track is selected by any row, missing
coverage of any exact consecutive accepted-hit interval—including hit 0 to hit
1—invalidates the scope before filtering and falls back to the ordinary BH
model for the whole track. It never assumes zero loss. Conversely, a track
with zero CSV rows is outside the oracle scope: it is explicitly logged and
counted and uses the ordinary configured BH model unchanged for its entire
workflow. This all-or-nothing track boundary prevents an undocumented
truth/BH hybrid within one fitted track while allowing a selected event to
contain unrelated secondary tracks for which no authoritative primary truth
exists.

The per-input-track status codes are:

| Status | Meaning |
|---:|---|
| `1` | Complete truth scope validated before filtering; eligible BH calls use the truth response. |
| `2` | Input track was not selected by the configured truth source; ordinary BH used. |
| `0` | Truth override disabled. |
| `-1` | Event-level `G4StepTuple` truth is invalid or unavailable; ordinary BH used. |
| `-2` | Runtime-hit/truth-anchor endpoint guard failed; ordinary BH used. |
| `-3` | Exact consecutive runtime interval mapping failed; ordinary BH used. |
| `-4` | Selected track/event was not processed, for example because of focused `SelectedEventIndices` steering or an unusable input track. |

### Mixture population and reduction

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `MaxComponents` | `12` | `12` | Posterior-reduction trigger/capacity. A BH split is updated before reduction, so this is not an instantaneous ceiling. Keep 24 only as an explicit comparison. |
| `ReductionTargetComponents` | `0` | `0` | Number retained after reduction; zero means use `MaxComponents`. Valid values are zero or `1..MaxComponents`. |
| `ReductionMergeCost` | `SymmetricKL` | same | Pair-ranking cost for moment merging: active `SymmetricKL` or default-off weighted `Runnalls`. Runnalls was tested and rejected for promotion. |
| `ComponentWeightCutoff` | `1e-4` | `1e-4` | Remove normalized target-measurement posterior components below this weight while retaining at least the largest and, when enabled, an identity lineage. |
| `ProtectIdentityLineage` | `true` | `true` | Preserve at least one exact no-radiation lineage through cutoff and reduction when the target component count exceeds one. |

Forward children from transition `i -> i+1` remain expanded through
measurement `i+1`; reverse children from `i+1 -> i` remain expanded through
measurement `i`. The exact innovation likelihood is applied and normalized
before cutoff and reduction. A transition can therefore temporarily require
roughly `MaxComponents * number-of-BH-modes` measurement updates.

### Forward and reverse publication

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `GSFOutputMode` | `BestBranch` | same | Forward/output publication: `BestBranch` or moment-matched `WeightedMean`. Weighted publication is default-off. |
| `ReverseFiltering` | `false` | `true` | Run the independent inward multi-component refit from the complete final forward mixture. This is the active production candidate. |
| `ReverseKappaSeedCov` | `100` | `100` | Multiply every full-mixture reverse-seed covariance by this factor. |
| `ReverseInitialWeightMode` | `ForwardPosterior` | same | Reverse-start weights: active `ForwardPosterior` or default-off `Uniform` diagnostic. |
| `ReverseOutputMode` | `BestBranch` | same | Publish the highest-ranked reverse branch or a moment-matched `WeightedMean`. |
| `ReverseSelectionMode` | `AggregateWeight` | same | Final branch score: active `AggregateWeight`; rejected diagnostics `DominantLineage` and `SurfaceConsistency`. |
| `SurfaceConsistencyUninformativeFloor` | `0.05` | same | Lower bound used only by `SurfaceConsistency`; 0.05 caps its selection Bayes factor at 20. |

`AggregateWeight` selects the component with the largest normalized weight.
`DominantLineage` multiplies that weight by the fraction supplied by its
strongest real pre-merge lineage. `SurfaceConsistency` multiplies it by a
bounded forward/reverse radiative-surface coincidence likelihood. Both
alternatives are retained only to reproduce rejected diagnostics.
`ProtectIdentityLineage` is a reduction safeguard, not another selection mode.

### Experimental ECAL component constraint

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `EcalComponentConstraint` | `false` | `false` | Enable a default-off, two-sided ECAL likelihood that can re-rank the already fitted final reverse components. It requires ordinary reverse filtering, `ReverseOutputMode=BestBranch`, and no CMSSW-like workflow. |
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
`GSFTracks` always preserves the unconstrained tracker-only result. When the
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
| `CmsGsfSmoothing` | `false` | `false` | Run the experimental CMSSW-like backward workflow instead of ordinary reverse filtering. |
| `CmsErrorRescaling` | `100` | `100` | Covariance scaling for the CMSSW-like backward seed; inactive unless `CmsGsfSmoothing=true`. |

`ReverseFiltering`, `GaussianSumSmoothing`, and `CmsGsfSmoothing` are
alternative workflows and must not be enabled simultaneously.

### Focused-event and component diagnostics

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `SelectedEventIndices` | empty | empty | Empty processes the normal event stream; otherwise process only the listed zero-based input entries. |
| `VerboseDump` | `false` | `false` | Print general filtering, track, and workflow diagnostics. |
| `VerboseSplitDump` | `false` | `false` | Dump component populations around BH splits, cutoff, and reduction. |
| `ComponentDebugDump` | `false` | `false` | Dump exact component states, innovation quantities, and lineage histories. |
| `SurfaceLineageMassDump` | `false` | `false` | Propagate and print aggregate BH-mode probability mass by surface. |
| `ComponentDebugMaxHistory` | `240` | `240` | Maximum process/lineage history retained per component for debug output. |
| `MaterialBHAuditCSV` | empty | empty | Structured runtime material/BH audit covering seed, forward, and reverse candidates plus the children returned by every executed split; empty disables it. |

The reverse template connects the first three verbose properties to
`GSF_VERBOSE_COMPONENTS`. Comprehensive focused-event validation normally
enables all three together. `MaterialBHAuditCSV` is algorithm-owned because a
downstream flat-tuple producer cannot reconstruct component-local paths or BH
calls after filtering. Each candidate row records the exact accepted-hit
bounds, component identity/weight and lineage, material mode and composition,
`pathTX0`, split threshold, and whether the BH call executed. Executed calls
add child rows under the same stable `call_id`, including the model's
conditional weight, retained-fraction mean and variance, and the resulting
child weight and momentum. The seed interval is flagged separately. Reverse
rows retain canonical inner-to-outer surface bounds while `direction=reverse`
and `bh_reverse=1` describe the executed workflow.

When `TruthBHLossOverride=true`, the same audit exposes
`truth_bh_loss_override=1` and the exact input `truth_retained_fraction` on
every candidate row of a truth-selected track. Executed child rows then record
conditional weight one, that same retained mean, and the `1e-12` variance
floor. A zero-row, out-of-scope track records
`truth_bh_loss_override=0` and ordinary BH child values throughout. This makes
it possible to verify the all-or-nothing track scope and seed, forward, and
reverse oracle dispatch without inferring them from final track momentum.
Every row also records `truth_bh_scope_status`, `truth_bh_scope_valid`, and a
quoted `truth_bh_scope_reason`. Invalid scopes therefore retain their reason
while their material candidates and ordinary BH children are still audited.

`GSF_MATERIAL_BH_AUDIT_CSV` controls this output in the reverse template.
This CSV is separate from the scalar/per-hit `RecGsfFlatTuple` ROOT tree.
Generated logs and CSV files are outputs, not project-status records. The
superseded forward-only material-transition CSV property was removed after
this audit became authoritative. Historical records may still mention it;
already generated cards that assign it are stale and must be regenerated from
their maintained template before use with the current package.

### Counterfactual loss scan

| Property | Compiled | Active reverse | Meaning |
|---|---|---|---|
| `CounterfactualLossScan` | `false` | `false` | Enable a truth-assisted, likelihood-only trial-loss diagnostic. |
| `CounterfactualTruthTransitionMap` | empty | empty | Comma-separated `event:transition` locations at which to test hypothetical losses. |
| `CounterfactualLossFractions` | `0.04,0.05,0.06,0.07,0.08,0.09,0.10,0.12` | same | Fractional momentum losses assigned to trial branches. |
| `CounterfactualLossVariance` | `2e-4` | same | Retained-momentum-fraction variance assigned to each trial branch. |

For example:

```text
CounterfactualTruthTransitionMap = "1:7,4:8"
```

This tests the configured losses at truth transition 7 for event 1 and
transition 8 for event 4. The scan reports validity, accepted-hit count,
cumulative measurement log-likelihood, and final hypothetical momentum.
Trial branches never enter or reweight the live mixture and cannot become the
published track. This is a truth-assisted mechanism study, not a production
loss estimator. When `CounterfactualLossScan=false`, the other three
properties have no effect.

### Collection handles

The data handles are configurable separately from the 45 properties:

| Role | Default collection |
|---|---|
| input reconstructed tracks | `CompleteTracks` |
| output refitted tracks | `GSFTracks` |
| input ECAL clusters | `EcalCluster` |
| paired ECAL-constrained output tracks | `GSFTracksEcalConstrained` |
| truth particles | `MCParticle` |
| per-input-track truth-oracle status | `GSFTruthBHLossStatus` |

### Flat-tuple paired-track branches

`RecGsfFlatTuple` keeps the existing ordinary tracker-only branches unchanged
and, when `GSFTracksEcalConstrained` is present in the event store, fills a
parallel constrained-track scalar set:

| Branches | Meaning |
|---|---|
| `gsf_pT`, `gsf_p`, `gsf_eta`, `gsf_theta`, `gsf_phi`, `gsf_d0`, `gsf_z0`, `gsf_omega`, `gsf_tanl`, `gsf_chi2`, `gsf_ndf`, `gsf_nhits`, `gsf_type` | Ordinary unconstrained `GSFTracks` result. |
| `ecal_gsf_pT`, `ecal_gsf_p`, `ecal_gsf_eta`, `ecal_gsf_theta`, `ecal_gsf_phi`, `ecal_gsf_d0`, `ecal_gsf_z0`, `ecal_gsf_omega`, `ecal_gsf_tanl`, `ecal_gsf_chi2`, `ecal_gsf_ndf`, `ecal_gsf_nhits`, `ecal_gsf_type` | Paired `GSFTracksEcalConstrained` result. |
| `ecal_gsf_available` | One when a constrained track is present for the tuple row; otherwise zero. |
| `ecal_gsf_changed` | One when the constrained and ordinary AtIP track parameters or fit quality differ; otherwise zero. |
| `res_pT_gsf`, `res_pT_ecal_gsf` | Ordinary and constrained fractional pT residuals relative to the first truth particle. |
| `truth_bh_scope_status`, `truth_bh_scope_valid` | Status code above and a convenience one/zero validity tag for `CompleteTracks` index 0. Older inputs without `GSFTruthBHLossStatus` receive the disabled/invalid defaults `0,0`. |

The constrained branches always exist in newly produced flat files. When the
experiment is off or the paired collection is absent, `ecal_gsf_available=0`
and its scalar/residual fields are zero. The constrained track deliberately
has no duplicate hit-vector branches: the experimental collection copies the
ordinary GSF tracker hits, so `gsf_hit_*` is the common hit information.

The flat tuple contains only the oracle scope status, not the runtime
material/BH audit. Its LCIO and GSF hit vectors reproduce associated output
hits, not the subset that was successfully matched and used internally. Use
`MaterialBHAuditCSV` when the invalidity reason, exact runtime interval, or
component-call information is required.

### Historical `DumpGsfTrks` card compatibility

`DumpGsfTrks/gsf.py.bk` with `method="reverse"` explicitly configures all 45
properties and silently inherits none. Its reverse material, split/cutoff, and
ECAL settings agree with the production baseline: split/cutoff `1e-4`,
`DD4hepBetweenSurfaces`, and ECAL off. Its current explicit `BHModel` is the
user-selected, default-off `CEPCRuntimeGenericGrid5Clear` experiment rather
than the production `CEPC2GeV85StepConditioned` model. For the current
1,000-event material/BH diagnostic, `MaterialBHAuditCSV` is explicitly set to
an input-sample/method-specific filename under `tuplepath`. This campaign
steering does not change the compiled or reverse-template default, which
remains empty/off. The card's `RecGsfFlatTuple` instance still exposes both
ordinary `gsf_*` and default-zero `ecal_gsf_*` scalar branch sets.

The maintained `gsf.py.bk` template keeps the compiled and active
reverse-template `TruthBHLossOverride=false` base value. For the current
1,000-event diagnostic, `subtrkjobs.sh` passes `truth_bh_override=true` and
`dump_gsftrk.sh` rewrites each generated per-job card to true. The template
uses `TruthBHLossSource="G4StepTuple"` and points `TruthBHLossInput` at the
sample-qualified
`gsf_material_steps-<sample>.root` produced by its simulation stage. It keeps
`TruthBHLossInputTrackIndex=0` and
`TruthBHLossMaxEndpointDistance=5.0`. The compiled and active reverse-template
values remain false, `CSV`, empty input, track index 0, and 5.0 mm. This is
deliberate truth-diagnostic campaign steering, not a production-default change.
Generated truth-on cards append `truth-bh`; generated off controls append
`truth-bh-off` to their GSF EDM, flat-tuple, and audit filenames.

### Configuration-maintenance contract

The 45-property inventory above is part of the configurable interface, not a
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

`ActsAtlas` is the ACTS default ATLAS-derived parameterization retained as a
non-CEPC control. New steering should use one of the five canonical spellings
listed in the property table.

## Geant4 transition dataset

`GsfMaterialStepRecorderAnaElemTool` records authoritative Geant4 pre/post-step
truth. Its tuple includes true track-step order, sensitive-volume and touchable
identifiers, track-length coordinates, and momentum directions. Tracker-region
DD4hep constants are explicitly converted to millimetres.

For BH-model dataset production, the recorder can also write a default-off
`dd4hep_surface_tuple` using the same DD4hep `MaterialManager::materialsBetween`
primitive and `length/radLength` sum as `DD4hepBetweenSurfaces` in the GSF. It
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
Enable the tree with:

```python
steprec.RecordDD4hepSurfaceIntervals = True
```

This mode requires the complete raw step stream: `RecordZeroLoss=True`,
`MinStepLengthMm=0`, and `MinAbsLossGeV=0`. Initialization fails instead of
silently constructing incomplete intervals when those requirements are not
met. The original `g4step_tuple` is unchanged, and no additional tree is
created while the option remains off.

Build outgoing-current transition rows with:

```bash
source setup.sh
build.105.0.0.x86_64-el9-gcc11-opt/run python3 \
  Reconstruction/RecGsfTracking/scripts/build_g4_transition_dataset.py \
  'path/to/gsf_material_steps*.root' \
  --output /tmp/gsf_transitions.csv
```

The builder aggregates primary-electron steps from entry into one sensitive
element to entry into the next. It maps the CEPC TPC lower/upper sensitive
half-layer pair to one reconstructed pad-row anchor. `g4_t_over_x0` remains a
Geant4 diagnostic; `reco_t_over_x0` is intentionally empty until the rows are
matched to the owned `RecGsfTracking` surface transitions.

Compare a G4 transition CSV with the GSF material audit using
`scripts/compare_g4_reco_material_transitions.py`. Select
`--reco-column path_t_over_x0` for the active current-surface calculation or
`--reco-column geometry_path_t_over_x0` for DD4hep volume integration. In
matched event 11, the respective Geant4 and DD4hep totals are 0.0737544 and
0.0739544 X0; the hard-eBrem transition agrees to 0.064%. Select
`--reco-column interval_path_t_over_x0` only for the diagnostic crossed-cradle
sum, which is not authoritative material semantics.
