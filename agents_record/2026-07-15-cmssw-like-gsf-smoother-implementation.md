# CMSSW-like GSF smoother implementation (2026-07-15)

## Source basis

The implementation follows the pinned CMSSW source audit at commit
`d58a3bbcbae2d3a107f43ab08fe06599f68629e5`, particularly
`GsfTrajectorySmoother.cc`, `TrajectoryStateCombiner.cc`, and
`BasicMultiTrajectoryState.cc`. The ACTS comparison is in
`2026-07-15-cmssw-acts-gsf-source-validation.md`.

## Implemented semantics

- New default-off `CmsGsfSmoothing`, mutually exclusive with the other three
  backward/smoothing workflows.
- Seed from the complete forward prediction at the outermost measurement,
  before its update; multiply its covariance by `CmsErrorRescaling` (100 by
  default); then apply the outermost hit in the backward pass.
- Reuse exact MarlinTrk updates and likelihoods, reverse BH convolution,
  posterior cutoff, and KL reduction inward.
- Moment-collapse the backward prediction at each interior surface and combine
  it with the stored moment-collapsed forward posterior using the CMSSW
  information formula. These are verbose diagnostics because the CEPC output
  currently serializes only an IP state.
- Publish the innermost backward-filtered mixture, moment-collapsed on its
  surface and transported as one Gaussian to the IP, matching CMSSW's endpoint
  choice rather than combining at the innermost hit.

The reverse template exposes `GSF_CMS_GSF_SMOOTHING` and
`GSF_CMS_ERROR_RESCALING`; enabling the former defaults the independent
reverse filter off.

## Validation

The package built and installed successfully. A comprehensive component dump
on hard-loss event 11 from `/tmp/gsf-match-tracks.root`, with 24 components and
the default five-component CEPC BH model, completed 234/234 hits. The output
had 19 components, pT 1.9895 GeV, and chi2/ndf 484.0/462, versus generator
truth 2.0004 GeV and LCIO 1.7938 GeV. The prior same-code independent reverse
filter was about 1.9830 GeV, so this is a distinct recorded result. Outputs are
`/tmp/gsf-cms-event11-flat.root` and `/tmp/gsf-cms-event11.root`.

This is mechanical focused validation, not physics validation. Events 16/17
were unavailable in the local focused input; population validation remains.
