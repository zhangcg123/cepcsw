# Global one-loss evidence refitter experiment

Date: 2026-08-22

Branch: `test_new`

Base branch/head: `dev` at `de9fb34`

## Scope and repository boundary

The user explicitly requested an isolated branch and a new refitter under the
GSF package, using `CompleteTracks` as input, while keeping every ready
workflow unchanged. The implementation is therefore a separate Gaudi
algorithm, `RecGsfGlobalLossRefitter`, inside
`Reconstruction/RecGsfTracking`. It is not scheduled by
`DumpGsfTrks/gsf.py.bk`, `options/run_gsf_reverse_template.py`,
`dump_gsftrk.sh`, or the batch scripts. It writes `GlobalLossTracks` and does
not read, write, or modify `GSFTracks`.

The active production GSF implementation and its 43-property configuration
contract remain unchanged. The only shared source extension is a const
`BetheHeitlerSplitter::mixture(tX0)` query; `split()` now obtains the same
mixture through that query before applying it, preserving the existing
component application path.

## Proposed model

The experiment asks whether downstream measurements can make the BH decision
globally instead of committing to a locally selected component during the
ordinary GSF evolution.

Accepted hits are ordered inner to outer. Every hypothesis starts from the
input `CompleteTracks::AtLastHit` state with its full covariance scaled by
`OuterSeedCovarianceScale`, then consumes all inward hits with the baseline
MarlinTrk `addHit(reference) -> initialise -> addAndFit(target)` update.
Coverage-checked DD4hep material between consecutive matched hit points
provides the BH prior. The bank contains:

- `H0`: no radiative transition;
- `H(j,k)`: exactly one radiative accepted-hit interval `j` and one
  non-identity BH mode `k`.

For a radiative history, retained fraction `z` is continuous. The marginalized
history evidence is

```text
log Z(j,k) = log P(H(j,k))
           + log integral L(all inward hits | H(j,k), z)
                          N(z | mean(j,k), variance(j,k)) dz.
```

The integral is evaluated by Simpson quadrature within the physical
`ProcessSigmaWindow`. The history prior replaces the identity-mode weight at
interval `j` with mode `k` and retains identity weights at other scanned
intervals. After history selection, local interval-halving profiles the
reported `z`. There is no KL reduction, component pruning, ECAL information,
truth input, or multi-loss history.

Direct MAP history selection was rejected during development because it
selected a false radiative history in the clean control. The final experimental
algorithm records identity and best-radiative evidence separately and
publishes radiation only when

```text
log Z(best radiative) - log Z(identity)
    >= MinimumRadiativeLogBayesFactor.
```

The compiled default is 3, approximately 20:1 evidence. This is a conservative
mechanism gate, not a population-tuned or validated threshold. Setting it to 0
restores direct maximum-evidence history selection.

## Configurable and output contract

The new algorithm has 14 algorithm-specific steering properties. Defaults are:

```text
ElectronHypothesis=true
BHModel=CEPC2GeV85StepConditioned
BHSplitThreshold=1e-4
MSOn=true
ElossOn=false
OuterSeedCovarianceScale=100
ProcessSigmaWindow=3
ProfileGridPoints=9
ProfileRefinementIterations=6
MinimumRetainedFraction=0.05
MinimumRadiativeLogBayesFactor=3
CandidateIntervalIndices=[]
SelectedEventIndices=[]
VerboseDump=false
```

`ActsAtlas` is rejected because this one-loss prior requires an exact identity
atom. The complete option validation and meaning are documented in
`Reconstruction/RecGsfTracking/README.md`.

Successful output tracks contain one `AtIP` state and copy the input hit
relations. Output-aligned diagnostics record input-track index, selected
interval/mode, retained fraction, t/X0, likelihood, prior kernel, selected
evidence, identity evidence, best-radiative evidence, and radiative log Bayes
factor. `GlobalLossStatus` is per input track:

```text
 0  output written
-1  event excluded by SelectedEventIndices
 1  fewer than five associated hits
 2  no usable AtLastHit state
 3  fewer than five matched hits
 4  no valid hypothesis fit
```

## Focused validation and steering

Input:

```text
trk-e--20-85-822751.root
SelectedEventIndices=[3,4]
CandidateIntervalIndices=[]
BHModel=CEPC2GeV85StepConditioned
BHSplitThreshold=1e-4
MSOn=true
ElossOn=false
OuterSeedCovarianceScale=100
ProcessSigmaWindow=3
ProfileGridPoints=9
ProfileRefinementIterations=6
MinimumRetainedFraction=0.05
MinimumRadiativeLogBayesFactor=3
```

The card was temporary (`/tmp/run_global_loss_822751.py`) and the output was
temporary (`/tmp/global_loss_targeted_822751.root`). Neither is part of a ready
workflow or tracked project artifact.

Results:

| zero-based event | truth category | identity pT | best decision | radiative log BF | selected pT | truth reference |
|---:|---|---:|---|---:|---:|---|
| 3 | one 6.425% truth eBrem in matched interval 5 | 33.5438 GeV | interval 6, mode 2, 2.64145% loss | +4.27347 | 34.3063 GeV | 35.813 GeV |
| 4 | no Geant4 eBrem | 48.7885 GeV | identity retained | +0.380292 | 48.7885 GeV | clean control |

The exact-default unrestricted scan therefore passes only the narrow
mechanical objective: strong-enough evidence changes the known-loss case in
the correct momentum direction, while weak evidence cannot alter the clean
control.

It does not close the physics problem. In event 3 the truth-matched interval is
5 and the truth loss is 6.425%, but the global bank selects the adjacent outer
interval 6 and only 2.641%. The result remains below truth and does not improve
over the best existing GSF behavior. The error persisted when:

- the covariance scale was raised from 100 to `1e6`;
- deterministic KalTest energy loss was switched on and off;
- the production five-component and runtime-generic five-component BH models
  were compared;
- evidence quadrature was increased from 9 to 17 points;
- direct profile-MAP history selection was replaced by marginalized evidence.

The outer fitted-state prior, quadrature resolution, deterministic ionization
steering, and choice between those two BH tables are therefore not sufficient
explanations. The all-hit likelihood can prefer a physically wrong nearby
piecewise-curvature history, most likely by compensating ordinary measurement,
transport, or residual-model imperfections. This is direct evidence that
"let all downstream hits choose one loss" is not by itself a solution.

## Current conclusion and next gates

`RecGsfGlobalLossRefitter` is an isolated research instrument, not a candidate
replacement and not ready for workflow integration. Before any population
claim or card integration:

1. Diagnose the interval-5 versus interval-6 likelihood crossover in event 3
   hit by hit, using matched no-eBrem controls.
2. Separate the gain in residual chi-square from innovation-determinant and BH
   prior contributions; establish which measurements create the wrong
   preference.
3. Test the mechanically stable algorithm on the required hard events 11, 16,
   and 17 and on held-out clean controls only after that crossover is
   understood.
4. Add multi-loss histories only if the single-loss localization closes; doing
   so now would multiply an unresolved degeneracy.
5. Keep the evidence threshold default experimental and do not tune it on the
   two focused events.

## Paused outgoing material/BH focus

This branch experiment temporarily interrupts but does not supersede the
material/BH consistency investigation on `dev`. Its frozen production baseline,
coverage-corrected DD4hep endpoint machinery, unresolved 11,175 invalid
coverage groups, 4,790 topology-clear invalid forward paths in 223 events,
BH-response population mismatch, and runtime-model controls remain exactly as
recorded in the pre-branch `AGENTS.md` and these authoritative records:

- `2026-08-18-runtime-material-path-and-bh-input-consistency.md`
- `2026-08-18-material-path-mode-direction-symmetry.md`
- `2026-08-19-dd4hep-matched-hit-material-endpoint-and-ownership-validation.md`
- `2026-08-19-dd4hep-between-surfaces-default-promotion.md`
- `2026-08-19-dd4hep-material-recorder-surface-interval-extraction.md`
- `2026-08-19-dd4hep-forward-reverse-tpc-path-closure.md`
- `2026-08-20-dd4hep-boundary-coverage-repair-and-direction-closure.md`
- `2026-08-20-sensitive-interval-radiation-and-ebrem-reference.md`
- `2026-08-20-runtime-material-bh-audit-recorder.md`
- `2026-08-21-legacy-material-transition-csv-removal.md`
- `2026-08-21-1k-material-bh-audit-campaign-steering.md`
- `2026-08-21-truth-bh-loss-oracle-control.md`
- `2026-08-21-in-process-truth-bh-g4step-tuple-reader.md`
- `2026-08-21-unbiased-runtime-material-bh-closure.md`
- `2026-08-21-category-aligned-runtime-interval-bh-fit.md`
- `2026-08-21-runtime-interval-bh-model-integration-and-focused-ab.md`

Resume that focus by reproducing invalid coverage paths at seed/event `60:9`,
`104:14`, `149:57`, and `444:99`, then locating the first surface where a
truth-compatible lineage loses rank. No material source/mode, split/cutoff,
capacity, KL, reverse seed, ECAL, or publication tuning is authorized before
that branch-local closure.
