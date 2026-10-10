# Baseline interval-identification tupliser

User authorized a separate algorithm inside RecBreakpoint, with a separate card
under options, for future one/two-eBrem-interval identification. Only the baseline
KF is used. Production breakpoint/GSF cards, shared KF code and installed runtime
are unchanged. No batch jobs or classifier training are part of this development.

Implementation and schema: Reconstruction/RecBreakpoint/docs/interval-identification-tuple.md.
Card: Reconstruction/RecBreakpoint/options/run_interval_identification.py.
New helper: BaselineKFDiagnostics.{h,cpp}; new algorithm:
RecBreakpointIdentification.cpp. All updates and smoothing are native baseline
operations. Truth lookups occur only after both feature passes finish.

Build-tree smoke test on rec-barrel-1.root, entries 0..17: 18 events, 20 tracks.
Includes explicit gates 11,16,17 and two multi-track controls (entries 2 and 13).
All forward/backward passes and all whole-track truth associations succeeded.
The additional production-style copied-outer-state inward continuation reproduced
stored CompleteTracks pT to 1.2e-16 relative precision for all 20 tracks. This
continuation is distinct from the independent backward diagnostic pass.
Initial four-event gate validated 931 hit predictions in each direction and
931 native-smoothed residuals. Full schema/label gate passed on all 20 tracks.
Truth individual-step loss sums agree with all interval-label loss sums.
Test artifacts: /tmp/bpid-validated-on.root and .log. Tests are read-only and
versioned under tests/check_interval_identification.py.
Paired RecordTruth=false rerun: /tmp/bpid-validated-off.root and .log.
Every non-truth branch is exactly identical (including NaN masks and statuses).
Full sample: 4,194 hits and 4,174 adjacent intervals; all hit updates and native
smoothed residual evaluations succeeded. Production-continuation IP parameters
differ from stored input by at most 2.61e-8 in the saved native conventions;
covariance entries by at most 4.21e-12. pT agrees to floating-point precision.
The outgoing current-focus text below is preserved verbatim before replacing
that section; no global laws or other history were removed.

Two integration issues were caught before accepting the output: current REC
does not contain EventHeader (default off, use source+entry+track identity),
and native KalTest has a sixth timing coordinate. The latter is retained in
native prediction diagnostics; ordinary track-state fields are the five-helix
marginal. All tested measurement projectors had zero timing derivative.
Read-only uproot checks use MultithreadedFileSource rather than the local
fsspec asynchronous path, which stalled in this environment.

Limitations: first version uses barrel radius hit ordering, native baseline
first/middle/last prefit (not strictly disjoint-hit seeds), and public float EDM
updated/smoothed states. Exact baseline process-noise matrices are retained;
separate reconstructed DD4hep t/X0 and explicit surface axes remain unrecorded.
Do not train on truth material as a substitute. No physics validation claimed.

## Outgoing current focus, preserved verbatim

Active work on local `breakpoint` is one default-on ECAL absolute-loss
KF/RTS refit. `EcalLossReferenceMode` replaces the three overlapping controls
with Off, NoReference (live forward reference), PreReference (diffuse state at
hit i), and PostReference (same state with curvature from diffuse momentum at
i+1 plus neutral ECAL energy). All active modes share the same fit and ECAL
energy/error prior. The last two require DiffuseAugmentedRTS; diffuse covariance
is not imported. The maintained card/batch variable is BP_ECAL_LOSS_REFERENCE_MODE.
Compiled/card defaults are Persistent6D and ECAL PostReference, with diffuse
enabled to provide that reference. Free-loss supports both Persistent6D and
LocalMarginal through the same fitter and unchanged SigmaLogLoss/objective.
The default migration is recorded in
`agents_record/2026-10-08-breakpoint-persistent-post-defaults.md`.
The old ECAL-to-log-prior diagnostic is retired; ordinary/free-loss/truth-prior
fits retain their own log-loss machinery. The update is tested through the
build-tree runtime; the shared InstallArea is not updated.

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
safety, tail control and held-out population checks before promotion.
Controller migration, regression evidence, and the outgoing focus are in
`agents_record/2026-10-08-breakpoint-ecal-controller.md`; the twelve-event
physics comparison remains in
`agents_record/2026-10-08-breakpoint-postloss-plus-ecal-reference.md`.
