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

`RecGsfTracking` builds, installs, and reads `CompleteTracks`. Smoother and
reverse runs write three row-aligned endpoint views: BestBranch to
`GSFTracksBestBranch`, the moment-matched endpoint to
`GSFTracksWeightedMean`, and the maximum of the complete five-dimensional IP
mixture density to `GSFTracksFullMixtureMode`; CMS-like retains its fixed
single `GSFTracks` output. FullMixtureMode is automatic/default-on for those
two workflows, has a persisted optimization-status collection and flat-tuple
fields, and is mechanically available but not physics-validated. Its
definition, output/fallback contract, and focused mechanical gates are in
`agents_record/2026-08-24-full-mixture-mode-endpoint.md`. Its
underlying positive-weight final components are also persisted automatically
with input/output track mapping, normalized weight, IP kappa, kappa variance,
method source, and validity. The flat `final_mixture_component_*` vectors are
sufficient to reconstruct the one-dimensional pT marginal and are empty for
forward, CMS-like, and global-loss; their contract and gates are in
`agents_record/2026-08-25-final-mixture-component-flat-tuple.md`. Its
complete component lineage is now also persisted automatically for smoother
and reverse jobs. The `lineage_node_*` and `lineage_edge_*` flat vectors keep
every evaluated seed, BH child, measurement result, and KL output, including
states later rejected, cut, or merged; forward, CMS-like, and global-loss
leave them empty. Split/merge diamonds remain directed acyclic graphs, and
the graph never steers the fit. Its schema, code maps, and mechanical gates
are in `agents_record/2026-08-25-component-lineage-dag-flat-tuple.md`. Its
component measurement updates use the baseline-compatible
MarlinTrk `initialise -> addAndFit` path, exact accepted innovation quantities,
full Gaussian innovation likelihoods, and exact accepted inter-surface
transport Jacobians. Forward filtering, an independent reverse
multi-component refit, a KL reduction-aware experimental smoother, and a
CMSSW-like experimental backward workflow are mechanically operational.
The separate experimental `RecGsfGlobalLossRefitter` is also mechanically
available as the fourth explicit `method="global-loss"` choice in the
maintained `DumpGsfTrks/gsf.py.bk`. It consumes `CompleteTracks`, writes
`GlobalLossTracks`, and is scheduled instead of `RecGsfTracking`; the flat
tuple maps that collection into the existing `gsf_*` schema. This availability
does not validate the method or make it the production candidate. The card
default remains `reverse`, and the three established `RecGsfTracking`
workflows are unchanged.
A default-off ECAL component-re-ranking prototype is also mechanically
operational. It preserves `GSFTracksBestBranch` and writes its paired result separately;
its focused evidence is promising only for retained bimodal alternatives and
is not population-validated.
A separate default-off truth BH-loss oracle can replace existing BH-call
responses on explicitly selected tracks while leaving the downstream GSF
workflow unchanged. It is a mechanism diagnostic only and never production
steering.
The normal simulation event can now optionally embed exact
`SimTrackerHit -> Geant4-step` provenance in two PODIO collections. The
default-off oracle's current batch source follows the standard reconstructed-
hit truth associations into those collections, so the maintained workflow no
longer requires or supports a side material tuple or prejoined CSV input.
The active `dump_gsftrk.sh` worker accepts one `STAGES` subset of `sim`, `trk`,
and `gsf` (default `trk,gsf`) and executes selected stages in physical order.
Each stage consumes a predecessor produced in the same job or, when that
predecessor is omitted, the corresponding existing tuple from the input tuple
path. Calorimeter digitization/reconstruction and `EcalCluster` are not part of
this worker while the ECAL prototype is paused.
A passive interval recorder now persists, in the final
GSF EDM and flat tuple, fractionally integrated Geant4 t/X0/eBrem truth,
DD4hep t/X0 between the same exact truth hooks, and summaries of the actual
forward/reverse component paths. These values never steer the GSF. The
compiled, active reverse-template, and maintained-card default is on.

The active production candidate remains the reverse multi-component refit. It
starts from the complete final forward mixture, scales each full covariance by
`ReverseKappaSeedCov` (default 100), repeats measurement updates inward, and
publishes the selected branch, moment-matched mixture, and full joint-density
mode in separate row-aligned collections. It has demonstrated interaction-point momentum recovery in many
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
  whose complete property steering is part of the documentation contract
  except for the deliberate inherited
  `RecordTruthMaterialIntervals=true` default.
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
  --target RecGsfTracking RecGsfFlatTuple -j4
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

The immediate checkpoint is now to use the completed passive component-
lineage DAG to locate the first inward measurement where a truth-compatible
lineage loses posterior rank, is cut, or is merged in bad events, with matched
good controls. Decompose that crossover into prior/BH weight, exact local
`dchi2`, and `logDetInnovation`; do not infer it from final weights alone.
Only after this unscaled baseline is understood should a reviewed control
scale the BH-component variance term newly added to each child continuation
covariance by `BetheHeitlerSplitter` before propagation to the next hit. Scale
1 must reproduce the recorded baseline. Do not scale the whole predicted
covariance or change BH means, mode priors, deterministic covariance
transport, or production defaults. The original mechanics and variance-study
boundary are preserved in
`agents_record/2026-08-25-reverse-posterior-weight-and-bh-variance-handoff.md`;
the implemented graph contract and validation are preserved in
`agents_record/2026-08-25-component-lineage-dag-flat-tuple.md`.

`RecGsfGlobalLossRefitter` isolates the downstream-selection part of this
question in a separate algorithm. It reads `CompleteTracks`, compares identity
with all exactly-one-radiative-interval histories after consuming every inward
hit, and writes `GlobalLossTracks`. The maintained card now exposes it through
the exclusive `method="global-loss"` selector and maps its result into the
stable flat `gsf_*` schema; `reverse` remains the default and the existing
`RecGsfTracking` workflows remain unchanged. A frozen-code 30-event
diagnostic includes 25 topology-clear events and five secondary-
activity controls. In a selected ten-event topology-clear hard-loss panel at
truth intervals 5--7, it published radiation in nine events but selected the
exact truth interval in only four, the adjacent wrong interval in five, and
identity in one. It produced genuine recoveries, including file 19 entry 4
from -31.98% to -1.52%, but also a catastrophic tail in file 66 entry 66:
truth interval 5 had `z=0.2077`, while the method selected interval 4 and
profiled to the lower bound `z=0.05`, publishing 169.23 GeV for 40.68 GeV
truth (+315.96%) with log Bayes factor +113.3. Consuming all hits therefore
does not remove the adjacent-interval or loss-magnitude ambiguity.

Treat the global refitter as a diagnostic instrument, not a candidate
replacement. The maintained-card selector is only mechanical availability, not
a physics endorsement. Do not add multi-loss histories or tune its evidence
gate or retained-fraction floor. First
decompose the file-66 entry-66 per-hit likelihood crossover between selected
interval 4 and truth-compatible interval 5, using file 19 entry 4 and file 66
entry 22 as correct-history controls. The formula, option/output contract, and
initial event-3/4 gate are preserved in
`agents_record/2026-08-22-global-one-loss-evidence-refitter.md`.
The expanded sample selection, all event tables, exact steering, aggregate
results, catastrophic boundary-profile tail, and ordered next diagnostic are
preserved in
`agents_record/2026-08-22-global-one-loss-expanded-event-gate.md`.
The exact broken-session Git/worktree snapshot, completed validation, unresolved
interval-5/6 crossover, and fresh-session restart procedure are preserved in
`agents_record/2026-08-22-test-new-session-recovery-handoff.md`.
The separate-workflow integration, explicit steering, stable flat-schema
adapter, regression gates, and unchanged-default boundary are preserved in
`agents_record/2026-08-22-global-one-loss-maintained-card-integration.md`.

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

A same-code topology-clear no-eBrem control now establishes that the
independent reverse pass is a correlated refit rather than an independent
second measurement of the track. It seeds from the final forward posterior
and reuses the inward hits. In a five-event `ReverseKappaSeedCov` sweep, the
mean absolute displacement from LCIO increased monotonically from `0.0137%`
at scale 1 to `0.2688%` at scale 100 as the forward-posterior memory was
diluted. On the 95-event clean core, scale 1 retained essentially the LCIO
uncertainty and undercoverage (median relative-sigma ratio `0.985`; pull RMS
`1.531` versus `1.519`), while scale 100 broadened the reported uncertainty
by a median factor `1.618` and reduced pull RMS to `1.165`. Therefore the
apparently favorable scale-1 central values are baseline preservation, not
independent truth recovery; covariance inflation mitigates but does not
mathematically remove the correlated prior because its means, components, and
default weights still come from the forward posterior. Keep the production
scale 100 frozen and do not tune this property as a performance fix. The
identity-lineage non-equivalence and the covariance/Kappa validation are
preserved in
`agents_record/2026-08-24-no-ebrem-identity-lineage-kf-non-equivalence.md`
and
`agents_record/2026-08-24-reverse-kappa-correlated-prior-validation-handoff.md`.

The committed production steering contract is `DD4hepBetweenSurfaces`,
`BHSplitThreshold=1e-4`, `ComponentWeightCutoff=1e-4`, and ECAL off. The
maintained `DumpGsfTrks/gsf.py.bk` now agrees with those physics settings and
explicitly steers 41 of the 42 supported properties while deliberately inheriting the
compiled `RecordTruthMaterialIntervals=true` default. The retired side material ROOT
producer and runtime material/BH audit CSV are absent from the current source
and cards. Their CSV and `G4StepTuple` truth-oracle readers are also removed;
embedded simulation provenance plus the default-on final
truth-material EDM/flat-tuple records are the maintained recording path.
`TruthBHLossOverride=false` remains the compiled, reverse-template, and
maintained `gsf.py.bk` base value. A truth-on batch submission passes
`truth_bh_override=true`; `dump_gsftrk.sh` then rewrites each generated per-job
card to true and reads the embedded `GsfG4MaterialSteps` and
`GsfSimTrackerHitG4StepLinks` collections through the event's exact tracker-hit
associations, uses input track 0 with a 5 mm integrity guard, and tags
GSF/flat outputs with `truth-bh`. The standard simulation writer creates
the collections and `trk` preserves them before its output is passed directly
to GSF. No calorimeter stage or side material tuple is part of this maintained
worker. This campaign steering is not a production-default change.
Truth mapping remains all-or-nothing, but event/track validation failures no
longer terminate the job: the complete selected track falls back to ordinary
BH and records an explicit validity/status tag in EDM and the flat tuple.
`RecordTruthMaterialIntervals=true` is the compiled and active reverse-template
default and is deliberately inherited by the maintained card. Its two embedded
provenance collections are unconditional base `PodioInput` members. It writes
`GSFTruthMaterialIntervals`, `GSFTruthMaterialRecordStatus`, and 50
`truth_material_*` flat branches. This remains passive recording, not
production steering or a physics-impact test.

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
- The retired default-off material recorder emitted midpoint-to-midpoint
  DD4hep intervals with the same `materialsBetween` primitive used by GSF. A
  three-event seed-107 comparison matched 693 reconstructed non-seed
  intervals: summed DD4hep differences were at most 0.0034% in VXD/ITK and
  0.0327% in OTK; the TPC sum differed by 0.815% because truth midpoints and
  digitized hit endpoints are not identical. The three separately handled
  seed paths were not emitted by the historical GSF CSV audit. This validates the
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
- The former `MaterialBHAuditCSV` component-call recorder and the side
  `GsfMaterialStepRecorderAnaElemTool` ROOT producer are retired from the
  active source interface and maintained cards. The corresponding CSV and
  side-ROOT runtime readers are also removed. Historical files, records, and
  Git history remain available for interpreting earlier studies; stale cards
  assigning any removed interface must be regenerated.
- The default-on passive truth-material recorder writes one final EDM object
  per consecutive accepted-hit interval. It keeps fractionally integrated
  Geant4 t/X0/eBrem truth, DD4hep t/X0 between the same exact hooks, and
  direction-separated summaries of runtime component paths. A one-event
  recorder-on/off gate produced 231 intervals and 50 flat branches while the
  full GSF track and every pre-existing flat branch remained exactly equal.
  The existing EventData truth-oracle output also remained exactly equal with
  recording disabled. This validates persistence and non-interference only;
  no effect on GSF performance has been audited.
- The default-off truth BH-loss oracle replaces each already executed BH
  response on a truth-selected track with one unit-weight child at the matched
  Geant4 eBrem retained fraction, then runs the unchanged downstream workflow.
  The current `EventData` source follows each accepted reconstructed hit
  through its standard truth association to the exact `SimTrackerHit`, then
  through an embedded link to the exact Geant4 step range and hook. Spatial
  distance is only an integrity guard; it does not choose the truth hit or
  step. Missing, ambiguous, incomplete, nonmonotonic, or nonphysical selected-
  track provenance invalidates the whole truth scope. An invalid scope uses
  ordinary BH for the whole track and emits a negative status instead of
  failing the event. Embedded `EventData` is now the only supported oracle
  source; there is no source-selector or external-input-path property.
  A one-event 2 GeV, 85 degree mechanical chain persisted 629 selected Geant4
  steps and 233 complete SimTrackerHit links, preserved both counts through
  tracking, calorimeter digitization, and calorimeter reconstruction, then
  read the reconstructed event, exactly matched 231 accepted-hit anchors, and
  closed 18 oracle calls with status 1. No side material tuple was produced,
  and a truth-off historical input without the new collections still ran.
  A forced 0.1 mm endpoint-guard failure also completed with ordinary BH,
  replaced zero calls, and persisted status -2. This is a mechanism gate, not
  population validation.
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
3. In those and other bad events, use the persisted component-lineage DAG to
   locate the first surface where a truth-compatible lineage loses posterior
   rank or is removed. Decompose its prior, `dchi2`, and innovation-
   determinant terms, and compare matched good controls.
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
The subsequent 1,000-event campaign steering that re-enables only the
comprehensive per-job audit is preserved in
`agents_record/2026-08-21-1k-material-bh-audit-campaign-steering.md`.
The truth BH-loss oracle contract, all-or-nothing track scope, audit extension,
focused A/B, and below-threshold truth-loss finding are preserved in
`agents_record/2026-08-21-truth-bh-loss-oracle-control.md`.
The in-process material-tuple reader, strict matching contract, CSV-equivalence
gate, and batch steering are preserved in
`agents_record/2026-08-21-in-process-truth-bh-g4step-tuple-reader.md`.
The invalid-scope ordinary-BH fallback, EDM/flat/audit validity contract, and
focused positive/negative controls are preserved in
`agents_record/2026-08-22-truth-bh-invalid-scope-fallback-and-validity-tag.md`.
The embedded PODIO data model, exact SimTrackerHit-to-Geant4-step provenance,
maintained no-side-tuple workflow, EventData oracle source, and one-event
mechanical validation are preserved in
`agents_record/2026-08-22-embedded-simhit-g4-step-event-provenance.md`.
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
The passive final-event interval schema, explicit non-steering boundary,
maintained-card difference, and exact recorder-on/off gate are preserved in
`agents_record/2026-08-22-passive-truth-material-interval-final-tuple.md`.
The subsequent explicit default-on decision and unchanged non-steering
boundary are preserved in
`agents_record/2026-08-22-passive-truth-material-recorder-default-promotion.md`.
The removal of the side material ROOT producer and runtime BH-audit CSV,
retained historical-reader boundary, synchronized 45-property surface, and
one-event final-tuple smoke check are preserved in
`agents_record/2026-08-22-retired-side-material-helper-outputs.md`.
The subsequent removal of the CSV and side-ROOT truth-oracle readers, the
embedded-only oracle contract, and synchronized 43-property surface are
preserved in
`agents_record/2026-08-22-retired-material-helper-readers.md`.
The maintained worker's simplification from the former five-stage calorimeter
chain to direct `sim -> trk -> GSF` input is preserved in
`agents_record/2026-08-22-direct-tracker-to-gsf-workflow.md`.

The paused ECAL boundary, deferred work, and links to its complete evidence are
preserved in
`agents_record/2026-08-18-ecal-focus-handoff-to-material-bh.md`. Older tracker,
BH, KL, and workflow evidence remains available under `agents_record/` and is
historical unless explicitly reactivated.

The separate-workflow integration, explicit steering, stable flat-schema
adapter, unchanged reverse regression, and global-loss smoke gate are preserved
in `agents_record/2026-08-22-global-one-loss-maintained-card-integration.md`.
