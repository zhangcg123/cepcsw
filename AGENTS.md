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
- The exact accepted inter-surface transport Jacobian is now also exposed
  through the user-authorized `MeasurementUpdate` extension. An opt-in
  retained-lineage RTS smoother builds and runs on every retained focused
  branch, but its first 11/16/17 validation stays essentially at LCIO momentum
  and is not yet statistically validated.
- The forward surface workflow now preserves filtered measurement history in
  each branch and applies process convolution only to a separate continuation
  state. Low-weight cutoff and KL reduction operate on common-surface states;
  focused events 11, 16, and 17 retain all 234 hits with finite KL distances
  and no measurement-update rejection.
- An opt-in reverse multi-component filtering workflow now traverses the
  audited focused hit sequence inward, reuses the exact posterior likelihood,
  applies direction-reversed process convolution and current-surface
  reduction, and can publish either the default highest-weight reverse branch
  or a moment-matched reverse mixture at the IP. Events
  11, 16, and 17 retain 234/234 hits with zero reverse rejection and IP pT of
  1.9785, 1.9970, and 2.2591 GeV, each closer to truth than LCIO.
  This workflow is a second reverse refit, not a Gaussian-sum smoother, and its
  starting state may reuse forward measurement information.
- A five-event 2 GeV muon control shows that reverse filtering without the
  electron hypothesis leaves pT essentially unchanged from LCIO. Forcing the
  same electron BH and split/reduce strategy onto the muons also does not raise
  every pT. The workflow is therefore not a universal momentum inflator.
- True Geant4 pre/post-step data is the authoritative energy-loss truth.
  SimTrackerHit momentum is only a detector-level cross-check.
- The electron loss tail is physically established. At 1 GeV and theta 85
  degrees, the 200-electron/200-muon G4-step comparison measured mean event
  losses of 0.007116 and 0.000898 GeV with comparable material budgets.
- None of the available Bethe-Heitler models is validated for CEPC tracker
  steps.
- The selectable BH implementations are now only `ActsAtlas` and
  `CEPC2GeV85StepConditioned`; the former `Current` and `GlobalSim2GeV85`
  implementations and aliases have been removed.
- A third `ActsAtlas` reference option now faithfully implements ACTS's default
  ATLAS thresholds, analytic thin-step Gaussian, and existing six-component
  polynomial tables. It is a comparison model, not CEPC validation.
- Categorized exact-pair tests now demonstrate interaction-point momentum
  recovery for many hard-bremsstrahlung events, but a substantial unrecovered
  tail and clean-track degradation remain. This is still a research
  implementation, not a validated production reconstruction algorithm.
- Broad GSF-versus-LCIO performance claims and mainline integration must wait
  for clean-track preservation, reproducible tail control, and independent
  held-out validation.

### Project laws and work scope

These constraints are active and mandatory:

- Keep implementation changes inside `Reconstruction/RecGsfTracking` unless
  the user explicitly authorizes a broader scope for a concrete reason.
- Do not modify KalTest, TrackSystemSvc, MarlinTrk, DDKalTest, or other shared
  CEPCSW packages to compensate for a GSF-specific workflow or state-management
  problem unless the user explicitly authorizes a narrow shared change for a
  demonstrated interface requirement.
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
- Work only on the `optimizing` branch. Do not switch, create, rename, or
  delete local or remote branches unless the user explicitly revokes or
  replaces this restriction.
- Do not perform Git operations while carrying out the optimization work
  unless the user explicitly requests a specific Git action. Maintain
  `AGENTS.md` and dated `agents_record/` records so progress and rationale
  survive long autonomous runs without relying on commits.
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

The active concentration is preserving the corrected hard-eBrem recovery while
eliminating radiative-cluster over-selection on tracks with no true owned
eBrem. The conditioned artifact now fits eBrem-attributed loss only, avoiding
double counting with deterministic `ElossOn=True`. It contains an exact
`z=1` no-eBrem atom whose lineage is protected through cutoff and KL reduction.

Reconstruction-aligned Geant4 surface ownership classifies the matched
1000-event sample as 407 no-eBrem, 437 light-eBrem, and 156 hard-eBrem events.
On the 407 clean events, LCIO and reverse no-BH have central-68 widths of
0.278607% and 0.275195%; therefore the second reverse refit does not itself
degrade the core. Enabling BH changes only 40 events by more than 0.1%, but 15
by more than 1%, broadening the width to 0.321295% while leaving the median near
zero. On 155 successful hard events, LCIO versus eBrem-only reverse BestBranch
has median residual -12.8099% versus -0.2915%, with 48 versus 74 events inside
1%. Seed 74 entry 4 remains the known failure.

The worst clean outlier, seed 23 entry 8, proves that identity preservation is
working: its identity reverse state has pT 1.99839 GeV and weight 0.291 for
2.00036 GeV truth. A localized inner-hit fluctuation instead favors flexible
radiative hypotheses; KL aggregation produces a radiative component of weight
0.377, which post-reduction `BestBranch` publishes at 2.30485 GeV. Thus the
remaining blocker is the semantics of choosing the heaviest KL-merged cluster,
not a missing identity component or generic reverse-refit bias.

Proceed in this order:

1. Quantify, for the 15 clean events changed by more than 1%, how unmerged
   lineage weights become KL-cluster weights and where the selected cluster
   first overtakes the identity.
2. Test reduction and BestBranch semantics that retain physical-lineage meaning
   without adding a new measurement-evidence gate. Validate any implementation
   change first with a complete verbose seed-23/event-8 dump.
3. Repeat complete verbose checks on hard-loss events 11, 16, and 17, then run
   the 407 clean and 156 hard surface-owned categories. Require finite complete
   tracks without new rejection or covariance failures.
4. Address the known seed-74/event-4 rejection separately only after the
   selection semantics preserve both the clean core and hard recovery.

Success means retaining the demonstrated categorized hard-loss recovery while
matching, rather than biasing or broadening, the LCIO no-eBrem core. Independent
held-out validation and broad energy/angle coverage remain required before any
production-performance claim.

Current non-goals: adding a new measurement-evidence selection threshold,
rewriting the reverse refit after its no-BH control passed, fitting SimHit
momentum, treating ACTS coefficients as CEPC validation, premature runtime
optimization, or additional shared-package changes.

The exact identity construction, corrected category provenance, controls,
eventwise diagnosis, and hard-category evidence are preserved in
`agents_record/2026-07-12-ebrem-only-identity-and-clean-control.md`.
