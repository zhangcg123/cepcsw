# CMS-like per-layer smoothing diagnostic

Date: 2026-08-27

## Outcome

The CMS-like workflow now records a default-on, passive per-layer smoothing
diagnostic on its existing accepted backward measurement lineage nodes. It
answers a narrower question than changing the refit: if the inner and outer
hit information is combined at a layer, how strongly would the complete-hit
compatibility favor each surviving backward component, both with and without
the component's already accumulated prior weight?

The diagnostic does not replace a component state or weight. It does not feed
the backward recursion, weight cutoff, KL reduction, innermost product
mixture, or any of the BestBranch, WeightedMean, or FullMixtureMode endpoint
publications. No configurable property was added. It is populated
automatically for CMS-like jobs and remains empty for the forward, smoother,
reverse, and global-loss workflows.

This is a mechanical diagnostic, not evidence that per-layer smoothing
improves the GSF physics performance.

## Definition

At accepted interior hit `i`, the implementation retains these Gaussian
messages:

```text
F_pred(i) = moment match of accepted forward predictions before hit i
F_upd(i)  = moment match of the live forward mixture after hit i,
            including normal cutoff and reduction
B_pred(i,k) = backward component k predicted to hit i before its update
```

Two products are evaluated for every accepted backward component:

```text
Q_other(i,k) proportional to F_pred(i) * B_pred(i,k)
Q_all(i,k)   proportional to F_upd(i)  * B_pred(i,k)
```

`Q_other` contains the available inner and outer hit information but omits hit
`i`; `Q_all` additionally contains hit `i` exactly once. The Gaussian product
normalization supplies `all_other_log_overlap`.

The current hit is evaluated at `Q_other` with the residual, measurement
projector, and reference innovation covariance from the exact accepted
MarlinTrk `addAndFit` update. KalTest's accepted `GetDeltaChi2()` is the value
used by the live component weights and can differ slightly from a freshly
reconstructed residual quadratic. The transported diagnostic therefore
anchors both terms to the accepted reference and applies only the linearized
change:

```text
dchi2_smooth = dchi2_accepted
             + quadratic(Q_other) - quadratic(B_pred)

logdet_smooth = logdet_accepted
              + logdet(S(Q_other)) - logdet(S(B_pred))

logL_local = -0.5 * [m*log(2*pi) + logdet_smooth + dchi2_smooth]
```

Here `m` is the one- or two-dimensional measurement size. A negative
transported chi-square caused by the finite linearization correction is
clamped to its physical lower bound zero.

The two diagnostic ranking scores are:

```text
log_evidence = all_other_log_overlap + logL_local

log_weight_with_prior = log(backward_prior_weight) + log_evidence
```

They are normalized independently within each
`(input_track_index, hit_index)` group. `normalized_evidence` removes the
existing backward prior and exposes the relative all-hit compatibility;
`normalized_weight_with_prior` shows what remains after restoring that prior.
Their comparison is the intended audit of whether the full hit information
could overcome a small radiative BH/lineage weight.

The forward side is moment-matched once per layer, so the recording cost grows
with the number of forward and backward components rather than constructing
the full `N_forward * N_backward` product population at every layer. The full
component product remains limited to the established innermost CMS-like
publication layer.

## Persisted schema

The following row-aligned EDM collections contain one value per lineage node:

- `GSFLineageNodeCmsSmoothValid`
- `GSFLineageNodeCmsSmoothForwardPredictedKappa`
- `GSFLineageNodeCmsSmoothForwardPredictedKappaVariance`
- `GSFLineageNodeCmsSmoothForwardUpdatedKappa`
- `GSFLineageNodeCmsSmoothForwardUpdatedKappaVariance`
- `GSFLineageNodeCmsSmoothAllOtherKappa`
- `GSFLineageNodeCmsSmoothAllOtherKappaVariance`
- `GSFLineageNodeCmsSmoothAllHitKappa`
- `GSFLineageNodeCmsSmoothAllHitKappaVariance`
- `GSFLineageNodeCmsSmoothAllOtherLogOverlap`
- `GSFLineageNodeCmsSmoothLocalDChi2`
- `GSFLineageNodeCmsSmoothLocalLogDetInnovation`
- `GSFLineageNodeCmsSmoothLocalLogLikelihood`
- `GSFLineageNodeCmsSmoothLogEvidence`
- `GSFLineageNodeCmsSmoothLogWeightWithPrior`
- `GSFLineageNodeCmsSmoothNormalizedEvidence`
- `GSFLineageNodeCmsSmoothNormalizedWeightWithPrior`

`RecGsfFlatTuple` mirrors these as
`lineage_node_cms_smooth_*`. It additionally derives
`lineage_node_cms_smooth_all_other_pT` and
`lineage_node_cms_smooth_all_hit_pT` as `1/abs(kappa)`. The existing
`lineage_node_input_track_index`, `lineage_node_hit_index`,
`lineage_node_no_radiation`, BH fields, original local innovation fields, and
normalized backward posterior provide the component identity and comparison
columns. Non-applicable nodes have `cms_smooth_valid=0` and NaN floating
fields.

## Focused mechanical gate

Build and install passed for `RecGsfTracking` and `RecGsfFlatTuple` in the
EL9/LCG 105 build. The focused run used input
`trk_large_20260823/trk-e--2.0-85-1.root`, zero-based events 11, 16, and 17,
and the frozen production comparison settings:

```text
method = cms-like
BHModel = CEPC2GeV85StepConditioned
MaterialPathMode = DD4hepBetweenSurfaces
MaxComponents = 12
ComponentWeightCutoff = 5e-3
CmsErrorRescaling = 100
```

The job completed all 18 event rows while fitting only the selected events.
It emitted 691 per-layer diagnostic summaries. The accepted valid candidate
counts were 2,079, 1,535, and 1,357 for events 11, 16, and 17. Every new flat
vector had exactly the lineage-node length. Both normalized scores summed to
one per input-track/per-hit group, with a maximum absolute error of
`6.66e-16`.

Evaluating every original backward predicted state through the transported
diagnostic reproduced the live accepted chi-square to `5.68e-14` absolute and
the accepted innovation log-determinant to `3.55e-15` across the 691 layer
summaries. A same-input comparison across the final two passive score
implementations found zero absolute difference in all tested
BestBranch/WeightedMean/FullMixtureMode `omega`, `d0`, `z0`, `phi`, `tanl`,
`pT`, and `p` endpoint scalars for events 11, 16, and 17.

## Interpretation boundary and next analysis

The backward pass is seeded from the final forward fit, so its outer message
is statistically correlated with the saved forward messages. In addition,
the forward component mixture is reduced to one Gaussian moment at each
layer. Consequently, `normalized_evidence` and
`normalized_weight_with_prior` are relative diagnostic scores, not calibrated
Bayes probabilities or proof that two independent filters agree.

The next analysis should stay passive. At truth eBrem layers in matched bad
and good controls, identify the identity and truth-compatible radiative
backward nodes and tabulate:

```text
existing backward prior
existing local dchi2 and normalized posterior
smoothed local dchi2
normalized evidence without prior
normalized weight with prior
all-other and all-hit kappa/pT
```

The decisive comparison is whether the truth-compatible lineage gains rank in
the evidence-only column but loses it again when the inherited prior is
restored. Only after that pattern is demonstrated on focused and held-out
events should any reviewed experiment feed a per-layer smoothing score back
into live component weights.

## 2026-08-28 full-state compatibility extension and 2% gate

The passive schema was extended with the raw compatibility of the
forward-updated moment match and each backward-predicted component:

```text
delta = mu_Fupdated - mu_Bpred
Csum = C_Fupdated + C_Bpred
all_hit_compatibility_dchi2 = delta^T Csum^-1 delta
all_hit_compatibility_logdet = log(det(Csum))
all_hit_log_overlap = -0.5 *
    [5*log(2*pi) + all_hit_compatibility_logdet
                 + all_hit_compatibility_dchi2]
```

The new row-aligned EDM collections are:

- `GSFLineageNodeCmsSmoothAllHitCompatibilityDChi2`
- `GSFLineageNodeCmsSmoothAllHitCompatibilityLogDet`
- `GSFLineageNodeCmsSmoothAllHitLogOverlap`

`RecGsfFlatTuple` mirrors them as:

- `lineage_node_cms_smooth_all_hit_compatibility_dchi2`
- `lineage_node_cms_smooth_all_hit_compatibility_logdet`
- `lineage_node_cms_smooth_all_hit_log_overlap`

They are finite under the existing `cms_smooth_valid=1` contract and NaN
elsewhere. They are passive and do not modify the product state, component
weight, reduction, or endpoint publication. No configurable property was
added.

A focused CMS-like rerun of job 12 entry 2 used
`CEPC2GeV85StepConditioned`, `MaxComponents=10`,
`ComponentWeightCutoff=1e-4`, `ReverseKappaSeedCov=1`, and
`CmsErrorRescaling=1`. All pre-existing endpoint scalars and all pre-existing
CMS smoothing vectors were bit-for-bit equal to the pre-extension output. The
three new vectors had the common 5,306-node lineage length and 1,960 finite
accepted-interior entries. Events 11, 16, and 17 also completed with 2,012,
1,883, and 1,780 finite entries, respectively.

For sibling radiative and identity hypotheses define

```text
delta_score = score_radiative - score_identity
relative_factor = exp(-0.5 * delta_score)
```

where `score = dchi2 + logdet`. Directly differencing the separately stored
`all_hit_log_overlap` reproduced `relative_factor` for every evaluable member
of an eight-event approximately-2% truth-loss panel. This closes the
mechanical definition. The population result was not uniformly favorable:
six of seven evaluable events preferred identity more strongly under this
full-state compatibility, while one event preferred the radiative sibling by
a factor near 980. The remaining event lost energy in the first accepted
interval and has no interior forward-updated/backward-predicted comparison.
This sharp two-sided behavior makes the quantity a useful diagnostic, not a
validated replacement for live weighting.

### Interpretation correction from the kappa-only audit

The interval and reverse-BH alignment were rechecked in source. A truth
interval `i -> i+1` is applied by the reverse loop at `reverseHit=i` before
updating hit `i`; `BetheHeitlerSplitter(reverse=true)` maps
`kappa_child = kappa_parent * retained_fraction`, which is the intended
outer-to-inner restoration of the pre-loss momentum. The analysis also chose
the recorded BH child whose retained-fraction mean is nearest the exact truth
fraction. The observed identity preference is therefore not an index or loss-
sign mistake.

It is also not primarily caused by the other four helix coordinates. Repeating
the comparison with only kappa and its marginal variance gave the following
identity/truth-like chi-squares for the seven evaluable events:

```text
event   identity     truth-like
98/15   0.002491     0.443095
59/72   0.008383     0.098949
41/72   0.003225     1.924286
12/2    0.028545     4.212290
32/31   2.214134     1.524469
9/57    0.017386     4.838264
49/25   0.126395     7.516719
```

The decisive limitation is that this run used `CmsErrorRescaling=1` and the
CMS-like backward seed is cloned from the final forward prediction. It already
carries the forward-fit mean, covariance, component identities, and weights
derived from the inner measurements. The subsequently propagated `B_pred` is
therefore correlated with `F_updated`; it is not an independent outer-hit
message. Moreover, `F_updated` is one moment match after normal cutoff and KL
reduction rather than a forward state conditioned on the same backward
lineage. In six events the identity `B_pred` consequently already remains
close to the forward pre-loss kappa, and applying the approximately-2% reverse
correction moves the nominally truth-like child away from it. Event 32/31 is
the lone opposite case.

Thus the recorded full-state overlap is mechanically correct for the two
states supplied to it, but it must not be described as an independent two-
filter likelihood or global Bayes evidence. A physics-valid test requires a
backward message initialized independently of the forward fit (and preferably
lineage-conditioned forward messages), not merely another covariance rescale.

### `CmsErrorRescaling=100` control

The same eight approximately-2% events were rerun with only the CMS backward
seed covariance scale changed from 1 to 100. The five-component
`CEPC2GeV85StepConditioned` model, `MaxComponents=10`, weight cutoff `1e-4`,
truth override off, event selection, and passive scoring implementation were
unchanged.

The raw CMS backward hit-compatibility chi-squares were:

```text
event   scale-1 identity/truth-like    scale-100 identity/truth-like
63/62   1.130426 / 1.113171            1.141309 / 1.123976
98/15   5.304280 / 5.303995            5.335069 / 5.334745
59/72   0.534329 / 0.175043            0.626631 / 0.135749
41/72   1.244140 / 1.791716            1.103385 / 1.590813
12/2    0.384767 / 1.606363            0.377419 / 1.468550
32/31   2.563757 / 3.077609            0.145240 / 0.145324
9/57    5.139733 / 5.103443            2.227657 / 2.193533
49/25   2.436005 / 2.543042            2.748358 / 2.850650
```

The raw five-dimensional `F_updated` versus `B_pred` chi-squares were:

```text
event   scale-1 identity/truth-like    scale-100 identity/truth-like
98/15    8.478697 /  8.882631           8.528287 /  8.915902
59/72    9.266170 / 11.592032           9.282520 / 11.472329
41/72    2.071528 /  4.773061           2.022923 /  4.386237
12/2     1.688702 /  5.576647           1.738400 /  5.348761
32/31   23.040780 /  7.374187          27.325259 /  6.759769
9/57     0.023051 /  4.842729           0.140692 /  3.179806
49/25    0.455837 /  7.686619           1.389332 /  4.045387
```

Event 63/62 has no interior F/B comparison. Covariance inflation changes the
magnitudes, notably making the event-32/31 truth-like preference stronger and
reducing several identity/truth separations, but the F/B preference remains
identity in six of seven events. This is expected because scaling changes the
backward seed covariance only; it does not remove the forward-derived seed
mean, components, or weights and therefore does not make the messages
independent.

### Reverse-corrected 4--6% loss control

Eight topology-clear, single-interval events were selected from the 4--6%
truth-eBrem category because Reverse FullMixtureMode substantially improved
their stored LCIO residual. They were rerun from the current build with the
original `CEPCRuntimeCategoryAligned15Clear` model, `MaxComponents=10`, cutoff
`1e-4`, reverse seed covariance scale 1, truth override off, and CMS backward
scales 1 and 100. The fifteen-component model was retained because the
five-component production control has no approximately-5% child; its relevant
means bracket 5% near 2.2% and 10.5%.

The same-code Reverse rerun reproduced the selection:

```text
event   truth loss   matched BH mean   LCIO residual   Reverse FullMixture
48/43     5.643%         5.35%            -5.170%            +0.323%
52/44     4.650%         4.10%            -4.738%            -0.116%
66/46     4.995%         5.35%            -4.602%            +0.116%
61/36     5.444%         5.35%            -4.628%            -0.291%
33/10     5.417%         5.35%            -4.723%            +0.526%
74/45     4.520%         4.10%            -4.299%            -0.177%
34/98     5.090%         5.35%            -4.052%            +0.009%
99/96     5.692%         5.35%            -4.371%            +0.537%
```

Raw local hit-compatibility chi-squares for identity versus the matched-loss
sibling from the dominant identity parent were:

```text
event   Reverse scale 1       CMS scale 1          CMS scale 100
48/43   0.5744 / 1.1096      0.5648 / 1.1333      1.9072 / 0.6170
52/44   0.6736 / 0.4896      0.6703 / 0.4893      0.6140 / 0.3697
66/46   0.8661 / 2.3264      0.8649 / 2.3597      1.2515 / 0.8350
61/36   9.5179 / 11.6139     9.5506 / 11.6505     5.2153 / 6.4407
33/10   0.4813 / 1.8336      0.4748 / 1.8528      5.8625 / 0.3931
74/45   0.6380 / 7.4896      0.6426 / 7.5332      0.6625 / 6.4395
34/98   4.4636 / 5.5959      4.2840 / 5.4006      0.2724 / 0.1437
99/96   8.6923 / 10.2097     8.6498 / 10.1649     0.3281 / 0.4073
```

The raw five-dimensional `F_updated` versus `B_pred` chi-squares were:

```text
event   CMS scale 1 identity/truth     CMS scale 100 identity/truth
48/43      116.8658 /  6.4788              124.4372 /  5.4051
52/44       12.8667 / 14.8060               12.9559 / 14.6130
66/46       36.9934 / 18.7509               41.7723 / 13.7258
61/36       37.4933 / 29.8557               38.7322 / 18.9279
33/10       17.1645 /  6.0602               26.2816 /  4.7452
74/45       27.3926 / 15.6183               28.1589 / 13.9928
34/98       37.1515 / 20.8775               49.1501 /  3.8631
99/96       37.8823 / 20.5009               47.1273 /  3.1103
```

At scale 1, the Reverse and CMS backward local chi-square prefers the matched
loss sibling in only one of eight events. Scale 100 changes the CMS local
preference to the matched sibling in five of eight. The full F/B chi-square
prefers it in seven of eight at both scales and generally more strongly at
scale 100. Nevertheless, the audited matched sibling from the dominant
identity parent has lineage fate 3 (weight cutoff) in all eight Reverse runs.
The correct final pT therefore cannot be attributed simply to survival of this
one locally matched child; other parent lineages, interval histories, and the
final mixture endpoint remain involved. The F/B score also remains correlated
and is not a calibrated likelihood despite its favorable 5% ordering.
