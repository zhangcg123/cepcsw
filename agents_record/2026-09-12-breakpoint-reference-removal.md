# Remove reference-trajectory iterations; clarify objective equivalence

## Request and scope

The user explicitly requested removal of the newly developed fixed-b
reference-trajectory iterations, followed by an explanation of why a complete
smoothed trajectory score can equal a forward-filter score. The preceding
implementation misinterpreted the requested objective change as a reference
model iteration. This change removes that feature; it does NOT change the
Minuit objective to a smoothed quadratic. Further objective changes await
agreement with the user.

Only RecBreakpoint, its dedicated controls/docs/tests and project records were
changed. Branch remains breakpoint; no remote operation. User edits to
DumpGsfTrks/gsf.py.bk, DumpGsfTrks/sim.py.bk and subtrkjobs.sh are untouched.
Shared KF/GSF packages and generated historical ROOT files/cards are untouched.

## Removal

- Removed FreeLossReferenceIterations, FreeLossReferenceTolerance,
  FreeLossReferenceObjectiveTolerance, their validation, and dedicated
  BP_FREE_LOSS_REFERENCE_* environment steering.
- Removed fitWithReference, ReferenceFitSettings, reference-update helper site,
  native affine reference update/advance helpers and runtime convergence loop.
- Restored direct BreakpointFitter::fit calls for every fixed-b free trial and
  final conditional pair. All runtime source files and card/helpers/tests for
  batch handling are identical to the parent of 88b548f.
- Removed eleven iteration-specific flat fields: three controls, three trial
  diagnostics, three final convergence diagnostics, reference means and
  reference covariances. The maintained schema returns from 171 to 160 fields.
- Archived the removed reference-trajectory.md verbatim under
  2026-09-12-retired-reference-trajectory-contract.md. Historical test tables
  remain in 2026-09-12-breakpoint-reference-trajectory.md.
- Preserved independent affine-coordinate likelihood unit coverage and added
  forward / complete smoothed / marginal quadratic equivalence checks. Those
  tests do not introduce runtime iterations or a new likelihood controller.

Existing frozen cards explicitly assigning removed properties must be
regenerated. Historical experiment scripts/cards are preserved, not silently
rewritten to run a different algorithm.

## Mathematical distinction

The exact statement is conditional on the same fixed affine Gaussian model:

```text
minimum complete trajectory quadratic
 = complete quadratic at RTS conditional mean
 = sum of forward innovation quadratics
 = marginal measurement quadratic r^T S^-1 r.
```

Complete means measurement + process + seed penalties, with original V, Q
and seed covariance. It is not a sum of smoothed hit residuals alone. Singular
process directions obey exact constraints, not arbitrary pseudoinverse freedom.
The previous/current Minuit objective additionally contains log det S and
M log(2*pi). These terms are not included in smoothedTotalChi2. Dropping them
is a change of objective. Merely evaluating the complete quadratic through
smoothed states cannot add new hit information to that same Gaussian model.

The native nonlinear forward diagnostics can differ from the frozen affine
model. Do not claim exact equality to arbitrary published KF chi2. The
implementation's complete smoothed score uses the captured affine measurement
score; native measurement residuals are also saved separately. See
Reconstruction/RecBreakpoint/docs/smoothed-objective.md for all definitions and
the whitened least-squares derivation.

## Verification

Generated evidence stays uncommitted under
TrackingPerformanceStudies/breakpoint_reference_removal_20260912/.

- Private library compiled from the restored maintained source.
- All 24 batch helper tests passed; all 29 algorithm properties are explicit.
- RecBreakpointTransportTest passed.
- RecBreakpointLikelihoodTest passed dense Gaussian checks, nonidentity
  affine-coordinate invariance, and independent forward/complete-smoothed/
  marginal quadratic comparisons with zero and singular process noise at
  absolute tolerance 1e-10.
- Two private jobs, four rows: 2:68 and 12:11,16,17. The last is a separate
  secondary-topology control, not a clean-track physics example.
- Compared against prior reference-feature OFF runs: all 160 retained fields
  agree exactly for all rows (including nested vectors), and 11,172 verbose
  mean/covariance records match exactly. Only the 11 retired fields disappear.
- Actual free marginal versus complete smoothed quadratics:
  2:68 = 416.0405192325439 vs 416.0405193394644;
  12:11 = 453.3558262876808 vs 453.3558263550789;
  12:17 secondary = 448.13378869242354 vs 448.1337887049117.
  Event 12:16 is a no-interval ordinary copy and has no free objective.

Shared rebuild and install succeeded. Both installed jobs/four rows match
the old iteration-OFF reference in all 160 retained fields and 11,172 verbose
mean/covariance records. Installed configurable checks confirm all three
iteration controls absent; sigma 0.001 and backward scale 100 unchanged.
Built/installed libRecBreakpoint.so SHA256 agree:
c765a01ef13fadd2b26999e6dc015e7818a2950e2ab4d26ab5a0eccd717de95e.
The existing user confirmation of no batch
jobs applies; no batch submission was made here. The build emitted filesystem
clock-skew and known ROOT configuration warnings; runtime checks are required
in addition to build success.

## Status curation

The complete outgoing AGENTS.md is preserved in
2026-09-12-agents-before-reference-removal.md. Its Introduction/global status
and all active laws/compile instructions are unchanged. Only Current focus is
replaced. No history directory was moved; no manifest migration applies.
No historical evidence is deleted. The removal demonstrates reproducibility,
not improved physics performance or correctness of the previous objective.
