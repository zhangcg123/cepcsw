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
- The selectable BH implementations are now only `ActsAtlas` and
  `CEPC2GeV85StepConditioned`; the former `Current` and `GlobalSim2GeV85`
  implementations and aliases have been removed.
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

The active concentration is an execution proof of a 2 GeV pT, 85-degree,
step-`t/X0`-conditioned electron Bethe-Heitler mixture. Ten 1000-event primary-
electron Geant4 jobs are complete. Their 2,574,697 owned transitions contain
9,528 eBrem rows within `t/X0 < 0.03`; 783 higher-thickness rows remain in the
source data but are excluded from the first reconstruction-range table. The
source ROOT files, transition CSVs, and conversion audit JSON files are archived under
`BHModelComparisonStudies/CEPC2GeV85StepConditioned/production/` and must not
be paired with older nominally matching `tuples/trk-*` files. The
provisional five-stratum extractor is
`scripts/extract_cepc_step_conditioned_mixture.py`; its current generated
artifact is `/tmp/cepc2gev85-step-mixture-10seeds.csv`. The transition count is
257.47 owned surface-to-surface intervals per event, not independent events.
The full sample has 9,985 eBrem-containing intervals and 0.131152 GeV mean
eBrem-attributed loss per event; 9,528 such intervals are inside `t/X0 < 0.03`.
The constrained eight-knot, five-component execution artifact and its dense
diagnostic table/plot are under
`Reconstruction/RecGsfTracking/data/CEPC2GeV85StepConditioned/`; they are
finite and normalized with an explicit vanishing-tail zero-material limit.
The artifact is integrated as `BHModel="CEPC2GeV85StepConditioned"`. A verbose
forward event-11 run produced finite 234/234-hit output with 1047/0/0 accepted,
recovered, and rejected component updates, but pT remained 1.7934 GeV,
essentially LCIO rather than 2.0004 GeV truth.
Detailed production, spectrum, and state-by-state diagnostic information is recorded in
`agents_record/2026-07-12-2gev85-transition-pipeline-and-state-diagnosis.md`.

Reconstruction material ownership is no longer the immediate blocker.
`MaterialPathMode="DD4hepBetweenSurfaces"` performs component-local DD4hep
volume integration between successive predicted measurement surfaces and
processes the outgoing hit-0 transition exactly once. In matched event 11, 232
common transitions sum to `0.0737544 X0` in Geant4 and `0.0739544 X0` in
DD4hep, a Geant4/DD4hep ratio of 0.99730. The hard-eBrem ITK transition is
`0.00719995 X0` versus `0.00720455 X0`, a ratio of 0.99936. The legacy
current-surface total is only `0.0191777 X0`; crossed-cradle sums remain
diagnostic only and are not authoritative material semantics.

The first truth-to-branch diagnosis was invalidated because it compared paired
Geant4 truth with a different detector realization having the same generated
MC event. A corrected rerun on `/tmp/gsf-match-tracks.root` confirms that the
hard hit-6 interval is not split when the mixture is at `MaxComponents`; after
cutoff it splits one surface late. The thin transition 150 remains below the
fixed split threshold. The paired run retains 234/234 hits with 929/0/0
accepted/recovered/rejected updates but stays at LCIO pT. Full provenance and
the invalidated audit are preserved in the July 12 record.

An independent exact-pairing check on the new ten-event files confirms the
same gate in a moderate case. Seed 1 event 3 has true `z=0.910276` on hit 6 to
7, a 233-hit track, and matching MC/VXD/ITK/TPC signatures between `sim` and
`trk`. DD4hep matches the owning Geant4 thickness, but 12 pre-existing
components prevent children from being created there; splitting occurs after
hit 7 instead. The run retains 233/233 hits and remains at LCIO pT.

Split-before-budget ordering is now implemented. On the same paired moderate
event, the hit-6 transition expands 12 parents to 60 children, cutoff and
same-surface KL reduction return it to 12, and the truth-covering `z~=0.899`
child survives with weight 0.0165. The hit-7 likelihood promotes it to weight
0.917. The run retains 233/233 hits, but the published forward IP state remains
at LCIO pT because it preserves the inner filtered history rather than carrying
the selected downstream loss lineage back to the IP.

The corrected event was also rerun with retained-lineage RTS smoothing and
TopN reduction. All 12 lineages smooth successfully, including a final
correctly timed hit-6-loss lineage, but their inner pT values change only from
`1.7980208` to about `1.79809 GeV`. The final best TopN lineage instead uses a
low-loss hit-6 child. Thus smoothing is operational, but neither endpoint
selection nor the RTS backward correction currently recovers IP momentum.

ACTS source inspection shows that its GSF uses a second backward
multi-component filtering pass, not component RTS smoothing. The local reverse
workflow has been aligned in ordering, direction-aware DD4hep intervals,
split-before-budget, KL reduction, and final mixture moment matching. Its first
paired run reaches finite reverse states across the hard interval but terminates
after hit 6 during the next reverse split/reduction. Reverse BH/reduction
stability is now the immediate execution blocker; the incomplete run has no
physics interpretation.

The termination was repaired: recursive KL ancestry strings caused memory
exhaustion during reverse cloning, so diagnostic histories are now bounded
without changing filter mathematics. The completed paired ACTS-style run has
233/233 hits, 2,457/0 accepted/rejected reverse updates, seven reverse splits
and reductions, and an 11-component moment-matched IP mixture with pT
`1.99286 GeV` versus `2.0004 GeV` truth and `1.7980 GeV` LCIO. This is paired
execution evidence, not statistical validation.

Three additional exact-pair 233-hit events give mixed backward-mixture results:
seed/event 1/5 gives pT `2.0302` versus `2.0004 GeV` truth, 2/7 remains at
`1.8538` versus `1.8521 GeV` LCIO, and 7/9 gives `2.0047 GeV`. In the failed
early-loss event, correct ~2.06 GeV reverse children exist, but adjacent-hit
likelihoods do not distinguish them from the no-loss state and the BH prior
leaves the no-loss endpoint at weight 0.977. This is an observability/posterior
weighting failure, not missing mixture support.

A further selected batch of 20 exact-production long-track eBrem events spans
principal `z=0.555-0.991`. The ACTS-style reverse mixture improves absolute pT
error in 19/20, with mean `77.3 -> 32.8 MeV`, median `36.7 -> 15.3 MeV`, 13/20
within 20 MeV and 17/20 within 50 MeV. All 16 inner-transition cases improve;
the dominant failure is a very large penultimate-transition loss with
insufficient downstream lever arm. This deliberately selected same-production
sample is execution evidence, not an unbiased or held-out performance result.

Proceed in this order:

1. Repeat the split-before-budget check on the paired hard event 11, including
   truth-covering child creation, posterior selection, reduction, and lineage.
2. Audit the completed backward mixture state by state across hit 6 and compare
   its component weights and moment match with truth before expanding samples.
3. Separately audit the fixed split threshold before changing it.
4. Run complete verbose events 11, 16, and 17 with 4-5 retained hypotheses and
   require finite 234-hit output without covariance failure or measurement
   rejection.

Success for this phase means that the producer-to-fit-to-GSF pipeline executes,
the fitted mixture is finite and normalized, and events 11, 16, and 17 produce
finite 234-hit states without covariance failure or measurement rejection.
This does not establish predictive physics performance; independent held-out
validation and broad energy/angle coverage remain required before such claims.

Current non-goals: further reverse-refit investigation, global-model tuning,
pruning/runtime optimization before the matched model exists, treating delayed
TopN as a final policy, fitting to SimHit momentum, assuming ACTS ATLAS
coefficients validate CEPC, broad performance plots, or additional shared-
package changes.
