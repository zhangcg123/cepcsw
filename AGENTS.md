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
A default-off ECAL component-re-ranking prototype is also mechanically
operational. It preserves `GSFTracks` and writes its paired result separately;
its focused evidence is promising only for retained bimodal alternatives and
is not population-validated.
A separate default-off truth BH-loss oracle can replace existing BH-call
responses on explicitly selected tracks while leaving the downstream GSF
workflow unchanged. It is a mechanism diagnostic only and never production
steering.

The active production candidate remains the reverse multi-component refit. It
starts from the complete final forward mixture, scales each full covariance by
`ReverseKappaSeedCov` (default 100), repeats measurement updates inward, and
publishes either the highest-weight branch or an optional moment-matched
mixture. It has demonstrated interaction-point momentum recovery in many
hard-bremsstrahlung events and favorable central light/hard performance, but
it also creates clean-track degradation and extreme tails. The KL smoother is
largely LCIO-like and forfeits much of the hard-loss recovery. The CMSSW-like
workflow has a different core/tail tradeoff and remains default-off.

The active defaults are `MaterialPathMode=DD4hepBetweenSurfaces`,
`MaxComponents=12`, `ComponentWeightCutoff=1e-4`, `SymmetricKL` reduction
ranking, identity-lineage protection enabled, and the five-component
`CEPC2GeV85StepConditioned` Bethe-Heitler model. Preserve 24 components and
`CurrentSurface` as explicit comparison settings. The weighted `Runnalls`
ranking, the six-component `CEPC2GeV85StepConditioned6` model, and the
runtime-interval `CEPCRuntimeGenericGrid5Clear` and
`CEPCRuntimeCategoryAligned5Clear` models remain default-off controls; none is
validated or approved as a replacement.

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

The active question is whether the exact material thickness supplied to the
Bethe-Heitler (BH) splitter at the first wrong branch decision is consistent
with Geant4 energy-loss truth and with the domain of the configured BH mixture.
The investigation must distinguish a material path/ownership defect, an
interval-collapse granularity problem, a BH response mismatch, and a later
measurement/selection effect. The ECAL prototype is paused and remains
default-off; this focus does not authorize changes outside `RecGsfTracking`.

Freeze the production baseline, with the 2026-08-18 material-direction
correction, the 2026-08-19 matched-hit endpoint correction and explicit
DD4hep default promotion, and the 2026-08-20 boundary-coverage correction as
the material changes since tag
`gsf-memory-leak-fixed-2026-08-08`: five-component
`CEPC2GeV85StepConditioned`, `MaxComponents=12`,
`ReductionTargetComponents=0`, `SymmetricKL`, identity-lineage protection,
`ComponentWeightCutoff=1e-4`, forward-posterior reverse weights, reverse full
covariance scale 100, `AggregateWeight`, and reverse `BestBranch` publication.
`MaterialPathMode=DD4hepBetweenSurfaces` now governs both outward and inward
propagation, integrates between matched hit points, and evaluates the scalar
reverse path in canonical inner-to-outer order. Returned DD4hep segments must
cover the full endpoint distance; a detected leading boundary omission is
recovered by a bounded inward nudge and audited cap. Before the direction
correction, reverse propagation accidentally always used DD4hep regardless of
steering; pre-fix reverse tuples must therefore be rerun for same-code
comparisons.
The 24-component setting has already been tested: it preserves both genuine
recovery and false radiative modes and does not solve selection.

The committed production steering contract is `DD4hepBetweenSurfaces`,
`BHSplitThreshold=1e-4`, `ComponentWeightCutoff=1e-4`, and ECAL off. The
maintained `DumpGsfTrks/gsf.py.bk` now agrees with those physics settings and
explicitly leaves `MaterialBHAuditCSV` empty/off, matching the algorithm and
reverse-template defaults. Enable the comprehensive audit only in temporary
diagnostic cards; normal maintained-card batch jobs produce no material CSV.
`TruthBHLossOverride=false` remains the compiled, reverse-template, and
maintained-card default. The compiled and reverse-template oracle source/input
remain `CSV`/empty. The maintained batch card instead preconfigures the
default-off `G4StepTuple` source and its per-job material-tuple path, track 0,
and a 5 mm endpoint guard so enabling the diagnostic needs no external CSV
join. CSV-selected tracks still require every exact consecutive accepted-hit
interval, while zero-row tracks are logged and use ordinary BH throughout.

Current boundary evidence and definitions:

- A Geant4 pre/post step is truth-side energy-loss information. It is not one
  runtime GSF propagation interval or one BH call.
- `CurrentSurface` supplies surface-owned normal thickness projected by the
  component incidence. `DD4hepBetweenSurfaces` integrates geometry material
  from the component's current state to the already matched target-hit point
  between bounding measurement surfaces.
- The BH-call population is one executed
  `split(parent, pathTX0, bz[, reverse])` per valid component path above the
  split threshold. Candidate paths, event-hit medians, hit-level split counts,
  and Geant4 transitions are not interchangeable with this population.
- The earlier mode-dispatch defect is fixed: `MaterialPathMode` governs both
  outward and inward propagation. The subsequently exposed TPC path-value
  defect is also fixed. All 666 internal-TPC intervals in a paired three-event
  rerun now agree forward/reverse within 1.35e-9 relative, and the three event
  totals agree within 7.1e-8%. All 5,971 forward and 7,127 reverse component
  paths were valid.
- The bounded-target endpoint defect is fixed for `DD4hepBetweenSurfaces`:
  the target hit now anchors the material segment while finite-point,
  matched-surface, and propagation-direction guards remain. In the selected
  30-event rerun, all 8,144 displayed forward and 67,918 reverse paths were
  valid, versus 141 and 1,034 invalid evaluations before the correction.
- On seed 107 entry 0, the restored inner VXD surface-to-surface paths agree
  with Geant4 ownership (half current sensor, support/gap, half target sensor)
  within 0.10% per tested interval and 0.027% in their three-interval sum.
  This validates the endpoint/ownership mechanism, not the collapsed BH
  response or production performance.
- The default-off material recorder can now emit midpoint-to-midpoint DD4hep
  intervals with the same `materialsBetween` primitive used by GSF. A
  three-event seed-107 comparison matched 693 reconstructed non-seed
  intervals: summed DD4hep differences were at most 0.0034% in VXD/ITK and
  0.0327% in OTK; the TPC sum differed by 0.815% because truth midpoints and
  digitized hit endpoints are not identical. The three separately handled
  seed paths are not emitted by the current GSF CSV audit. This validates the
  extractor mechanism, not BH closure.
- The material recorder independently evaluates both endpoint orders with the
  same coverage invariant. A three-event final-code rerun produced 696 valid
  interval pairs, all equal within 1e-9 X0. Thirty-seven reverse queries
  required audited boundary recovery and no forward query did. The restored
  internal-TPC paths remain about 4.4e-5 X0, below the production
  `BHSplitThreshold=1e-4`, so the repair closes material accounting but does
  not add production BH branch choices there.
- Same-input pre/post controls showed no branch-scale discontinuity and
  retained the hit count and NDF for hard events 11, 16, and 17 and seven
  held-out no-tracker-eBrem events. The largest relative momentum change was
  1.3e-6. This is a focused regression check, not population momentum
  validation; the pre-existing pathological momenta in the current tuple
  remain.
- `DD4hepBetweenSurfaces` was explicitly promoted to the compiled, reverse-
  template, and maintained-card default on 2026-08-19 after the endpoint fix.
  This is a steering decision, not a claim that its BH response or population
  momentum performance is validated. A no-assignment seed-107 entry-0 smoke
  selected DD4hep on all 2,146 displayed reverse paths, with zero invalid
  forward or reverse paths, and finalized successfully. `CurrentSurface`
  remains an explicit control.
- A selected 30-event DD4hep audit, explicitly using split/cutoff `1e-4` and
  ECAL off, contained 4,297 actual BH parent calls: 1,953 forward (including
  ten initial seed-material calls) and 2,344 reverse. Forty-three calls were
  above the last CEPC BH knot (`0.024494897427831779 t/X0`), 19 were at least
  `0.03 t/X0`, the maximum was `0.0319038626 t/X0`, and none reached 0.05.
- The CEPC model saturates above its last knot by returning the last-knot
  mixture. The selected audit therefore does not support the broad statement
  that “too many” calls exceed the model range, but rare saturated calls can
  still seed the winning lineage and require branch-local diagnosis.
- The seed-216 dispatch check contained much larger DD4hep intervals, including
  about `0.183 t/X0`, but it was not part of the uniform 30-event population.
- The 30 events were deliberately selected good/bad examples across no/light/
  hard eBrem. They are mechanism evidence, not an unbiased rate or validation.
- An unbiased 41,100-event recorder reference supplies 13 repeated truth-side
  sensitive-midpoint interval categories plus an `ITKB1 -> TPC` row in which
  the truth traversal lacks an ITKB2 anchor. It includes DD4hep t/X0,
  total/eBrem statistics, per-interval eBrem probability, and global eBrem
  share. This is a proxy/reference taxonomy, not the authoritative runtime
  interval list: runtime bounds are consecutive accepted/matched GSF hits.
  Similar-t/X0 categories must not be silently pooled, and a truth-side skipped
  anchor must not be called a runtime skipped anchor until the reconstructed
  hit pair is checked.
- `MaterialBHAuditCSV` is a default-off, separate structured recorder for the
  exact runtime candidates and executed BH calls. It records the seed path,
  forward and reverse accepted-hit bounds, parent identity/weight/lineage,
  DD4hep path and material composition, split decision, returned BH mixture,
  and child state. It does not change the flat-tuple schema. The superseded
  forward-only `MaterialTransitionCSV` recorder has been removed from the
  active source interface; its historical files and evidence remain valid.
- The default-off truth BH-loss oracle replaces each already executed BH
  response on a truth-selected track with one unit-weight child at the matched
  Geant4 eBrem retained fraction, then runs the unchanged downstream workflow.
  The original strict CSV source remains available. The in-process
  `G4StepTuple` source lazily reads one event at a time, reconstructs the
  recorder's sensitive-midpoint intervals, strictly matches them to ordered
  accepted hits on one configured `CompleteTracks` index, and fails on a
  missing/ambiguous primary, nonmonotonic match, or excessive endpoint
  distance. It is batch plumbing for the same diagnostic, not new truth logic.
  The focused five-event gate closed all 80 oracle calls. A same-code fixed
  60-event topology-clear stress/control A/B then closed 1,156 calls and
  reduced all-panel mean absolute pT residual from 4.646% to 2.052%, its 68%
  absolute quantile from 0.580% to 0.317%, and the count above 3% from eight to
  two. Overshoots and underestimates improved strongly, but good-control mean
  absolute residual broadened from 0.0330% to 0.0912%. Forty-three truth-loss
  intervals in 35 events reached a BH call; eleven loss intervals in ten
  events remained below threshold. Event 13 consequently worsened from
  -4.05% to -13.98%, while event 35 remained at -92.97%. This is selected
  response-versus-gating evidence, not held-out validation or authority to
  tune the threshold.
- The first production-scale runtime audit contains 40,040 auditable events
  from 411 nonempty files; 35,582 are topology clear and 4,458 have secondary
  tracker activity. Valid, spatially matched paths close forward/reverse below
  `4.21e-10` relative. Replacing exact runtime thickness with matched recorder
  DD4hep or Geant4 thickness changes the aggregate BH prediction by only about
  0.2%, so the normal endpoint-level material difference is not the principal
  BH-response discrepancy.
- The unbiased audit also exposes 11,175 invalid coverage groups. In the
  topology-clear forward population, 4,790 paths in 223 events are invalid,
  including 209 TPC-to-OTK paths and several Geant4 losses above 20%. Some
  return a plausible nonzero material sum but fail the full-distance coverage
  invariant and therefore never execute BH. The earlier focused all-valid
  statement does not generalize to this population.
- Every valid topology-clear forward candidate below `BHSplitThreshold=1e-4`
  is internal TPC gas. The 7,867,363 such paths contain 6,080 Geant4 eBrem
  intervals and 16.0% of the clear sample's total eBrem loss, but production
  intentionally performs no split. On 345,997 actual clear valid BH calls,
  the current model overpredicts eBrem probability (`0.0772` versus `0.0665`),
  mean loss (`0.00653` versus `0.00296`), and the >20% tail (`0.00960` versus
  `0.00405`). The secondary-activity control biases in the opposite direction;
  inclusive calibration therefore hides a strong population dependence.
- There are 1,533 inclusive valid forward calls above the last BH knot, 567 at
  or above `0.03 t/X0`, and 49 at or above `0.05`; the maximum is `0.6475`.
  Three hundred thirteen span multiple truth sensitive intervals. Constant
  last-knot saturation is rare but is an explicit interval-collapse risk.
- Two default-off, x-only five-component models are now integrated in parallel
  with the unchanged production model. `CEPCRuntimeGenericGrid5Clear` uses the
  generic logarithmic grid; `CEPCRuntimeCategoryAligned5Clear` uses knots
  aligned to observed interval bands. Their compiled responses agree with the
  packaged JSON tables to at most `1.7e-14` over more than 1,200 audited calls,
  and the generalized interpolation leaves the current default bit-for-bit
  unchanged on the focused and 60-event reruns. A same-seed topology-clear
  stress panel selected the 20 largest positive residuals, 20 most negative,
  and 20 best remaining controls under the default. Both candidates reduce the
  overshoot mean absolute residual from `5.365%` to about `1.306%`, principally
  by removing one `+79.4%` no-eBrem false overshoot. Generic-grid does not
  improve the under-group mean (`8.538% -> 8.549%`); category-aligned reduces
  it to `8.425%`, driven mainly by one event. Good-control mean absolute
  residual broadens from `0.0330%` to about `0.041%`, but no control crosses
  1% and candidate maxima remain below `0.18%`. Excluding the single false
  overshoot, all-panel mean absolute gains are only `0.025` and `0.067`
  percentage points. The category fit still underpredicts the secondary-
  activity control and its thickest knot has only 12 extreme-loss training
  entries. This selected panel is not held-out population validation; neither
  model is approved as a replacement, and both use only `pathTX0`.

Proceed in this order:

1. Keep the coverage-corrected endpoint and production baseline frozen. Print
   exact steering and use temporary cards for further material controls.
2. Reproduce the invalid coverage paths in seed/event `60:9`, `104:14`,
   `149:57`, and `444:99` with explicit requested/covered-length diagnostics
   and matched valid controls. Review a focused coverage repair before any
   source change, then rerun the same-code material audit.
3. In those and other bad events, locate the first surface where a
   truth-compatible lineage
   loses posterior rank or is removed, and compare matched good controls.
4. At that crossover, compare `CurrentSurface`, DD4hep interval composition,
   and Geant4 pre/post-step truth between the same spatial boundaries. Compare
   directions only for equivalent component states.
5. Keep both integrated runtime-interval models default-off. After coverage
   and branch-local closure, run same-code held-out population A/B tests before
   considering a default change. Keep the internal-TPC split-threshold control
   separate, test energy/angle/region only as held-out diagnostics of x-only
   sufficiency, treat topology activity only as a reported control, and handle
   last-knot saturation explicitly. Use the truth BH-loss oracle as a
   response-only A/B and classify below-threshold truth losses separately.
6. Classify the cause as path/ownership, interval-collapse granularity, BH
   response, or downstream measurement/selection before proposing a change.
   Then apply the focused verbose, hard-event 11/16/17, and held-out clean-track
   validation gates.

Success means identifying a truth-blind discrepancy at or before the first bad
branch decision that separates bad events from matched good controls and
predicts a same-code correction without sacrificing the no-eBrem core. If the
exact path and BH response close correctly at the crossover, record that result
and move downstream instead of retuning material.

Current non-goals are further material-source or material-mode changes before
branch-local closure, tuning split/cutoff thresholds, capacity, KL ranking,
reverse seed covariance, final publication heuristics, or the ECAL selector;
fitting SimHit momentum; truth-dependent runtime logic other than the explicit
default-off truth BH-loss oracle diagnostic; changing source outside
`RecGsfTracking`; or changing shared tracking packages.

The exact definitions, selected audit, steering warning, and complete ordered
investigation are preserved in
`agents_record/2026-08-18-runtime-material-path-and-bh-input-consistency.md`.
The direction defect and correction are preserved in
`agents_record/2026-08-18-material-path-mode-direction-symmetry.md`.
The bounded-endpoint correction, invalid-path rerun, forward/reverse check,
and representative Geant4 ownership closure are preserved in
`agents_record/2026-08-19-dd4hep-matched-hit-material-endpoint-and-ownership-validation.md`.
The explicit DD4hep default decision, synchronized option surface, historical-
card boundary, and validation requirements are preserved in
`agents_record/2026-08-19-dd4hep-between-surfaces-default-promotion.md`.
The material-recorder interval implementation, tuple contract, and paired
three-event GSF comparison are preserved in
`agents_record/2026-08-19-dd4hep-material-recorder-surface-interval-extraction.md`.
The forward/reverse runtime closure failure, identical-endpoint control, and
review boundary for a canonical-direction correction are preserved in
`agents_record/2026-08-19-dd4hep-forward-reverse-tpc-path-closure.md`.
The production coverage repair, runtime/recorder direction closure, focused
hard-event checks, and held-out no-eBrem A/B are preserved in
`agents_record/2026-08-20-dd4hep-boundary-coverage-repair-and-direction-closure.md`.
The truth-side sensitive-interval radiation/eBrem table, its proxy boundary,
the corrected `ITKB1 -> TPC` interpretation, and required GSF runtime closure
are preserved in
`agents_record/2026-08-20-sensitive-interval-radiation-and-ebrem-reference.md`.
The exact runtime material/BH audit schema, interval authority, focused
mechanical validation, and non-interference A/B are preserved in
`agents_record/2026-08-20-runtime-material-bh-audit-recorder.md`.
The removal of the superseded forward-only material recorder, the synchronized
45-property option surface, and its no-change regression are preserved in
`agents_record/2026-08-21-legacy-material-transition-csv-removal.md`.
The truth BH-loss oracle contract, all-or-nothing track scope, audit extension,
focused A/B, and below-threshold truth-loss finding are preserved in
`agents_record/2026-08-21-truth-bh-loss-oracle-control.md`.
The in-process material-tuple reader, strict matching contract, CSV-equivalence
gate, and batch steering are preserved in
`agents_record/2026-08-21-in-process-truth-bh-g4step-tuple-reader.md`.
The production-scale eventwise join, valid-path closure, unresolved coverage
population, split-threshold accounting, BH-response calibration, and revised
ordered investigation are preserved in
`agents_record/2026-08-21-unbiased-runtime-material-bh-closure.md`.
The x-only category-aligned knot design, exact-runtime interval fit,
same-population generic-grid control, held-out closure, secondary-control
failure, interface boundary, and integration gates are preserved in
`agents_record/2026-08-21-category-aligned-runtime-interval-bh-fit.md`.
The two default-off runtime selectors, packaged tables, exact response audit,
default-regression check, and focused good/bad A/B are preserved in
`agents_record/2026-08-21-runtime-interval-bh-model-integration-and-focused-ab.md`.

The paused ECAL boundary, deferred work, and links to its complete evidence are
preserved in
`agents_record/2026-08-18-ecal-focus-handoff-to-material-bh.md`. Older tracker,
BH, KL, and workflow evidence remains available under `agents_record/` and is
historical unless explicitly reactivated.
