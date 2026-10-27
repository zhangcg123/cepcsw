# Delayed neutral-energy update experiment

## Branch and scope

Created 2026-10-28 on `breakpoint-delayed-neutral` from `breakpoint` at
`e4a77baa0459689c02b86c068bb6ce8587bc9dab`. The original branch is preserved.
The tested temporary implementation is now checkpointed on this branch;
`ExperimentalDelayedNeutral` remains OFF by default. No installation or remote
push is part of this checkpoint. Existing unrelated working-tree edits are
neither committed nor reverted.

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
