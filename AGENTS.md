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

The active focus is calibration of the reverse measurement likelihood at
decisive surfaces 5--8. Final-posterior publication is no longer the leading
uncertainty: scalar and full five-dimensional density modes repair many false
clean tails but lose the same genuine light-eBrem recoveries, and the natural
mean/median/component-centre alternatives fail clean-track safety. The next
question is whether reverse innovations and transported covariances make
identity-versus-radiative likelihood ratios overconfident or directionally
inconsistent before reduction.

Freeze the production baseline exactly as committed in
`DumpGsfTrks/gsf.py.bk` and tagged by
`gsf-memory-leak-fixed-2026-08-08`: five-component
`CEPC2GeV85StepConditioned`, `MaxComponents=12`,
`ReductionTargetComponents=0`, `SymmetricKL`, identity-lineage protection,
`ComponentWeightCutoff=1e-4`, forward-posterior reverse weights, reverse full
covariance scale 100, `AggregateWeight`, and reverse `BestBranch` publication.
The 24-component setting has already been tested: it preserves both genuine
recovery and false radiative modes and does not solve selection.

Current causal evidence:

- In the zero-overlap held-out no-eBrem diagnosis, all 25 targeted clean-core
  degradations selected strong g2/g4 modes at surfaces 5--9. Only 4/25 first
  crossed above identity immediately after reduction; 21/25 were already
  driven by forward prior or measurement likelihood. KL can amplify a mode,
  but is not its principal generator.
- Of 52 held-out good light-eBrem recoveries, 50 selected a radiative branch;
  identity was better in only one. Of 100 misses, 92 selected identity and
  eight selected only g1. Global identity preference is not viable.
- False and genuine reverse-only radiative branches have overlapping birth
  odds and accumulated exact likelihood support: median odds 0.037 versus
  0.041 and median log-likelihood gain 3.89 versus 3.41. Both most often gain
  their strongest evidence near surface 5. A simple evidence threshold is
  not a discriminator.
- A representative-parent counterfactual retained the heavier real parent's
  state and covariance at every merge while summing weights. Three false
  radiative selections persisted and two genuine recoveries survived.
  Moment-centroid interpolation is not the principal selection cause.
- In the 500-event stress test, the scalar kappa density mode retained all 213
  ordinary clean controls and repaired 19/37 clean positive tails, but lost
  7/50 genuine light recoveries. A direct final-mixture contrast shows the
  full 5D mode makes effectively the same choices: 19/19 clean repairs and
  0/7 light recoveries. Removing covariance correlations does not change the
  result. The earlier discarded-correlation explanation is rejected.
- Covariance-volume, non-kappa Mahalanobis-distance, process-surface, and
  process-mode distributions overlap between false clean and genuine light
  branches. Mean and median estimators retain more genuine recovery but fail
  clean safety. No natural final posterior functional passes both gates.
- The new 5,000-event Geant4 sample contains 1,166,680 primary-electron
  transitions and 4,685 eBrem transitions. Seed-parity held-out tests find no
  reproducible incident-momentum, polar-angle, or transition-group dependence
  of eBrem occurrence after t/X0 conditioning. Conditional loss-shape effects
  at surfaces 5--11 are not stable between halves; only a small outer-tracker
  effect repeats. A multidimensional energy/angle/layer BH model is therefore
  not supported as the next change.

Working hypothesis: false and genuine radiative modes are already ambiguous
under the calculated reverse likelihood before final publication. The
remaining actionable possibility is a calibration defect in the exact
innovation likelihood or transported covariance at surfaces 5--8, including
direction-dependent covariance coverage or information reuse. This does not
rehabilitate rejected final selectors, KL ranking changes, capacity 24, or a
literal layer-conditioned BH table.

Proceed in this order:

1. Reuse the frozen topology-clean no-eBrem/light-eBrem cohorts and identify
   matched decisive-surface contrasts at 5--8. Keep truth out of runtime
   decisions; use it only to label diagnostic outcomes.
2. For identity and competing radiative hypotheses, measure whitened
   innovation pulls, chi-square coverage, log-determinant calibration, and
   cumulative likelihood ratios by surface. Compare forward and reverse
   evaluations of the same accepted measurement where mechanically possible.
3. Audit exact inter-surface covariance transport and measurement ownership
   for directional consistency and accidental information reuse. Do not tune
   a global covariance scale or add an evidence threshold.
4. Form an implementation candidate only if one calibration discrepancy is
   reproducible in independent clean and light cohorts and predicts both false
   selection and genuine recovery. Otherwise record these boundary cases as
   statistical/information-limited ambiguity under the current inputs.
5. Gate any candidate on focused verbose events, hard-loss events 11/16/17,
   and a same-code held-out population A/B. Report secondary topology and
   broad energy/angle controls separately.

Success means correcting a demonstrated likelihood-calibration defect while
preserving useful light/hard radiative branches and the no-eBrem LCIO core. A
new independent light-tail dataset remains necessary before any
production-performance claim.

Current non-goals are repeating the 24-component study, further final
mean/median/mode publication heuristics, promoting Runnalls, an ad hoc
likelihood threshold, global covariance or process-prior rescaling, an
energy/angle/literal-layer BH table without held-out truth support,
truth-dependent runtime logic, fitting SimHit momentum, or changing shared
tracking packages.

The completed full-posterior contrast, correction of the discarded-correlation
hypothesis, and broad held-out BH dimension audit are preserved in
`agents_record/2026-08-09-full-posterior-and-bh-dimension-audit.md`.

The completed 500-event q/p-marginal density-mode stress test is preserved in
`agents_record/2026-08-08-qoverp-density-mode-500-event-screen.md`.

The completed 500x2 diagnosis and outgoing focus are preserved in
`agents_record/2026-08-08-completed-500x2-diagnostic-and-kl-focus-transition.md`.

The focused merge traces and representative-parent counterfactual are
preserved in
`agents_record/2026-08-08-kl-merge-causality-and-representative-state-counterfactual.md`.

The complete pre-curation live file and its links are preserved in
`agents_record/2026-07-23-AGENTS-pre-curation-snapshot.md`. The migration map
and curation rationale are in
`agents_record/2026-07-23-AGENTS-curation-map.md`.
