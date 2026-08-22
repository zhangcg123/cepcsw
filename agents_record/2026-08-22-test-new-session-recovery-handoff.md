# `test_new` broken-session recovery handoff

Date: 2026-08-22
Branch at handoff: `test_new`
Source implementation commit: `8c0809a` (`feat(gsf): add global one-loss evidence refitter`)
Base: `de9fb34` (`dev` and `origin/dev`)
Remote status: `test_new` has not been pushed

## Purpose and authority

This is the complete restart record for the session that implemented and
tested the isolated global one-loss diagnostic refitter. The Codex session
became unreliable because its local sandbox helper disappeared, so the user
requested a durable recovery record before starting a fresh normal session.
The sandbox failure is an execution-environment defect, not evidence of a
CEPCSW build or source failure.

This record supplements, and does not replace,
`2026-08-22-global-one-loss-evidence-refitter.md`. That earlier record is the
algorithm and experiment record. This file adds the exact Git/worktree state,
the decisions that must survive the restart, and the narrow next diagnostic.
The live `AGENTS.md` remains authoritative if a later decision supersedes
this handoff.

## Exact Git and worktree state

At the recovery inspection:

```text
branch: test_new
HEAD:   8c0809a feat(gsf): add global one-loss evidence refitter
base:   de9fb34 Run GSF directly from tracker output
remote: origin/dev -> de9fb34
```

The `test_new` branch was created only because the user explicitly requested
it. It is local and has not been pushed. Do not push, merge, rebase, rename,
delete, or otherwise change branches unless the user gives a new explicit Git
instruction.

The files belonging to commit `8c0809a` were clean at the start of this
handoff:

- `AGENTS.md`
- `Reconstruction/RecGsfTracking/README.md`
- `Reconstruction/RecGsfTracking/src/BetheHeitlerSplitter.cpp`
- `Reconstruction/RecGsfTracking/src/BetheHeitlerSplitter.h`
- `Reconstruction/RecGsfTracking/src/GlobalLossRefitter.h`
- `Reconstruction/RecGsfTracking/src/GsfAlgorithm.cpp`
- `agents_record/2026-08-22-global-one-loss-evidence-refitter.md`

The wider worktree was already very dirty and belongs to the user or to older
work. The snapshot counted 115 tracked modified/added paths, 22 tracked
deletions, and 2,524 untracked entries. Known unrelated tracked changes
included:

- `DumpGsfTrks/README.md`
- `DumpGsfTrks/trk.py.bk`
- `dump_gsftrk.sh`
- `subtrkjobs.sh`

Never run `git reset`, `git clean`, a bulk checkout, or bulk staging in
this worktree. Do not infer that an untracked output is disposable. Stage only
explicit paths after inspecting their diffs. Generated ROOT files, logs,
plots, temporary cards, and batch products are not source/status content.

Temporary focused-test artifacts existed under `/tmp` and are reproducible,
not authoritative project state:

- `/tmp/run_global_loss_822751.py`
- `/tmp/global_loss_targeted_822751.root`
- `/tmp/gsf_split_regression_override.py`
- `/tmp/gsf_split_regression_flat.root`
- `/tmp/gsf_split_regression_edm.root`

The broken session's normal sandboxed shell and `apply_patch` both failed:

```text
bwrap: execvp codex-linux-sandbox: No such file or directory
```

Only approved unsandboxed repository reads and this requested documentation
write were used after that failure. No source or workflow behavior was changed
during recovery recording.

## User request and implementation boundary

The user asked for a new experimental refitter on `test_new` that consumes
`CompleteTracks`, uses all downstream/inward hits to compare global radiation
histories, and leaves the ready production workflows unchanged. It was to live
inside the GSF package and reuse existing tracking/material/BH machinery where
possible.

The resulting `RecGsfGlobalLossRefitter` is intentionally separate from
`RecGsfTracking`:

- It reads `CompleteTracks`.
- It writes `GlobalLossTracks`, not `GSFTracks`.
- No maintained run card or batch workflow instantiates it.
- It is not a production candidate and has no default runtime effect.
- It changes no shared CEPCSW package.
- It does not use ECAL or truth to choose a published physics result.
- It does not implement multiple radiative intervals.

Do not integrate it into `DumpGsfTrks/gsf.py.bk`, `dump_gsftrk.sh`, or
batch cards unless the user explicitly reopens that decision after the
likelihood crossover is understood.

## Committed implementation map

Commit `8c0809a` contains the experimental implementation and durable
documentation:

- `GlobalLossRefitter.h` defines the separate Gaudi algorithm and its
  hypothesis evaluation, matched-hit preparation, DD4hep material lookup,
  evidence integration, selection gate, and output collections.
- `GsfAlgorithm.cpp` registers the new component without scheduling it.
- `BetheHeitlerSplitter.h/.cpp` adds a const `mixture(double tX0)` query.
  The existing `split()` uses the same query and then follows its unchanged
  state-transformation path. The production splitter path was smoke-tested
  after this refactor.
- `Reconstruction/RecGsfTracking/README.md` documents the new algorithm,
  formula, 14 properties, collections, status codes, and experimental
  boundary.
- `AGENTS.md` records the live focus and prohibition on premature
  integration, multi-loss extension, or evidence-gate tuning.
- `2026-08-22-global-one-loss-evidence-refitter.md` records the
  implementation and focused experiment in detail.

## Algorithm and statistical decision

For each usable input track, matched accepted hits are ordered by increasing
radius. The refit starts from `CompleteTracks::AtLastHit`, scales the complete
outer covariance, and consumes measurements inward through the baseline
MarlinTrk-compatible operation:

```text
addHit(reference) -> initialise(backward) -> addAndFit(target)
```

The finite hypothesis bank is:

```text
H0             identity: no radiative interval
H(j,k,z)       one radiative interval j, non-identity BH mode k,
               continuous retained momentum fraction z
```

There is no KL reduction or component competition inside this instrument. A
hypothesis is propagated through every inward hit before histories are
compared. Its intended evidence is:

```text
log Z(j,k) = log P(H(j,k))
           + log integral [
               L(all inward hits | H(j,k), z)
               N(z | mean(j,k), variance(j,k)) dz
             ]
```

The one-dimensional integral is evaluated by Simpson quadrature over the
configured BH-mode window. After a discrete history wins, `z` is profiled
more finely only to publish the selected state and diagnostics. The evidence,
not the profiled maximum alone, selects the discrete history.

The final publication policy is deliberately conservative:

```text
publish best radiative history only when
log Z(best radiative) - log Z(identity) >= MinimumRadiativeLogBayesFactor
```

The default gate is 3, approximately 20:1 evidence odds. It is an experimental
clean-track guard, not a calibrated physics threshold. The ungated all-hit
likelihood can still prefer a false radiative history on a clean track.

## Complete algorithm-specific defaults

The dedicated option-surface audit found 14 algorithm-specific properties:

| Property | Default | Meaning |
|---|---:|---|
| `ElectronHypothesis` | `true` | Use the electron mass hypothesis. |
| `BHModel` | `CEPC2GeV85StepConditioned` | Bethe-Heitler mixture table. |
| `BHSplitThreshold` | `1e-4` | Minimum finite nonnegative interval t/X0 for radiative hypotheses. |
| `MSOn` | `true` | Enable multiple scattering in the MarlinTrk refit. |
| `ElossOn` | `false` | Disable baseline deterministic energy loss; radiation is represented by the tested history. |
| `OuterSeedCovarianceScale` | `100` | Scale the full `AtLastHit` covariance before the inward refit. |
| `ProcessSigmaWindow` | `3` | BH-mode integration window in component sigma. |
| `ProfileGridPoints` | `9` | Initial odd Simpson/profile grid size. |
| `ProfileRefinementIterations` | `6` | Refinements of the winning history's reported `z`. |
| `MinimumRetainedFraction` | `0.05` | Lower physical bound on retained fraction. |
| `MinimumRadiativeLogBayesFactor` | `3` | Evidence advantage required to publish radiation. |
| `CandidateIntervalIndices` | `[]` | Empty means all eligible intervals; otherwise restrict by radius-sorted interval index. |
| `SelectedEventIndices` | `[]` | Empty means all events; otherwise zero-based event-entry selection. |
| `VerboseDump` | `false` | Emit detailed hypothesis diagnostics. |

The audit also counted one input collection and 13 output collections. It
confirmed that the existing `RecGsfTracking` algorithm still exposes exactly
43 properties and that no maintained card schedules this new algorithm.
Because these are properties of a separate unscheduled algorithm,
`DumpGsfTrks/gsf.py.bk` was intentionally not modified.

The audit corrected README details for aliases, strict threshold behavior,
tie behavior, interval indexing, profile semantics, diagnostic infinities,
status/output alignment, and output data types. It also caught a missing
nonfinite guard for `BHSplitThreshold`; that validation was fixed before the
commit.

## Output and status contract

The algorithm writes successful tracks to `GlobalLossTracks`. Per-output
diagnostic collections are aligned through `GlobalLossInputTrackIndex`:

- `GlobalLossSelectedInterval`
- `GlobalLossSelectedMode`
- `GlobalLossRetainedFraction`
- `GlobalLossSelectedTX0`
- `GlobalLossLogLikelihood`
- `GlobalLossLogPrior`
- `GlobalLossLogPosteriorEvidence`
- `GlobalLossIdentityLogEvidence`
- `GlobalLossBestRadiativeLogEvidence`
- `GlobalLossRadiativeLogBayesFactor`

Identity outputs use interval `-1`, mode `-1`, retained fraction `1`, and
t/X0 `0`. `GlobalLossStatus` is aligned to every input track:

| Status | Meaning |
|---:|---|
| `0` | Successful output written. |
| `-1` | Event excluded by `SelectedEventIndices`. |
| `1` | Fewer than five associated hits. |
| `2` | Missing or unusable `AtLastHit` state. |
| `3` | Fewer than five successfully matched hits. |
| `4` | No valid hypothesis fit. |

Read the README for the precise likelihood/prior diagnostic conventions; they
are not interchangeable with the marginalized decision evidence.

## Exact focused validation

The final focused run used the normal EL9/LCG 105 environment, input
`trk-e--20-85-822751.root`, exact algorithm defaults, and only:

```python
SelectedEventIndices = [3, 4]
CandidateIntervalIndices = []
```

The relevant result was:

| Zero-based event | Truth classification | Truth pT | Identity refit pT | Best radiative history | Log BF | Published pT |
|---:|---|---:|---:|---|---:|---:|
| 3 | 6.425% Geant4 eBrem, truth-matched interval 5 | 35.813 | 33.5438 | interval 6, mode 2, `z=0.973585` (2.64145% loss) | +4.27347 | 34.3063 |
| 4 | no Geant4 eBrem | control | 48.7885 | false radiative alternative below gate | +0.380292 | 48.7885 identity |

This is a narrow mechanical success only:

- All inward hits can move the known-loss event toward truth.
- The explicit evidence gate preserves the clean identity in this control.
- The chosen radiative location is nevertheless adjacent interval 6, not
  truth-matched interval 5.
- The fitted loss, 2.64%, substantially underestimates the 6.425% truth loss.
- The result is not better than the best existing GSF result for this event.
- The clean event is protected by publication policy; the raw all-hit
  likelihood itself is not proven clean-safe.

The following changes did not resolve the interval/loss mismatch:

- `OuterSeedCovarianceScale=100` versus `1e6`
- `ElossOn=false` versus `true`
- default `CEPC2GeV85StepConditioned` versus
  `CEPCRuntimeGenericGrid5Clear`
- profile grid 9 versus 17 points
- candidate interval 5 only, intervals 5--7, or all eligible intervals
- direct profiled MAP selection versus marginalized evidence

Do not repeat that option sweep as the next action. It already shows that the
unresolved issue is not likely to be fixed by a coarse knob change.

## Mechanical and regression validation completed

- `RecGsfTracking` built and installed successfully in
  `build.105.0.0.x86_64-el9-gcc11-opt`.
- The longstanding KalTest warnings and ROOT PCM warning did not fail the
  build or run.
- The focused global-loss job finalized successfully and wrote the expected
  output/diagnostic collections.
- The ordinary `RecGsfTracking` split path was smoke-tested after the
  `mixture()` refactor with a temporary output override. It reported
  `Fitted: 1 / 1` and `Processed 4 events`; no stored output was
  overwritten.
- An early invocation failed only because `setup.sh` had not been sourced
  and `${DD4hepINSTALL}` was unresolved. Always source `setup.sh` before
  build or Gaudi execution.

These checks establish mechanical isolation and focused regression safety.
They are not population or physics validation.

## Current conclusion

The one-loss global-evidence idea is viable as a diagnostic: downstream hits
contain enough information to improve a known-loss state, and a separate
identity-vs-radiative evidence decision can shield one clean control. It does
not yet demonstrate correct global recovery. The unresolved fact is:

```text
event 3 prefers H(interval 6, mode 2, z=0.973585)
over the truth-compatible interval-5 history, including z_truth=0.93575
```

Until that crossover is localized, adding histories, tuning the gate, or
integrating this refitter would hide rather than solve the mechanism. The main
material/BH investigation is paused at this downstream-selection diagnostic,
not superseded by it.

## Exact next diagnostic

Start with committed source frozen and use a temporary card. The next question
is not which global option gives the best event-3 pT. It is:

```text
At which inward measurement, and through which innovation term, does the
interval-6 history overtake the truth-compatible interval-5 history?
```

For zero-based event 3, compare four fixed histories through exactly the same
ordered accepted hits:

1. identity;
2. interval 5 evaluated at truth retained fraction `z_truth=0.93575`;
3. the best profiled interval-5 BH mode/history;
4. selected interval 6, mode 2, `z=0.973585`.

At every reverse measurement, record separately for each history:

- accepted `deltaChi2`;
- `logDetInnovation`;
- `-0.5 * (deltaChi2 + logDetInnovation)`;
- cumulative measurement log likelihood;
- the discrete BH/history prior, kept separate from measurement evidence;
- surface/hit identity, detector region, residual, and innovation covariance.

Locate the first surface where interval 6 overtakes truth-compatible interval
5. At that spatial boundary, inspect DD4hep interval t/X0 and corresponding
Geant4 truth loss, then compare event 4 and at least one additional clean
matched control. Compare equivalent component states; a direction-only total
is not enough.

Classify the crossover before proposing a change:

- measurement residual/state transport;
- covariance or innovation-determinant normalization;
- discrete BH prior/response;
- collapsed interval placement/material ownership;
- or a later publication/selection artifact.

Prefer reusing `VerboseDump`. If implementation is truly needed, make the
smallest reversible diagnostic addition inside `RecGsfTracking`, validate
one focused event verbosely, and do not alter ready cards. If a configurable
property is added or changed, project law requires a dedicated sub-agent
option audit and synchronized authoritative documentation.

After branch-local closure, and only then:

- repeat hard-loss events 11, 16, and 17;
- run same-code held-out clean-track controls;
- return to the ordered invalid-coverage/material-BH checklist in `AGENTS.md`;
- consider a correction only if it predicts recovery without sacrificing the
  no-eBrem core.

Do not add multi-loss histories until the interval-5/6 crossover is understood.

## Explicit non-goals and safety boundary

The restart must not:

- change shared tracking packages;
- integrate the experimental refitter into maintained cards or workflows;
- tune the evidence gate on events 3 and 4;
- add multi-loss histories before local closure;
- reintroduce ECAL or truth-dependent production selection;
- claim validation from this two-event result;
- alter frozen production defaults;
- reset, clean, overwrite, or bulk-stage the dirty worktree;
- commit generated outputs;
- push, merge, or change branches without explicit user authorization.

## Fresh-session restart sequence

Read, in order:

1. `AGENTS.md`;
2. this recovery handoff;
3. `agents_record/2026-08-22-global-one-loss-evidence-refitter.md`;
4. `Reconstruction/RecGsfTracking/README.md`, section
   `Experimental global one-loss refitter`.

Then verify state without modifying it:

```bash
cd /aifs/user/data/zhangcg/gsfdev/CEPCSW
git branch --show-current
git log -3 --oneline --decorate
git status --short --branch -- \
  AGENTS.md \
  Reconstruction/RecGsfTracking/README.md \
  Reconstruction/RecGsfTracking/src/BetheHeitlerSplitter.cpp \
  Reconstruction/RecGsfTracking/src/BetheHeitlerSplitter.h \
  Reconstruction/RecGsfTracking/src/GlobalLossRefitter.h \
  Reconstruction/RecGsfTracking/src/GsfAlgorithm.cpp \
  agents_record/2026-08-22-global-one-loss-evidence-refitter.md \
  agents_record/2026-08-22-test-new-session-recovery-handoff.md
```

When implementation or execution resumes:

```bash
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt \
  --target RecGsfTracking -j4
cmake --install build.105.0.0.x86_64-el9-gcc11-opt
```

Use temporary cards and temporary output paths for focused diagnostics. Before
touching source, state the interval-5/6 likelihood question and inspect the
current diff so the fresh session continues rather than reconstructing or
broadening the investigation.
