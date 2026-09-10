# Deploy removal of the likelihood cross-check

The user confirmed "no batch for now", establishing a safe shared-library
deployment window after the pending-install status in the removal record.
No Condor submission or remote operation is part of this deployment.

Rebuilt RecBreakpoint and RecBreakpointLikelihoodTest through the configured
EL9/LCG105 build and installed the package into the normal InstallArea.
Build and installed plugin SHA256 agree:
c765a01ef13fadd2b26999e6dc015e7818a2950e2ab4d26ab5a0eccd717de95e.
The build retains existing clock-skew/ROOT configuration warnings; successful
compilation, generated configurables and explicit runtime checks determine
deployment validity, not the absence of warnings.

Both compiled numerical tests and all24 batch tests passed. The generated
configurable has no FreeLossCheckLikelihoods property; default SigmaLogLoss
remains0.001 and BackwardSeedScale100. Maintained card FreeLossFit remains ON.
The private-build gate preceding deployment is recorded in
2026-09-10-breakpoint-likelihood-check-removal.md.

Deployment regression scripts/results are under
TrackingPerformanceStudies/breakpoint_likelihood_check_deploy_20260910/.
They run the current maintained card against the installed library (no private
library prepended): seed2:68 normal and forced failure, seed12:11/16/17 normal.
Seed12:17 is a secondary control, not a clean physics validation sample.

The three installed runs/five track rows passed. All160 tuple fields and12,796
full verbose state/covariance records match the private build exactly.
The installed configurable property audit and built/installed library hash
comparison also passed. The forced-failure case retains the exact input-KF
fallback. This establishes mechanical deployment equivalence, not a new
performance or physics-validation result.

Existing frozen cards assigning the removed property must be regenerated;
existing tuples and historical prepared cards were not rewritten. Source,
physics defaults and the current objective/KF fallback were not changed by
deployment. Only RecBreakpoint build/install outputs and status records change.

The complete outgoing AGENTS status is preserved in
2026-09-10-agents-before-likelihood-check-deployment.md. Global status/laws and
essential commands are preserved; the current pending-install status is
replaced with the verified deployment result. No history directory was moved.
