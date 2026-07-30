# CEPCSW GSF Development

## 1. Introduction and global status

This project develops an electron Gaussian Sum Filter (GSF) refit for CEPCSW.
Its goal is to model tracker-material bremsstrahlung and recover the electron
state at the interaction point more accurately than the standard
`CompleteTracks` result.

The intended physics chain is:

```text
Geant4 pre/post-material-step truth
  -> CEPC step-t/X0-conditioned Bethe-Heitler mixture
  -> multi-component filtering and smoothing
  -> validated interaction-point track parameters
```

`RecGsfTracking` builds, installs, reads `CompleteTracks`, and writes
`GSFTracks`. Its component measurement updates use the baseline-compatible
MarlinTrk `initialise -> addAndFit` path, exact accepted innovation quantities,
full Gaussian innovation likelihoods, and exact accepted inter-surface
transport Jacobians. Forward filtering, an independent reverse
multi-component refit, a KL reduction-aware experimental smoother, and a
CMSSW-like experimental backward workflow are mechanically operational.

The active production candidate remains the reverse multi-component refit. It
starts from the complete final forward mixture, scales each full covariance by
`ReverseKappaSeedCov` (default 100), repeats measurement updates inward, and
publishes either the highest-weight branch or an optional moment-matched
mixture. It has demonstrated interaction-point momentum recovery in many
hard-bremsstrahlung events and favorable central light/hard performance, but
it also creates clean-track degradation and extreme tails. The KL smoother is
largely LCIO-like and forfeits much of the hard-loss recovery. The CMSSW-like
workflow has a different core/tail tradeoff and remains default-off.

The active defaults are `MaxComponents=12`, `ComponentWeightCutoff=1e-4`,
`SymmetricKL` reduction ranking, identity-lineage protection enabled, and the five-component
`CEPC2GeV85StepConditioned` Bethe-Heitler model. Preserve 24 components as an
explicit comparison setting. The weighted `Runnalls` ranking and the
six-component `CEPC2GeV85StepConditioned6` model remain default-off controls;
neither is validated or approved as a replacement.

Geant4 pre/post-step data is the authoritative energy-loss truth.
SimTrackerHit momentum is only a detector-level cross-check. Existing Geant4
studies establish a real electron loss tail and bounded fractional-loss
transfer compatibility over the tested 2--10 GeV and theta 85--20 degree
samples. None of the available Bethe-Heitler models is validated for general
CEPC tracker steps.

Population studies show real central recovery, especially for losses at
transitions 5--11, but also new extreme tails. Losses at transitions 0--4 are
predominantly information-limited. Forced-electron-hypothesis muon controls do
not show a universal momentum inflation, but do show clean-core broadening and
outliers. This is a research implementation, not a validated production
algorithm. Broad performance claims and mainline integration require clean
track preservation, reproducible tail control, and independent held-out
validation.

Historical evidence, resolved incidents, exact experiment tables, runbooks,
and superseded decisions live under `agents_record/`. Load historical records
only for regression evidence, design rationale, experiment comparison, or
explicit provenance. Historical detail does not override this live status.

### Project laws and work scope

- Keep implementation changes inside `Reconstruction/RecGsfTracking` unless
  the user explicitly authorizes broader scope for a concrete reason.
- Do not modify KalTest, TrackSystemSvc, MarlinTrk, DDKalTest, or other shared
  CEPCSW packages to compensate for a GSF-specific state-management problem
  unless the user explicitly authorizes a narrow shared interface change.
- Do not hand-code a parallel Kalman measurement update when the baseline
  MarlinTrk interface can provide the required operation.
- Treat Geant4 pre/post-step records as material-energy-loss truth. Do not
  present SimTrackerHit momentum as an exact material transition.
- Do not claim Bethe-Heitler or GSF validation from successful execution,
  finite output, improved chi-square, or selected-sample improvement alone.
  Validation requires interaction-point momentum recovery against generator
  truth in categorized hard-loss events, clean-track safety, and held-out
  population checks.
- Preserve unrelated working-tree changes. Keep source/documentation changes
  separate from generated ROOT files, logs, plots, tables, notebooks, and
  batch cards.
- Use `dev` as the active development branch. Do not switch, create, rename,
  delete, merge, or rebase branches unless the user explicitly requests the
  specific branch operation.
- Use Git frequently during development: inspect status and diffs, and create
  focused checkpoint commits after coherent, proportionately verified core
  implementation or project-knowledge changes. Track, commit, and push all
  C/C++ implementation and header changes across the repository, together
  with documentation, `AGENTS.md`, `.agents/` maintenance content, and durable
  `agents_record/` status/history records. This Git rule does not broaden the
  separate implementation-scope law: edits outside
  `Reconstruction/RecGsfTracking` still require explicit authorization. Also
  track the specifically maintained workflow card `DumpGsfTrks/gsf.py.bk`,
  whose complete property steering is part of the documentation contract.
  Keep other run cards/options, analysis scripts, build files, generated ROOT
  files, logs, plots, tables, notebooks, batch cards, and experiment outputs
  uncommitted unless the user explicitly authorizes a specific exception. Do
  not change branches unless the user explicitly requests it.
- Keep `AGENTS.md` limited to global status, active laws, essential commands,
  and the current focus. Before replacing or removing unique detail, preserve
  it in a dated `agents_record/` entry; replace rather than append focus.
- Whenever a `RecGsfTracking` configurable property is added, removed,
  renamed, or its default/allowed values change, assign a dedicated sub-agent
  to audit the complete option surface. In the same change, synchronize the
  authoritative property reference in
  `Reconstruction/RecGsfTracking/README.md` and the explicit effective
  steering in `DumpGsfTrks/gsf.py.bk`; document intentional historical-card
  differences in `DumpGsfTrks/README.md`.
- Validate every implementation step with comprehensive verbose component
  dumps on a focused event. Once mechanically stable, repeat on hard-loss
  events 11, 16, and 17 before population validation. Build success, finite
  output, or lower chi-square is not a sufficient gate.
- Exclude the stable 133-event secondary-tracker-activity set from
  single-track optimization counts and representative selection, but always
  report it separately as a topology/control population.
- Use same-code direct A/B reruns for final-selection claims; stored outputs
  can drift as the implementation changes.

### Compile and run

Run commands from the repository root. For a complete configured build:

```bash
source setup.sh
./build.sh
```

For the normal focused EL9/LCG 105 development cycle:

```bash
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt \
  --target RecGsfTracking -j4
cmake --install build.105.0.0.x86_64-el9-gcc11-opt
```

Run Gaudi options through that environment:

```bash
source setup.sh
build.105.0.0.x86_64-el9-gcc11-opt/run \
  gaudirun.py path/to/options.py
```

Use a small `SelectedEventIndices` list for component diagnostics. Generated
ROOT files and logs are outputs, not status records.

## 2. Current focus

The active question is why a minority of topology-clean no-eBrem/light-eBrem
tracks select discrete radiative modes and develop positive momentum tails,
especially around surfaces 5--8, despite favorable population-level light and
hard recovery.

The current baseline is the five-component conditioned BH model with
`MaxComponents=12`, aggregate-weight final selection, identity-lineage
protection, `ComponentWeightCutoff=1e-4`, posterior cutoff/KL reduction, and
the independent reverse refit.
Keep 24 components as a comparison capacity where component retention matters.

Current evidence:

- The expanded sample has 4,990 matched events from 499 usable seed files;
  seed 464 is missing its `gsf_tuple`. Before topology exclusion it contains
  2,045 no-eBrem, 2,148 light-eBrem, and 797 hard-eBrem events; the active
  single-track populations are 2,032, 2,132, and 694.
- Recovery is strongly layer-dependent: transitions 0--4 are mostly
  information-limited, 5--6 form the boundary, and 7--11 show strong central
  recovery. Default optimization therefore targets remaining failures at
  transitions 5--11.
- The 19-overshoot/18 matched-control audit and broader transition-7--8 studies
  point to heterogeneous prior/likelihood decisions with a recurring coupled
  surface/mode mismatch. KL reduction is not a recurring local cause.
- Global process-prior reweighting, rank publication, dominant-unmerged
  lineage selection, g3 splitting, bounded noisy-OR surface scoring, a simple
  added 5--8% truth-surface component, the six-component conditioned model,
  and promotion of Runnalls ranking have all failed their current gates.
- Capacity changes can preserve both genuine recovery and false radiative
  modes. The user-selected default is 12; prior evidence favoring 24 on some
  tail samples remains a required comparison, not a conflicting default.

Proceed in this order:

1. Compare the 21 persistent positive-LCIO amplifications, including the 17
   stored above +1 percentage point and nine drifted controls, directly with
   the full 19-overshoot/18-control audit at the selected surface and mode.
   Seek a physically interpretable discriminator before changing the model.
2. Test any candidate first with comprehensive dumps on the overshoot/control
   set, the five ordinary light representatives, clean 62/9, hard 1/3, and
   hard-loss events 11, 16, and 17. Require complete finite tracks with no new
   measurement rejection or covariance failures.
3. Only after that gate, run the full clean/light/hard populations, report the
   secondary-topology population separately, and use forced-BH muons plus
   representative 10 GeV/85-degree and 10 GeV-pT/20-degree electrons as
   transfer and safety controls.
4. Return to seed 74/event 4 and the missing seed-464 tuple only after the
   light-tail mechanism is resolved without sacrificing clean or hard results.

Success means reducing the light-eBrem tail and improving its core without
weakening hard-loss recovery or broadening/biasing the no-eBrem LCIO core.
Independent held-out validation and broad energy/angle coverage remain
mandatory before a production-performance claim.

Current non-goals are an ad hoc measurement-evidence threshold, `WeightedMean`
publication, global covariance tuning, global process-prior rescaling, fitting
SimHit momentum, treating ACTS/CMSSW pedigree as CEPC validation, premature
runtime optimization, reviving rejected selection heuristics, or making
additional shared-package changes.

The complete pre-curation live file and its links are preserved in
`agents_record/2026-07-23-AGENTS-pre-curation-snapshot.md`. The migration map
and curation rationale are in
`agents_record/2026-07-23-AGENTS-curation-map.md`.
