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

The active concentration is preserving hard-eBrem recovery while eliminating
the bias and broadening introduced on tracks without true tracker eBrem. The
step-conditioned model and reverse workflow execute; `ReverseOutputMode` now
defaults to `BestBranch`, with `WeightedMean` retained for comparison.

The matched 1000-event sample contains 381 no-eBrem, 457 light-eBrem, and 162
hard-eBrem events according to primary-electron Geant4 tracker steps and a 10%
single-or-cumulative hard-loss boundary. On no-eBrem events, LCIO has median pT
residual -0.0190% and central-68% width 0.2929%. Weighted reverse output gives
+0.2307% and 0.5786%; best branch improves this to +0.1047% and 0.3446% but
does not restore the LCIO core. On 161 successful hard-eBrem events, LCIO,
weighted reverse, and best reverse have median residuals -10.459%, -0.0310%,
and -0.1678%, with 59, 84, and 88 events inside 1%. Seed 74 entry 4 fails when
all forward components are rejected at hit 4.

This evidence means the algorithm often finds useful loss hypotheses, but the
no-loss hypothesis is not protected well enough. The leading suspected
mechanisms are repeated near-unity rather than exact-unity reverse corrections,
KL merging of the identity lineage, and possible degradation intrinsic to the
second reverse refit's reuse of forward-filtered measurement information.

Proceed in this order:

1. Verify and, if needed, add an exact identity/no-eBrem component with retained
   fraction 1 and near-zero variance. Derive its probability from all Geant4
   transitions, including no-eBrem transitions, with probability approaching
   one as `t/X0 -> 0`.
2. Preserve that identity lineage through cutoff and KL reduction instead of
   merging it with nearby loss components.
3. Run the identical reverse workflow with radiative BH convolution disabled
   on the categorized no-eBrem sample to separate BH-induced degradation from
   reverse-refit degradation.
4. If the no-BH reverse control still broadens the core, replace the current
   measurement-reusing second refit with a statistically consistent
   forward/backward message or smoother formulation.

Success means retaining the demonstrated categorized hard-loss recovery while
matching, rather than biasing or broadening, the LCIO no-eBrem core. Independent
held-out validation and broad energy/angle coverage remain required before any
production-performance claim.

Current non-goals: adding a new measurement-evidence selection threshold,
global tuning before the identity/control diagnosis, fitting SimHit momentum,
treating ACTS coefficients as CEPC validation, premature runtime optimization,
or additional shared-package changes.

The complete outgoing execution focus, categorized evidence, and rationale for
this plan are preserved in
`agents_record/2026-07-12-conditioned-bh-reverse-performance-and-optimization-plan.md`.
