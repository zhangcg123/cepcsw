# Separate ordinary and optional free-loss output pairs

## Requested contract

The user requested preservation of ordinary breakpoint RTS/backward results,
with two ADDITIONAL free-loss results. When FreeLossFit is off, the extra
collections and corresponding flat result branches must copy ordinary results.
Implemented on `breakpoint`; no GSF/shared KF source or workflow change.

Six output collections are always registered:

| Pair | RTS collection | Backward collection |
|---|---|---|
| Ordinary | BreakpointTracksRTS | BreakpointTracksBackwardFilter |
| Free loss | BreakpointTracksFreeLossRTS | BreakpointTracksFreeLossBackwardFilter |
| Truth prior | BreakpointTracksTruthOverrideRTS | BreakpointTracksTruthOverrideBackwardFilter |

CompleteTracks remains the input/KF baseline, not a seventh newly fitted
collection. The two new configurable names are OutputTracksFreeLossRTS and
OutputTracksFreeLossBackwardFilter. Six outputs must differ from each other
and the input. Defaults and explicit dedicated card steering agree.

## Implementation

RecBreakpoint retains the ordinary PairedFitResult unchanged and holds the
optional optimized pair separately. The latter never overwrites ordinary
tuple fields or publication. When absent, the new publication/serialization
uses a reference to the existing ordinary pair: no refit or numerical copying
approximation. All six outputs use a single publishTrack helper, preserving
the established IP/first-hit/last-hit states, chi2, NDF and ordered hit links.

FitPairTuple is a separate serialization helper for pT, packed EDM IP parameters
and covariance, per-pair loss estimates/variances, all three chi2 totals/lists,
and five complete native per-hit state/covariance sequences. Its reset keeps
the member addresses booked into ROOT stable. FreeLossTuple still owns only
the optimizer diagnostics. No new KF/RTS/backward implementation was introduced.
FreeLossFitter additionally validates both final endpoint curvatures before
returning an optimized pair, so an invalid endpoint goes through its ordinary
fallback rather than replacing successful ordinary output.

Always-present additional result branches add32 fields, for164 total flat
fields. All are prefixed free_loss_. The prior28 optimizer fields remain.
Optimizer scores remain NaN/empty when disabled; RESULT fields copy ordinary
values instead. free_loss_result_status is0 absent,1 ordinary copy,2 optimized.
EDM BreakpointFreeLossStatus/RTSIndex/BackwardIndex provide input-track-row maps,
including excluded tracks. Free-loss optimizer status retains its existing
separate off/empty/applied/unsupported/failed codes. Unsupported modes/multiple
intervals or failed searches copy ordinary results. An invalid ordinary fit
does not create fake copies: absent outputs, indices-1, NaN pT and empty vectors.

TruthOverride remains a positive-sigma prior-center comparison. When off, its
extra pair now always copies ORDINARY results, independent of FreeLossFit.
Historical replacement-mode tuples (including CopiedFreeLikelihood tags)
remain untouched. Presence of free_loss_result_status distinguishes the new
schema. Optimized endpoint covariances remain conditional on b, and optimizer
error is not injected. No objective, bounds, priors, seeds or loss-selection
behavior was otherwise changed.

Full schema and behavior: Reconstruction/RecBreakpoint/docs/free-loss-fit.md.
README and maintained run_breakpoint.py are synchronized. No new batch stage
or shell control is needed; the dedicated batch helper already freezes the card.

## Build/install and regression

The user had confirmed no batch jobs were running. Rebuilt the configured
RecBreakpoint target and installed only that package, Configurables and plugin
databases. A first compile exposed that fullKey() returns DataObjID, not string;
the uniqueness set now correctly stores DataObjID. The build then passed.
Filesystem clock-skew warnings remain external to the changed source.

Build and installed library SHA256:
`fc13c3bc0a631689f872f17bee1060a5ea30f3999187b8f8fec6f598f3a73dca`.

Local artifacts: TrackingPerformanceStudies/breakpoint_free_loss_parallel_20260910/.
The test-only card executes the maintained card and adds PodioOutput to examine
actual collections, while the maintained card still writes only the flat tuple.
Ten jobs completed: off/on seed2:68; off/on seed12:11/16/17; truth-off with
free fitting off/on; Persistent6D unsupported fallback; Manual=[4,5] multiple
fallback; empty list; and invalid Manual=[9999]. Settings otherwise match the
previous sigma0.001, BackwardSeedScale100, FirstMiddleLast regression.
There were14 attempted rows:13 successes and one expected ordinary failure.
Seed12:17 remains a secondary-activity control, not a clean optimization event;
12:16 has no truth-selected interval in this exact input.

check.py / verification.json establish:

- All104 original ordinary/truth fields and ordinary verbose full-state dumps
  match the pre-change ordinary references exactly on all13 successful rows.
- The extra free-fit pT, loss estimates, chi2 lists, and all five state/covariance
  arrays match the corresponding prior optimized or ordinary-copy references
  exactly. Applied b, likelihood and Minuit status are unchanged.
- Nine copied rows have exact serialized EDM payload equality: type, chi2,
  NDF, all three track states/covariances/pivots and original hit references.
- Extra IP flat parameters/covariances agree exactly with their EDM tracks.
- EDM status/index maps agree with flat results and input row counts across
  all452 output events, including unselected and invalid tracks.
- Free fitting on with truth override off preserves the ordinary-copy truth
  pair, with PriorCenter labeling and unchanged configured sigma.
- All164 branch names are unique. Installed new output-name defaults agree.

A separate initialization test with colliding ordinary/free RTS names failed
as expected with the explicit six-output uniqueness error. Both compiled CTests
passed (2/2), as did the dedicated batch unit tests (23/23). No new Condor batch
was submitted. These are mechanical preservation gates, not physics validation.

## Repository and memory

The project-status-curator workflow saved the full outgoing AGENTS.md in
2026-09-10-agents-before-free-loss-parallel-pair.md before replacing the affected
Current focus paragraphs. Global introduction, laws and build instructions are
retained byte-for-byte; two substantive headings remain. No history directory
moved and no unique evidence was deleted. The older deployment/replacement
records remain historical rather than being rewritten as new-schema evidence.

Implementation, card, build and documentation changes are checkpointed locally;
generated ROOT files, logs, binaries, analysis scripts and prepared cards are
not staged. Unrelated gsf.py.bk, sim.py.bk and subtrkjobs.sh edits are preserved.
No remote operation is performed.
