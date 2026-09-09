# TruthOverride now supplies a prior center with the shared sigma_b

## User contract and scope

The user explicitly rejected fixing b with zero variance. Truth assignment
must use the same breakpoint fitter and sigma_b as the ordinary fit, changing
only b's prior center. This supersedes the fixed-loss oracle described in the
2026-09-09 records. Those records and existing ROOT outputs are historical;
do not reinterpret their sigma_b=0 resolution or chi2 results as the revised
method. Full outgoing AGENTS/README snapshots are preserved alongside this
record as 2026-09-10-agents-before-truth-prior-center.md and
2026-09-10-breakpoint-readme-before-truth-prior-center.md.

Changes are confined to RecBreakpoint implementation/tests, its dedicated
local card, and project documentation. Shared KF/GSF code, GSF cards, batch
scripts and user edits remain untouched. No remote operations or batch
submissions. The dedicated card remains untracked under the run-card policy.

## Implementation

RecBreakpoint copies the ordinary FitSettings and populates generic
intervalMeanLogLoss entries with b_truth=-log(z_truth). It does not change
sigmaLogLoss, lossStateMode, maxFitIterations, seeds, hit selection or material
settings. The fitter has no TruthOverride-specific loss-state mode. Its common
intervalSettings resolver selects a per-interval center when supplied, otherwise
the global MeanLogLoss. It leaves SigmaLogLoss unchanged. Generic centers are
validated against selected indices and finite nonnegative values.

The resolver serves local outward loss birth, backward loss birth including
relinearized passes, and persistent-6D birth including its RTS iterations.
Both calls use the same BreakpointFitter::fit. LocalMarginal retains its
one-pass RTS/inward-only iteration behavior; Persistent6D retains both iterated
continuations. Each pair freezes its own original forward seed for inward
iterations. All existing restrictions on interval counts and iterations apply
equally. No positivity restriction on the fitted posterior is introduced.

TruthOverride remains default-on. It still uses exactly the selected intervals,
and never adds unselected losses. A Manual interval with zero truth loss gets
b_prior=0 with the common positive variance, not a loss locked at zero. Disabled
or empty-list extra results remain copies; invalid truth does not substitute
a guessed prior. The existing relation-driven truth reader is unchanged.

Same sigma_b means the same loss-prior uncertainty. It does not require equal
predicted covariance matrices (the Jacobian depends on the center), nor equal
posterior sigma_b after the measurements.

## Tuple distinction

- truth_override_loss_treatment = "PriorCenter" identifies the new contract.
- truth_override_prior_sigma_log_loss saves the shared configured sigma.
- truth_override_log_loss remains the matched truth INPUT.
- truth_override_{rts,backward}_fitted_log_loss and the corresponding
  _fitted_log_loss_variance vectors save the adjustable fitted posteriors.
  They align with breakpoint_interval, including disabled copies.
- truth_override_{rts,backward}_fit_iterations records the actual passes.
- Existing four endpoints, IP covariances, three chi2 lists/totals and statuses
  remain. Result status2 now identifies an active truth-centered fit; absent
  treatment metadata in historical files means the old fixed-loss contract.

## Verification

Package build and package-only install passed. Build and installed library
SHA256 agree:
6a421ff7f5b9e3e88f0f914079033abaa632e0a3788086ceb35817374a363359.
The standalone C++ transport test passes, including same-positive-sigma loss
transport at centers 0, 0.03 and 0.4. All 19 batch helper tests pass.

Focused ROOT/logs and reproducible run/check scripts are under the uncommitted
TrackingPerformanceStudies/breakpoint_truth_prior_center_20260910 directory.
Smoke and gate checks passed: ordinary output branches exactly reproduce old
references and same-code override-off results for seed2:68, seed12:0/11/16/17,
and multi-interval seed2:25/30. Seed12:17 is secondary activity control, never
counted as a clean optimization example. Verbose full endpoint/covariance
dumps are enabled. Per-hit chi2 sums, IP covariance checks and finite positive
loss posterior variances pass. Empty-list outputs remain exact copies.

Additional gates cover LocalMarginal/Persistent6D, one/three iterations, sigma
0.02/0.05/0.1, BackwardSeedScale100 and a manually selected no-loss interval.
The no-loss interval has prior b=0 but updates to b=0.00213933; it exactly
matches the ordinary zero-centered-prior result instead of being locked.

For seed2:68, both priors have sigma_b=0.05. Truth supplies b=0.0320917542;
the extra RTS fits b=0.0414792164 with variance0.000101658762, showing that
truth is a prior center and the loss remains adjustable. Ordinary RTS/backward
pT remains 44.97146298/44.99156384 GeV, while the new truth-centered pair gives
45.02188003/45.03333297 GeV. The old fixed-b result was
44.67695907/44.69090711 GeV and is not expected to reproduce under new semantics.
This is a mechanical check, not evidence of improved physics performance.

An exact equivalence check compares each truth-assisted result to an ordinary
Manual fit supplied the identical prior center/sigma under the same mode,
iteration and seed controls. All seven cases pass: LocalMarginal one/three
iterations, Persistent6D one/three iterations, sigma0.02/0.1, and backward
scale100 (the first four use sigma0.05, scale1). Endpoint pT, IP covariance,
fitted b/variance, all per-hit score lists and their totals, and iteration
counts match exactly. There were 20 local jobs / 28 selected-event
configurations in total. No population performance claim is made.
The equivalence check also matches every verbose per-hit RTS/backward 5D
state and full covariance dump exactly against the corresponding manual-prior
run. The AGENTS global introduction, active laws/scope and compile section
were verified byte-identical to the outgoing snapshot; only current focus
changed. No historical directory move or manifest was needed.
