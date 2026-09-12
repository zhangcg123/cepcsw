# Explicit RTS-smoothed free-loss objective, 2026-09-13

## Request and scope

Following the normalization decision, the user said "lets go": implement
direct evaluation using the complete RTS-smoothed chi2, retaining both
normalization terms. Work stays in RecBreakpoint and its tests/documentation.
No GSF/shared KF source, maintained card, batch controller, reference-trajectory
iteration, or remote branch was changed. Existing user edits in
DumpGsfTrks/gsf.py.bk, DumpGsfTrks/sim.py.bk and subtrkjobs.sh remain untouched.

## Implemented objective and source contract

```text
J(b) = complete smoothed chi2(b) + log det S_all(b) + M log(2*pi)
complete smoothed chi2 = measurement chi2 + process chi2 + seed chi2
```

b = log(p_before/p_after) is fixed at each Minuit trial; sigma_b=0.
S_all is the joint covariance of all measurements in the trial's captured
affine model, NOT the RTS-smoothed state covariance. M counts measured
coordinates. The measurement, process and seed terms use the original V, Q
and seed covariance, with the existing supported-noise treatment for singular Q.

FreeLossFitter now reads pair.rts.smoothedTotalChi2 from the existing fitter
and passes it, together with that same trial's captured Gaussian model, to
evaluateSmoothedTrackLikelihood. The helper factors the whitened joint
measurement covariance with QR and adds log det S_all and M log(2*pi).
It does not compute/substitute the marginal quadratic. Negative/nonfinite
smoothed chi2 invalidates the trial rather than selecting another objective.
The unchanged fitter already checks smoothedChi2Status before scoring.

evaluateTrackLikelihood remains an independent marginal-reference helper for
numerical tests. No production Minuit path calls it. Both helpers use the same
covariance-factorization and normalization code. No Kalman or RTS recursion is
duplicated or altered. No run-card property or tuple branch changes.
free_loss_quadratic now contains the complete RTS chi2 used by Minuit and
equals free_loss_smoothed_chi2 for an applied result; nll2 retains both terms.

All six endpoint outputs, fixed-b conditional covariance, largest-truth-interval
selection, ordinary/truth-prior sigma=0.001, forward seed scale=1, backward
seed scale=100, MS on / ionization loss off, Minuit scan/starts/bounds/tolerance,
and failure/copy handling remain unchanged. There is no new reference loop.

## Verification

Generated, untracked evidence is under:
TrackingPerformanceStudies/breakpoint_smoothed_objective_20260913/.
build_private.py builds maintained sources without installation. run.py runs
the maintained card with only input/output/event/verbosity overrides. analyze.py
compares every tuple field and the verbose state dumps. Inputs are the same
read-only tracker files in gsf_doublebhoff_freshseed_diagnostic/.

- Before editing implementation, a private baseline library was built from the
  current source. Its four new event rows reproduce all 160 fields of the prior
  installed removal regression exactly. Thus these are direct reruns, not
  copied historical pT numbers.
- The 24 batch tests pass with all 29 properties explicitly assigned.
- RecBreakpointLikelihoodTest and RecBreakpointTransportTest pass. The former
  checks an independent dense normalized likelihood, a separate forward KF
  sum and complete trajectory least squares, zero/singular process noise,
  correlated measurements and affine-origin invariance. New checks verify
  direct use of the supplied smoothed chi2, unchanged normalization, valid
  zero chi2 and rejection of negative/NaN/infinite scores.
- Private before/after runs: seed 2 entry 68 and seed 12 entries 11,16,17.
  Entry 12:17 is a secondary-tracker-activity control, not a clean-track
  optimization event. Entry 12:16 has no selected interval and copies ordinary.
- Both libraries write the same 160-field schema. All 104 non-free-fit fields
  per row agree exactly, as do 6,517 ordinary/truth verbose mean/covariance
  records. The no-interval row agrees in all 160 fields.
- Both published free endpoint pT values agree exactly in each row (the native
  endpoint serialization has finite precision). This does NOT mean all free
  states/covariances are bit-identical: the selected b changes slightly with
  numerical objective differences, and detailed free fields reflect that.
- In every applied new row, free_loss_quadratic equals the complete saved
  free_loss_smoothed_chi2 exactly and agrees with its per-hit sum. The inferred
  normalization dimension is unchanged: 464 for 2:68 and 466 for 12:11,17.
- At the same valid coarse-scan b values, maximum absolute J differences are
  2.23280494537903e-6 (2:68, 14 points), 6.877302439534105e-6
  (12:11, 12 valid points), and 2.0844709069933742e-8 (12:17, 14 points).
  12:11 has the same invalid high-b trials in both evaluations. The difference
  near each selected minimum is much smaller; no precision/scan policy changed.
- A further two jobs load the normal CMake-built library directly, without
  replacing InstallArea. All 160 tuple fields and all 11,172 verbose
  mean/covariance records agree exactly with the new private-library runs.

All pT values below are in GeV and come from these direct runs:

| Seed:entry | Truth | KF | Free RTS before / after | Free backward before / after |
|---|---:|---:|---:|---:|
| 2:68 | 44.6834373474 | 43.5577944177 | 45.0333001562 / 45.0333001562 | 45.0514899961 / 45.0514899961 |
| 12:11 | 9.1510772705 | 9.0975901523 | 9.1251144748 / 9.1251144748 | 9.1281023518 / 9.1281023518 |
| 12:16, no interval | 38.3607025146 | 38.2617749807 | 38.2505151692 / 38.2505151692 | 38.2873976786 / 38.2873976786 |
| 12:17, secondary control | 31.7556037903 | 18.2205148520 | 18.2161864126 / 18.2161864126 | 18.2242904898 / 18.2242904898 |

| Seed:entry | Selected b before | Selected b after | Complete RTS chi2 used after | J after |
|---|---:|---:|---:|---:|
| 2:68 | 0.04169652619639807 | 0.041696524135979116 | 416.0405193062817 | 13.792938530630636 |
| 12:11 | 0.003560273471153901 | 0.0035602734884332145 | 453.3558263563748 | -133.7461204126696 |
| 12:17, control | 2.3952800215823987e-9 | 2.3937290586923798e-9 | 448.1337887215674 | -203.6705124033699 |

## Build/install status and remaining work

The private library and normal CMake RecBreakpoint target build successfully.
The normal build reports the known filesystem-clock-skew and ROOT C++-standard
warnings; numerical tests and direct library runs provide the actual gates.
Shared installation was initially deferred: the read-only hep_q check could
not import htcondor in this environment, which did NOT establish an empty
queue. The user subsequently confirmed "there is not any batch jobs" on
2026-09-13. Only the package-local cmake_install.cmake was then run; it installs
libRecBreakpoint, its generated Python configurable and component/confdb files.
No other package or workflow card was changed and no batch jobs were submitted.

Before installation, the preceding installed library had SHA256:
c765a01ef13fadd2b26999e6dc015e7818a2950e2ab4d26ab5a0eccd717de95e.
After installation, built and installed libraries match byte-for-byte, SHA256:
7ed0302ff8f490f6e59c452ac3e12fda21615556bb9f4c230977b3463f574540.

Both installed regression jobs completed with exit code zero: 2:68 and
12:11,16,17. Outputs are in the study's installed/ directory, with install.log
and installed_analysis.log alongside the earlier private/build evidence.
All 160 fields in all four rows and all 11,172 verbose mean/covariance records
agree exactly with the new private and direct-CMake-library runs. The complete
analysis script passes, including the pre-change rerun, ordinary/truth
preservation, matching-b objective, and direct RTS-chi2 checks. The pT tables
above also describe the installed rerun; 12:17 remains a secondary control.

This change implements the requested explicit evaluation, not a demonstrated
physics improvement. The fixed-affine Gaussian identity predicts near agreement
with the previous objective. Extreme large-loss minima from prior studies
remain unresolved; no new population claim is justified by these checks.

## Project-memory preservation

The entire outgoing AGENTS.md is saved byte-for-byte in
2026-09-13-agents-before-smoothed-objective-implementation.md. Its global status,
laws and compile instructions remain unchanged in live AGENTS.md. Only its
Current focus is replaced; the outgoing removal evidence and prior decision
remain in that snapshot and their existing dated records. No history directory
was moved, renamed or pruned.
Before updating the deployment status, another complete snapshot was saved in
2026-09-13-agents-before-smoothed-objective-install.md. Global status, active
laws and compile instructions are again preserved unchanged; no unique history
was deleted.
