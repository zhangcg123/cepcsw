# Remove runtime likelihood-equivalence audit

The user explicitly requested complete removal of FreeLossCheckLikelihoods
from the run card and source, after clarifying that it was a numerical audit
rather than another fitting objective.

## Removed

- The Gaudi property, fitter-settings bool, function argument and runtime
  reverse-order QR / joint-state SVD audit, including disagreement rejection.
- BP_FREE_LOSS_CHECK in the maintained card and dedicated batch control list.
- The five audit-only flat fields: free_loss_check_likelihoods,
  free_loss_reverse_order_nll2, free_loss_joint_smoothed_nll2,
  free_loss_trial_reverse_order_nll2, free_loss_trial_joint_smoothed_nll2.
- Audit-only result fields and tuple storage/reset/fill logic.

The normalized QR objective itself is unchanged. Required valid-model/nonfinite
guards, Minuit convergence handling and input-KF fallback remain. Free fitting
is still card-default ON, sigma_b0.001 and largest-truth-interval selection are
unchanged. Ordinary forward/backward/smoothed chi2 diagnostics are NOT removed.
An independent dense-reference unit test remains outside event processing;
it now tests the single production objective, not optional runtime variants.

All29 remaining algorithm-specific configurable properties are explicitly
assigned by the standard run card. Existing frozen cards assigning the deleted
Gaudi property must be regenerated before deployment of the new plugin. Old
tuples keep their historical schema; no outputs/cards were rewritten.

## Validation and deployment

All24 batch tests, shell syntax and git diff checks passed. A private library
built from the maintained source passed its independent likelihood unit test
for zero/singular process noise. Private regenerated Gaudi configurables are
used for the event regression; no temporary alternate fitting algorithm exists.
Build/test artifacts live in
TrackingPerformanceStudies/breakpoint_likelihood_check_removal_20260910/.

Private event regression passed eight jobs/11 track rows, covering active/off,
no loss, largest of multiple truth losses, forced optimization failure and
unsupported configurations. Seed12 events11/16/17 were included;17 remains a
secondary control. All160 retained fields match exactly for every row; all
27,876 verbose state/covariance lines match. Exactly the five audit-only fields
are absent. The private configurable no longer exposes the deleted property.
This is a mechanical regression, not additional physics validation.

Shared build/install is intentionally pending: the user recently intended to
submit jobs, and has not yet confirmed whether any are active. The asynchronous
question was sent. The site's hep_q command fails importing htcondor, so its
exit status0 is NOT proof of an empty queue. Installed plugin SHA256 remains
7e5d2cdf922956f06ecc12d2b0e295deb9a913b7518ac3c8b70458bd21f77a39.
Do not claim shared deployment until an inactive-job window is established.
The new private build does not change the shared build/install artifacts.

## Memory preservation

The complete pre-edit AGENTS.md is retained in
2026-09-10-agents-before-likelihood-check-removal.md. Global status, active
laws/scope and essential commands are byte-identical. Only the current focus
was updated; historical schema and audit explanations remain in dated records.
No remote changes, batch submissions, generated-output commits or shared
KF/GSF changes were made.
