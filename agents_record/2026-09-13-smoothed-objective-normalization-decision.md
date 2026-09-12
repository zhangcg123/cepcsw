# Retain normalization in the proposed smoothed objective

The user stated that the log-determinant and normalization terms should remain.
The agreed target for a future explicit RTS-based objective evaluation is:

```text
J(b) = complete smoothed chi2(b)
     + log det S_all(b)
     + M log(2*pi).
```

Complete smoothed chi2 contains measurement, propagation and seed contributions
from the RTS-smoothed trajectory using the original noise covariances. S_all is
the same full joint measurement covariance used in the existing likelihood.
It is NOT an RTS state covariance. M counts all measured coordinates. The
constant M log(2*pi) is retained even though it does not move the minimum with
an unchanged hit set. Dropping log det S_all was an assistant proposal, not a
necessary consequence of using RTS states, and is superseded by this decision.

The source still evaluates the chi2 through the captured forward-built Gaussian
model and already includes both terms. Minuit does not yet consume
smoothedTotalChi2 directly. No source, cards, build/install, batch jobs, produced
data or remote state changed in this documentation-only step. Reference-trajectory
iterations stay removed. Same-model algebraic equivalence concerns complete
sums, not natural layer-by-layer smoothed contributions or an unrestricted
native nonlinear tracking claim.

Used project-status-curator to preserve the complete outgoing AGENTS.md in
2026-09-13-agents-before-smoothed-normalization-decision.md and update only the
current-focus decision. All global status, laws and compile instructions remain
unchanged; no history directory was moved. git diff --check passed.
