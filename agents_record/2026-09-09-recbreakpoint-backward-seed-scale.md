# RecBreakpoint backward seed covariance scale

## Request and scope

The user requested a configurable backward seed covariance scale and asked
whether RTS starts from the forward endpoint or runs a new filter. Changes
are confined to RecBreakpoint, its dedicated card, and documentation on local
test_breakpoint. No GSF/shared KF source, maintained batch card or remote
operation is included. Existing unrelated edits are preserved.

## Control and precise seed contract

New Gaudi property: `BackwardSeedScale`, compiled/card default1. Values must
be finite and strictly positive. Initialization rejects zero, negative, NaN
and infinity before creating a flat output; the fitter also validates its
internal settings. The card exposes `fit.BackwardSeedScale` and optional
`BP_BACKWARD_SEED_SCALE`. Every flat row saves `backward_seed_scale`.

For N ordered hits and scale s:

```text
x_backward_seed = x_first_forward_updated[N-1]
P_backward_seed = s * P_first_forward_updated[N-1]
```

P is the complete 5x5 track covariance, including off-diagonal terms. The mean
is unchanged; standard deviations scale by sqrt(s), correlation coefficients
are unchanged. Persistent6D supplies its 5D track marginal here, not a sixth
loss-coordinate backward seed. No new prefit is constructed. The outermost
hit remains a copied seed without a repeated hit update, so its backward
local chi2 is0 and the first inward update is at N-2.

`finishBackward` copies the original forward endpoint before scaling it;
the forward buffers and RTS never change. Scale1 skips multiplication and
extra covariance normalization to preserve existing arithmetic exactly.
Other scales retain covariance validation. Scaling may overflow for extreme
finite numbers, in which case ordinary covariance validation fails the fit;
positive finite input is not a promise of a numerically usable covariance.

Inward relinearization always passes the frozen original first-forward fit
back to finishBackward. Each pass applies s once to a fresh copy; it does not
multiply an already-scaled or inward-updated covariance. The independent
breakpoint loss prior is not scaled by this control.

The existing `SeedScale` remains a separate control of the loose initial
forward prefit covariance. This change does not make backward independent of
forward evidence and does not solve evidence reuse by itself.

## RTS starts from the forward endpoint, not from scratch

For the current forward pass, its terminal condition is:

```text
x_RTS[N-1] = x_forward_updated[N-1]
P_RTS[N-1] = P_forward_updated[N-1]
```

RTS uses the saved forward predicted/updated states, covariance and transition
matrices to recurse inward. It does not start a fresh backward measurement
filter and never reads BackwardSeedScale. Only the explicitly enabled outer
Persistent6D relinearization loop repeats the forward-fit/RTS cycle with
original priors and new expansion references. That is not a fresh RTS seed.

## Native no-breakpoint reference

VerifyKFReference still computes both unchanged native reference endpoints.
Its backward reference uses an unscaled forward endpoint. Therefore the
empty-list backward equality gate applies only at BackwardSeedScale=1. At
other scales the unscaled native value is still saved as a comparison;
forward/RTS equality checks remain active. This avoids incorrectly failing a
deliberately rescaled fit against a different native setup.

## Verification and evidence

Package-only build/install succeeded; generated RecBreakpointConf.py exposes
BackwardSeedScale=1. Built and installed libRecBreakpoint.so SHA256:
`e242662d8093b71e50f2d652b21a13c021e0e8cd9688b31fdc415d281e7e774a`.

Artifacts are under
`TrackingPerformanceStudies/recbreakpoint_backward_seed_scale_2026-09-09/`:
run.py, check.py, jobs/results JSON, per-job ROOT and verbose logs. These are
uncommitted experiment outputs. The old regression references are under
`TrackingPerformanceStudies/recbreakpoint_parallel_2026-09-09/`.

Initial smoke: seed2 entry68, interval7, Persistent6D, one pass, b prior0 with
sigma0.05, FirstMiddleLast seed, MS on/Eloss off, backward scales1 and100.
Both pass full covariance/mean checks and exact scale1 regression. Across
scales the monitored forward and RTS means/covariances/chi2 are identical.

| Scale | RTS pT GeV | Backward pT GeV | Forward chi2 | Backward chi2 | RTS complete chi2 |
|---|---:|---:|---:|---:|---:|
| 1 | 44.97146298 | 44.99156384 | 416.80484015 | 414.81586593 | 416.74617905 |
| 100 | 44.97146298 | 44.96104723 | 416.80484015 | 414.73321384 | 416.74617905 |

All four initialization-negative jobs (0, -1, NaN, infinity) fail with the
positive-finite validation error and create no flat output.

The broader gate passed 23 additional paired cases: seed12:11/16/17 at
scales1/100 in each of Persistent6D, LocalMarginal and TruthOverride (18),
seed12:11 with Persistent6D iteration limit10 at scales1/100 (2), seed12:0
empty-list at scales1/100 (2), and seed12:11 at scale0.5 (1). Together with
the smoke, 25 paired cases cover five unique input entries. Seed12:17 is
secondary-activity CONTROL, not a clean optimization example.

For every case, full verbose 5D/6D covariance dumps are finite, symmetric and
positive definite. The backward seed mean equals the original forward seed
mean exactly, and every seed covariance element equals s times its original
forward value (relative tolerance2e-14). Iterated cases compare against the
FIRST-pass forward matrix, not the iterated RTS forward endpoint, establishing
that scaling is not compounded or sourced from the wrong pass. Backward's
last-hit chi2 remains0. All three hit lists have the right length and sum to
their scalar totals; smoothed-score status is valid.

All monitored scale1 pT, covariance vectors, loss estimates and chi2 values
match previous paired outputs exactly. Across the same-code scale variations,
all monitored forward/RTS values remain exactly unchanged; only backward
quantities change. These are mechanical checks, not a population performance
claim. For seed12:11, the iterated RTS pT stays9.4249365645 GeV, while the
backward pT is9.4252685831 at scale1 and9.4243487622 at scale100. Both backward
fits use3 passes, while the unaffected RTS uses4. The larger scale must not
be promoted as a physics default based on this selected comparison.

## Knowledge preservation

The complete outgoing AGENTS.md is preserved in
`2026-09-09-agents-before-backward-seed-scale.md`. Section1 (global status,
all active laws and compile/run commands) is unchanged. The current-focus
seed description is replaced in place; no historical directory was moved
or removed. No directory-migration manifest is needed. The README and the
self-contained mathematical walkthrough now distinguish this backward scale
from the unscaled RTS boundary condition.
