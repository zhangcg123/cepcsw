# RecBreakpoint: optional iterated relinearization

## Authority and scope

User explicitly requested implementation after discussion of the equivalence
of local-marginal and persistent-6D fixed-linearization results. Base source
checkpoint: `9a851e8`, local branch `test_breakpoint`. Only RecBreakpoint,
its dedicated card, tests and documentation were changed. Existing GSF, shared
KF, maintained batch cards and remote branches were not changed.
The dedicated card remains an uncommitted workflow file under the project law.

## Implemented contract

`MaxFitIterations=1` is the compiled/card default and preserves the earlier
one-pass implementation. Values 2--20 enable iterated filtering/smoothing;
these require exactly one breakpoint, `LossStateMode=Persistent6D`, and
`BackwardMode=RTS`. `RelinearizationTolerance=0.001` is finite and positive.

The first pass is unchanged. Each later pass uses the previous smoothed
trajectory as its expansion reference, including the fitted static loss b.
Native geometry/material prediction supplies F and Q at that reference.
The loss map kappa_after=exp(b)*kappa_before is linearized at its reference
track/b state. A coordinate-only helix pivot change makes reference and live
states comparable. All predictions retain the affine offset:

```text
predicted = f(reference) + F(reference) * (live - reference)
Ppredicted = F(reference) * Plive * F(reference)^T + Q(reference)
measurement model = h(reference) + H(reference) * (live - reference)
```

Native KalTest Filter(), through a package-local site, performs the actual
5D/6D measurement updates. No independent handwritten production measurement
update was added. H_b=0, but track/b cross covariance still updates b.
The complete variable-dimension RTS pass is repeated, including the birth
boundary back to the upstream 5D trajectory and IP publication.

Every pass restarts from the same original seed prior before hit 0 and the
same independent Gaussian b prior at birth. The previous posterior covariance
never becomes a new prior. Each real hit contributes once to each newly solved
linearized problem. The loss is applied at the one configured interval only;
its coordinate is then transported unchanged except for measurement updates.
This is relinearizing the model, NOT recalibrating the prior distribution.

The stopping metric is the maximum absolute smoothed-coordinate change over
all hits, divided by that coordinate's previous smoothed standard deviation,
also including the fitted b change divided by its previous sigma. Phi
differences wrap. Stop below tolerance or at the pass limit. No damping,
line-search or positivity projection is implemented, and there is no guarantee
of a global optimum. The loss remains at the upstream measurement surface.

## Output and failure semantics

Automatic scalar fields: `one_pass_pt`, `fit_iterations`, `iteration_status`,
`iteration_error`. Automatic per-pass vectors: `iteration_pt`,
`iteration_log_loss`, `iteration_log_loss_variance`, `iteration_step_norm`,
`iteration_linearized_chi2`.

Status 0 means one pass; 1 converged; 2 reached the limit; -1 a later pass
failed and the last completed fit was retained, with a warning and error text.
An overall successful track status alone does not establish convergence.
The first trace step norm is zero by convention. The chi2 trace describes
each affine filter's innovation bookkeeping, not a shared nonlinear objective.
It may increase between passes and is not the selection/stopping criterion.
Full hit-by-hit state/covariance vectors and verbose dumps describe the final
completed pass; verbose output additionally prints all scalar iteration traces.
The published pT inherits EDM omega float storage, as in the previous code;
loss/state fitting uses doubles.

## Build and validation

EL9 target RecBreakpoint rebuilt and package subdirectory installed. Standalone
TransportTest passed, including loss-map finite differences, full-6D covariance
transport and new affine-offset/original-prior invariance arithmetic checks.
No shared installation rebuild was required. Existing ROOT PCM/geometry
warnings and filesystem clock-skew warnings remain visible in logs; they did
not prevent these runs.

Artifacts (uncommitted):
`TrackingPerformanceStudies/recbreakpoint_relinearization_2026-09-09/`.
`run_tests.py`, `jobs_regression.json`, the paired ROOT/log files,
`check_results.py`, `results_regression.json` reproduce this gate.
Controls and their logs are produced by `check_controls.py`.

Inputs: `gsf_doublebhoff_freshseed_diagnostic/trk-e--2.0-85-SEED.root`.
Zero-based event entries; matched ordered hit cell IDs, truth pT and stored
CompleteTracks pT were checked between each pair. Settings: FirstMiddleLast,
SeedScale=1, b prior mean=0/sigma=0.05, MS on, Eloss off, MaxChi2PerHit=1e100,
RTS/Persistent6D, max passes 1 versus 10, tolerance 0.001. Truth supplies only
the reference pT, never loss magnitude or iterative steering. The five
single-eBrem cases retain the previously audited truth-selected fixed intervals;
seed12 fixed interval 5 is a mechanical regression, not an assertion that all
its losses belong to this one interval.

All eight pairs have bit-identical one-pass pT and first-pass fitted b/variance
between the N=1 and N=10 runs. The final-pass audit checked full predicted,
filtered and smoothed covariance finiteness/symmetry/positive definiteness;
ordinary 6D covariance transport closure; identity transport/zero process
noise for b; b continuity between hits; static-b RTS consistency; and the
original birth mean 0 and variance 0.0025, not the preceding pass posterior.
All eight converged in 3--4 passes with no iteration failure.

Configuration controls also passed: MaxFitIterations=0, iterative empty-list
and iterative LocalMarginal requests fail initialization explicitly. A two-pass
seed12:11 run stops with fit_iterations=2 and iteration_status=2, retaining its
second-pass pT. Failure-retains-last semantics are implemented but no artificial
mid-iteration numerical failure was injected. Documentation/card steering was
checked for the new properties. The outgoing snapshot and unchanged complete
AGENTS section 1 were verified; both live section headings remain present.

Residual = 100*(pT_reco-pT_truth)/pT_truth. Momenta are GeV.

| Seed:entry | Interval | Truth pT | One-pass pT | Iterated pT | One-pass residual % | Iterated residual % | One-pass b | Iterated b | Passes |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 12:11 | 5 | 9.15107727 | 9.50620040 | 9.42493656 | +3.880670 | +2.992645 | +0.03810427 | +0.03576574 | 4 |
| 12:16 | 5 | 38.36070251 | 39.01869670 | 38.64070042 | +1.715282 | +0.729908 | -0.01338226 | +0.01003934 | 4 |
| 12:17 | 5 | 31.75560379 | 17.91896149 | 18.10227514 | -43.572285 | -42.995021 | -0.00761683 | -0.00617710 | 4 |
| 5:84 | 5 | 36.68881607 | 35.76436192 | 36.26543530 | -2.519717 | -1.153978 | -0.00752647 | -0.00425777 | 4 |
| 5:92 | 6 | 30.11129570 | 29.44185295 | 29.46069380 | -2.223228 | -2.160657 | -0.01595222 | -0.01489631 | 3 |
| 3:10 | 4 | 31.65271568 | 31.28193057 | 31.28410975 | -1.171416 | -1.164532 | -0.00431077 | -0.00446358 | 3 |
| 3:33 | 4 | 24.86041832 | 24.58591722 | 24.58915736 | -1.104169 | -1.091136 | -0.00029882 | +0.00003303 | 3 |
| 6:17 | 13 | 22.77745056 | 22.67851309 | 22.66701430 | -0.434366 | -0.484849 | +0.00473446 | +0.00420976 | 3 |

Seven selected cases reduce absolute residual; one worsens slightly. Negative
fitted loss remains in both seed5 negative controls and seed3:10. Seed12:17
remains severely underestimated. This is a mechanically successful opt-in
method, NOT evidence that relinearization solves the negative peak or that
the b prior/location is correct. No new clean-track population, secondary
control population, or held-out physics validation was run in this gate.

## Next questions

Inspect final-pass b/cross-covariance histories of the still-negative cases.
Keep linearization, prior assumptions, positivity and within-interval loss
placement as separate hypotheses. Before changing defaults, add categorized
population and clean-track controls. Do not turn fitted-b convergence into a
physics-validity claim or automatically feed truth into the fit.

The complete outgoing AGENTS status is preserved in
`2026-09-09-agents-before-relinearization.md`. The section 1 global status,
active laws and compile instructions are retained unchanged; section 2 is
replaced with this current focus. No history directory was moved or deleted.
