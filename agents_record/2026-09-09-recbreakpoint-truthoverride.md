# RecBreakpoint LossStateMode=TruthOverride

## Request, implementation and limits

User explicitly requested adding TruthOverride to LossStateMode. Base checkpoint
`6ad1b50`, local `test_breakpoint`. Default remains Persistent6D. Source changes
are confined to RecBreakpoint; its dedicated card/build registration are updated
locally and remain uncommitted under the existing workflow/build-file law.
No GSF/shared KF source, maintained GSF card, batch workflow or remote change.

TruthOverride fixes the loss at each user-configured BreakpointIntervals entry.
It does not find/select intervals automatically or correct unselected intervals.
RTS and BackwardFilter are supported at MaxFitIterations=1 only. The existing
5D/local-joint machinery applies each deterministic loss with zero added loss
variance; MeanLogLoss and SigmaLogLoss are ignored. No singular six-dimensional
state is introduced. Forward applies exp(b) before crossing; backward propagates
first then applies exp(-b) at the same upstream surface before its measurement.
All native hit updates, material/MS handling, ElossOn, seeds and backward
evidence reuse remain unchanged. An empty interval list performs the ordinary
5D reference without accessing truth. This is an opt-in oracle, not production.

## Exact truth source, no workflow sidecar

The module compiles the unchanged
`Reconstruction/RecGsfTracking/src/TruthBHLossEventData.cpp` directly, with that
source directory on its private include path and links GsfTruthEventData and
GsfTruthEventDataDict. It does NOT link/run the GSF algorithm plugin. Reusing
the maintained reader avoids a separate interpretation of the truth hooks.
These are the additions to the package's local CMake registration.

The card derives additional PodioInput collections from the final
fit.LossStateMode and fit.BreakpointIntervals, including manual edits. In
TruthOverride with a nonempty list it loads the six tracker SimTrackerHit and
truth-association collections plus GsfG4MaterialSteps and
GsfSimTrackerHitG4StepLinks. Other modes keep their existing input list.
No side ROOT tuple, CSV or prior GSF execution is required.

The relation chain is TrackerHit -> MCRecoTrackerAssociation -> SimTrackerHit
-> exact G4 provenance hook. Matching checks all ordered hits for one primary
electron, complete/unambiguous associations and monotonic hooks. No nearest-hit
search is used. New property TruthMaxEndpointDistance=5 mm validates distance
to the already associated hook. Native runtime material is not replaced.

Geant4 eBrem process subtype3 momentum losses whose post-step points fall in
the hook interval (start,end] are summed. The effective response is
z=1-sum(eBrem delta-p)/p_at_start_hook, then b=-log(z). The start momentum is
interpolated within its G4 hook step. This matches the maintained GSF oracle's
eBrem-only response, not total loss including ionization. Step-fraction t/X0
is recorded passively, never used to steer the breakpoint fit. Truth remains
collapsed at the original upstream surface, not its exact within-interval
emission position.

## Tuple/error contract

Automatic truth_override_status: 0 not requested/empty interval list, 1 valid
match, -1 invalid/missing event truth, -2 invalid track associations/hooks,
-3 configured interval outside the matched track. truth_override_error records
the failure reason. Invalid truth causes the affected fit row to have status=-1
and no published track; it never falls back to the nominal Gaussian prior.
An earlier PodioInput collection-loading failure can prevent reaching this code
and thus prevent writing a per-track tag. Truth status1 is not fit success.

truth_override_interval aligns vectors for retained_fraction, log_loss,
momentum_before, ebrem_loss, tx0, first_step, last_step, start_fraction,
end_fraction (all prefixed truth_override_). Scalars also retain g4_track_id
and max_endpoint_distance. Local/prior/fitted loss fields equal the selected
truth value, with variance0. Persistent6D vectors are empty. The event-local
reader releases its maps after execute, and each track's truth map/tuple fields
are reset before matching. TruthDiagnostics remains only generator pT reporting.

## Build and tests

EL9 RecBreakpoint target built and its package subdirectory installed.
Standalone TransportTest passed, including zero-variance fixed-loss covariance
mapping and inverse closure. Generated datamodel/clock-skew warnings remain
visible; no shared source workaround was made.

Artifacts: TrackingPerformanceStudies/recbreakpoint_truthoverride_2026-09-09/.
run_tests.py and check_results.py cover smoke/regression/zero/normal stages;
invalid_hook_options.py forces the endpoint validity check without editing data.
Inputs are the existing gsf_doublebhoff_freshseed_diagnostic tracker ROOTs.
Settings: FirstMiddleLast, SeedScale1, MS=true, Eloss=false, MaxChi2PerHit1e100,
TruthOverride, one pass, explicit selected intervals. No truth interval auto
selection occurs in the card or fitter. Five previously audited single-eBrem
intervals retain their prior choices; seed12 fixed[5] is a mechanical control.

Sixteen fits (eight events x two modes) passed status/schema and full native
covariance finiteness/symmetry/positive-definiteness checks. Applied truth b,
hook bounds and endpoint cell IDs for the five single-eBrem cases agree with
the independent earlier Geant4 audit to 2e-12 in b. Each fixed local/final
variance is exactly0. All truth matches are valid.

Momenta GeV; residual %=100*(pT_reco/pT_truth-1); zero-based entries.

| Seed:entry | Interval | Truth b | RTS pT | Backward pT | RTS residual % | Backward residual % |
|---|---:|---:|---:|---:|---:|---:|
| 12:11 | 5 | 0.000000000 | 9.09490795 | 9.09472192 | -0.613800 | -0.615833 |
| 12:16 | 5 | 0.000000000 | 38.25051517 | 38.25936203 | -0.287240 | -0.264178 |
| 12:17 | 5 | 0.000000000 | 18.21618641 | 18.21754890 | -42.636309 | -42.632019 |
| 5:84 | 5 | 0.007820389 | 36.70972058 | 36.70499512 | +0.056978 | +0.044098 |
| 5:92 | 6 | 0.009028550 | 30.15777210 | 30.14959615 | +0.154349 | +0.127196 |
| 3:10 | 4 | 0.007187070 | 31.64903733 | 31.65278156 | -0.011621 | +0.000208 |
| 3:33 | 4 | 0.009398768 | 24.82126647 | 24.81262686 | -0.157487 | -0.192239 |
| 6:17 | 13 | 0.009168465 | 22.76375012 | 22.76233327 | -0.060149 | -0.066370 |

All three seed12 interval5 truth losses are zero. This is not a test with each
event's correct nonzero loss locations; 12:17 is a secondary-activity control,
not a clean optimization event. The five previously negative cases improve
when supplying their known loss and removing its uncertainty, but this does
not separate prior-center, prior-width or location effects and is not a
population physics validation.

Seed12:0 is a zero-eBrem control. TruthOverride[5] (z=1, b=0) exactly reproduces
the empty-list pT in each mode: RTS9.285455987120015 and
Backward9.285988222946647 GeV. This is not a claim of equality to stored KF
with potentially different Eloss settings.

Normal-mode regressions reran seed12:11/16/17 in Persistent6D/RTS and
LocalMarginal/BackwardFilter at MaxFitIterations1 and10. All twelve pT,
fitted-b/b-variance and iteration-count results exactly reproduce the previous
implementation; truth fields stay disabled/empty. The checker was adjusted
to read both 25-entry and 36-entry verbose covariance formats before this
normal-mode audit passed; no fit code change was needed.

The failure test sets TruthMaxEndpointDistance=1e-12 mm on otherwise-valid
seed5:84. Its first hit is 0.00814318 mm from the associated hook. The job
finishes, but the row has status=-1, truth_override_status=-2, a populated
error string, and empty applied/fitted-loss vectors. No nominal-prior fallback
occurs. Missing-collection and out-of-range status paths are implemented but
were not separately injected in this gate. The immutable section1/laws and
complete outgoing AGENTS snapshot were verified.

## Recovery and next work

Use the oracle as a mechanism control for the still-negative fitted-loss cases,
keeping interval choice, mode, seed and material settings paired. Keep it
default-off and do not equate truth-loss input with exact energy-loss position
or guaranteed perfect pT. No broader source/workflow change authorized.

The complete outgoing AGENTS is preserved in
2026-09-09-agents-before-breakpoint-truthoverride.md; section1 global status,
laws and compile commands stay unchanged. Section2 is replaced with the new
focus. Existing RTS/backward iteration results remain in their dated records.
No history directory or prior evidence was deleted.
