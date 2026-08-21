# GSF event-production workflow

This directory contains the maintained cards for the five-stage
single-particle workflow:

```text
particle gun + Geant4 simulation
  -> sim-<particle>-<pT>-<theta>-<seed>.root
  -> digitization and tracking
  -> trk-<particle>-<pT>-<theta>-<seed>.root (CompleteTracks)
  -> calorimeter digitization
  -> calodigi-<particle>-<pT>-<theta>-<seed>.root
  -> calorimeter reconstruction
  -> rec-<particle>-<pT>-<theta>-<seed>.root
  -> GSF refit
  -> GSF EDM and flat tuples
```

The simulation event itself carries the optional truth-diagnostic provenance.
`keep *` preserves it through tracking and calorimeter processing, so the GSF
truth-oracle control needs no side ROOT tuple or event-number join. The normal
GSF remains truth-blind; only the explicit default-off
`TruthBHLossOverride=true`, `TruthBHLossSource="EventData"` control reads it.

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

`Simulation/DetSimAna/src/GsfMaterialStepRecorderAnaElemTool.{h,cpp}` remains
available only for reproducing historical `G4StepTuple` studies. It is not in
`sim.py.bk`, `DetSimAlg.AnaElems`, or the maintained batch dependency chain.
When explicitly configured, it writes `g4step_tuple` in `RECREATE` mode with
vector branches for run/event/track identifiers, pre/post/midpoint positions,
momenta and retained fraction, energy deposits, local `step_tX0`, material and
process labels, sensitive flags, and touchable paths. Its `event_id` restarts
per job, so historical joins require the file/sample identity as well. The
legacy analyzer remains
`G4MaterialStepComparison/scripts/analyze_g4step_tuple.py`. This compatibility
path does not justify generating a side tuple for the current workflow.

## Maintained smoke test

Run one complete event before mass production:

```bash
source setup.sh
./dump_gsftrk.sh e- 20 20 85 910817 1 true
```

The momentum and theta arguments remain sample labels because the maintained
simulation card intentionally uses its hardcoded broad gun ranges; see below.

After the job, require all of the following before scaling up:

1. the Gaudi job terminates successfully;
2. all five stage outputs exist and have the requested event count;
3. `podio-dump sim-<sample>.root` lists nonempty `GsfG4MaterialSteps` and
   `GsfSimTrackerHitG4StepLinks` for the selected primary;
4. those two collections remain present in `trk-`, `calodigi-`, `rec-`, and
   GSF EDM output through `keep *`;
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

The worker accepts six legacy-compatible arguments, an optional seventh
`truth_bh_override` boolean, and an optional eighth `gsf_only` boolean. With
`gsf_only=false`, all five stages execute in order: simulation, tracking,
calorimeter digitization, calorimeter reconstruction, and the final GSF refit:

```bash
./dump_gsftrk.sh particle momentum_mag momentum_trn theta seed nevt \
  [truth_bh_override] [gsf_only]
```

The generated GSF card requires and reads
`$WORKDIR/rec-<sample>.root`. With the optional argument omitted or false, the
truth oracle remains off and the generated card and GSF/flat/audit outputs use
the explicit `truth-bh-off` suffix. With it true, the worker enables the
in-process `EventData` oracle and uses the `truth-bh` suffix. The paired names
prevent either A/B member from overwriting the other. The worker neither
creates nor checks a side material tuple. With `gsf_only=true`, it reuses the
sample-qualified `rec-<sample>.root` and runs only the final GSF step.

Campaign submission scripts may set `TRUTH_BH_OVERRIDE=true` explicitly and
pass it to every worker. Truth-on and truth-off outputs receive distinct
suffixes.

The four non-GSF cards retain the official TDR-o1-v01 `PodioInput`, algorithm,
service, and `keep *` configuration; their workflow differences are limited to
batch steering, plus the intentional embedded GSF truth collections in the
standard simulation writer. Numerical-library thread counts default to one because an
unconstrained OpenBLAS initialization exhausted the account's
`RLIMIT_NPROC=300` during the 2026-08-17 smoke test; set `CEPCSW_JOB_THREADS`
explicitly to override this. The script still uses exact textual substitutions
and does not prevent ordinary-mode output overwrites, validate ROOT
collection/event integrity, or record a manifest. Generated cards remain
artifacts rather than source configuration.

### `gsf.py.bk`

The current `gsf.py.bk` contains the comparison card previously named
`gsf_reverse_new.py.bk`. It keeps the active reverse workflow and agrees with
the production material, split/cutoff, and ECAL settings:
`DD4hepBetweenSurfaces`, split/cutoff `1e-4`, and
`EcalComponentConstraint=False`. Its current explicit `BHModel` is the
default-off `CEPCRuntimeGenericGrid5Clear` experiment, not the production
`CEPC2GeV85StepConditioned` model; preserve that as deliberate campaign
steering until the user changes it.

The authoritative explanation of all 46 `RecGsfTracking` properties, their
compiled defaults, active reverse-template values, allowed modes, and
diagnostic status is maintained in
`Reconstruction/RecGsfTracking/README.md`.

For this maintained workflow, `gsf.py.bk` explicitly configures all 46
properties and silently inherits none. Its explicit
`TruthBHLossOverride=false` is the template's off-side base value. A truth-on
submission passes `truth_bh_override=true`, and `dump_gsftrk.sh` rewrites each
generated per-job card to enable the truth-dependent BH-loss oracle. The card
uses `TruthBHLossSource="EventData"`, an empty `TruthBHLossInput`,
`TruthBHLossInputTrackIndex=0`, and
`TruthBHLossMaxEndpointDistance=5.0` mm. `EventData` is an intentional
maintained-card difference from the compiled and reverse-template `CSV`
default; only generated truth-on cards differ from the false override base.
This remains a diagnostic campaign, not production steering.
Use the package README for the complete configuration reference and the
reverse template for the production-baseline settings.

The maintained card explicitly sets `RecordTruthMaterialIntervals=true`, in
agreement with the compiled and active reverse-template default. It therefore
requests `GsfG4MaterialSteps` and `GsfSimTrackerHitG4StepLinks` whether or not
the truth BH-loss override is enabled. The property passively writes the exact
Geant4 t/X0 between associated truth hooks, the DD4hep integral between those
same positions, and forward/reverse runtime material summaries to the final
EDM and flat tuple. The EDM outputs are `GSFTruthMaterialIntervals` and
`GSFTruthMaterialRecordStatus`; the flat branches use the `truth_material_`
prefix. It never replaces a runtime path or BH response and cannot change split
gating, component weights, or track selection. Historical inputs without the
embedded collections require an explicit
`RecordTruthMaterialIntervals=false` control card; this affects diagnostic
availability only.
Invalid event or track truth no longer terminates the batch job. The selected
track instead uses ordinary BH throughout and records a negative validity code
in `GSFTruthBHLossStatus`, `truth_bh_scope_status`/
`truth_bh_scope_valid` in the flat tuple, and the comprehensive audit. Status
`1` is the only valid truth-oracle scope; the full code table is maintained in
the package README.

For the current 1,000-event material/BH diagnostic, `MaterialBHAuditCSV` uses
an input-sample/method-specific filename under `tuplepath`, so parallel jobs
do not overwrite one another. With `tuplepath=""`, each audit is written in
the project root. This campaign setting enables the comprehensive recorder;
the compiled and reverse-template defaults remain empty/off. The audit is a
generated CSV, not a branch of `RecGsfFlatTuple`. The superseded forward-only
material-transition CSV property remains removed. Historical records and
already generated `rungsf-*` cards may still mention it; regenerate cards from
`gsf.py.bk` before use with the current package.

Generated `rungsf-*` cards are batch artifacts and preserve the explicit
material mode in force when each card was created. Many predate the 2026-08-19
default promotion and therefore still say `CurrentSurface`; others record
earlier DD4hep studies. Do not reinterpret or bulk-rewrite those historical
cards as the current default. Regenerate a card from `gsf.py.bk` for a new
production run, and set `CurrentSurface` explicitly only for a comparison.

The card still reads `EcalCluster` from the reconstructed-event input but keeps
the local ECAL component-constraint experiment off. When explicitly enabled,
its
cluster-energy observation accepts only clusters inside both the configured
phi and theta windows around the extrapolated outer GSF direction. For the
reverse `BestBranch` workflow, `GSFTracks` remains the tracker-only baseline
and the paired result is written as
`GSFTracksEcalConstrained`; the card's `keep *` output rule retains both. This
is an experimental comparison path, not a change to the active baseline.

The same card's `RecGsfFlatTuple` output retains the ordinary tracker-only
`gsf_*` fields, records `truth_bh_scope_status` and
`truth_bh_scope_valid`, and adds the paired constrained `ecal_gsf_*` fields,
`ecal_gsf_available`, `ecal_gsf_changed`, and `res_pT_ecal_gsf`. Default-off
jobs remain valid: their ordinary fields are populated and their constrained
fields are marked unavailable and zeroed. The constrained track copies the
ordinary tracker hits, so the flat tuple does not duplicate the `gsf_hit_*`
vectors.

Although it retains the `.bk` name, `gsf.py.bk` is the maintained runnable
comparison card for this workflow. When the package adds, removes,
renames, or changes a configurable property, a dedicated sub-agent must audit
this workflow in the same change.
`DumpGsfTrks/gsf.py.bk` must explicitly steer the property, and any deliberate
difference from the active reverse template must be summarized here. The
authoritative property meanings and full inventory remain in
`Reconstruction/RecGsfTracking/README.md`.

The maintained template defaults to `method="reverse"`. Its input path is a
top-level `inputfilename` steering variable, which the worker replaces with
the preceding `rec-<sample>.root` path. Its output names contain particle,
method, and seed, but not momentum or angle; reusing the same particle and
seed at another production point can still overwrite outputs.

### `subtrkjobs.sh`

The current loop requests six electron jobs, seeds 5 through 10, with 100
events each, `TRUTH_BH_OVERRIDE=true`, and `GSF_ONLY=false`. It therefore runs
the full five-stage chain and requests 6 GB per batch job. The comment in the
script that calls this “ten jobs” is stale; the brace range is authoritative.
Its `pT=2 GeV`, `theta=85 deg` values label the sample but do not override the
maintained simulation card's broad gun ranges.

## Production readiness

Before a new campaign, address the remaining production safeguards:

1. use unique, common sample identities for simulation, tracking, GSF EDM,
   flat tuple, audit CSV, and logs;
2. fail on missing/stale inputs, existing outputs, or tuple-integrity errors;
3. verify exact collection and event pairing between every stage;
4. record a manifest containing the exact commands and effective settings;
5. audit the one-seed outputs before scaling up.

The old tuples and stored study results document previous experiments but do
not prove that the present templates reproduce those samples. Final physics
comparisons require same-code, exactly paired fresh outputs.
