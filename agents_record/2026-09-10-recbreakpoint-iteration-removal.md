# Removal of RecBreakpoint repeated relinearization

User request: remove the existing relinearization iterations from the run card
and source before considering a different free-loss optimization. This change
does not implement a free-b fit, shift any loss prior, or alter GSF/shared KF.

## Removed

- Gaudi MaxFitIterations and RelinearizationTolerance, corresponding FitSettings.
- fitIterated and the optional reference-pass branches in fitPersistent and
  finishBackward; one forward pass now feeds one RTS and one backward result.
- KalmanAdapter seedRelinearized, both advanceRelinearized overloads,
  advanceBackwardRelinearized, and iteration-only affine update/pivot helpers.
- Iteration history/state members and 19 flat fields: one_pass_pt,
  fit_iterations, iteration_{error,linearized_chi2,log_loss,log_loss_variance,
  pt,status,step_norm}; backward_fit_iterations and the seven analogous
  backward_iteration_* fields; truth_override_{rts,backward}_fit_iterations.
- The two run-card assignments and supported batch BP controls. Stale
  BP_MAX_ITERATIONS / BP_ITERATION_TOLERANCE now fail explicitly in the card
  and batch preparation rather than being ignored.

Existing ROOT files, generated campaign cards and plot outputs were not
rewritten. Regenerate prepared cards assigning the removed Gaudi properties.
The dedicated maintained run_breakpoint.py is updated locally but remains
untracked under the project's run-card policy. Existing tracked batch helper
and tests are included in the focused change. User-owned sub/dump scripts,
GSF cards and Reconstruction/CMakeLists.txt changes are preserved untouched.

## Preserved

Ordinary and truth-prior RTS/backward collections, their row maps and statuses,
all endpoint pT and extra IP parameters/covariances, fitted losses/variances,
per-hit states including persistent6D, the three independent per-hit chi2
lists/totals, and smoothed-score decomposition. The ordinary KF Jacobians and
passive affine measurement score are required by the one-pass fit and remain;
they are not repeated relinearization. Priors and uncertainty are unchanged.
LocalMarginal still supports multiple intervals; Persistent6D still permits
at most one. Empty intervals use the 5D reference. No new positivity bound.

BackwardSeedScale scales a copy of the pair's forward endpoint covariance
once, without affecting RTS or the loss-prior sigma. TruthOverride still
changes only selected prior centers at the shared positive sigma; it is not
the historical fixed-loss oracle. Copied off/empty outputs are unchanged.

## Direct regression gate

Artifacts: TrackingPerformanceStudies/breakpoint_iteration_removal_20260910/.
Before runs used the installed pre-removal library and an exact snapshot of
the maintained card. All before runs completed before the rebuild. After runs
used the rebuilt/installed package and current card, with identical physics
settings and inputs. No Condor submissions were made for this gate.

Nine configurations, 13 track rows per side (18 local jobs total):

| Case | Seed:entries | Controls beyond default test sigma0.001 |
|---|---|---|
| smoke | 2:68 | Truth selection, LocalMarginal, truth override on |
| hard | 12:0,11,16,17 | Same; 17 is secondary-activity control |
| multi | 2:25,30 | Multiple truth-selected intervals |
| persistent | 2:68 | Persistent6D |
| off | 2:68 | TruthOverride=False |
| empty | 12:0 | Manual empty intervals |
| zero | 12:0 | Manual interval5, no assigned loss |
| backward100 | 2:68 | BackwardSeedScale=100 |
| sigma05 | 2:68 | SigmaLogLoss=0.05 |

Both sides enabled VerboseDump and VerifyKFReference. Every retained flat
field (104) is exactly equal, including strings, scalar/vector values, four
endpoint momenta, covariance/parameter arrays, chi2 lists and loss posteriors.
Equal NaNs are treated as equal. The only schema difference is the expected
19 removed fields. All 21,819 verbose full-state/covariance lines match
exactly. These 13 rows cover seven unique seed/event keys under different
controls; they are not 13 independent physics events.

Installed configurable inspection confirms the retired properties are absent
and retained prior, mode, truth-override and seed controls remain. Exact
row results and schema are in regression_summary.json; check.py verifies the
comparison and logs are retained. These are mechanical regression checks,
not additional population physics validation.

Build/install of RecBreakpoint passed. The build emitted existing filesystem
clock-skew warnings; explicit installed-property and direct-run checks verify
the new implementation. All 21 Python batch tests passed, including rejection
of retired controls. The submission-sigma plumbing test now normalizes only
its temporary fixture's default to avoid depending on the user's campaign
sigma; the actual submission script was not changed. The standalone C++
transport/covariance test passed with /usr/bin/g++. An initial standalone
attempt through system ccache could not write /run/user/...; the successful
direct-compiler build avoids that environment restriction.

## Memory preservation / migration map

Global status, active laws and compile/run commands in AGENTS.md were retained
verbatim. The Current focus section was replaced, not appended. All outgoing
detail is preserved in these complete snapshots:

- 2026-09-10-agents-before-breakpoint-iteration-removal.md
- 2026-09-10-breakpoint-readme-before-iteration-removal.md
- 2026-09-10-breakpoint-walkthrough-before-iteration-removal.md

Current option/schema documentation and the fixed-linearized walkthrough now
describe one-pass operation. Historical iteration gates remain historical;
no history directory was moved or deleted, so no migration manifest is needed.
No remote operations or branch changes. Free-b/profile-objective optimization
remains a discussed proposal, not an implemented or validated capability.
