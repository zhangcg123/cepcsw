# RecBreakpoint: paired endpoints and three per-hit chi2 lists

## Request and scope

The user requested removal of the backward/RTS choice, automatic per-hit
forward, backward and final smoothed chi2 lists, and both final endpoints in
parallel. Work stays on local `test_breakpoint` inside RecBreakpoint plus its
dedicated card and project documentation. No GSF, shared KF, batch card or
remote changes are part of this implementation. Existing unrelated dirty
files remain untouched.

Outgoing status is preserved verbatim in
`2026-09-09-agents-before-parallel-breakpoint.md`; outgoing package reference
is preserved in `2026-09-09-recbreakpoint-readme-before-parallel.md`.
Their historical single-mode names and results remain historical evidence,
not the current interface.

## Interface and workflow

`BackwardMode` is removed from the Gaudi property surface, fitter settings,
and tuple. The dedicated card rejects the retired `BP_BACKWARD_MODE`
environment variable, rather than silently selecting something else.

One initial outward fit supplies two separate continuations (not CPU threads):

* RTS publishes `BreakpointTracksRTS` through existing `OutputTracks`.
* BackwardFilter publishes `BreakpointTracksBackwardFilter` through new
  `OutputTracksBackwardFilter`.

Both collections retain original ordered hits and IP/first/last states.
`BreakpointOutputIndex` and `BreakpointBackwardOutputIndex` map input rows to
their respective output indices; absent outputs are -1. `BreakpointStatus`
describes the pair: 1 success, -1 failed attempted fit, 0 excluded track.
An exception in either fit currently fails the pair. A passive score failure
does not destroy otherwise valid endpoints and is separately tagged.

Backward copies the complete FIRST-pass forward endpoint posterior without
inflation, does not update the last hit again, and refilters N-2 through 0.
It still reuses forward information; this is NOT an independent Bayesian
two-filter smoother. Its material-aware native IP propagation is retained.
RTS is independent of the backward refilter and retains geometric IP
extrapolation. Neither continuation feeds fitted states to the other.

Persistent6D carries the selected loss coordinate in the outward filter and
joint RTS. Its backward continuation retains the established local-joint
inverse-loss treatment, not a newly invented persistent inward6D method.
LocalMarginal and TruthOverride support multiple selected intervals;
Persistent6D supports at most one. Empty lists are the ordinary 5D reference.

For MaxFitIterations>1, Persistent6D iterates RTS and backward independently.
LocalMarginal retains one-pass RTS and iterates backward only. The backward
seed remains the original FIRST-pass forward endpoint, not the final iterated
RTS endpoint. Existing original-prior retention, stopping and last-complete-
pass failure behavior are preserved. TruthOverride still requires one pass.

Flat `breakpoint_pt` remains an RTS alias; explicit `rts_pt` and `backward_pt`
are always saved. Existing fitted/local/smoothed loss fields refer to RTS;
`backward_fitted_log_loss` and its variance refer to inward results. Both have
separate iteration histories. `backward_seed_forward_chi2` records the frozen
forward bookkeeping used by backward, which can differ from the final
iterated RTS `forward_chi2`.

## Exact chi2 contract

All lists use outward ordered-hit indices and have length `hit_count`:

| List | Meaning |
|---|---|
| `forward_local_chi2` | Native forward measurement-update increments; `local_chi2` alias retained |
| `backward_local_chi2` | Native backward increments; final hit is zero because it is the copied seed |
| `smoothed_local_chi2` | Complete final RTS affine quadratic objective allocated to receiving hits |

Totals are `forward_chi2` (`filter_chi2` alias), `backward_chi2` and
`smoothed_chi2`. Each equals the corresponding list sum. Do NOT add forward
and backward totals: their data overlap.

The complete smoothed objective uses original measurement V, process Q and
initial prior covariance. It does not use the reduced posterior covariance
as a replacement noise model. Measurement projection and Jacobians are native
KalTest calls; there is no parallel hand-coded measurement update.

For a transition, at the RTS conditional mean:

```text
d = x_smoothed[target] - x_predicted[target]
u = inverse(P_predicted[target]) * d
conditional mean process deviation = Q * u
process penalty = transpose(u) * Q * u
```

The identity follows by conditioning the independent process noise on the
predicted target and then all measurements. It avoids a pseudoinverse of
singular scattering Q and avoids adding noise to the deterministic/static b.
It also handles a rectangular 5->6 loss-birth transition. It is valid at RTS
means, not an arbitrary trial trajectory. The independent b prior is included
ONCE in the birth Q for scoring; no second loss-prior penalty is added.

`smoothed_measurement_chi2` uses the last forward pass's affine measurement
reference and the final RTS state. `smoothed_process_chi2` assigns each
incoming process penalty to its target hit (zero at hit 0).
`smoothed_seed_chi2` is the original seed-prior displacement penalty, assigned
only to hit 0. Their sum per hit is `smoothed_local_chi2`.
`smoothed_native_measurement_chi2` separately evaluates the nonlinear native
measurement residual at the final RTS mean; it remains hit-only, not the
complete goodness score.

`smoothed_chi2_status` is 1 valid, 0 not evaluated, -1 unavailable with an
error string and NaN total. A passive score error does not change filtering.
The RTS EDM Track.chi2 carries the complete smoothed total; the backward
track carries its backward increment sum. NDF remains measurement dimensions
minus five as bookkeeping, not a calibrated loss/prior degrees-of-freedom
prescription. None of these is an eBrem posterior probability or a normalized
model-comparison likelihood; comparing different noise/prior models also
requires their normalization terms.

## Mechanical evidence

Artifacts are under
`TrackingPerformanceStudies/recbreakpoint_parallel_2026-09-09/` (uncommitted
ROOT/log/analysis output), with `jobs_*.json`, `results_*.json`, `run.py`,
`check.py`, `check_affine_score.py`, and `check_edm.py`.

Package-only build and install succeeded. Existing standalone transport,
seed-selection, covariance and loss-map helper tests passed (1/1 CTest).
An independent NumPy linear filter/RTS test passed 100 trials with rectangular
birth, singular Q and zero Q: the complete score and forward innovation sum
agree to a maximum absolute difference 9.094947017729282e-13; independently
reconstructed process residuals agree with Q*u and the pseudoinverse penalty.

Initial smoke: seed2 entry68 interval7, Persistent6D and TruthOverride.
Both endpoints reproduce their preceding separate runs within the explicit
relative pT tolerance 2e-9. Native smoothed measurement sums reproduce the
previous external native-site replay. All per-hit sums/decompositions and
finite positive-definite verbose 5D/6D covariances pass.

Focused gate: seed12 entries 11/16/17, interval5, both ordinary loss modes
with iteration limits 1 and10, plus one-pass TruthOverride. Empty-list tests
exercise all three loss modes on seed12:0; further controls use first
interval, zero truth loss, and multiple LocalMarginal/TruthOverride intervals.
All 22 pairs pass. Seed12:17 is secondary-activity CONTROL, not a clean-track
optimization event. Zero truth and empty-list results agree exactly.

Selected default Persistent6D one-pass gate values:

| Seed:entry | RTS pT GeV | Backward pT GeV | Forward chi2 | Backward chi2 | Smoothed complete chi2 |
|---|---:|---:|---:|---:|---:|
| 12:11 | 9.50620040 | 9.44063021 | 453.22404525 | 452.90302937 | 453.10686237 |
| 12:16 | 39.01869670 | 38.64983747 | 430.08064821 | 426.98156994 | 430.05356552 |
| 12:17 control | 17.91896149 | 18.10458846 | 448.80273321 | 444.94279173 | 448.10676438 |

For the same three iterated Persistent6D/RTS fits, the complete final score
minus final forward increments is -2.69e-7, -8.01e-8, +1.56e-9. One-pass
native nonlinear KalTest increments need not equal the affine complete
score; e.g. the empty-list seed12:0 totals are 469.17140199 and464.11754299.
The implementation preserves and labels both, rather than silently calling
them identical. Small chi2 or selected endpoint success is not physics
validation.

The initial EDM probe exposed a pre-existing backward IP location tag of0
(Other), although its parameters were propagated to the origin. The package
adapter now explicitly labels that result AtIP. This changes metadata only,
not momentum, covariance, propagation or fitting.

Final range gate: the remaining seven previous 1--5% single-eBrem examples
were run with Persistent6D and TruthOverride (14 pairs). Together with the
two seed2 smoke cases this replays all eight examples in both loss modes:
2:68 interval7; 3:12 interval5; 3:75 interval8; 3:83 interval233;
3:85 interval9; 3:94 interval231; 4:65 interval230; 5:69 interval5.
All paired endpoints and losses pass the available prior separate-run
references, including first/last-region loss placement. Both native smoothed
measurement sums agree with the earlier external native-site replay.

After the metadata correction, the package was rebuilt/installed and
seed12:11/16/17 rerun again. Their pT and all three total chi2 values are
unchanged from the first paired gate. A new seed12:0 empty-list EDM probe
passes collection names, status/index mappings, 232 original hits, and the
three state locations [AtIP, AtFirstHit, AtLastHit] for BOTH collections.
EDM/flat pT and chi2 agree within float serialization tolerance (1e-7 relative).
Generated configurable defaults and rejection of the removed property/card
environment switch pass. In total: 41 fully checked paired flat fits
(82 endpoints, across 12 unique input events including the secondary control)
plus one final paired EDM roundtrip; this is not a population study.

Final installed/built libRecBreakpoint.so SHA256:
`7d0a7bbc66c8be28ac74409f7a02d93c917ec1259bcd2e0084d26be656200f9a`.
The build still emits existing ROOT PCM and clock-skew warnings; the package
target, installation, generated configurable checks and final executions
complete successfully. No shared code was changed to suppress those warnings.

The publication checker initially failed on the genuine location-tag issue
above. An auxiliary interactive attempt to inspect a PODIO Vector3d using
Python list conversion hung/crashed and was interrupted; the final checker
uses only bounded track-state collection access and passes. This was not a
fit-job crash and did not alter any input or fit result.

## Knowledge migration map

AGENTS section1/global laws/commands: retained unchanged. Outgoing section2:
preserved in the complete pre-edit AGENTS snapshot, then replaced with the
paired-endpoint focus. Complete outgoing README: preserved separately before
rewriting the authoritative options and tuple reference. No history directory
was renamed or removed, so no directory-migration manifest was necessary.
