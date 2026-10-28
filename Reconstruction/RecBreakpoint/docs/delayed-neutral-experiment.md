# Delayed neutral-energy update experiment

## Branch and scope

Created 2026-10-28 on `breakpoint-delayed-neutral` from `breakpoint` at
`e4a77baa0459689c02b86c068bb6ce8587bc9dab`. The original branch is preserved.
The tested temporary implementation is now checkpointed on this branch;
`ExperimentalDelayedNeutral` remains OFF by default. No installation or remote
push is part of this checkpoint. Existing unrelated working-tree edits are
neither committed nor reverted.

Installation update (2026-10-28): the user subsequently requested installation;
the package is now installed in the project-default
`InstallArea/x86_64-el9-gcc11-opt`. The default remains OFF. See the deployment
record at the end of this document; earlier no-install statements describe the
validation checkpoints, not the current deployment.

Set `BP_EXPERIMENTAL_DELAYED_NEUTRAL=1` in the dedicated run card, with free-loss
and neutral measurements enabled. This affects ONLY the cumulative
`neutral_ecal_free_loss_rts_*` result. Other families retain their existing
behavior, including ordinary augmented RTS. Missing neutral input retains the
earlier accepted result. The diagnostic
`neutral_ecal_free_loss_optimizer_neutral_ecal_delayed_update` records actual use.

## Algorithm

Keep b = log(p_before/p_after) and the configured SigmaLogLoss. For each Minuit
prior-center trial:

1. Run the shared forward KF and RTS with the enabled beam/charged observations,
   but without the neutral observation. Retain the explicit post-loss state.
2. At that smoothed post-loss state, evaluate E_gamma = p_after*(exp(b)-1)
   and its measurement derivative H.
3. Apply the neutral measurement once. Use the pre-update smoother
   cross-covariances to condition every trajectory marginal, including IP and
   terminal ECAL, through native KalTest scalar updates.
4. Score the complete resulting affine Gaussian model for Minuit, including
   seed, tracker, process-noise and enabled ECAL/beam terms exactly once.

No hit is processed a second time. No loss/transport Jacobian iteration is added.
The breakpoint loss map still uses its original forward reference; this is NOT
the earlier absolute-loss-coordinate experiment. Singular process covariance
is handled on its supported subspace, without adding artificial process noise.
For accepted trials, a direct complete-trajectory quadratic is checked against
the captured model's marginal-likelihood quadratic.

## Focused evidence

Ten selected tracks: seven single-eBrem tracks with actual cumulative 111 and
three no-neutral controls. The seven deliberately include known bad cases;
these are not population performance estimates.

Residual = 100*(pT_reco-pT_truth)/pT_truth, in percent. Events are
seed:zero-based event_index, input track 0.

| Event | Current early update | Delayed update | KF + neutral addition |
|---|---:|---:|---:|
| 1:5 | +1.02629 | -0.46714 | -0.50460 |
| 1:194 | +0.03291 | +0.03268 | +0.66893 |
| 2:10 | +46.47494 | +1.17281 | +1.23090 |
| 2:92 | -2.88989 | -2.89277 | -2.47104 |
| 3:59 | -0.46251 | -0.46271 | -0.34741 |
| 8:130 | +148.08735 | -0.66948 | -0.64523 |
| 10:2 | +26.05899 | +0.87549 | +0.93702 |

All seven applied the delayed observation. Five improve in absolute residual;
two worsen slightly. Counts within +/-1% are 2/7, 5/7, 5/7 respectively.
Simple addition was independently recomputed using the baseline KF IP direction,
electron mass and selected neutral energy from the same rec inputs.

For 2:10, measured neutral energy is 5.558326 GeV. Current final full-formula
energy is 15.617121 GeV despite its affine estimate being 5.558110 GeV. Delayed
full-formula/affine estimates are both 5.558345 GeV. Across the seven tracks,
the delayed full-formula/affine difference is at most 1.65e-5 GeV; the final
p_before-p_after-E_gamma difference is at most 5.42e-5 GeV.

## Validation and limits

- Five-event installed/OFF regression: every existing branch agrees exactly
  (865 main-tree and 57 neutral-tree branches).
- Ten paired runs: all non-target branches unchanged; all OFF pT endpoints
  reproduce the stored production tuples exactly.
- No-neutral controls 1:11, 1:16, 1:17 retain identical target endpoints.
- The new C++ test compares every node mean/covariance with a dense joint
  Gaussian calculation, including 5D-to-6D birth, singular process noise,
  complete trajectory score and marginal likelihood.
- No better valid sampled trial was ignored in these seven accepted delayed
  fits. One trial at 1:194 failed native measurement projection, not the
  accepted fit. Existing optimizer acceptance policy is unchanged.
- Installed library SHA256 stayed
  `a7f75cd2c64b91a846df1823d69b206babe7a968b07aa55f9de9189c7ba4089b`.
- Branch transfer: normal CMake package build succeeded, all 13 RecBreakpoint
  CTest tests passed, and fresh OFF/ON reruns of 1:5 using the branch-built
  library reproduce every temporary-experiment branch exactly in both trees.
  No installation was performed. Make reported filesystem clock-skew warnings;
  the generated configurable and runtime-loaded branch library were checked.

Not validated: broad population performance, endcap, multiple breakpoints,
backward fits, or other constraint combinations. No superiority over simple
addition is established. Full raw evidence, scripts and isolated binary remain
in `/tmp/bp-delayed-neutral-ab-pCpNAi`; temporary files may be cleaned by the OS.

## Expanded local comparison, 2026-10-28

The original temporary directory was unavailable on the current host. Reused the
saved comparison harness with the branch-built library, without installation or
any further fitter changes. Expanded evidence is retained under
`TrackingPerformanceStudies/breakpoint_delayed_neutral_ab_20261028/`:
`full_comparison.csv`, `full_summary.json`, paired ROOT files/cards/logs,
`full_runs.json`, the two installed-runtime regression runs, `ctest.log`, and
the copied branch runtime. A/B differ only in `ExperimentalDelayedNeutral`.

Sample: all 39 topology-clear, single-recorded-eBrem, accepted cumulative-111
tracks in the previous first-ten-seed neutral audit, plus controls 1:11, 1:16,
1:17. This includes the original seven and **32 additional signal tracks**.
It is conditioned on the earlier fit/neutral selection, not a population or
held-out performance test. All 20 paired jobs exited successfully; all 39 ON
signal tracks applied the delayed measurement. No Condor jobs were submitted.

The frozen campaign uses Persistent6D, SigmaLogLoss=0.001, truth-selected
breakpoint, beam/charged/neutral=111, PreReclustering inputs, conditional
ElectronPhoton charged routing, and all backward filters OFF. Simple addition
is independently recalculated from the same rec input's KF AtIP direction,
KF momentum and collected neutral energy, with electron-mass conversion.

| Method | Median absolute pT residual | Within +/-1% | Within +/-5% | RMS residual |
|---|---:|---:|---:|---:|
| Current early neutral | 1.151% | 16/39 | 33/39 | 25.842% |
| Delayed neutral | 0.859% | 23/39 | 38/39 | 5.552% |
| KF + neutral addition | 0.937% | 21/39 | 38/39 | 15.570% |

Absolute residual improves in 26 tracks and worsens in 13. Among the 32 newly
tested signal tracks, early/delayed/addition counts within +/-1% are 14/18/16;
within +/-5% they are 29/31/31. Thus the gain is not solely from revisiting the
original seven examples, but some individual changes are tiny.

| Owner region | Tracks | Early within +/-1% | Delayed within +/-1% | Addition within +/-1% |
|---|---:|---:|---:|---:|
| VXD | 16 | 5 | 12 | 12 |
| ITK | 14 | 7 | 7 | 7 |
| TPC | 9 | 4 | 4 | 2 |

The clear improvement is concentrated in VXD; this sample does not show a
general ITK/TPC core improvement over the existing refit.

The maximum affine-versus-full-formula neutral-energy difference falls from
69.192 GeV to 0.001187 GeV. However, the frozen loss-map inconsistency is not
universally removed: maximum |p_before-p_after-E_gamma| remains 0.155157 GeV.
Both delayed maxima occur in the bad-input case 6:87: collected neutral energy
47.0755 GeV versus truth photon birth energy 1.07097 GeV. Its early/delayed/
addition pT residuals are +33.2325%, +33.8973%, +96.7637%, respectively, with
the fit prior center at its upper bound b0=1. This case remains in every main
comparison; delayed evaluation cannot repair an inappropriate energy input.
The exact cluster-association cause was not audited here.

Regression and deployment:

- All 13 C++ tests pass again, including the dense-Gaussian conditioning test.
- Across all 42 pairs, every non-target main-tree branch and the entire neutral
  side tree are unchanged; all three no-neutral control endpoints are exact.
- Fresh installed-versus-branch-OFF runs of 2:10 and 6:157 match all 865 existing
  main-tree fields exactly. Historical stored endpoints are **not universally
  bit-identical**: 20 field differences are listed in `full_summary.json`, with
  maximum 0.00209942 GeV (8:130 beam-only free-loss IP pT). Their cause has not
  been established; conclusions use the fresh same-code A/B, not stored values.
- No accepted fit in this sample ignores a valid recorded trial better by more
  than 0.001 in the objective. This does not close the separate optimizer audit.
- Loaded-library records verify the isolated branch runtime for every A/B job.
  Its SHA256 is `911962a6b7e9e91d533018fc699c522a7e3c3d68e50dd58dbbdea774872ec986`.
- Installed library SHA256 remains
  `a7f75cd2c64b91a846df1823d69b206babe7a968b07aa55f9de9189c7ba4089b`;
  `breakpoint` remains at `e4a77baa0459689c02b86c068bb6ce8587bc9dab`.
  No installation, default promotion, remote push, or PFA/shared-KF change.

Conclusion: delayed neutral evaluation substantially reduces the previously
observed overshoots and is competitive with simple addition in this selected
sample. It is not a complete cure for input contamination or frozen loss-map
effects, and it is not yet a validated production replacement.

## Project-default deployment (2026-10-28)

Installed only the RecBreakpoint package using its generated CMake install
script, after a focused rebuild and all 13 C++ tests passed. The existing
runtime/configuration was backed up to
`/tmp/recbreakpoint-pre-delayed-install-mupUIS` before replacement. No GSF,
PFA, shared KF, submission helpers, or default constraint settings changed.

Installed and branch-build library SHA256 both equal
`911962a6b7e9e91d533018fc699c522a7e3c3d68e50dd58dbbdea774872ec986`.
The installed configurable exposes `ExperimentalDelayedNeutral=false` and
accepts explicit true. A fresh one-event delayed run (seed 2, event 10) loaded
the library from InstallArea, confirmed through the live process maps, and
terminated successfully. Its card, log, tuple and comparison check are under
`TrackingPerformanceStudies/breakpoint_delayed_neutral_ab_20261028/`, named
`installed_delayed_smoke.*` and `check_installed_smoke.py`.
All 869 main-tree branches and 57 neutral side-tree branches reproduce the
isolated delayed-build result exactly, including the applied-delayed flag.

Batch readiness is a separate pending step: `batch_breakpoint.py` still needs
to preserve `BP_EXPERIMENTAL_DELAYED_NEUTRAL` in frozen cards; submission stage
and input/output directories must be set for breakpoint-only reruns. No batch
was submitted and no existing tuple was overwritten.
