# Forward kappa seed covariance: standard-KF-scale gate

Date: 2026-08-28

## Question

The CMS-like identity-controlled compatibility study showed that its saved
forward identity is not independent: the forward GSF copies the smoothed
all-hit `CompleteTracks::AtFirstHit` mean and gives kappa a very small seed
variance, `KappaSeedCov=1e-7`. Before changing the seed construction to a
fresh prefit, test whether using the standard KF's nominal initial curvature
variance scale reduces that correlation on the frozen near-two-percent-loss
panel.

## Standard KF reference

The normal `CompleteTracks` final refit first calls the covariance-only
`KalTestTool::Fit` overload. It creates a helix prefit from the first, middle,
and last two-dimensional hits, then assigns this loose diagonal covariance in
EDM track parameters:

| parameter | variance |
|---|---:|
| d0 | `1e6 mm2` |
| phi | `1e2 rad2` |
| omega | `1e-4 mm-2` |
| z0 | `1e6 mm2` |
| tanLambda | `1e2` |

At the TDR tracker field `B=3 T`, `omega=alpha*kappa` with
`alpha=B*2.99792458e-4=8.99377374e-4`. The KF omega variance therefore
corresponds to `Var(kappa)=1e-4/alpha2=123.628 GeV-2`. The focused test used
the rounded value `KappaSeedCov=123.6` requested by the user. This test changes
only the kappa entry of the existing GSF seed covariance; it does not yet copy
the standard KF's prefit construction or its other four loose variances.

## Frozen A/B setup

The executable source was commit `c0a5cbf` and the 28-event selected panel was
rerun on both sides with:

```text
method = cms-like
BHModel = CEPCRuntimeCategoryAligned15Clear
MaterialPathMode = DD4hepBetweenSurfaces
MaxComponents = 10
ComponentWeightCutoff = 1e-4
ProtectIdentityLineage = true
CmsErrorRescaling = 1
TruthBHLossOverride = false
```

The only A/B difference was:

```text
baseline: KappaSeedCov = 1e-7
test:     KappaSeedCov = 123.6
```

The selected `(input file, zero-based entry)` pairs were:

```text
63/62 98/15 59/72 41/72 12/2 32/31 9/57 49/25
70/19 74/35 67/46 80/17 78/87 28/91 32/89 79/1
65/66 92/11 83/4 94/34 59/71 89/18 23/84 5/83
58/34 44/73 13/57 48/94
```

All 28 broad-seed jobs completed. The persisted seed lineage on event 12/2
records kappa variance `123.6` exactly, versus `1e-7` on the baseline side.
Event 13/57 still has no runtime BH split at its internal-TPC truth-loss
interval. Event 63/62 still lacks an exact forward-updated identity message at
loss interval zero. The same 26 events therefore support the requested fixed-
forward identity-versus-truth-like comparison on both sides.

Generated ROOT files, logs, and analysis CSVs were kept under `/tmp` and are
not project source or durable inputs. The complete numerical results needed to
interpret the gate are reproduced below.

## Identity-controlled local compatibility

Define the reported delta at the truth-loss hit as:

```text
delta = chi2(F_updated_identity, B_pred_truth_like)
      - chi2(F_updated_identity, B_pred_identity)
```

Negative favors the truth-like backward child. The aggregate change was:

| quantity | `1e-7` | `123.6` |
|---|---:|---:|
| truth-like wins | 4/26 | 9/26 |
| median delta | +17.54 | +3.04 |
| moment-matched truth-like wins | 7/26 | 9/26 |

Every one of the 26 event deltas moved in the truth-like direction. The mean
shift was `-9.00 chi2`, the median shift was `-4.63`, and the least negative
shift was `-0.257`. Five events changed from identity to truth-like preference;
none changed in the opposite direction.

| event | hit | delta at `1e-7` | delta at `123.6` | preference |
|---:|---:|---:|---:|---|
| 98/15 | 4 | +7.6097 | -0.0761 | identity -> truth-like |
| 59/72 | 3 | +11.7143 | +2.7535 | identity -> identity |
| 41/72 | 6 | +6.9248 | +2.5631 | identity -> identity |
| 12/2 | 6 | +28.0209 | +6.4637 | identity -> identity |
| 32/31 | 8 | -16.2706 | -22.7725 | truth-like -> truth-like |
| 9/57 | 226 | +103.8919 | +100.4032 | identity -> identity |
| 49/25 | 229 | +90.0632 | +86.0044 | identity -> identity |
| 70/19 | 9 | +21.3707 | +18.2675 | identity -> identity |
| 74/35 | 231 | +77.8514 | +76.6466 | identity -> identity |
| 67/46 | 8 | -5.8639 | -12.5388 | truth-like -> truth-like |
| 80/17 | 5 | +6.5544 | -0.2596 | identity -> truth-like |
| 78/87 | 5 | +7.9699 | +4.6775 | identity -> identity |
| 28/91 | 4 | +2.1571 | +0.0431 | identity -> identity |
| 32/89 | 7 | -3.5809 | -14.8741 | truth-like -> truth-like |
| 79/1 | 9 | +2.2614 | -7.1076 | identity -> truth-like |
| 65/66 | 8 | -12.2103 | -14.7484 | truth-like -> truth-like |
| 92/11 | 5 | +49.7225 | +11.4768 | identity -> identity |
| 83/4 | 232 | +81.6197 | +80.3995 | identity -> identity |
| 94/34 | 231 | +95.0329 | +93.6786 | identity -> identity |
| 59/71 | 3 | +32.9089 | -3.7077 | identity -> truth-like |
| 89/18 | 231 | +76.0300 | +75.7726 | identity -> identity |
| 23/84 | 5 | +25.4505 | +3.3244 | identity -> identity |
| 5/83 | 6 | +13.7029 | -4.5954 | identity -> truth-like |
| 58/34 | 232 | +105.0707 | +101.3446 | identity -> identity |
| 44/73 | 5 | +6.8439 | +2.1178 | identity -> identity |
| 48/94 | 231 | +72.7525 | +68.2134 | identity -> identity |

The broad seed helps predominantly at early/silicon hits. None of the eight
late losses at hits 226--232 changes preference; their remaining identity
advantages are `68--101 chi2`. Seed correlation therefore explains a real
part of the early-hit identity preference, but it does not explain the late-
loss incompatibility.

## Published pT endpoint check

On this selected 28-event panel, the pT residual endpoint summaries were:

| CMS-like endpoint | mean abs, `1e-7` | mean abs, `123.6` | median abs, `1e-7` | median abs, `123.6` | eventwise improvements |
|---|---:|---:|---:|---:|---:|
| BestBranch | 1.1209% | 1.2369% | 1.3540% | 1.4755% | 15/28 |
| WeightedMean | 1.1665% | 1.1332% | 1.3722% | 1.1110% | 17/28 |
| FullMixtureMode | 1.2004% | 1.1230% | 1.5161% | 1.3975% | 19/28 |

The broader seed produced meaningful FullMixtureMode recoveries for 67/46,
32/89, and 79/1, but worsened 32/31 and 65/66. The aggregate endpoint changes
are selected-panel diagnostics, not population validation or authority to
change the default.

## Conclusion and boundary

`KappaSeedCov=123.6` is mechanically viable under the focused setup and
unambiguously reduces the identity-controlled correlation: all auditable
events move toward the truth-like hypothesis, and truth-like local preference
increases from 4/26 to 9/26. The test does not make the messages independent.
The seed mean remains the smoothed all-hit `CompleteTracks::AtFirstHit` value,
the other four GSF seed variances remain much tighter than the standard KF,
and hit zero remains treated as already filtered.

Keep the compiled and maintained-card default `KappaSeedCov=1e-7` unchanged.
Before claiming a correct two-filter construction, the next reviewed design
choice is whether to adopt a fresh geometric prefit, the full loose covariance,
and an explicit first-hit update. The persistent late-loss failure should be
diagnosed separately rather than attributed to the forward seed alone.
