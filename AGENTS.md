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
- The exact MarlinTrk prediction and innovation quantities are now exposed to
  `RecGsfTracking`, and component posterior weights use the full Gaussian
  innovation likelihood including `det(S)^(-1/2)` with stable log-space
  normalization. Focused events 11, 16, and 17 run successfully with these
  diagnostics, but this statistical correction has not recovered truth
  momentum.
- The forward surface workflow now preserves filtered measurement history in
  each branch and applies process convolution only to a separate continuation
  state. Low-weight cutoff and KL reduction operate on common-surface states;
  focused events 11, 16, and 17 retain all 234 hits with finite KL distances
  and no measurement-update rejection.
- An opt-in reverse multi-component filtering workflow now traverses the
  audited focused hit sequence inward, reuses the exact posterior likelihood,
  applies direction-reversed process convolution and current-surface
  reduction, and publishes a consistent reverse best-branch IP state. Events
  11, 16, and 17 retain 234/234 hits with zero reverse rejection and IP pT of
  1.9785, 1.9970, and 2.2591 GeV, each closer to truth than LCIO.
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
- Validate every GSF implementation step with comprehensive verbose component
  dumps on a focused event before proceeding. Once stable, repeat the check on
  hard-loss events 11, 16, and 17. Build success, finite output, or lower
  chi-square is not a sufficient gate.
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

The focused tracking-workflow roadmap is complete. The current concentration
returns to component-local CEPC material semantics and physics modelling:
incidence-path-corrected `t/X0`, then a step-conditioned CEPC BH mixture. The
resolved hit update, likelihood, forward continuation state, focused navigation
audit, and reverse filtering are not the active blockers.

With `ReverseFiltering=True`, events 11, 16, and 17 produce IP pT values of
1.9785, 1.9970, and 2.2591 GeV against truth 2.0004 GeV, compared with LCIO
1.7934, 1.8118, and 1.5790 GeV. Event 17 overshoots, but the user has explicitly
deferred that issue. This is focused workflow evidence, not broad validation of
the current nominal material estimate, BH model, or arbitrary-track navigation.

Proceed in this order:

1. Use event 11 for step-level verbose development and events 11, 16, and 17 as
   the primary validation set.
2. Compute component-local, incidence-path-corrected `t/X0` with explicit
   pre/post surface ownership and no forward/reverse double counting.
3. Validate the corrected material transition in two event-11 steps and then
   in complete verbose runs of events 11, 16, and 17.
4. Fit a
   Bethe-Heitler mixture conditioned on actual per-step `t/X0` using
   primary-electron tracker-volume Geant4 eBrem truth.
5. Retain 4-5 hypotheses with current-surface KL reduction and a low-weight
   cutoff; then determine whether the provisional component-age policy is
   still needed.
6. Extend navigation validation beyond monotonic focused tracks before claiming
   general tracking support.
7. Only then run broad GSF-versus-LCIO performance studies.

Success means that a retained hard-loss branch accumulates measurement support
and produces a finite, full-hit interaction-point state closer to generator
truth than LCIO across the focused events without reintroducing recovery or
catastrophic smoothing.

Current non-goals: treating delayed TopN as a validated final policy, reopening
resolved recovery/segmentation investigations without a fresh reproduction,
restoring removed KF/initialization experiments, using immediate TopN target 1
as a recovery benchmark, fitting the final BH model to SimHit momentum,
assuming ACTS's ATLAS BH coefficients validate CEPC physics, or modifying
shared KalTest/MarlinTrk packages for a GSF-local design issue.
