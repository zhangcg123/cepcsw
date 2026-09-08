# Optional backward-filter relinearization

## Request and scope

User requested applying iterations to BackwardFilter after the RTS iterative
comparison. Base checkpoint `fb3281e`, local `test_breakpoint`. Implementation
and dedicated-card comments remain within RecBreakpoint. No GSF/shared KF,
maintained batch card, or remote change. Dedicated card remains uncommitted
under the existing project law.

## Deliberately distinct workflow

Select `LossStateMode=LocalMarginal`, `BackwardMode=BackwardFilter`, one
`BreakpointIntervals` entry, and `MaxFitIterations>1` (up to 20). Default 1
retains the old forward/backward fit. Persistent6D still supports RTS only;
the extension does NOT introduce persistent-6D inward propagation.

The ordinary initial forward pass is run once. Freeze its complete terminal
posterior mean and covariance as the inward seed. The first backward pass is
unchanged. Each additional pass starts again from that frozen seed, with the
same independent configured b prior, and revisits N-2 through hit 0. The last
hit is not updated again. Earlier inward posteriors never become new seeds.
Thus this iterates only the inward filter, not the outward fit. It preserves
the original outward-evidence reuse, and is not an independent smoother.

Reference positions are the previous inward-filtered trajectory and its final
fitted b. There is no RTS pass, including for reference generation. Native
geometry/material F/Q and measurement H are re-evaluated at the references.
At selected i+1 -> i, propagate first; apply exp(-b) to curvature at the
upstream target before measurement i. The inverse loss derivative is
minus the mapped reference curvature. Affine prediction retains both offsets:

```text
x_pred = inverse_loss(propagate(x_ref), b_ref)
       + F_total * (x_live - x_ref) + d_loss * (b_prior - b_ref)
P_pred = F_total * P_live * F_total^T + Q_mapped
       + var(b_prior) * d_loss * d_loss^T
Cov(b, x_pred) = var(b_prior) * d_loss^T
```

Native KalTest Filter performs the hit update at the affine measurement
reference. The live helix is 5D; retained b/helix cross covariance follows
subsequent inward transports and native hit updates, so all remaining inner
hits refine b. Original b-prior mean/variance are not replaced by fitted ones.
An independent augmented-6D covariance assembly checks the inverse prediction.
No positivity constraint or truth steering was added. The loss remains at the
same upstream surface. No damping/global optimum guarantee.

The stopping metric uses the maximum inward-filtered helix-coordinate change
and fitted-b change, normalized by their previous uncertainties. Tolerance
remains 0.001. Existing iteration status/history/failure-retains-last semantics
are reused. `iteration_linearized_chi2` contains inward hit-update chi2 sums;
published track chi2 retains the original outward bookkeeping. It is not a
common nonlinear optimization objective. Forward vectors remain the fixed
initial pass; backward vectors/full verbose matrices describe the last
completed inward pass. RTS smoothed and Persistent6D fields remain empty here.

## Verification and exact results

Rebuilt and installed RecBreakpoint in EL9. Standalone TransportTest passed,
including the additional inverse affine-offset test. All experiments used
verbose full 5D matrix output and the independent native KF reference.
Artifacts, runners/checkers, paired ROOT files, logs and result JSON:
`TrackingPerformanceStudies/recbreakpoint_backward_iteration_2026-09-09/`.

The initial seed12:11 smoke converged in three passes before the final gate.
The final eight pairs compare N=1 against N=10 on the same input entries/cell
IDs, truth pT and stored KF pT. Settings: FirstMiddleLast, SeedScale1, mean b0,
sigma0.05, MS on, Eloss off, max hit chi2=1e100, LocalMarginal/BackwardFilter.
Each of the five earlier single-eBrem cases exactly reproduces its stored
one-pass backward pT and b. Every iterative first-pass pT/b/variance agrees
with its simultaneous N=1 run. Full 5D forward state/covariance strings match
at every hit between N=1 and N=10; the terminal backward seed equals the full
forward endpoint exactly. Every dumped forward/backward covariance is finite,
symmetric and positive definite. Inverse joint covariance closure passes;
all tuples have valid fits and explicit convergence status1.

An additional same-code RTS regression reran seed12:11/16/17 with
Persistent6D at MaxFitIterations1 and10: all six pT, fitted b and b-variance
values, and iteration counts, exactly reproduce the pre-extension gate.

Momenta GeV; residual %=100*(pT_reco/pT_truth-1). Entry indices zero-based.

| Seed:entry | Interval | One-pass pT | Iterated pT | One-pass residual % | Iterated residual % | One-pass b | Iterated b | Passes |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| 12:11 | 5 | 9.44063021 | 9.42526858 | +3.164141 | +2.996274 | +0.03666612 | +0.03573618 | 3 |
| 12:16 | 5 | 38.64983747 | 38.64378388 | +0.753727 | +0.737946 | +0.01010374 | +0.01000583 | 3 |
| 12:17 | 5 | 18.10458846 | 18.10354507 | -42.987737 | -42.991022 | -0.00624326 | -0.00628108 | 3 |
| 5:84 | 5 | 36.26564810 | 36.26465329 | -1.153398 | -1.156109 | -0.00423452 | -0.00425225 | 2 |
| 5:92 | 6 | 29.48028295 | 29.47905962 | -2.095601 | -2.099664 | -0.01409391 | -0.01403940 | 3 |
| 3:10 | 4 | 31.28641989 | 31.28547956 | -1.157233 | -1.160204 | -0.00446933 | -0.00448905 | 2 |
| 3:33 | 4 | 24.58525463 | 24.58523507 | -1.106835 | -1.106913 | +0.00018964 | +0.00018876 | 2 |
| 6:17 | 13 | 22.65791087 | 22.65791295 | -0.524816 | -0.524807 | +0.00369997 | +0.00370688 | 3 |

Seed12 interval5 is a mechanical gate, not the correct truth interval for all
three events. Prior topology records classify 12:17 as a secondary-activity
control; report it separately from clean optimization conclusions. The other
five intervals come from the prior single-eBrem audit. No new topology or
clean-track population study was performed.

Three of eight absolute residuals improve (6:17 negligibly); five worsen
slightly. All converge in 2--3 passes. The tested negative-loss failures remain.
Backward relinearization is mechanically operational but has no demonstrated
population improvement. Do not change defaults on this evidence.

## Memory maintenance and next step

Inspect converged b/helix correlations of the still-negative cases. Keep prior,
positivity and within-interval placement hypotheses distinct. Both iterative
paths need clean-track and categorized population controls before default or
physics claims. No automatic truth steering or broader changes authorized.

Outgoing complete AGENTS preserved in
`2026-09-09-agents-before-backward-iteration.md`. All global status/laws/compile
instructions (section1) stay unchanged; section2 is replaced, not appended.
The previous RTS implementation and eight-pair evidence remain in
`2026-09-09-recbreakpoint-iterated-relinearization.md`. No history directory was
moved or removed.
