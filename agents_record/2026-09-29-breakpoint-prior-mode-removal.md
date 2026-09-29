# Remove the experimental primary breakpoint loss-prior modes

## Request and scope

On 2026-09-29 the user resumed the project and asked to simplify RecBreakpoint,
starting by removing LocalMarginal Unconstrained and LocalMarginal Fixed.
This is a removal, not a replacement optimizer or a physics improvement claim.
The working branch remains breakpoint. No remote operation or shared-library
installation is authorized by this change.

## Removed and retained contracts

- Removed the public LossPriorMode property and its FitSettings member: only
  the Gaussian primary fit remains, in LocalMarginal and Persistent6D forms.
- Removed the diffuse regression implementation, its dedicated numerical test,
  transient response/covariance fields, and the public Fixed dispatch.
- Removed loss_prior_mode and the ten unconstrained_* flat branches. Existing
  files are untouched and remain interpretable through the archived contract.
- Removed BP_LOSS_PRIOR_MODE from batch forwarding. Both the standalone card
  and batch preparation reject any supplied value before running/preparing jobs.
  Direct old Gaudi cards assigning LossPriorMode fail as an unknown property;
  regenerate such cards rather than silently changing their physics steering.
- Retained the Gaussian MeanLogLoss/SigmaLogLoss controls, both endpoint
  continuations, both state representations, interval selection, and all six
  endpoint output slots.
- Retained FreeLossFit, including its INTERNAL sigma_b=0 conditional trials,
  normalized RTS objective, bounds, failure fallback and ordinary-copy outputs.
  This is not the removed selectable Fixed primary workflow.
- Retained TruthOverride as a separate adjustable truth-centred Gaussian prior
  with the same configured sigma; it is not a fixed-truth oracle.

The retained C++ implementation and package CMakeLists are exactly the versions
at 8220a0e, immediately before experimental commit 145c405. Card retirement
guards, regression tests, documentation and current status are updated rather
than reverted. No GSF/shared KF files or unrelated user edits are changed.

## Verification

27 batch/card tests pass, including rejection of all retired environment values,
unchanged Gaussian settings, preservation of both LossStateMode choices, and
explicit steering of all 29 maintained configurable properties.

Focused direct before/after gates use existing tracker inputs, not regenerated
simulation or another fitting source tree:

| Configuration | Seed:entry | Additional outputs tested |
|---|---|---|
| LocalMarginal, truth-selected interval | 2:68; 12:11,16,17 | FreeLossFit on, TruthOverride on |
| Persistent6D, truth-selected interval | 12:11,16,17 | FreeLossFit off/copies, TruthOverride on |
| LocalMarginal, Manual=[5,7] | 12:11 | Multiple intervals; free copies and truth-centred pair |

All use sigma=0.001, SeedScale=1, FirstMiddleLast, BackwardSeedScale=100,
MSOn=true, ElossOn=false, VerboseDump=true and VerifyKFReference=true.
The 12:17 rows are secondary-activity controls, not clean-track performance.

The package rebuild and both retained CTests pass: RecBreakpointTransport and
RecBreakpointLikelihood. All eight event/configuration rows have status=1.
Every one of the 160 retained tuple fields is bit-identical before/after,
including loss estimates, uncertainties, hit identities, six endpoint results,
free-loss diagnostics and per-hit chi2. The only removed fields are the eleven
retired prior-mode/diffuse diagnostics listed above.

All verbose state/covariance records are exactly equal:

| Job | Equal verbose records |
|---|---:|
| LocalMarginal seed 2 | 2,784 |
| LocalMarginal seed 12 | 8,388 |
| Persistent6D seed 12 | 9,744 |
| Manual multiple intervals seed 12 | 2,796 |
| Total | 23,712 |

The build reported pre-existing NFS clock-skew and ROOT PCM lookup warnings.
Compilation/linking of the changed files is present in build.log; actual
runtime comparisons above pass. This is a removal/regression gate, not a new
population-performance or physics-validation result.

Temporary commands, logs, tuples and comparison script are confined to
/tmp/recbreakpoint-remove-priors-20260929.ETxgpP/ and are not committed.

## Deployment

The rebuilt library is byte-identical to the existing installed pre-experiment
Gaussian-only library. Both have SHA256:
7ed0302ff8f490f6e59c452ac3e12fda21615556bb9f4c230977b3463f574540.
The regenerated and installed RecBreakpointConf.py files differ only in their
generated timestamp comment. Neither exposes LossPriorMode. Thus the maintained
card and both schemas agree without overwriting any installed library or config.
No install, remote operation, batch submission or existing-output cleanup was
performed. The changes are checkpointed locally; generated artifacts are excluded.

## Historical preservation

The full pre-edit AGENTS.md is preserved in
2026-09-29-agents-before-breakpoint-simplification.md. The removed mathematical
contract is preserved verbatim, with a historical banner, in
2026-09-29-retired-unconstrained-loss-contract.md. The original experimental
source/test files remain recoverable from commit 145c405.
The detailed 2026-09-13-unconstrained-breakpoint-loss.md is unchanged.
Its unresolved native/captured loss-response discrepancy was not resolved by
this removal and must not be presented as an established detector limit.

Only AGENTS.md section 2 was replaced. Section 1, all active project laws,
scope restrictions and compile/run instructions remain unchanged. No history
directory was moved, so a directory-migration manifest was not applicable.
