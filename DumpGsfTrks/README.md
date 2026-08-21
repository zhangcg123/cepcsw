# GSF Event Production and Geant4 Material-Step Tuples

This directory contains template cards used by the historical three-stage
single-particle workflow:

```text
particle gun + Geant4 simulation
  -> sim-<particle>-<pT>-<theta>-<seed>.root
  -> gsf_material_steps-<particle>-<pT>-<theta>-<seed>.root
  -> digitization and tracking
  -> trk-<particle>-<pT>-<theta>-<seed>.root (CompleteTracks)
  -> GSF refit
  -> GSF EDM and flat tuples
```

The material-step tuple is produced during simulation, not during tracking or
GSF refitting. The current batch orchestration is stale and must not be used
unchanged for a new production campaign; see the review findings below.

## Authoritative recorder

The implementation is:

```text
Simulation/DetSimAna/src/GsfMaterialStepRecorderAnaElemTool.h
Simulation/DetSimAna/src/GsfMaterialStepRecorderAnaElemTool.cpp
```

It is registered as `GsfMaterialStepRecorderAnaElemTool` and called through
`DetSimAlg.AnaElems`. It writes a ROOT file in `RECREATE` mode containing one
`g4step_tuple` entry per Geant4 event. Step quantities are stored in vectors;
`step_count` is their common length.

For each accepted `G4Step`, the recorder stores:

- run, event, track, parent, primary, and track-step identifiers;
- pre/post/midpoint positions and pre/post momentum vectors;
- `loss = pre_p - post_p`, `dp = post_p - pre_p`, and
  `retained = post_p/pre_p`;
- kinetic-energy change, deposited energy, step length, material radiation
  length, and `step_tX0 = step_length/material_radlen`;
- material, process name/subtype, volume/copy number, sensitive flag, and full
  pre/post touchable path.

This is the authoritative material-loss truth for GSF/BH studies.
`SimTrackerHit::getMomentum()` remains a sensitive-hit cross-check and is not a
pre/post-material-step pair.

### Selection semantics

The production-oriented settings in `sim.py.bk` and the maintained smoke-test
card are:

```python
steprec.PDGs = [11, -11]
steprec.PrimaryOnly = True
steprec.TrackerOnly = True
steprec.MinStepLengthMm = 0.0
steprec.MinAbsLossGeV = 0.0
steprec.RecordZeroLoss = True
```

Important interpretations:

- `PrimaryOnly=True` records the primary electron/positron only. It excludes
  secondary electrons and photons, but the primary's momentum change across
  an eBrem step is retained.
- `TrackerOnly=True` is a geometric cylinder cut using
  `tracker_region_rmax/zmax`. It is not a test that the current volume is a
  tracker subdetector. Material inside those bounds can be recorded.
- A step is retained when either endpoint is inside that cylinder.
- Zero-loss and momentum-gain steps are retained with the current settings.
  Physics analyses must select the desired particle, process, ancestry, and
  volume at the same vector index.
- `step_tX0` uses the Geant4 step length and the pre-step material radiation
  length. Geant4 limits a material-boundary step to its current material, so
  this is the appropriate local convention.
- `event_id` restarts in each job. Use `(production point, seed/file,
  event_id)` as the event identity; never join different files on `event_id`
  alone.

## Maintained smoke test

Use the environment-configurable card below to verify a build before mass
production:

```bash
source setup.sh
GSF_STEPREC_EVTMAX=5 \
GSF_STEPREC_SEED=1 \
GSF_STEPREC_ENERGY_GEV=2.00764 \
GSF_STEPREC_THETA_DEG=85 \
GSF_STEPREC_OUTPUT=/tmp/gsf-material-steps-e2pt-theta85-seed1.root \
GSF_STEPREC_EDM_OUTPUT=/tmp/sim-e2pt-theta85-seed1.root \
build.105.0.0.x86_64-el9-gcc11-opt/run gaudirun.py \
  Reconstruction/RecGsfTracking/options/run_gsf_material_step_recorder_test.py
```

`GtGunTool.EnergyMins/Maxs` are total momentum/energy settings for this
ultrarelativistic single-electron use. For a requested transverse momentum,
the gun magnitude is

```text
p = pT / sin(theta).
```

For `pT=2 GeV` and `theta=85 deg`, this is approximately `2.00764 GeV`.

After the job, require all of the following before scaling up:

1. the Gaudi job terminates successfully;
2. both ROOT files exist and are nonempty;
3. `g4step_tuple` exists and has exactly the requested number of event
   entries;
4. every event has equal vector-branch lengths equal to `step_count`;
5. primary-electron steps exist and their momentum, position, `retained`, and
   `step_tX0` values are finite;
6. output filenames contain the full particle, momentum/angle, and seed
   identity and do not already exist unless replacement is intentional.

The general tuple analyzer is:

```bash
G4MaterialStepComparison/scripts/analyze_g4step_tuple.py \
  --sample electron=/tmp/gsf-material-steps-e2pt-theta85-seed1.root \
  --outdir /tmp/g4step-analysis-e2pt-theta85-seed1
```

## Review of the production templates

Review date: 2026-08-17.

### `sim.py.bk`

The recorder is attached to `DetSimAlg.AnaElems`, uses the full TDR geometry,
writes the EDM simulation file and the independent material-step ROOT file,
and records primary electron or muon steps with no loss/length threshold. The
particle gun intentionally generates a broad sample:

```python
gun.EnergyMins = [10]
gun.EnergyMaxs = [50]
gun.ThetaMins = [40]
gun.ThetaMaxs = [140]
```

These hardcoded ranges are intentional and are not replaced by
`dump_gsftrk.sh`. Its legacy momentum-magnitude argument is unused, while the
transverse-momentum and theta arguments remain filename labels only. The
recorder PDG list is `[11, -11, 13, -13]`.

### `dump_gsftrk.sh`

The worker accepts six legacy-compatible arguments plus an optional seventh
`truth_bh_override` boolean. In its current prepared-input mode, simulation,
tracking, calorimeter digitization, and calorimeter reconstruction execution
are commented; it generates those cards but runs only the final GSF refit:

```bash
./dump_gsftrk.sh particle momentum_mag momentum_trn theta seed nevt \
  [truth_bh_override]
```

The generated GSF card requires and reads
`$WORKDIR/rec-<sample>.root`. With the optional argument omitted or false, the
truth oracle remains off and established output names are unchanged. With it
true, the worker also requires the paired nonempty
`$WORKDIR/gsf_material_steps-<sample>.root`, enables the in-process
`G4StepTuple` oracle, and writes the generated card and GSF/flat/audit outputs
with a `truth-bh` suffix so the ordinary A/B member is not overwritten. The
material tuple must be produced by the same simulation events as the
reconstructed input; the GSF endpoint guard rejects a mismatched tuple.

`subtrkjobs.sh` passes the optional control through. For example,
`TRUTH_BH_OVERRIDE=true bash subtrkjobs.sh` submits the truth-oracle member;
the default remains false. Its truth-mode batch logs also receive a
`_truth-bh` suffix. The current script does not create a missing material
tuple: run the simulation/material-recorder stage first and retain that
separate ROOT file.

The four non-GSF cards retain the official TDR-o1-v01 `PodioInput`, algorithm,
service, and `keep *` configuration; their workflow differences are limited to
batch steering, plus the intentional GSF material-step recorder in the
simulation card. Numerical-library thread counts default to one because an
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

For this historical workflow, `gsf.py.bk` explicitly configures all 46
properties and silently inherits none. The truth-dependent BH-loss oracle is
explicitly off, but the card is batch-ready for the in-process truth reader:
`TruthBHLossSource="G4StepTuple"`, `TruthBHLossInput` points through the
top-level `truthbhinputfilename` variable to the simulation stage's per-job
`gsf_material_steps-<sample>.root`, `TruthBHLossInputTrackIndex=0`, and
`TruthBHLossMaxEndpointDistance=5.0` mm. The source and nonempty path are
intentional differences from the compiled and reverse-template defaults of
`CSV` and empty input; they have no effect while the override remains false.
Use the package README for the complete configuration reference and the
reverse template for the production-baseline settings.

Both runtime material outputs are explicit. `MaterialTransitionCSV=""` keeps
the legacy forward, non-seed comparison table off. For the current campaign,
`MaterialBHAuditCSV` uses an input-sample/method-specific filename under
`tuplepath`, so parallel jobs do not overwrite one another. With
`tuplepath=""`, `dump_gsftrk.sh` runs from `WORKDIR` and the audit lands in the
project root. This nonempty card setting enables the default-off algorithm
diagnostic; it does not change the compiled default. The audit is a generated
CSV, not a branch of `RecGsfFlatTuple`.

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
`gsf_*` fields and adds the paired constrained `ecal_gsf_*` fields,
`ecal_gsf_available`, `ecal_gsf_changed`, and `res_pT_ecal_gsf`. Default-off
jobs remain valid: their ordinary fields are populated and their constrained
fields are marked unavailable and zeroed. The constrained track copies the
ordinary tracker hits, so the flat tuple does not duplicate the `gsf_hit_*`
vectors.

Although it retains the `.bk` name, `gsf.py.bk` is the maintained runnable
comparison card for this historical workflow. When the package adds, removes,
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

The current submission loop requests 100 electron seeds with 100 events each
and now consumes the matching prepared `pT=2 GeV`, `theta=85 deg` tracker file
from `tuples285`. Three sampled prepared files (seeds 1, 100, and 334) each had
100 events and every simulated calorimeter collection required by the official
digitization card. The earlier full-chain workflow passed a separate 10-event
smoke test with electron seed 910817 on 2026-08-17; that test remains evidence
for the cards, not a statement that the currently commented stages execute.

## Production readiness

Before a new campaign, address the remaining production safeguards:

1. use unique, common sample identities for simulation, material-step,
   tracking, GSF EDM, flat tuple, and logs;
2. fail on missing/stale inputs, existing outputs, or tuple-integrity errors;
3. verify exact collection and event pairing between every stage;
4. record a manifest containing the exact commands and effective settings;
5. audit the one-seed outputs before scaling up.

The old tuples and stored study results document previous experiments but do
not prove that the present templates reproduce those samples. Final physics
comparisons require same-code, exactly paired fresh outputs.
