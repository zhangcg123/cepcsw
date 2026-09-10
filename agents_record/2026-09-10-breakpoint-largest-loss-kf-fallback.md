# Largest truth interval, KF fallback, explicit card defaults

## Requested contract

For multiple eBrem losses, account only for the largest one. A failed free-loss
optimization must fall back to KF. Set default sigma_b=0.001 and explicitly
write all algorithm interfaces in the standard run card.

Implementation interpretation, stated before editing: largest means the largest
summed absolute eBrem momentum loss (GeV) in one matched accepted-hit interval,
not the largest fraction or individual G4 emission. Truth selection now returns
only that interval; ties choose the innermost. All ordinary/free/truth endpoint
pairs share it. Other intervals are not fitted. Manual lists remain explicit
and unchanged; an unsupported multiple-Manual free optimization uses KF fallback.

## Implementation

- RecBreakpoint truth selection chooses the maximum positive interval loss.
  Existing hook matching, post-step assignment and upstream loss placement are
  unchanged. No truth loss amount is supplied to the free optimizer.
- FreeLossFitter accepts only valid converged Minuit results: Minimize true,
  status0, finite EDM, reproducible finite objective and valid final endpoints.
  A finite scan/non-converged point alone is no longer an accepted result.
- Failed or unsupported free fits clone the original CompleteTracks into both
  free collections. This is not another KF refit and not an ordinary breakpoint
  copy. Result status3 distinguishes it from ordinary-copy1 and optimized2.
- FitPairTuple writes the KF IP and covariance into the free endpoint fields;
  free_loss_kf_chi2 is the original published KF score. Unavailable refit scores
  are NaN and per-hit/loss vectors empty. Ordinary/truth results stay separate.
  If no valid input IP exists, free fallback remains absent; no fake state is made.
- Off/no interval still copies ordinary. Ordinary fit failure leaves extra
  pairs unattempted. TruthOverride remains a shared-positive-sigma prior center.
- SigmaLogLoss defaults0.001 in the Gaudi property, FitSettings and card; the
  existing submission default was already0.001. FreeLossFit remains ON in the
  maintained card and false as the compiled compatibility default.
- All30 algorithm-specific Gaudi properties/collection controls are explicitly
  assigned in the card. Added a source-to-card coverage test. No new controller
  or retired relinearization was introduced.

## Gates

Evidence: TrackingPerformanceStudies/breakpoint_largest_kf_20260910, including
run.py, jobs/provenance, ROOT/log outputs, check.py and verification.json.
The additional truth_multiple run uses seed2 entries25,30 and the same standard
card with verbose dumps; its exact environment appears in the execution log.

Eight local jobs yielded11 successful rows. Cases: default/free off, hard
seed12 entries11/16/17, forced Minuit failure (max calls1), unsupported
Persistent6D, empty Manual, multiple Manual, and two multiple-truth examples.
Seed12:16 is a no-loss control in this input; seed12:17 remains a secondary
control, not a clean physics-validation event.

| Seed:event | Largest selected interval | Summed truth loss (GeV) |
|---|---:|---:|
| 2:25 | 125 | 3.3899683952331543 |
| 2:30 | 4 | 5.008127689361572 |

Both selections were checked against the complete prior matched-interval loss
vectors in breakpoint_truth_prior_center_20260910/multi1.root. This comparison
tests interval selection only, not prior-sigma-dependent fit equivalence.

The forced failure gives free RTS/backward pT=43.557794417684924 GeV, exactly
the stored input KF pT for2:68; result status3 and optimizer status-2. Explicit
unsupported modes also select KF copies. Six copied-output rows (ordinary or
KF as appropriate) pass exact EDM IP/states/covariance/chi2/NDF/hit-reference
checks. Excluded-event row maps retain status0/index-1.

901 ordinary/truth field comparisons remain exact on unchanged selections.
13,685 verbose ordinary/truth state/covariance records remain exact against
the preceding common-native-IP implementation. All24 batch tests and both
compiled numerical tests pass. Installed Gaudi default sigma is0.001.
The package is built and installed; final source cleanup after the runtime
gate changes only indentation and a sentinel comment, not executable logic.

Event2:30 reaches the upper free-b bound; convergence is NOT physics validation.
No population claim, ROOT commit, Condor submission or remote operation is
part of this change. Shared KF/GSF sources/cards and unrelated edits are preserved.

## Memory migration

The complete outgoing AGENTS.md is preserved in
2026-09-10-agents-before-largest-loss-kf-fallback.md. Global status/laws/commands
are byte-identical; the sole Current focus section was replaced. All outgoing
integration/IP evidence remains in that snapshot and its linked dated records.
No history directory moved, no historical evidence deleted.
