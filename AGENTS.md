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

Global status:

- `RecGsfTracking` builds, installs, reads `CompleteTracks`, and writes
  `GSFTracks`.
- The component measurement-update workflow is operational. It uses the
  baseline-compatible MarlinTrk `initialise -> addAndFit` path; focused
  single- and multi-component tests no longer reproduce the former recovery
  and catastrophic smoothing failures.
- True Geant4 pre/post-step data is the authoritative energy-loss truth.
  SimTrackerHit momentum is only a detector-level cross-check.
- The electron loss tail is physically established. At 1 GeV and theta 85
  degrees, the 200-electron/200-muon G4-step comparison measured mean event
  losses of 0.007116 and 0.000898 GeV with comparable material budgets.
- Neither available Bethe-Heitler model is validated for CEPC tracker steps.
- The code has not yet demonstrated recovery of generated interaction-point
  momentum in known hard-bremsstrahlung events. It remains a research
  implementation, not a validated production reconstruction algorithm.
- Broad GSF-versus-LCIO performance claims and mainline integration must wait
  for reproducible hard-loss recovery.

### Project laws and work scope

These constraints are active and mandatory:

- Keep implementation changes inside `Reconstruction/RecGsfTracking` unless
  the user explicitly authorizes a broader scope for a concrete reason.
- Do not modify KalTest, TrackSystemSvc, MarlinTrk, DDKalTest, or other shared
  CEPCSW packages to compensate for a GSF-specific workflow or state-management
  problem.
- Do not hand-code a parallel Kalman measurement update when the baseline
  MarlinTrk interface can provide the required operation.
- Treat Geant4 pre/post-step records as the material-energy-loss truth. Do not
  present SimTrackerHit momentum as an exact material transition.
- Do not claim the Bethe-Heitler model or GSF physics performance is validated
  from successful execution, finite output, or improved chi-square alone.
  Validation requires demonstrated interaction-point momentum recovery against
  generator truth in categorized hard-loss events.
- Preserve unrelated working-tree changes. Stage files explicitly and keep
  source/documentation changes separate from generated ROOT files, logs,
  plots, tables, notebooks, and batch cards.
- Keep `AGENTS.md` current and concise. Before replacing a focus or removing
  detail, preserve every unique item in a dated `agents_record/` entry. Do not
  lose information during status migration.
- Load historical records only when regression evidence, design rationale,
  experiment comparison, or explicit provenance is needed. Historical detail
  must not override the current focus merely because it is more extensive.

### Compile and run

Run commands from the repository root. For a complete configured build and
install:

```bash
source setup.sh
./build.sh
```

For the normal focused GSF development cycle on the configured EL9/LCG 105
build:

```bash
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt \
  --target RecGsfTracking -j4
cmake --install build.105.0.0.x86_64-el9-gcc11-opt
```

Run a Gaudi option file through that build environment:

```bash
source setup.sh
build.105.0.0.x86_64-el9-gcc11-opt/run \
  gaudirun.py path/to/options.py
```

Use a small `SelectedEventIndices` list for component diagnostics. Generated
ROOT files and logs are outputs, not project-status records.

Historical evidence, resolved incidents, prior experiments, runbooks, and the
complete pre-curation project guide are preserved under `agents_record/`.
Load this file first. Load historical records only for regression analysis,
design rationale, experiment comparison, or an explicit provenance request.
When the project focus changes, preserve the outgoing focus in a dated
`agents_record/` entry and replace—not append to—the current-focus section.
Do not lose unique information during that migration.

## 2. Current focus

The current concentration is energy-loss inference and component lifetime, not
the resolved hit-update recovery problem.

The experimental `GlobalSim2GeV85` model provides the same five retained-
momentum hypotheses at every qualifying split and ignores the individual
material step `t/X0`. Its dominant prior is the near-no-loss component
`z=0.99995` with weight 0.5793. With immediate TopN target 1, lower-`z`
components are normally deleted after only one following hit, before enough
outer-hit curvature information exists to identify a hard loss.

The focused evidence is:

| event | truth pT [GeV] | LCIO pT [GeV] | GSF pT [GeV] |
|---:|---:|---:|---:|
| 11 | 2.000 | 1.793 | 1.793 |
| 16 | 2.000 | 1.812 | 1.812 |
| 17 | 2.000 | 1.579 | 1.579 |

GSF improves fit chi-square in these events but does not restore generated
momentum. Lower chi-square alone is not evidence of energy-loss recovery.

Proceed in this order:

1. Use hard-loss events 11, 16, and 17 as the primary validation set.
2. Retain 3-5 hypotheses across several hits after a split; add delayed
   reduction or an equivalent component-age/minimum-hit policy.
3. Track each branch's momentum, weight, chi-square, age, and ancestry until a
   lower-retained-fraction hypothesis becomes favored or is conclusively
   rejected.
4. Expose the true predicted measurement/crossing and residual on the current
   surface. Do not interpret a `TrackState.referencePoint` as a predicted hit.
5. Fit a Bethe-Heitler mixture conditioned on actual per-step `t/X0` using
   primary-electron tracker-volume Geant4 eBrem truth.
6. Represent loss as a distinct pre-material to post-material transition and
   validate backward smoothing to the interaction point.
7. Only then run broad GSF-versus-LCIO performance studies.

Success means that a retained hard-loss branch accumulates measurement support
and produces a finite, full-hit interaction-point state closer to generator
truth than LCIO across the focused events without reintroducing recovery or
catastrophic smoothing.

Current non-goals: reopening resolved recovery/segmentation investigations
without a fresh reproduction, restoring removed KF/initialization experiments,
using immediate TopN target 1 as a recovery benchmark, fitting the final BH
model to SimHit momentum, or modifying shared KalTest/MarlinTrk packages for a
GSF-local design issue.
