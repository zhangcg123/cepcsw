# GSF event-production workflow

This directory contains the maintained cards for single-particle event
production. The active batch worker can run any selected subset of:

```text
particle gun + Geant4 simulation
  -> sim-<particle>-<pT>-<theta>-<seed>.root
  -> digitization and tracking
  -> trk-<particle>-<pT>-<theta>-<seed>.root (CompleteTracks)
  -> manually configured GSF refit
  -> gsf_flat_<particle>_<method>_<seed>.root
```

The simulation event itself carries the optional truth-diagnostic provenance.
`keep *` preserves it through tracking, so the GSF truth-oracle control needs
no side ROOT tuple or event-number join. The normal GSF remains truth-blind.
The default-on passive material recorder and the optional default-off
`TruthBHLossOverride=true` control read the embedded provenance directly from
the tracker output's EventData. Calorimeter digitization and reconstruction are
not part of the maintained GSF worker at present.

## Embedded Geant4 truth provenance

The implementation spans:

```text
Simulation/GsfTruthEventData/gsftruth.yaml
Simulation/DetSimSD/include/DetSimSD/Geant4Hits.h
Simulation/DetSimAna/src/Edm4hepWriterAnaElemTool.{h,cpp}
```

With `Edm4hepWriterAnaElemTool.WriteGsfTruthEventData=True`, simulation writes
two PODIO collections into the ordinary event:

- `GsfG4MaterialSteps`: selected Geant4 pre/post material steps;
- `GsfSimTrackerHitG4StepLinks`: one relation from a persisted
  `SimTrackerHit` to its exact contributing step range and measurement hook.

The transient detector hit records the first/last contributing Geant4 step
and the hook convention when the hit is made. Combined silicon hits expose a
bounded contributing-step range; the writer resolves their traversal-midpoint
hook only inside that exact range. TPC pad-row hits record their center-crossing
step directly. The GSF then follows

```text
CompleteTracks TrackerHit
  -> MCRecoTrackerAssociation
  -> exact SimTrackerHit
  -> GsfSimTrackerHitG4StepLink
  -> ordered GsfG4MaterialSteps
```

No nearest-hit or global distance search chooses the truth correspondence. The
configured 5 mm distance remains only an integrity guard between the accepted
reconstructed hit and its already-associated exact G4 hook.

For each selected `G4Step`, `GsfG4MaterialSteps` stores:

- Geant4 track, parent, track-local step, PDG, and charge identifiers;
- pre/post positions and momentum vectors, `momentumLoss = pre_p - post_p`,
  and `retainedMomentumFraction = post_p/pre_p`;
- pre/post kinetic energy, total and non-ionizing deposited energy, step
  length, pre-material radiation length, and
  `stepTX0 = stepLength/materialRadiationLength`;
- process subtype, pre/post volume copy numbers, sensitive flags, step
  statuses, accumulated track lengths, and global times.

Run and event identity come from the containing PODIO frame. Material names
and touchable paths remain available only in the legacy side recorder; they
are not duplicated in this compact event-data model.

This is the authoritative material-loss truth for GSF/BH studies.
`SimTrackerHit::getMomentum()` remains a sensitive-hit cross-check and is not a
pre/post-material-step pair.

### Selection semantics

The maintained settings in `sim.py.bk` are:

```python
edm4hep_writer.WriteGsfTruthEventData = True
edm4hep_writer.GsfTruthPDGs = [11, -11, 13, -13]
edm4hep_writer.GsfTruthPrimaryOnly = True
edm4hep_writer.GsfTruthTrackerOnly = True
```

Important interpretations:

- `GsfTruthPrimaryOnly=True` records only primary tracks with a configured PDG.
  It excludes secondary electrons and photons, but the primary's momentum
  change across an eBrem step is retained.
- `GsfTruthTrackerOnly=True` uses the writer's tracker envelope. It is not a
  sensitive-volume test; support and gap material inside that envelope remains
  available between tracker hits.
- A step is retained when either endpoint is inside that cylinder.
- Zero-loss and momentum-gain steps are retained. Physics analyses must select
  the desired particle, process, and ancestry.
- `step_tX0` uses the Geant4 step length and the pre-step material radiation
  length. Geant4 limits a material-boundary step to its current material, so
  this is the appropriate local convention.
- Relations are event-local PODIO object relations, so no `event_id` join is
  involved.

### Legacy side recorder

The side `GsfMaterialStepRecorderAnaElemTool` implementation and its standalone
test card have been removed from the current source tree. Historical
`G4StepTuple` files are no longer accepted by `RecGsfTracking`; their schema,
retired producer, and retired oracle reader are preserved in Git history and
dated project records. Standalone historical analysis tools may still inspect
those files. They contain vector branches for run/event/track identifiers,
pre/post/midpoint positions, momenta and retained fraction, energy deposits,
local `step_tX0`, material and process labels, sensitive flags, and touchable
paths. Their `event_id` restarts per job, so historical joins require the
file/sample identity as well. The legacy analyzer remains
`G4MaterialStepComparison/scripts/analyze_g4step_tuple.py`. This compatibility
path does not justify generating a side tuple for the current workflow.

## Maintained smoke test

Run one complete event before mass production:

```bash
source setup.sh
./dump_gsftrk.sh e- 2.008 2.0 85 1 1 true \
  gsf_kappa_smoke sim_large_20260823 \
  trk,gsf CEPCRuntimeCategoryAligned9Clear
```

The transverse-momentum, theta, and seed arguments identify the existing
`sim-<sample>.root` filename and must match it. The legacy momentum-magnitude
argument remains unused.

After the job, require all of the following before scaling up:

1. the Gaudi job terminates successfully;
2. the tracker output and requested GSF flat output exist and have the
   requested event count;
3. `podio-dump <input_tuplepath>/sim-<sample>.root` lists nonempty
   `GsfG4MaterialSteps` and
   `GsfSimTrackerHitG4StepLinks` for the selected primary;
4. those two collections remain present in the `trk-` input and their derived
   truth/material information is populated in the GSF flat output;
5. every link used by the selected track has complete status, one exact
   SimTrackerHit relation, and consistent first/last/hook step relations;
6. output filenames contain the full particle, momentum/angle, and seed
   identity and do not already exist unless replacement is intentional;
7. the truth-on GSF log reports a valid `EventData` scope and the output
   `GSFTruthBHLossStatus` is `1` for the selected track.

## Review of the production templates

Review date: 2026-08-22.

### `sim.py.bk`

The standard `Edm4hepWriterAnaElemTool` is the only active analysis element.
It uses the full TDR geometry, writes the EDM simulation file, and embeds
primary electron or muon steps and exact SimTrackerHit provenance in that
file. It does not create an independent material-step ROOT file. The particle
gun intentionally generates a broad sample:

```python
gun.EnergyMins = [10]
gun.EnergyMaxs = [50]
gun.ThetaMins = [40]
gun.ThetaMaxs = [140]
```

These hardcoded ranges are intentional and are not replaced by
`dump_gsftrk.sh`. Its legacy momentum-magnitude argument is unused, while the
transverse-momentum and theta arguments remain filename labels only. The
embedded-truth PDG list is `[11, -11, 13, -13]`.

### `dump_gsftrk.sh`

The worker executes the stages selected by the comma-separated `stages`
argument in the fixed physical order `sim -> trk -> gsf`. The GSF stage runs
the one configuration manually maintained in `gsf.py.bk`; there is no
GSF-configuration-list control:

```bash
./dump_gsftrk.sh particle momentum_mag momentum_trn theta seed nevt \
  truth_bh_override output_tuplepath input_tuplepath \
  stages [bh_model]
```

Both tuple paths are required, relative to `WORKDIR`, and must differ.
Every selected stage writes below `output_tuplepath`. When its predecessor is
also selected, a stage reads that newly produced file; otherwise it reads the
required existing predecessor from `input_tuplepath`. Thus `trk,gsf` reads an
existing simulation tuple, while `gsf` reads an existing tracker tuple.
`sim,trk,gsf`, any two-stage subset, and each individual stage are supported.
The maintained GSF card passes its newly produced collections directly to
`RecGsfFlatTuple` in the same event loop and does not schedule `PodioOutput`.
After a successful GSF stage, the worker verifies that the flat tuple is
nonempty. It then removes the tracker tuple when the `trk` stage in that same
job produced it. A `gsf`-only job retains its pre-existing tracker input
because that file may be shared by other jobs or campaigns.
The optional `bh_model` argument defaults to
`CEPCRuntimeCategoryAligned9Clear`; `ActsAtlas` is the only other supported
value.
Select the refitter with `method` and set its method-specific properties
directly in `gsf.py.bk` before submitting a campaign.

`truth_bh_override` is a single boolean. With false, outputs use the explicit
`truth-bh-off` suffix; with true, they use `truth-bh`. Separate submissions
are required for a truth-on/truth-off comparison.

Campaign submission scripts may set `TRUTH_BH_OVERRIDE=true` explicitly and
pass it to every worker. Truth-on and truth-off outputs receive distinct
suffixes.

The simulation and tracking cards retain their official TDR-o1-v01
configuration, including tracking's `PodioInput`, services, and `keep *`.
`sim.py.bk` is invoked only when `sim` is selected. The retained
`calodigi.py.bk` and `rec.py.bk` cards are not invoked by `dump_gsftrk.sh`.
Numerical-library thread
counts default to one because an unconstrained OpenBLAS initialization
previously exhausted the account's `RLIMIT_NPROC=300`; set
`CEPCSW_JOB_THREADS` explicitly to override this. The script still uses exact
textual substitutions and does not prevent ordinary-mode output overwrites,
validate ROOT collection/event integrity, or record a manifest. Generated
cards remain artifacts rather than source configuration.

### `gsf.py.bk`

The current `gsf.py.bk` contains the comparison card previously named
`gsf_reverse_new.py.bk`. It keeps the established smoother and reverse
workflows and agrees with the production material-path, threshold/cutoff, and
ECAL settings:
`DD4hepBetweenSurfaces`, `BHSplitThreshold=1e-4`,
`MaxComponents=10`, `ComponentWeightCutoff=1e-4`, and
`EcalComponentConstraint=False`. The current live beam-boundary reverse
campaign explicitly selects `BeamSpotConstraint=True` and
`ForwardBHSplitting=True, InwardBHSplitting=True`; the compiled and inherited
active reverse-template defaults remain false/false. These gates control BH
child creation only; deterministic energy loss, multiple scattering,
material-path evaluation, and passive material recording remain active. Its
top-level `bh_model` selector explicitly uses the production baseline
`CEPCRuntimeCategoryAligned9Clear`, matching the compiled and active-template
default. `ActsAtlas` is the sole default-off model control. The retained
selector now denotes one effective identity plus nine globally optimized
radiative Gaussians fitted over 0.2--100% loss. Their untruncated shapes are
shared across knots and their weights are knot-local; the selector no longer
denotes the historical fixed-center proposal bank.

The authoritative explanation of all 43 `RecGsfTracking` properties, their
compiled defaults, active reverse-template values, allowed modes, and
diagnostic status is maintained in
`Reconstruction/RecGsfTracking/README.md`.

For this maintained workflow, `gsf.py.bk` explicitly configures 42 of the 43
properties. It deliberately inherits only the compiled
`RecordTruthMaterialIntervals=true` default. Its explicit
`TruthBHLossOverride=false` is the template's off-side base value. A truth-on
submission passes `truth_bh_override=true`, and `dump_gsftrk.sh` rewrites each
generated per-job card to enable the truth-dependent BH-loss oracle. The card
uses the fixed embedded-EventData source with
`TruthBHLossInputTrackIndex=0` and
`TruthBHLossMaxEndpointDistance=5.0` mm. No source selector or helper-file
input property remains. Only generated truth-on cards differ from the false
override base. This remains a diagnostic campaign, not production steering.
The current beam-on card must be submitted with the false override: the live
beam boundary intentionally rejects `TruthBHLossOverride=true` because the
embedded truth intervals have no beam-to-hit-0 truth hook.
Use the package README for the complete configuration reference and the
reverse template for the production-baseline settings.

The card explicitly enables the live beam-boundary experiment with nominal
comparison parameters `BeamSpotX=0.0` mm,
`BeamSpotY=0.0` mm, `BeamSpotSigmaX=0.0145` mm, and
`BeamSpotSigmaY=3.6e-5` mm. It is reverse-only and requires
`GaussianSumSmoothing=False`, `DD4hepBetweenSurfaces`,
`MaterialIPExtrapolation=False`, `TruthBHLossOverride=False`,
`EcalComponentConstraint=False`, and `InwardSeedCovarianceScale<=0`.

In this campaign, the forward three-hit prefit is moved to
the beam pivot and receives the scalar beam update. The canonical DD4hep
beam-to-hit-0 material is optionally BH-split under
`ForwardBHSplitting`, then transported with its complete KalTest Jacobian and
process noise and updated at hit 0 before the normal outward recursion.
Reverse completes its normal recursion through hit 0,
optionally BH-splits the same canonical boundary t/X0 under
`InwardBHSplitting`, propagates to the beam pivot, and applies the scalar beam
likelihood to component states, covariances, and weights before cutoff/KL.
Neither directional gate is silently enabled by the beam option. The final
live bank publishes through the ordinary `GSFTracksBestBranch`,
`GSFTracksWeightedMean`, and `GSFTracksFullMixtureMode` collections and their
existing flat fields. No separate beam collection, `beamspot_*` flat family,
or status bitmask remains; beam-off/beam-on A/B comparisons use distinct jobs
and output tuple paths.

Positive `InwardLookaheadDepth` can reach the beam in this mode. Probe targets
descend through hit 0 and then use the beam as one additional inward boundary;
the passive beam probe shares the live terminal update's hit-to-beam
propagation and scalar likelihood. The ordinary hit-0 update and subsequent
live beam-boundary processing still run. The maintained isolated beam card
keeps depth zero so this feedback remains a separate comparison axis.

The maintained card inherits the compiled and active reverse-template
`RecordTruthMaterialIntervals=true` default. `GsfG4MaterialSteps` and
`GsfSimTrackerHitG4StepLinks` are unconditional members of its base
`PodioInput` collection list, whether or not the truth BH-loss override is
enabled. The property passively writes the exact
Geant4 t/X0 between associated truth hooks, the DD4hep integral between those
same positions, and forward/reverse runtime material summaries to in-memory
EDM collections consumed by the flat-tuple algorithm. Other cards may persist
`GSFTruthMaterialIntervals` and `GSFTruthMaterialRecordStatus` with
`PodioOutput`; the maintained batch card does not. Its durable flat branches
use the `truth_material_` prefix. The recorder never replaces a runtime path
or BH response and cannot change split gating, component weights, or track
selection. Historical inputs without the
embedded collections require an explicit
`RecordTruthMaterialIntervals=false` control card; this affects diagnostic
availability only.
Invalid event or track truth no longer terminates the batch job. The selected
track instead uses ordinary BH throughout and records a negative validity code
in `GSFTruthBHLossStatus`, `truth_bh_scope_status`/
`truth_bh_scope_valid` in the flat tuple. Status `1` is the only valid truth-
oracle scope; the full code table is maintained in the package README.

Side material ROOT generation and the public runtime BH-audit CSV option are
retired from the maintained workflow. Historical tuples, audit CSVs,
standalone analysis tools, and project records remain available for
reproducing earlier studies, but stale cards that assign removed helper-input
properties must be regenerated. The current maintained recording path is the
final flat tuple's `truth_material_*` branches; the corresponding EDM
collections exist only in memory unless a separate card explicitly schedules
`PodioOutput`.

Generated `rungsf-*` cards are batch artifacts and preserve the explicit
material mode in force when each card was created. Many predate the 2026-08-19
default promotion and therefore still say `CurrentSurface`; others record
earlier DD4hep studies. Do not reinterpret or bulk-rewrite those historical
cards as the current default. Regenerate a card from `gsf.py.bk` for a new
production run, and set `CurrentSurface` explicitly only for a comparison.
Generated cards that assign the removed `TruthBHLossSource` or
`TruthBHLossInput` properties, or the removed `ReverseOutputMode` selector,
are incompatible with the current algorithm. They are retained only as
experiment artifacts and must not be run; regenerate them from the maintained
card to use embedded EventData and paired reverse publication.

The maintained card no longer requests `EcalCluster`, because its direct
tracker input does not contain calorimeter reconstruction and the local ECAL
component-constraint experiment remains off. The algorithm still supports that
default-off experiment, but reactivating it requires a separate reconstructed-
event input card that explicitly supplies `EcalCluster`; it is not part of this
worker. There is no endpoint-output selector: ordinary forward always writes
the final component with the largest normalized weight to `GSFTracks`. For
`method="smoother"` or `method="reverse"`, `GSFTracksBestBranch` stores that
same fixed BestBranch, `GSFTracksWeightedMean` stores the moment-matched
mixture, and
`GSFTracksFullMixtureMode` stores the maximum of the complete
five-dimensional final-mixture PDF. The latter is automatic/default-on and
`GSFFullMixtureModeStatus` distinguishes a successful mode (`1`) from a
tagged BestBranch fallback (negative status). The removed
`ReverseOutputMode` and `GSFOutputMode` properties must not appear in
regenerated cards.

The same card's `RecGsfFlatTuple` output records BestBranch in
`bestbranch_gsf_*`, the paired moment match in `weighted_gsf_*`, plus
`bestbranch_gsf_available`,
`weighted_gsf_available`, `weighted_gsf_changed`, and
their residuals, the full-mixture mode in `fullmixture_gsf_*` with
`fullmixture_gsf_available`, `fullmixture_gsf_changed`,
`fullmixture_gsf_status`, and `res_pT_fullmixture_gsf`, and records
`truth_bh_scope_status` and
`truth_bh_scope_valid`, and adds the paired constrained `ecal_gsf_*` fields,
`ecal_gsf_available`, `ecal_gsf_changed`, and `res_pT_ecal_gsf`. Default-off
jobs remain valid: their ordinary fields are populated and their constrained
fields are marked unavailable and zeroed. The constrained track copies the
BestBranch tracker hits, so the flat tuple records those once as
`bestbranch_gsf_hit_*`; forward GSF keeps generic `gsf_hit_*` vectors.
There is no method switch for the weighted flat schema: the branches always
exist, are populated only from an available `GSFTracksWeightedMean`
collection, and remain unavailable/zero for forward jobs.
The FullMixtureMode schema follows the same presence-only rule and is populated
only from `GSFTracksFullMixtureMode`; a negative status marks the deliberate
BestBranch fallback. It has no duplicate hit-vector branches.
The flat tuple also creates `final_mixture_component_*` vectors automatically,
with no run-card property. For every final positive-weight smoother/reverse
component they store the input/output track mapping, component index
and ID, source/validity codes, normalized weight, IP kappa, kappa variance, and
derived pT. The source code is `1` for smoother and `2` for the reverse terminal
mixture: ordinary `B_updated[0]` when beam-boundary mode is off, or the post-beam
cutoff/KL bank when it is on. Historical code `3`
means the retired CMS-like `B_smoothed[1]` endpoint, and historical code `4`
means its terminal-backward fallback.
They contain every published output track in the event and are empty for
forward jobs. These vectors are sufficient to
reconstruct the final one-dimensional pT marginal; the exact transformation
and branch contract are maintained in the package README.
It also creates `lineage_node_*` and `lineage_edge_*` vectors automatically
for smoother and reverse jobs, with no card property. These preserve
every evaluated seed, BH child, measurement result, and KL output, including nodes
later rejected, removed by cutoff, or consumed by a merge. They also preserve
every evaluated interior two-filter smoothing candidate for
`B_smoothed[i] = F_updated[i] x B_predicted[i]`, `0 < i < N-1`. The boundary
states reuse the existing live nodes: `B_smoothed[0] = B_updated[0]` and
`B_smoothed[N-1] = F_updated[N-1]`. Edge records retain split,
measurement, two-parent merge, forward-to-reverse-seed, and two-parent
smoothing ancestry. Forward jobs keep the branches present but empty.
The numeric code maps, graph key, state fields, and reconstruction contract
are maintained in `Reconstruction/RecGsfTracking/README.md`; the workflow
uses `keep *`, so no explicit output-command list is required here.
The BestBranch schema follows the same presence-only rule for
`GSFTracksBestBranch`; it is unavailable/zero for forward. Forward retains its
generic `gsf_*` fields from `GSFTracks`. Existing smoother/reverse ROOT files
are not renamed in place: regenerate them to obtain
`GSFTracksBestBranch` and `bestbranch_gsf_*`.

Although it retains the `.bk` name, `gsf.py.bk` is the maintained runnable
comparison card for this workflow. When the package adds, removes,
renames, or changes a configurable property, a dedicated sub-agent must audit
this workflow in the same change.
`DumpGsfTrks/gsf.py.bk` must explicitly steer the property unless the user has
deliberately designated it as a maintained inherited default, as for
`RecordTruthMaterialIntervals=true`; any such exception or deliberate
difference from the active reverse template must be summarized here. The
authoritative property meanings and full inventory remain in
`Reconstruction/RecGsfTracking/README.md`.

After retiring the configurable endpoint-output selectors and the deprecated
common curvature-seed alias,
`RecGsfTracking` has 37 compiled properties. This card explicitly steers 36
and deliberately inherits only `RecordTruthMaterialIntervals=true`.
Generated cards assigning the removed `CmsGsfSmoothing`, `CounterfactualLossScan`,
`CounterfactualTruthTransitionMap`, `CounterfactualLossFractions`, or
`CounterfactualLossVariance` properties, assigning the removed
`SurfaceConsistencyUninformativeFloor`, `GSFOutputMode`, `KappaSeedCov`,
`ForwardKappaSeedCov`, or `InwardKappaSeedCov` property, or
assigning the removed `ReverseSelectionMode` property with its former
`AggregateWeight`, `DominantLineage`, or `SurfaceConsistency` values are stale
experiment artifacts and must be regenerated rather than edited in place.
Existing counterfactual and reverse-selection tuples remain interpretable
because those retired controls persisted no unique EDM or flat-tuple schema.
Historical ordinary-forward tuples using `GSFOutputMode="WeightedMean"` must
instead be interpreted according to their original card because both endpoint
representations used the generic `GSFTracks` collection.

The separate `RecGsfGlobalLossRefitter` is also retired. Generated cards that
import it, select `method="global-loss"`, or assign
`RecGsfFlatTuple.UseGlobalLossTracks` are stale and must be regenerated. The
maintained card now accepts only `method="smoother"` and `method="reverse"`.
Historical global-loss ROOT outputs and dated records remain available, but
the current flat-tuple producer no longer maps `GlobalLossTracks` into
`gsf_*`.

The maintained template currently selects `method="reverse"`. It now sets
`InwardSeedCovarianceScale=-1.0` for the fresh-inward-seed campaign, while the
compiled default and active reverse template remain 100.
Positive values copy and scale the complete final forward mixture. A finite
value at or below zero instead selects one fresh standard-KF-style backward
seed, with an explicit outermost-hit update before recursion starts at `N-2`.
The maintained value -1 is only a mode selector, not a negative covariance
multiplier. It creates one unit-weight inward root, so the explicitly retained
`ReverseInitialWeightMode="ForwardPosterior"` setting is inert in this card.
The maintained card sets `ForwardSeed=1.0` and `BackwardSeed=1.0`
independently. A value of `1` uses all five FullLDCTracking-style diagonal
seed variances unchanged; any other finite positive value uniformly scales
all five. Old cards assigning `KappaSeedCov`, `ForwardKappaSeedCov`, or
`InwardKappaSeedCov` are stale and must be regenerated. `BackwardSeed`
affects only this fresh-seed mode; it is inert when a positive
`InwardSeedCovarianceScale` selects the copied-mixture seed.
The reverse branch explicitly sets `InwardWeightMode="LocalMeasurement"`,
matching the compiled and active reverse-template control. The retained
`F_updated x B_predicted` products remain passive diagnostics and do not
replace live reverse weights. Experimental `SmoothedMarginal` remains
available as a default-off comparison; it reuses overlapping forward evidence
at successive surfaces and is not a calibrated posterior.

The maintained card sets `InwardLookaheadDepth=0` for the current reverse
beam-boundary campaign, matching the compiled and inherited-template default.
For a positive depth `N`, temporary copies of each newly split reverse child
probe the farther-inward hits `i-1` through `i-N`; separately normalized probe
posteriors are averaged, then combined at equal status with the original BH
prior after both channels receive the ordinary adjacent-hit likelihood. The
live state is updated only once, and probe hits are reused by the later inward
recursion. This is an uncalibrated evidence-reuse diagnostic, not a production
default. The retired `NextMeasurement` and `NextNextMeasurement` strings are
invalid; use numeric depths one and two respectively when reproducing their
reach.

The same comparison card sets `ProtectIdentityLineage=True`, matching the
compiled and active-template default, so the beam-boundary comparison does not
also change the identity-lineage safeguard.

This same reverse branch enables both boundary directions:
`ForwardBHSplitting=True`, `InwardBHSplitting=True`. The two switches control
only BH child creation in the shared outward and independent inward filters.
The inward switch is inert for non-reverse methods; it does not control the
retained-graph smoother. With a positive inward seed scale, turning inward
splitting off does not remove components copied from the final forward
mixture. Passive above-threshold material counts can therefore remain nonzero
when a directional gate is off and must not be interpreted as executed split
counts. The output filename does not encode these switches, so campaigns with
different settings need distinct output tuple paths to prevent overwrites and
mislabeling. The maintained filename now includes `beamspot` or
`beamspot-off`; it still does not encode the two directional split switches.

The fresh-seed choice is explicit campaign steering, not a production-default
change. Its input
path is a
top-level `inputfilename` steering variable, which the worker replaces with
the preceding `trk-<sample>.root` path. Its output names contain particle,
method, and seed, but not momentum or angle; reusing the same particle and
seed at another production point can still overwrite outputs.

### `subtrkjobs.sh`

`subtrkjobs.sh` requires separate `INPUT_TUPLEPATH` and `OUTPUT_TUPLEPATH`
values. Its `STAGES` control accepts a comma-separated subset of `sim`, `trk`,
and `gsf`, defaulting to `trk,gsf`. It passes `BH_MODEL` and
`TRUTH_BH_OVERRIDE` to each worker; the GSF method and its method-specific
settings come directly from the manually maintained `gsf.py.bk`. Set
`TRUTH_BH_OVERRIDE` to either `true` or `false`.
Batch size and seed selection remain ordinary campaign controls. Set
`SEED_FIRST` and `SEED_LAST` to select the inclusive seed range; the worker
receives each selected seed because it is part of the input and output tuple
identity.
Batch stdout and stderr are written to
`<output_tuplepath>/outlog/`, keeping each campaign's logs with its tuples.
Generated per-job simulation, tracking, and GSF Python cards are written to
`<output_tuplepath>/runcards/`. The maintained `.py.bk` templates remain in
`DumpGsfTrks/`; generated cards are campaign artifacts and are not committed.
Campaign-option validation is centralized in `subtrkjobs.sh`; the worker does
not repeat its truth-boolean, tuple-path, or BH-model allow-list checks. It
retains only argument-count and runtime-state checks such as input existence,
overwrite protection, and stage completion.

## Production readiness

Before a new campaign, address the remaining production safeguards:

1. use unique, common sample identities for simulation, tracking, GSF flat
   tuple, and logs;
2. fail on missing/stale inputs, existing outputs, or tuple-integrity errors;
3. verify exact collection and event pairing between every stage;
4. record a manifest containing the exact commands and effective settings;
5. audit the one-seed outputs before scaling up.

The old tuples and stored study results document previous experiments but do
not prove that the present templates reproduce those samples. Final physics
comparisons require same-code, exactly paired fresh outputs.
