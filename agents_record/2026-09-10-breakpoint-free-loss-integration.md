# Optional normalized free-loss fit promoted into RecBreakpoint

## Request and scope

The user requested promotion of the current prototype into the standard
breakpoint package, with clear code and optional free fitting. Implemented on
local `breakpoint`, without changes to GSF/shared KF sources or their cards.
FreeLossFit defaults false to preserve existing campaigns; the user has not
answered the asynchronous question about choosing a different default.

The normalized-likelihood prototype, not the older chi2-only experiment, is
the promoted method. Its prior evidence remains in
`2026-09-10-breakpoint-normalized-free-loss-likelihood.md`.
No new remote action is performed. Generated ROOT files, plots, logs, runner
scripts and private binaries are not included in the implementation checkpoint.

## Maintained organization and behavior

- GaussianTrackModel is a data-only native affine model.
- KalmanAdapter captures native H/V/residuals without another measurement update.
- BreakpointFitter optionally captures native F/Q and pivot-offset bookkeeping.
  It remains the sole filter/RTS/backward implementation used in every trial.
- TrackLikelihood integrates the Gaussian trajectory variables, including
  covariance determinants and measurement normalization. Singular process noise
  uses supported square-root directions, not an inverse or invented variance.
- FreeLossFitter performs the blind bounded scalar Minuit2 search.
- FreeLossTuple records diagnostics, without fitting or likelihood logic.
- RecBreakpoint handles selection, fallback and the existing four collections.

The new Gaudi controls and defaults are FreeLossFit=false,
FreeLossMaxLogLoss=1, FreeLossMaxCallsPerStart=180,
FreeLossTolerance=0.001 and FreeLossCheckLikelihoods=false.
The dedicated card and batch helper support the corresponding BP_FREE_LOSS_*
controls. There is no new workflow stage or helper input. Package build files
link ROOT::Minuit2 and the isolated RecBreakpointLikelihood helper.

Free fitting supports one selected LocalMarginal interval. Each trial fixes
b=log(p_before/p_after), sigma_b=0, and evaluates the normalized marginal
likelihood of its native-forward frozen affine model. No truth amount enters
the search. Default bounds are [0,1]. The tested scan and three-start MIGRAD
strategy are preserved. Reversed-order QR and joint-smoothed SVD are optional
checks of the SAME likelihood, not alternative physics objectives. The current
backward refilter's copied seed is not used by this likelihood.

The ordinary pair is first obtained as a fallback. Empty intervals retain the
5D result; unsupported multiple intervals/Persistent6D and failed searches
retain the ordinary pair with explicit status/error fields. No intervals are
silently discarded. When applied, free fitting replaces the primary RTS and
backward pair. Native covariances remain conditional on b; the saved local
Minuit error is NOT injected. Boundary, scan-winner and non-converged results
are separately tagged. Applied does not certify convergence or physics accuracy.

TruthOverride remains the established positive-sigma prior-center comparison,
not a second free fit or a fixed-loss oracle. With override off, applied free
results are copied into its extra collections and explicitly tagged
CopiedFreeLikelihood with prior sigma0. Existing primary per-hit chi2 lists and
Track.chi2 retain their quadratic definitions; they are not replaced by -2logL.
The prior and free modes do not have identical uncertainty treatments.

All 28 free_loss_* flat branches are always present, including effective
steering, selected b/error/status/EDM/bounds, objective decomposition and aligned
trial arrays/errors. See the authoritative implementation guide:
`Reconstruction/RecBreakpoint/docs/free-loss-fit.md`.

## Mechanical validation

Artifacts: `TrackingPerformanceStudies/breakpoint_free_loss_integration_20260910/`.
The private plugin was compiled from the maintained sources and generated its
own Configurables, without overwriting shared build/installed libraries.
Source and binary hashes are in provenance.json. The maintained card was used
directly; the reference card merely omits the five new properties for the old
library. Tests are local, not Condor jobs.

Nineteen Gaudi job configurations completed: 12 integrated configurations and
seven old-library reference configurations. Coverage includes both loss-state
modes, enabled/disabled free fitting, truth on/off, empty/manual/multiple
intervals, sigma0.001/0.05 and BackwardSeedScale100.

Disabled/fallback comparisons preserve all104 legacy flat fields on ten
checked rows across eight comparisons. Full verbose covariance dumps agree
exactly wherever whole files were compared. One comparison selects only the
12:16 row from a mixed free-fit job; its fields are exact and its full ordinary
dump is covered by the free-off whole-file comparison.

Integrated free fits reproduce all3,724 prototype full-state/covariance lines
EXACTLY: 928 for2:68, 1,864 for12:11/17 and932 for4:11. This includes predicted,
filtered, smoothed and backward states. Their selected b, objective and endpoint
pT also agree. Every valid checked trial satisfies three-formulation agreement
within1e-4. The scan-winner lower-bound secondary control12:17 is retained and
tagged, not counted as a clean validation track.

Important correction to earlier prose: in this exact stored input and accepted
hit selection,12:16 has NO selected truth interval. It is a no-interval control,
not a multiple-interval gate. Explicit Manual=[4,5] on2:68 independently verifies
the multiple-interval fallback. The event labels and parsed logs are retained.

The first analysis check incorrectly matched backward_predicted as predicted;
a word-boundary fix removed this parser error. It did not require any change to
the fitter. The initial expectation of unsupported status for12:16 was also
corrected to NoInterval after inspecting the actual selected intervals.

Additional checks passed:

- Independent dense Gaussian reference for zero and rank-one process noise,
  including all three likelihood formulations and malformed empty-model rejection.
- Standalone RecBreakpointTransport CTest (1/1).
- Dedicated batch-helper unit tests (23/23).
- Privately generated Gaudi configurable defaults, including BackwardSeedScale100.
- CMake discovery of Eigen3::Eigen, ROOT::Matrix and ROOT::Minuit2.
- git diff --check.

The audit must load the private generated RecBreakpointConf explicitly: the
standard build/run environment otherwise prioritizes the old shared config.
The actual event runner prepends private paths after initializing its environment.
The standalone CMake build emitted filesystem clock-skew warnings; its fresh
test binary built and the test passed. No shared package rebuild was performed.

## Deployment status and next action

Source integration and private verification are complete. Shared build-tree
and installed plugins have NOT been replaced. The user has not yet confirmed
whether batch jobs are using them; the site queue client fails with missing
Python htcondor, so no idle-queue assertion can be made.

Do not submit the updated card against the old installed plugin. Once the user
confirms jobs are idle, use the normal CEPC environment to build/install the
RecBreakpoint target, audit installed properties and run a focused installed-card
test. Only then submit BP_FREE_LOSS_FIT=1 through the dedicated workflow.

This promotion is not population/physics validation. The empirical hit-derived
seed, local affine model, conditional covariance and selected-event residual
tails remain limitations. The old100-event chi2-only study must not be relabeled
as a normalized-likelihood population study.

## Project-memory migration

The project-status-curator workflow preserved the complete outgoing AGENTS.md
in `2026-09-10-agents-before-free-loss-integration.md` before replacing its focus.
Mapping: Introduction/global status, project laws, compile/run subsections are
retained byte-for-byte; the outgoing Current focus is preserved in that snapshot
and replaced with integrated status, limits and deployment next steps.
Two substantive headings remain. No history directory was moved or renamed,
so no history-move manifest is required. No historical files or evidence were
deleted, and no legacy-directory links were introduced.
