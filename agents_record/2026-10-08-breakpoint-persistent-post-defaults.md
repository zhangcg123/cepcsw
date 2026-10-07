# Persistent6D and PostReference defaults

## User decision and superseded defaults

The user requested Persistent6D as the default loss-state representation and
post-diffuse-plus-ECAL as the default ECAL reference. When asked about the
free-loss incompatibility, the user explicitly chose to extend free-loss to
Persistent6D rather than disable it or silently use LocalMarginal.

Previously LossStateMode defaulted to LocalMarginal in FitSettings, the Gaudi
property and the maintained card. EcalLossReferenceMode defaulted to Off in
the property/card; AGENTS described the ECAL refit as default-off.
DiffuseAugmentedRTS was compiled-off but card-on. Free-loss rejected
Persistent6D as unsupported and produced input-KF fallback outputs.
These defaults are superseded; previous physics results are not invalidated
or relabelled. The preceding controller contract and focus remain recorded in
2026-10-08-breakpoint-ecal-controller.md.

## Current contract

- LossStateMode defaults to Persistent6D in the helper, algorithm and card.
- EcalLossReferenceMode defaults to PostReference in the algorithm and card.
- DiffuseAugmentedRTS is compiled/card true, as required by PostReference.
  Disabling diffuse requires explicitly selecting ECAL Off or NoReference.
- FreeLossFit keeps compiled false/card true. Both Persistent6D and
  LocalMarginal are supported for one selected interval. Every Minuit trial
  and final refit uses the selected representation through BreakpointFitter.
  The Gaussian SigmaLogLoss, objective, optimizer and failure gates are
  unchanged. Beam-guided free-loss uses the same extension.
- LocalMarginal remains an explicit comparison mode. Persistent6D still
  allows at most one interval. Backward continuation remains local-joint;
  this is not a new persistent inward implementation.
- PostReference changes only the existing absolute-loss map reference.
  It does not import diffuse covariance or add another measurement update.
  The known one-pass limitations and status/fallback behavior remain.

No shared KF/GSF code, simulation cards, existing generated batch cards or
ROOT inputs are changed. Newly generated cards inherit these defaults unless
the user explicitly overrides the corresponding BP_* environment variables.

## Validation and deployment

Build-tree regression artifacts: /tmp/bp-defaults-0RyC4N/.
The package rebuild succeeds. All four RecBreakpoint CTests and all 36
batch/card tests pass. The likelihood test now checks rectangular 5D-to-6D
birth and downstream 6D normalization against both the 5D marginal and an
independent dense Gaussian reference, over several prior centers/widths.
The batch test checks frozen-card and compiled defaults, including protection
against worker ambient overrides.

Verbose local fits on seed 1 events 5, 11, 16 and 17 use the new unoverridden
defaults, then repeat with Persistent6D/PostReference/diffuse explicitly set.
Every branch in breakpoint and neutral_pfos agrees exactly between those two
runs (eight fitted rows total). Base and beam-guided free-loss both succeed
on 5, 16 and 17 (status 2); event 11 has no interval and copies ordinary
results (status 1). All retain prior sigma 0.001. PostReference fits event 5
(status 2), and correctly copies ordinary on the three no-candidate controls.
A separate LocalMarginal event-5 run confirms both optimizers still succeed.

Representative pT in GeV, seed 1 event 5:

| Representation | Truth | KF | Ordinary RTS | Free-loss RTS | ECAL PostReference |
|---|---:|---:|---:|---:|---:|
| Persistent6D | 13.8353595734 | 11.1430730254 | 11.1382674708 | 21.9499186147 | 13.7593359625 |
| LocalMarginal | 13.8353595734 | 11.1430730254 | 11.1382674708 | 21.9499225124 | 13.7593359625 |

This is a mechanical compatibility gate, not improved physics performance:
free-loss still overshoots that event in both representations. No tolerances,
priors, objective, convergence requirements or truth-based fallbacks were
changed to improve this example.

The initial PyROOT all-field reader crashed; uproot's default asynchronous
local reader also stalled. The independent uproot MultithreadedFileSource
reader successfully verified all branches and wrote comparison.json. These
reader incidents did not affect the three successful reconstruction jobs.
The shared InstallArea is not updated; no Condor jobs are submitted.
