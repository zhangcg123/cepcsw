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

The active concentration is the transition-matched Geant4 dataset and a CEPC
thin-step, `t/X0`-conditioned Bethe-Heitler mixture. The completed smoother,
ACTS reference, transition-gain, pruning, and incidence-corrected-material
evidence is preserved in
`agents_record/2026-07-11-retained-lineage-smoother-completion.md`; load it only
when detailed provenance or regression comparison is needed.

The immediate blocker is not another tracking-workflow change. Existing models
either ignore `t/X0`, collapse the actual thin CEPC range into one Gaussian, or
put so much full-distribution width inside each component that the backward
curvature correlation vanishes. The fitted model must instead describe the
actual owned reconstruction transition between measurement surfaces.

Proceed in this order:

1. Use primary-electron tracker-volume Geant4 pre/post-step eBrem truth; never
   fit the final model to SimTrackerHit momentum.
2. Aggregate every Geant4 loss occurring between the same owned reconstruction
   surfaces into one transition with `z=p_post/p_pre`, and pair it with the
   incidence-corrected reconstruction `t/X0`.
3. Cover the observed thin range, currently about `1e-4` to `1.7e-3 X0`, with
   explicit no/negligible-, small-, moderate-, large-, and extreme-loss
   hypotheses. Use `-log(z)` or an equivalent stable positive variable.
4. Fit smooth constrained functions `weight_j(t/X0)`, `mean_j(t/X0)`, and
   `variance_j(t/X0)`. Tail weights must vanish appropriately as `t/X0 -> 0`;
   each component variance must represent only its conditional width, not the
   full loss distribution.
5. Validate held-out means, variances, quantiles, and probabilities such as
   `P(z<0.95)`, `P(z<0.8)`, and `P(z<0.5)` in separate `t/X0` bins before
   integrating `BHModel="CEPCStepConditioned"`.
6. On two event-11 transitions, require a finite hard-loss child and useful
   backward curvature gain. Then run complete verbose events 11, 16, and 17,
   retaining 4-5 hypotheses long enough to accumulate measurement support.

Success means that the held-out transition distribution, including its rare
tail, is reproduced and a supported hard-loss lineage produces a finite
234-hit smoothed IP state closer to generator truth than LCIO across events 11,
16, and 17 without covariance failure or measurement rejection.

Current non-goals: further reverse-refit investigation, global-model tuning,
pruning/runtime optimization before the matched model exists, treating delayed
TopN as a final policy, fitting to SimHit momentum, assuming ACTS ATLAS
coefficients validate CEPC, broad performance plots, or additional shared-
package changes.
