# Common native IP propagation for RTS and backward endpoints

The user asked what RTS stored transitions mean, how the shared optimized loss
is used, and explicitly requested synchronizing both final IP operations to
native MarlinTrk propagation. Changed only the RecBreakpoint package and its
documentation; no shared KF/GSF source or run-card settings changed.

## Explanation checked against code

Transitions are buffered within the current forward pass, not loaded from an
old event or tuple. Each holds the next predicted state/covariance, full
propagation/loss Jacobian F, process noise Q and adjacent cross covariance
C=P_filtered*F^T. At selected intervals it also holds loss cross covariance
and loss-prior parameters. These permit the RTS correction with
G=C*inverse(P_predicted_next):

```text
x_smooth[i] = x_filtered[i]
            + G[i] * (x_smooth[i+1] - x_predicted[i+1])
```

The state is native (drho,phi0,kappa,dz,tanl), with wrapped phi differences.
This is the RTS recursion of the current linearized Gaussian model, using
the retained correlations rather than rerunning measurements inward.
The covariance is updated in the equivalent conditional/Joseph form.

There is one optimized b=log(p_before/p_after), found by the outer blind
Minuit search over normalized full-track likelihood. No truth loss amount
or ordinary fitted loss sets it. Every trial fixes b with sigma_b=0 in the
shared fitter. Both published free-loss endpoints use the same selected b.
Outward curvature is multiplied by exp(b); RTS uses the resulting stored
forward transition, without adding a second loss. The backward refilter
instead transports inward and multiplies curvature by exp(-b) at the selected
upstream surface before its hit update. Its copied/scaled forward seed and
fresh inward measurement updates remain distinct from RTS smoothing.

## Narrow implementation change

- LocalMarginal and Persistent6D RTS now call KalmanAdapter::propagateToIP,
  exactly as finishBackward already did.
- Removed the unused RTS-only atIP declaration/implementation, which had used
  THelicalTrack::MoveTo without native material propagation.
- Generalized the native error label from Backward KF to Native KF IP propagation.
- Both initialize a native MarlinTrk track from their own innermost state and
  invoke propagate((0,0,0)); the result is labeled AtIP.

This applies to ordinary, free-loss and truth-prior pairs. It honors the existing
MSOn/ElossOn switches, introduces neither a beam-spot measurement nor an extra
breakpoint, and leaves the stored hit states, chi2 definitions, likelihood model,
loss priors and seed controls unchanged. The two endpoint estimates remain
different; sharing the final propagation does not make them identical.

## Build/install and mechanical gate

The preceding user confirmation of no running batch jobs remained applicable.
The configured RecBreakpoint build and package-only install passed. CMake/Make
still reported filesystem clock skew. Build and installed plugin SHA256 agree:
`334c928e555eee5ad883e3a49575aa017381ad8d7f7faf87976a19f52aee487d`.

Artifacts: TrackingPerformanceStudies/breakpoint_common_native_ip_20260910/.
Ten local jobs repeat the preceding separate-pair regression using the maintained
card, with a test-only PodioOutput wrapper: free on/off seed2:68, free on/off
seed12:11/16/17, truth off with free off/on, Persistent6D fallback, multiple
Manual intervals, empty intervals and one invalid interval. Defaults for the
comparison are sigma0.001, BackwardSeedScale100, FirstMiddleLast, MSOn=true and
ElossOn=false. Truth-prior and copied outputs are covered. These are not Condor
jobs; no population campaign was submitted.

All ten completed:14 attempted rows,13 successes and one expected failure.
Seed12:17 remains a secondary-activity control;12:16 has no selected interval
in the actual accepted-hit input. check.py and verification.json establish:

- All36,936 verbose per-hit means/covariances are EXACTLY unchanged.
- All164 flat fields are compared. The ONLY changed fields across these jobs
  are free_loss_rts_ip_parameters/covariance and
  truth_override_rts_ip_parameters/covariance.
- ALL endpoint pT values, optimized b, objective/trial records, fit statuses,
  all local/total chi2s and backward quantities remain EXACTLY unchanged.
- Serialized extra ordinary-copy tracks still match ordinary EDM payloads,
  including all three states/covariances, chi2, NDF and hit references.
- Both compiled numerical CTests pass (2/2); batch unit tests pass (23/23).

Example free RTS pT before/after (GeV):2:68=45.03330015623118,
12:11=9.125114474780005,12:17=18.216186412586413. These values are unchanged;
the change is to IP transport/uncertainty consistency, not demonstrated pT
resolution improvement. The ordinary IP covariance is not a legacy standalone
flat branch; the copied FreeLoss/Truth outputs and serialized tracks expose it.

## Documentation and repository

README and free-loss-fit.md now explain RTS storage, the shared selected b,
directional loss mapping and the common native IP operation. No Gaudi property
was added/removed and no steering adjustment is required.

The project-status-curator workflow preserved the complete outgoing AGENTS.md
in 2026-09-10-agents-before-common-native-ip.md before replacing the current
IP/deployment summary. Global introduction, laws and compile/run instructions
remain byte-identical. Two substantive sections remain; no history directory
moved and no unique evidence was deleted. Earlier geometric-RTS outputs remain
historical references, not relabeled as native-propagated output.

Only source/headers and documentation are checkpointed. Generated ROOT files,
logs, private test wrappers and binaries remain uncommitted. Unrelated user
edits to gsf.py.bk, sim.py.bk and subtrkjobs.sh are preserved. No remote operation.
