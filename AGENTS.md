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
  reduction, and publishes a consistent reverse best-branch IP state. Events
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
- A third `ActsAtlas` reference option now faithfully implements ACTS's default
  ATLAS thresholds, analytic thin-step Gaussian, and existing six-component
  polynomial tables. It is a comparison model, not CEPC validation.
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

The active concentration is now the retained-lineage Gaussian-sum smoother.
The prior material-semantics and step-conditioned CEPC BH roadmap is preserved
in `agents_record/2026-07-11-retained-lineage-smoother-start.md` and is deferred,
not cancelled.

The user authorized a narrow TrackSystemSvc interface extension after the audit
showed that exact inter-surface transport Jacobians were unavailable to
`RecGsfTracking`. `MeasurementUpdate` now exposes the existing KalTest
propagator. The new opt-in `RetainedLineageSmoothing` path composes it with the
selected BH process Jacobian and runs an RTS recursion independently on each
retained forward history. It requires `TopN`, disables the experimental reverse
refit, and currently supports geometric IP extrapolation only.

Focused verbose validation is mechanically successful but physically
negative. Events 11, 16, and 17 retain 234/234 hits and smooth 7/7, 7/7, and
12/12 branches, yet give IP pT 1.7933, 1.8118, and 1.5789 GeV versus truth
2.0004 GeV and LCIO 1.7934, 1.8118, and 1.5790 GeV. The inner curvature and
variance change only minimally. Do not interpret this as hard-loss recovery or
as validation of the smoother.

The transition audit now localizes the negligible correction. Exact MarlinTrk
covariance closure is typically at the `1e-6` level and no worse than about
`1e-4` in the checked first transitions. Instead, the fixed global BH model
adds curvature variance of about `7.6e-6` even for its near-no-loss component
and about `4.4e-3` for its `z=0.975` component, versus an incoming variance near
`1.0e-7`. This reduces the first backward curvature gain to about `0.013` or
`2.3e-5`. The no-BH one-component control has gain near one and agrees with
LCIO pT within 0.0002 GeV. Three-hit delayed TopN retains more histories but
does not recover momentum.

The new faithful `ActsAtlas` reference also does not recover the focused
events. All current nominal transitions are below `0.002 X0`, so ACTS selects
one analytic Gaussian rather than a mixture. Events 11, 16, and 17 remain at
1.7934, 1.8118, and 1.5790 GeV. Event 11's first thin Gaussian adds about
`7.39e-5` curvature variance and reduces its first backward curvature gain to
`1.35e-3`. This is useful reference behavior but not evidence that ATLAS
coefficients describe CEPC material losses.

Component-local incidence correction is now implemented with explicit
outgoing-current-surface ownership. For each component,
`pathTX0 = normalTX0/abs(tangent·normal)` using the filtered helix tangent and
DD4hep surface normal at its pivot; the final forward surface has no outgoing
transition. Event-11's first two values become `0.000573798` and `0.000566019
X0`, and the largest focused corrected paths for events 11, 16, and 17 are
`0.00163546`, `0.00163468`, and `0.00164820 X0`. They remain below the ACTS
six-component threshold, so focused momenta are unchanged.

Proceed in this order:

1. Use event 11 for step-level verbose development and events 11, 16, and 17 as
   the primary validation set.
2. Validate the one-component (`ElectronHypothesis=False`) covariance pulls on
   the running large muon sample; the focused central-value check already
   agrees with LCIO.
3. Fit a per-step-`t/X0` CEPC BH mixture with narrow conditional components
   using primary-electron Geant4 eBrem truth; incidence-corrected ownership is
   now implemented.
4. Verify that the fitted conditional variances preserve useful backward RTS
   curvature gains across the first hard-loss transition.
5. Repeat comprehensive events 11, 16, and 17 before any broad performance
   claim or pruning-policy decision.

Success for the present stage means exact transition-consistency checks, a
one-component result consistent with standard smoothing, calibrated finite
covariances, and then a retained hard-loss lineage whose smoothed full-hit IP
state is closer to truth than LCIO across the focused events.

Current non-goals: treating the reverse refit as a Gaussian-sum smoother,
treating the first finite RTS output as validation, treating delayed TopN as a
validated final policy, reopening
resolved recovery/segmentation investigations without a fresh reproduction,
restoring removed KF/initialization experiments, using immediate TopN target 1
as a recovery benchmark, fitting the final BH model to SimHit momentum,
assuming ACTS's ATLAS BH coefficients validate CEPC physics, or making further
shared-package changes without a demonstrated interface need and explicit user
authorization.
