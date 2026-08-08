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

The active focus is representation-stable final estimation from the reduced
five-dimensional posterior. The scalar q/p-marginal density mode has now been
tested on a 500-event clean/light causal cohort: it strongly suppresses false
positive tails but loses genuine light-eBrem recoveries, so it is diagnostic
evidence rather than an implementation candidate. The next question is
whether the joint five-parameter state/covariance geometry separates those
two outcomes without truth input or an ad hoc threshold.

Freeze the production baseline exactly as committed in
`DumpGsfTrks/gsf.py.bk` and tagged by
`gsf-memory-leak-fixed-2026-08-08`: five-component
`CEPC2GeV85StepConditioned`, `MaxComponents=12`,
`ReductionTargetComponents=0`, `SymmetricKL`, identity-lineage protection,
`ComponentWeightCutoff=1e-4`, forward-posterior reverse weights, reverse full
covariance scale 100, `AggregateWeight`, and reverse `BestBranch` publication.
The 24-component setting has already been tested: it preserves both genuine
recovery and false radiative modes and does not solve selection. Do not repeat
or promote it as a candidate.

Current reduction-specific evidence:

- In the zero-overlap held-out no-eBrem diagnosis, all 25 targeted clean-core
  degradations selected strong g2/g4 modes at surfaces 5--9. Twenty were
  reverse-only and five inherited radiative forward support.
- Only 4/25 first crossed above identity immediately after reduction; 21/25
  were already driven by forward prior or measurement likelihood. KL is
  therefore a possible amplifier, not the sole generator of false modes.
- The protected no-radiation GSF branch was better in 24/25 but inside +-1%
  in only 23/25; a safety anchor must preserve the actual LCIO state, not only
  an identity lineage inside the GSF refit.
- Of 52 held-out good light-eBrem recoveries, 50 selected a radiative branch;
  identity was better in only one. Of 100 misses, 92 selected identity and
  eight selected only g1. Global identity preference is not viable.
- False and genuine reverse-only radiative branches have overlapping birth
  odds and accumulated exact likelihood support: median odds 0.037 versus
  0.041 and median log-likelihood gain 3.89 versus 3.41. Both most often gain
  their strongest evidence near surface 5. A simple evidence threshold is
  not a discriminator.

New causal evidence separates state interpolation from mass pooling:

- Three reverse-only clean failures and two reverse-only genuine light
  recoveries were traced through every influential merge and hit. The clean
  set includes a low-cost pooling flip, a forced high-cost pooling flip, and
  a next-hit likelihood flip for which reduction barely changes the relevant
  radiative state.
- A diagnostic counterfactual retained the heavier real parent's state and
  covariance at every merge while still summing weights. All three false
  radiative selections persisted and both genuine recoveries survived.
  Moment-centroid interpolation therefore changes pT modestly but is not the
  principal selection cause.
- Adjacent-surface merging occurs in both false and genuine cases. A strict
  surface-history prohibition cannot distinguish them. A KL-distance ceiling
  could stop one high-cost clean flip but not the low-cost pooling or
  measurement-driven cases, so it is not a general solution.
- The source counterfactual was fully reverted. A normal rebuild and fresh
  seed 16/event 14 run exactly restored the baseline pT 30.8103 GeV and final
  selected weight 0.3323; no experimental C++ diff remains.
- A same-code 500-event stress test used 250 topology-clean no-eBrem events
  (all 37 known positive tails plus 213 preserved controls) and 250
  outcome-stratified light-eBrem events. The scalar kappa density mode kept
  all 213 preserved clean controls inside +-1% and recovered 19/37 clean
  positive tails into that core.
- On the light cohort it increased +-1% containment from 106/250 to 121/250
  and reduced positive residuals above +1% from 64 to 39, but it lost 7/50
  genuine good recoveries and worsened partial recovery. In all seven losses
  it returned toward the original LCIO-like residual. Four mode-nearest
  components were identity-lineage and three radiative-lineage, so the effect
  is not equivalent to identity selection.
- The scalar mode is therefore rejected as a final-state publication mode.
  Its failure is consistent with discarding correlations between kappa and
  the other helix parameters: a narrow LCIO-like marginal peak can beat a
  broader but jointly supported radiative solution.

Working hypothesis: reduction must conserve probability mass to approximate
the filtering density, but neither the largest reduced Gaussian's pooled
weight nor a one-dimensional marginal mode is a generally sufficient final
state estimator. The former is representation-dependent after histories are
merged; the latter can discard joint measurement constraints that distinguish
genuine radiation from an LCIO-like narrow peak. This does not rehabilitate
the rejected `DominantLineage`, SurfaceConsistency, Runnalls, TopN, or
24-component controls.

Proceed in this order:

1. Contrast the seven light good recoveries lost by the scalar mode with the
   19 repaired clean positive tails. Capture the final components' complete
   five-parameter means, covariances, weights, lineages, and the decisive
   surface histories; keep filtering and exact likelihoods fixed.
2. Test whether a mathematically defined joint-posterior mode or another
   full-state density functional retains the radiative solutions that are
   broad in kappa but supported by correlations. Define its coordinate and
   covariance treatment before evaluating residuals. Do not splice a scalar
   kappa mode into an unrelated component.
3. In parallel, use the existing truth-assisted loss scan on
   identity-stuck light events to retain the separate diagnosis of absent,
   removed/merged, likelihood-losing, and information-limited useful modes.
   Truth remains diagnostic only.
4. Implement nothing until the offline estimator passes both clean and light
   causal gates and is demonstrably distinct from the rejected
   `DominantLineage`, SurfaceConsistency, and `WeightedMean` publications.
5. If a candidate survives, implement it default-off, follow the configurable
   property documentation law, then run focused events, hard-loss events
   11/16/17, and a same-code held-out population A/B with secondary topology
   reported separately.

Success means reducing reduction-created false confidence and preserving
useful light/hard radiative branches without biasing or broadening the
no-eBrem LCIO core. A new independent light-tail dataset remains necessary
before any production-performance claim.

Current non-goals are repeating the 24-component study, promoting Runnalls,
an ad hoc likelihood threshold, `WeightedMean` publication, global covariance
or process-prior rescaling, truth-dependent runtime logic, fitting SimHit
momentum, reviving rejected final-selection heuristics, or changing shared
tracking packages.

The completed 500-event q/p-marginal density-mode stress test and its rejection
as a publication candidate are preserved in
`agents_record/2026-08-08-qoverp-density-mode-500-event-screen.md`.

The completed 500x2 diagnosis and outgoing focus are preserved in
`agents_record/2026-08-08-completed-500x2-diagnostic-and-kl-focus-transition.md`.

The focused merge traces, representative-parent counterfactual, and refined
representation-dependence hypothesis are preserved in
`agents_record/2026-08-08-kl-merge-causality-and-representative-state-counterfactual.md`.

The complete pre-curation live file and its links are preserved in
`agents_record/2026-07-23-AGENTS-pre-curation-snapshot.md`. The migration map
and curation rationale are in
`agents_record/2026-07-23-AGENTS-curation-map.md`.
