# RecBreakpoint: selectable RTS or posterior-seeded backward filter

## Request and implementation

User requested a configurable choice between RTS smoothing and backward
filtering after reviewing the CompleteTracks IP publication path.
Added `BackwardMode`, accepting `RTS` (unchanged compiled/card default) and
`BackwardFilter`. The dedicated card exposes `BP_BACKWARD_MODE`.
All implementation changes are package-local in RecBreakpoint. No shared KF,
GSF source, maintained GSF card, batch workflow or remote branch was changed.

Both modes run the same outward breakpoint KF. RTS retains the original
retained-joint smoother and geometric IP extrapolation. BackwardFilter copies
the complete outward last-hit posterior with NO covariance inflation and does
not update that hit again. It revisits N-2 through 0 with native KalTest updates,
then publishes native material-aware IP propagation of backward-filtered hit 0.
At each selected interval i, it propagates i+1 to i before undoing the loss with
kappa_before=exp(-b)*kappa_after, prior to measurement i. This preserves the
outward map's upstream loss owner. The inverse derivative is -kappa_before;
the full covariance, including curvature cross terms, is transformed.

Backward selected crossings use fresh independent configured Gaussian loss
priors, not outward/RTS fitted losses. This deliberately reuses evidence already
present in the outward terminal seed, and is NOT a calibrated independent
two-filter smoother. Loss/state cross covariances follow the inward recursion;
Gaussian conditioning on native updated/predicted states lets later inner hits
refine earlier crossed scalar losses without an RTS pass or repropagation.
Negative fitted losses remain allowed. No auto truth steering is introduced.

Native propagation and measurement Filter remain KalTest calls. The new inverse
map is inserted between those calls; it is not a handwritten hit update.
The scalar loss covariance test is independently assembled as a six-dimensional
joint. The adapter checks the native current-track context on the split path.

## Outputs

Persisted `backward_mode` identifies the mode. Existing forward filtered/local
fields are unchanged. RTS `smoothed_*` fields retain their semantics and are
empty in backward mode. New backward predicted/filtered curvature and variance
vectors and backward_local_chi2 retain outward hit indexing. The outermost entry
is the copied seed, with chi2=0, not an additional measurement update.
Full five-dimensional backward states/covariances are printed in verbose logs.
`fitted_log_loss` and its variance are mode-independent outputs; the legacy
smoothed_log_loss aliases remain RTS-only. Local loss means refer to i+1 in RTS
and i in backward mode. Published chi2/ndf remain outward innovation bookkeeping.
The independent native KF reference now follows the selected backward mode.

## Verification

RecBreakpoint built and installed under EL9/LCG105. The generated configurable
reports BackwardMode=RTS. Standalone numerical tests passed, including inverse
loss derivatives/covariance and sequential scalar-loss conditioning. Invalid
mode with a valid input file fails initialization with the expected message.
An initial invalid-mode attempt failed earlier on missing default input and is
not counted as that gate. Early interrupted tests outside validated/ are not
the final evidence; only the final build's validated outputs are used below.

Final test artifacts:
TrackingPerformanceStudies/recbreakpoint_backward_mode_2026-09-08/validated/
Runner/checker/options and output_checks.log are in its parent directory.
Input: gsf_doublebhoff_freshseed_diagnostic/trk-e--2.0-85-12.root.
All 12 jobs and 28 fit rows passed status/schema/covariance checks. Every run
used verbose state/covariance output and an independent native reference.
RTS pT on 3/11/16/17 reproduces previous values within 1e-10 relative. The full
outward five-dimensional means and all 25 covariance entries are identical
between modes at every logged surface. The backward terminal seed exactly
copies the outward terminal state and covariance. Maximum inverse-map covariance
closure discrepancy in these runs is 2.15e-16 in normalized covariance units.
First/last edges [0,231] with nonzero prior 0.01 and two edges [5,7] passed on
11/16/17. The final loss posterior can differ from its local posterior, verifying
continued scalar-loss conditioning by subsequent inward hits.

## Same-code selected-breakpoint comparison

FirstMiddleLast, SeedScale=1, mean loss 0, sigma loss 0.05, MS=true,
Eloss=false and MaxChi2PerHit=1e100. Truth supplied the interval choice only.
No beam spot, ECAL or BH truth override. pT is GeV; residual is
100*(pT_reco/pT_truth-1).

| Event | Intervals | Truth pT | RTS pT | RTS residual % | Backward pT | Backward residual % |
|---|---|---:|---:|---:|---:|---:|
| 3 | [4] | 42.086566925 | 36.970100575 | -12.157006 | 41.772129418 | -0.747121 |
| 11 | [10] | 9.151077271 | 9.125044417 | -0.284479 | 9.124560107 | -0.289771 |
| 16 | [] | 38.360702515 | 38.250515169 | -0.287240 | 38.259362029 | -0.264178 |
| 17 | [2] | 31.755603790 | 17.466338203 | -44.997619 | 18.199295120 | -42.689501 |

These selected examples do not establish performance superiority. Event3's
large tail is reduced but it remains worse than stored KF/GSF. Event17 is a
secondary-tracker-activity control, not a clean-population example.

## Empty-list reproduction of CompleteTracks

With Eloss=true, MS=true, MaxChi2PerHit=200, identical first/middle/last seed,
and no breakpoints, the new independent native backward reference reproduces
stored CompleteTracks pT for all seven checked events at printed double
precision. RecBreakpoint's local backward-filter path differs by at most
3.01e-7 relative (double internal states versus native EDM boundaries).

| Event | Stored KF pT | Native backward reference pT | RecBreakpoint backward pT |
|---|---:|---:|---:|
| 0 | 9.288425568 | 9.288425568 | 9.288422776 |
| 3 | 41.929211405 | 41.929211405 | 41.929222072 |
| 6 | 21.651941801 | 21.651941801 | 21.651947489 |
| 11 | 9.097590152 | 9.097590152 | 9.097588813 |
| 15 | 18.499556413 | 18.499556413 | 18.499550875 |
| 16 | 38.261774981 | 38.261774981 | 38.261774981 |
| 17 | 18.220514852 | 18.220514852 | 18.220513509 |

This extends the previously incomplete matched-control comparison: matching the
publication path as well as the material settings reproduces these stored KF
values. It is not a general claim that hit-list refits reproduce every outcome
of the full pattern-recognition/merging/outlier-retry producer.
