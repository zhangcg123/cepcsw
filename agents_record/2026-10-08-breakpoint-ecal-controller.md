# One controller for the shared ECAL absolute-loss refit

## Scope and control contract

The user retained all three ECAL absolute-loss variants and requested one shared
controller. `EcalLossReferenceMode` now accepts `Off` (compiled/card default),
`NoReference`, `PreReference`, or `PostReference`. The previous properties
`AbsoluteNeutralLossRTS`, `AbsoluteNeutralDiffuseReference`, and
`AbsoluteNeutralReferenceSource` are removed, with no compatibility aliases.
The card and batch helper reject their old environment variables and direct
users to `BP_ECAL_LOSS_REFERENCE_MODE`. Batch preparation freezes the new mode.

All three active modes use the same neutral-cluster selector, absolute-energy
loss prior, persistent six-dimensional KF/RTS implementation and output path.
NoReference evaluates the loss map at the live forward state. PreReference
uses the diffuse smoothed state at upstream hit i. PostReference starts from
the same state, replacing only curvature using p_diffuse(i+1)+E_neutral and
the charge sign at i+1. PreReference/PostReference retain the existing
requirement that DiffuseAugmentedRTS is enabled. The diffuse covariance is
discarded; neither the track seed nor its live covariance is replaced.

The output collection/branch names and status meanings are preserved. The
existing `absolute_neutral_reference_source` string now records the new mode
verbatim (including Off and NoReference), rather than Forward,
UpstreamSmoothed, or PostLossPlusECAL. This is the intended metadata migration.

The old ECAL-to-log-prior recipe was card/diagnostic steering of the ordinary
fitter, not a distinct C++ ECAL fitter. Its active card recipe is removed;
historical results and analysis artifacts remain historical evidence. That
recipe used mean_b=log(1+E/p_after_reference), sigma_b=sigma_E/(p_after_reference+E)
with a fixed momentum reference. Ordinary, free-loss and truth-prior
MeanLogLoss/SigmaLogLoss support remains because those are separate fits.

## Audited mapping contract

The internal state is (drho,phi0,kappa,dz,tanLambda,L); phi0 is the KalTest
angle (EDM phi minus pi/2), and |kappa|=1/pT. ECAL energy initializes L and its
variance at the selected upstream hit. The loss is applied there before
native propagation to the next hit. The reference changes the expansion point
of that single loss mapping. Native propagation and hit updates follow the
live refit. PostReference takes a momentum at hit i+1 and does not transport
that reference back to the upstream pivot. It is not a whole-trajectory iteration.

The covariance uses J_interval=J_native*J_loss and the ORIGINAL live covariance.
It does not apply J_loss twice: the intermediate mapped five-dimensional
covariance supplies native propagation; the final six-dimensional covariance
is reconstructed as J_interval*P_live*J_interval^T+Q. The explicit loss is
applied only at birth, while all downstream steps carry the sixth coordinate.

## Regression evidence

The focused local old/new driver is `/tmp/bp-ecal-controller-0sAb5a/check.py`.
It freezes the pre-change and new cards and records library hashes. Six
track-0 identities are tested in every mode: 1:5, 1:11, 1:16, 1:17, 1:25,
and 2:10. This includes ordinary-copy controls and the two large-reference
discrepancies. Verbose dumps, ROOT files, and logs stay in that temporary
directory. The test compares every flat branch and vector, permitting only
the reference-source label migration above. No Condor jobs are submitted.

The package rebuilt and all 3 CTest targets passed (Transport, Likelihood,
Diffuse). All 35 batch/card tests passed, including mode freezing against
worker ambient overrides and early rejection of each retired environment
control. Actual Gaudi configuration inspection confirms the three old
properties are absent and EcalLossReferenceMode defaults to Off. Zero-event
initialization tests accept Off/NoReference without diffuse, reject
PreReference/PostReference without diffuse, and reject an invalid mode.

The 24 old/new track-row comparisons (6 tracks x 4 modes, 48 total evaluated
track fits) have zero field/vector differences after accounting for the
intentional reference-source label migration. This includes all non-ECAL
families in the breakpoint tree and the unchanged explicit 1:25 PostReference
failure. At 2:10 PreReference remains 155.784702538 GeV and PostReference
16.783711634 GeV; the cleanup does not claim a physics improvement. The
paired results are in `comparison.json` beside the driver.
The shared InstallArea has not been updated, and no batch jobs were submitted.

## Outgoing AGENTS focus, preserved verbatim

Active work on local `breakpoint` is the default-off absolute-neutral-loss
RTS and the reference used to linearize its loss map. The default-off
`AbsoluteNeutralDiffuseReference` now selects `UpstreamSmoothed` (previous
test/default) or `PostLossPlusECAL`, where the upstream reference momentum
is the diffuse post-break momentum plus selected neutral ECAL energy. This
changes only the loss-map reference; the original seed, live covariance and
independent ECAL loss prior/error remain. It is built/tested via `build.../run`
but not installed into the shared InstallArea.

In a same-code twelve-event local A/B at 4% ECAL error, the new reference
reduces median absolute pT residual from 6.57% to 0.98% and removes the
event 2:10 155.8 GeV tail. Eleven fits succeed and closely approach the old
log-loss ECAL-prior results. Event 1:25 instead fails the one-pass affine
map and falls back to ordinary RTS, yielding a 42.35% residual. The result
is promising but unsafe and not physics-validated. All non-absolute outputs
and four clean/control copies are unchanged.

Next: diagnose the distant forward mean versus consistent ECAL reference in
1:25 and design a full trajectory relinearization that keeps the original
hit/seed objective. Do not import the diffuse covariance as another prior
or suppress the failure with a truth-based cut. Require clean/light-loss
safety, tail control and held-out population checks before promotion. Full
numbers, definitions, tests and the preserved outgoing focus are in
`agents_record/2026-10-08-breakpoint-postloss-plus-ecal-reference.md`; the
initial prototype and earlier reference studies remain in dated records.
