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

## Review of the historical production templates

Review date: 2026-07-23.

### `sim.py.bk`

The recorder is correctly attached to `DetSimAlg.AnaElems`, uses the full TDR
geometry, writes the EDM simulation file and the independent material-step
ROOT file, and records primary electron steps with no loss/length threshold.

However, the particle gun currently contains:

```python
gun.EnergyMins = [10]
gun.EnergyMaxs = [50]
gun.ThetaMins = [40]
gun.ThetaMaxs = [140]
```

The template variables `momenta_low` and `theta_low` are not used by these
assignments. Consequently, the substitutions performed by `dump_gsftrk.sh`
do not configure the requested monoenergetic momentum or angle. A nominal
2 GeV, 85-degree job generated from this template would actually use uniform
10--50 GeV and 40--140 degree gun ranges.

The template also fixes `PDGs=[11,-11]`. A muon job will therefore produce an
empty material-step tuple even if the gun particle is changed to `mu-`.

### `dump_gsftrk.sh`

The script generates simulation, tracking, and GSF cards, but execution of
the first two stages is commented out:

```bash
#./run.sh ...runsim...
#./run.sh ...runtrk...
./run.sh ...rungsf...
```

It therefore does not currently generate new events or material-step tuples;
it only attempts the GSF refit using a pre-existing `trk-*.root` input.

The script performs unvalidated textual substitutions and does not stop on
errors (`set -euo pipefail` is absent), check input/output existence, prevent
overwrites, validate the generated Python, or record a manifest. Generated
cards are artifacts and should not be treated as source configuration.

### `gsf.py.bk` and the stale GSF-template reference

The current `gsf.py.bk` contains the comparison card previously named
`gsf_reverse_new.py.bk`. Its main reverse-filter settings agree with the
active baseline, including `ComponentWeightCutoff=1e-4`. It is an exact
active-baseline card for the GSF algorithm properties when `method="reverse"`.

The authoritative explanation of all 34 `RecGsfTracking` properties, their
compiled defaults, active reverse-template values, allowed modes, and
diagnostic status is maintained in
`Reconstruction/RecGsfTracking/README.md`.

For this historical workflow, `gsf.py.bk` explicitly configures all 34
properties, silently inherits none, and agrees with the active reverse
template. Use the package README for the complete configuration reference and
the reverse template for current runs.

Although it retains the `.bk` name, `gsf.py.bk` is the maintained runnable
comparison card for this historical workflow. When the package adds, removes,
renames, or changes a configurable property, a dedicated sub-agent must audit
this workflow in the same change.
`DumpGsfTrks/gsf.py.bk` must explicitly steer the property, and any deliberate
difference from the active reverse template must be summarized here. The
authoritative property meanings and full inventory remain in
`Reconstruction/RecGsfTracking/README.md`.

Its output names contain only `method` and `seed`, not particle, momentum, or
angle. Reusing a seed at another production point can overwrite or mix
outputs. The card is also configured by editing source rather than through a
validated parameter interface.

`dump_gsftrk.sh` still tries to copy `gsf_reverse_new.py.bk`, which is no
longer present. The GSF stage therefore fails before launch in the current
directory state.

### `subtrkjobs.sh`

The current submission loop requests 500 electron seeds, 10 events each, at
nominal `pT=2 GeV`, `theta=85 deg`. Because of the hard-coded gun ranges and
disabled simulation/tracking commands described above, submitting it now
would not create that intended sample.

## Production readiness

Do not submit `subtrkjobs.sh` or run `dump_gsftrk.sh` unchanged. Before a new
campaign, the runner should be revised to:

1. pass explicit particle, momentum magnitude, theta, seed, event count, and
   output directory to environment-configurable cards;
2. enable simulation and tracking deliberately, followed by the current
   reverse-GSF template;
3. use unique, common sample identities for simulation, material-step,
   tracking, GSF EDM, flat tuple, and logs;
4. fail on missing/stale inputs, existing outputs, nonzero stage status, or
   tuple-integrity failures;
5. record a manifest containing the exact commands and effective settings;
6. smoke-test one seed and audit exact event pairing before batch submission.

The old tuples and stored study results document previous experiments but do
not prove that the present templates reproduce those samples. Final physics
comparisons require same-code, exactly paired fresh outputs.
