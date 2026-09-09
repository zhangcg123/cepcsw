# RecBreakpoint interval source: Truth / Manual / reserved Auto

## Contract

User requested a choice between truth eBrem intervals and a reconstruction-based
automatic method still under development. Interval selection is now independent
of loss parameterization and fixed-loss override:

```python
fit.IntervalSelectionMode = "Truth"  # compiled/card default
fit.LossStateMode = "LocalMarginal"   # compiled/card default
fit.TruthOverride = False            # unchanged default; extra pair is copied
fit.BreakpointIntervals = []         # Manual-only input list
```

- Truth: use the existing reconstructed-hit association -> SimTrackerHit ->
  exact Geant4 hook reader; select every matched ordered-hit interval with
  ebremLoss>0. No new energy-loss cutoff. Several emissions in one interval
  produce one breakpoint. No correction outside the accepted first/last hits.
- The existing interval is unchanged: (start,end] hook assignment of process
  subtype3 momentum losses, indexed hit[i] -> hit[i+1]. Upstream loss placement
  and all propagation/measurement mathematics remain unchanged.
- Ordinary fitting receives only the indices. MeanLogLoss/SigmaLogLoss remain
  its priors; selected truth loss values are NOT substituted into that fit.
- TruthOverride independently controls the extra fixed-loss RTS/backward pair.
  Its optional oracle reuses the same validated match and selected intervals.
- A valid no-loss truth track gets an empty effective list and the 5D reference;
  the additional pair is copied. Truth is still read to establish that list.
- Invalid truth cannot identify intervals: the ordinary track fails with
  interval-selection status/error, and the extra pair remains unattempted.
  This differs deliberately from an oracle-only failure in Manual mode,
  where the previously computed ordinary pair is still retained.
- Manual: preserve the explicit BreakpointIntervals path, including the empty
  no-breakpoint reference. Nonempty manual lists are rejected in Truth mode.
- Auto: reserved control value with an explicit initialization error until the
  reconstruction-based selector exists. Never silently becomes an empty list.
- LocalMarginal supports arbitrary selected interval counts. Persistent6D
  remains available for at most one; MaxFitIterations>1 requires exactly one.
  Dynamic incompatible counts fail that track rather than discarding intervals
  or silently overriding requested settings.

Default changes implement the requested Truth-location/LocalMarginal campaign,
not a claim of improved physics. No GSF/shared KF source edits, automatic loss
positivity constraint or emission-position fit were introduced.

## Schema and card/batch integration

New flat fields:

- interval_selection_mode;
- interval_selection_status: 0 unattempted, 1 Manual, 2 valid Truth,
  -1 invalid event truth, -2 invalid track association;
- interval_selection_error;
- selected_breakpoint_interval, retained even if the later fitter fails;
- interval_selection_truth_ebrem_loss (GeV) and _retained_fraction,
  aligned with the selected intervals; empty for Manual;
- interval_selection_truth_g4_track_id and _max_endpoint_distance (mm).

The dedicated card requests embedded truth for Truth selection independently
of TruthOverride. BP_INTERVAL_SELECTION_MODE is captured by the batch helper.
Regenerate older prepared cards in a new directory; explicit fixed-list tests
must add Manual, and the current default loss treatment is LocalMarginal.
Existing prepared cards and the user's recent changes to the two breakpoint
shell scripts were not modified or staged.

## Build and tests

Package built and installed; generated Gaudi configuration confirms Truth and
LocalMarginal defaults. Build/install libRecBreakpoint.so SHA256 both:
`95e5f08502b05e586ddf3571d4212d16acef6d0971b78eb9fc6f69d03b678fb9`.

Artifacts: TrackingPerformanceStudies/breakpoint_interval_selection_20260909/.
Same ready tracker inputs and seeds as prior gates; b prior0, sigma_b.05,
SeedScale=BackwardSeedScale=1, FirstMiddleLast, MS on, Eloss off, one pass,
LocalMarginal, verbose states/covariances and native no-breakpoint checks.

Initial Truth on/off pairs select:

| Seed:entry | Effective interval list | Note |
|---|---|---|
| 2:68 | [7] | Prior single-eBrem smoke |
| 12:0 | [] | No-loss reference |
| 12:11 | [10] | Actual truth location, not old fixed interval5 |
| 12:16 | [] | No positive eBrem between accepted hooks |
| 12:17 | [2] | Secondary-activity control, not clean optimization |

All five ordinary fits succeeded with unchanged ordinary fields between
TruthOverride on/off. Prior means remain0. Copied extra results equal ordinary
ones; oracle provenance matches selection losses/z when nonempty. Seed2:68
reproduces preceding pTs: ordinary44.97146297895819/44.991563840607256 GeV;
oracle44.676959069487786/44.690907113843544 GeV.
Invalid hook tolerance1e-12mm yields track status-1, selection status-2 and no
guessed empty-list fit. Auto and nonempty-list+Truth fail initialization before
creating a flat tuple. Seven batch-planning tests continue to pass, including
capture of Manual selection control. No real Condor jobs were submitted.

These are mechanism/regression gates, not categorized population validation.

Final same-code Truth-versus-Manual gate passed for all seven selected events:
the five above plus seed2:25 selecting [5,125] and seed2:30 selecting [4,9].
For the latter two, primary Geant4 emissions at separated radii were used only
to choose test events (a read-only scan of the ready seed2 steps). The runtime
selector itself uses associations/hooks, not radial matching. All ordinary
and extra endpoint, score and saved state fields agree exactly between Truth
selection and Manual reruns of its effective lists; selection metadata differs
as explicitly labeled. TruthOverride on/off preserves ordinary fields exactly.
These are seven event configurations x three runs (Truth off/on, Manual on),
including the separately reported seed12:17 secondary-activity control.

An additional seed2 first-ten-event sample produced12 attempted tracks per
on/off run. Eleven succeeded; event4 track0 failed because accepted hit230 had
zero provenance-bearing associated SimTrackerHits. Both runs tagged the error
and did not substitute the empty-list baseline. Events2 and5 have two input
tracks and are control examples, not clean optimization representatives.
No single-track optimization count or population improvement claim is inferred.

## Project memory

Complete outgoing AGENTS.md is preserved in
2026-09-09-agents-before-truth-interval-selection.md. Only current focus is
updated; global status/laws/compile instructions remain unchanged. No history
directory move/deletion occurred, so no manifest is required. The snapshot
covers every replaced current-focus statement; the two substantive sections
and all active laws are retained.
