# Free-loss fitting uses shared SigmaLogLoss

## Request and scope

User: "the free loss fit needs the same sigmalogloss controlling its sigma_b,
same as the others". Follow-up: "No active jobs; install after validation".
Starting local branch breakpoint, parent checkpoint 42273bf. No remote action.
Implementation stays inside RecBreakpoint; dedicated card/submission comments
and project records are synchronized. Existing GSF/sim cards, subtrkjobs.sh,
inputs, batch cards and data outputs were not changed by this task.

## Exact current meaning

```text
b = log(p_before / p_after)
fractional loss = 1 - exp(-b)
ordinary pair:   b prior = N(MeanLogLoss, SigmaLogLoss^2)
free-loss pair:  b prior = N(mu_optimized, SigmaLogLoss^2)
truth pair:      b prior = N(b_truth, SigmaLogLoss^2)
```

Minuit now varies the prior center mu, not a fixed exact loss b. Every trial
and final output fit uses the existing BreakpointFitter with the SAME positive
SigmaLogLoss; no second KF/RTS/backward implementation. Both free endpoints
share that center and width but can have different fitted losses and posterior
variances. Minuit's error on mu is neither SigmaLogLoss nor posterior sigma_b
and is not added to the track covariance. Covariances remain conditional on
the chosen prior center: uncertainty in estimating that center is not integrated.

The old assignment settings.sigmaLogLoss=0 is removed. Gaussian capture is
allowed for positive-sigma LocalMarginal fits. Existing transition.noise
already includes sigma^2*g*g^T at the selected birth edge, with g the propagated
state derivative with respect to b. That covariance enters both the complete
RTS score and the captured measurement-covariance determinant. No duplicate
loss penalty or separate log(sigma) normalization is added.

The objective is unchanged in form:

```text
J(mu; sigma) = complete RTS chi2(mu; sigma)
            + log det S_all(mu; sigma) + M log(2*pi)
```

Complete RTS chi2 includes measurement, process and seed residuals. S_all is
the joint measurement covariance, not the smoothed-state covariance; M is the
number of measured coordinates. The scan, multistart MIGRAD, convergence gate,
repeat check, and KF failure fallback remain. The bound [0,FreeLossMaxLogLoss]
now bounds mu, not the posterior b and not the Gaussian support. No reference
iterations, runtime likelihood cross-check option or new property was added.

One LocalMarginal interval remains the supported optimized case. FreeLossFit
off or no selected interval copies ordinary results. Unsupported Persistent6D
free fitting/manual multiple intervals and failed optimization copy input KF
to free slots. The ordinary and truth pairs are always retained separately.
The primary Unconstrained/Fixed modes removed in 42273bf are not restored.

## Steering and schema

Same controller: fit.SigmaLogLoss, default 0.001. Batch controller:
BP_SIGMA_LOG_LOSS in subbreakpointjobs.sh, frozen into generated cards.
All 29 properties and their defaults remain unchanged.

New flat branches (163 total versus previous 160):

- free_loss_treatment = "PriorCenter";
- free_loss_prior_mean_log_loss: optimized center; NaN if not optimized;
- free_loss_prior_sigma_log_loss: configured width, including off/fallback rows.

free_loss_b remains an alias for the optimized center for schema compatibility;
free_loss_trial_b similarly records trial centers. free_loss_b_error is Minuit's
center error. Endpoint fitted_log_loss and fitted_log_loss_variance fields hold
the actual updated loss and its variance. The legacy
free_loss_covariance_conditional flag (exact-fixed-b output) is false; this
does not mean center-estimation uncertainty has been marginalized.
Old tuples lacking the new treatment tag retain their historical fixed-b
interpretation. No old tuple is rewritten. Existing cards retaining positive
SigmaLogLoss automatically use the new algorithm after deployment; obsolete
LossPriorMode assignments must still be removed/regenerated.

## Mechanical checks

Build targets: RecBreakpoint, RecBreakpointTransportTest,
RecBreakpointLikelihoodTest. Two CTests and 28 Python batch/card tests pass.
The added likelihood test independently checks dense covariance and latent
least squares for one rank-one Gaussian loss kick feeding downstream hits,
three prior centers and sigma=0,0.001,0.01,0.05. Zero is a numerical limiting
case only; live free fits require positive sigma. Added batch test verifies
both .001 and .05 reach the free-enabled frozen card through the single sigma
controller.

Temporary evidence directory:
`/tmp/recbreakpoint-shared-sigma-20260930.uXIVXW/`.
The drivers call the maintained card and rebuilt package, NOT temporary C++.
Before outputs were produced from the previous built package before rebuilding.
Inputs: gsf_doublebhoff_freshseed_diagnostic/trk-e--2.0-85-{2,12}.root.
Selected seed:event identifiers: 2:68 and 12:11,16,17. Filenames do not describe
their actual momenta; the table below gives generator pT. Event12:16 has no
selected interval; event12:17 is secondary activity and is reported only as a
control, not part of clean-track optimization evidence.

Six jobs per version (10 rows): LocalMarginal/free-on for 2:68 and 12:11,16,17;
wide sigma=.05 for 2:68; Persistent6D/free-off for 12:11,16,17;
LocalMarginal/free-off for 2:68; unsupported Persistent6D/free-on for 2:68.
All jobs exit zero. Every one of the 104 non-free fields per row compares
exactly, as do 18,310 ordinary/truth verbose state/covariance records.
This includes ordinary/truth output values, per-hit chi2 and loss variance.
Off/empty outputs copy ordinary values. Unsupported free fitting explicitly
reports status -1/result status3 and copies input KF with no invented diagnostics.

Four additional native Gaussian refits set MeanLogLoss to the optimized
center and FreeLossFit=False. All 29 detailed free-pair tuple fields compare
exactly against the optimized output, using the off-slot as an ordinary copy.
All 4,650 five-state/full-covariance verbose records also compare exactly.
The four rows are 2:68 at both sigmas, 12:11 and secondary-control12:17.
Thus final free output is demonstrably the same shared Gaussian fit.

### Focused values, pT in GeV

| Seed:event | SigmaLogLoss | Truth pT | Old fixed-b free RTS | New free RTS | Old fixed-b free backward | New free backward |
|---|---:|---:|---:|---:|---:|---:|
| 2:68 | 0.001 | 44.6834373474 | 45.0333001562 | 45.0332919530 | 45.0514899961 | 45.0511903377 |
| 2:68 | 0.05 | 44.6834373474 | 45.0333001562 | 45.0326069926 | 45.0514899961 | 45.0175800224 |
| 12:11 | 0.001 | 9.1510772705 | 9.1251144748 | 9.1251138011 | 9.1281023518 | 9.1279466428 |
| 12:16, no interval | 0.001 | 38.3607025146 | 38.2505151692 | 38.2505151692 | 38.2873976786 | 38.2873976786 |
| 12:17, secondary control | 0.001 | 31.7556037903 | 18.2161864126 | 18.2158642785 | 18.2242904898 | 18.2242837726 |

For2:68, sigma=.001: optimized center .04169545637606972, fitted RTS b
.04169635517032824; fitted RTS variance 9.906278032648194e-7 and backward
variance 9.913013204611506e-7. For sigma=.05: optimized center
.039041896330079144, fitted RTS b .04169382646342617; fitted RTS variance
.00010148523507306443 and backward variance .0001097336663682692.
The old optimized loss variances were zero; ordinary and truth fits are unchanged.
These numbers establish shared-width mechanics, not population improvement.

## Deployment and provenance

User explicitly confirmed no active jobs and authorized installation after
validation. Used only the RecBreakpoint package cmake_install.cmake, not a
repository-wide install. Backed up its five installed artifacts to
installed-before.tar in the temporary evidence directory. A checksum manifest
of all installed .so files confirms only libRecBreakpoint.so changed.

Previous installed library SHA256:
`7ed0302ff8f490f6e59c452ac3e12fda21615556bb9f4c230977b3463f574540`.
New build and installed library SHA256:
`8689b40e378ce8886e7e0784de983a17c0d943be2b3d014939106d0a73482b0d`.

Installed-runtime smoke comparison reproduces all 163 tuple branches and all
verbose states exactly against the build-tree run for 2:68. The gate is recorded
in installed-gate.log in the evidence directory. Build-tree overrides are omitted
for that run. No Condor submission or remote Git operation was performed.
Existing clock-skew and ROOT PCM/setup warnings are recorded in logs; they
did not prevent compilation, tests or runtime completion.

## Status preservation and non-goals

The project-status-curator skill preserved the exact previous AGENTS and both
old mathematical/workflow documents before changing their live contracts:

- 2026-09-30-agents-before-free-loss-shared-sigma.md;
- 2026-09-30-retired-fixed-free-loss-contract.md.

Migration map: AGENTS section1 and all laws retained unchanged; previous
section2 preserved in full in its snapshot, replaced with current focus.
No history directory was moved/deleted, so no relocation manifest applies.
The unresolved Sep13 native/captured loss-response discrepancy is not resolved
by this change. No claim of irreducible detector limits, optimized physics
performance or validation follows from this selected mechanical gate.
