# CMS-like per-layer smoothing diagnostic

Date: 2026-08-27

Interpretation correction (2026-08-28): every historical `double_*` or
"F/B compatibility" value in this record uses the moment-matched
`F_upd(i)` defined below. It does not hold the forward side to the identity
lineage. Those values remain valid for their stated moment-matched question,
but they must not be described as
`F_updated_identity` versus `B_pred_identity/radiative`. The separate passive
identity-controlled channel, regression gate, and corrected comparison are
documented in
`agents_record/2026-08-28-cms-like-forward-identity-compatibility.md`.

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

### Finer-BH approximately-2% loss control

The earlier eight-event approximately-2% panel was rerun from the same current
build with `CEPCRuntimeCategoryAligned15Clear`, `MaxComponents=10`, component
weight cutoff `1e-4`, truth override off, Reverse seed covariance scale 1, and
CMS-like backward covariance scales 1 and 100. The source tree was not changed.
The BH15 child nearest truth had a 2.25% mean in all eight intervals. Depending
on interval thickness, its raw BH prior weight was only 0.0374--0.7192%, while
the identity prior was 78.94--98.85%.

The pT residuals `(pT_reco-pT_truth)/pT_truth` were:

```text
event   truth loss    LCIO       Reverse FullMix   CMS FullMix s1   CMS FullMix s100
63/62     2.068%     -2.2918%       -2.2919%          -2.2907%          -2.2935%
98/15     1.952%     -2.0899%       -2.0790%          -2.0830%          -2.1145%
59/72     1.972%     -1.9449%       -1.9369%          -1.9373%          -1.9556%
41/72     1.980%     -1.3823%       -1.3787%          -1.3878%          -1.4125%
12/2      2.019%     -2.0305%       -2.0131%          -2.0098%          -2.0185%
32/31     1.934%     -1.5041%       +0.7131%          +0.1437%          +0.1858%
9/57      1.906%     +0.0635%       +0.0540%          +0.0247%          +0.0236%
49/25     2.016%     -0.1496%       -0.1366%          -0.1320%          -0.1089%
```

Relative to the corresponding five-component runs, the BH15 FullMixtureMode
residual changed by less than 0.021 percentage point in seven of eight events.
Event 32/31 was the exception: Reverse changed from +0.7956% to +0.7131%, CMS
scale 1 from +0.3323% to +0.1437%, and CMS scale 100 from +0.3345% to +0.1858%.
The selected-panel mean absolute residual changed from 1.340% to 1.325% for
Reverse, 1.279% to 1.251% for CMS scale 1, and 1.289% to 1.264% for CMS scale
100. This is a selected mechanism panel, not a population performance result.

Raw local hit-compatibility chi-squares for identity versus the 2.25% sibling
from the same dominant identity parent were:

```text
event   Reverse scale 1       CMS scale 1          CMS scale 100
63/62   1.1362 / 1.1206      1.1304 / 1.1150      1.1413 / 1.1258
98/15   5.3077 / 5.3074      5.3043 / 5.3040      5.3351 / 5.3348
59/72   0.5423 / 0.1214      0.5343 / 0.1257      0.6266 / 0.0926
41/72   1.2391 / 1.7594      1.2441 / 1.7669      1.1034 / 1.5665
12/2    0.3824 / 1.9210      0.3848 / 1.9346      0.3774 / 1.7606
32/31   2.5454 / 3.0122      2.5638 / 3.0332      0.1452 / 0.1439
9/57    5.1390 / 5.1046      5.1397 / 5.1052      2.2277 / 2.1952
49/25   2.3872 / 2.5012      2.4360 / 2.5540      2.7484 / 2.8610
```

The matched child had lower local chi-square in four of eight events at scale
1 and five of eight at CMS scale 100. The raw five-dimensional
`F_updated`-versus-`B_pred` chi-squares were:

```text
event   CMS scale 1 identity/truth     CMS scale 100 identity/truth
98/15      8.4777 /   8.7007              8.5277 /  8.7396
59/72      9.2562 /  11.9663              9.2726 / 11.7841
41/72      2.0722 /   8.0216              2.0231 /  7.0315
12/2       1.6879 /  17.3373              1.7384 / 16.1982
32/31     23.4787 /   7.8625             27.7917 /  6.8381
9/57       0.0220 / 103.9331              0.1401 /  7.0533
49/25      0.4499 /  90.5648              1.3822 /  6.1528
```

Event 63/62 has no interior F/B comparison. Only event 32/31 prefers the
matched-loss child under the full F/B chi-square at either covariance scale.
Thus the finer grid does supply a loss mean much closer to 2%, but that is not
the dominant limitation in this panel. The very small radiative prior and the
correlated forward/backward state evidence still leave most endpoints near the
identity/LCIO solution. BH15 representation alone does not solve 2% loss
selection.

### Exact in-interval eBrem positions for the 2% panel

The embedded provenance was then used to place each Geant4 subtype-3 eBrem
step inside the exact accepted-hit interval. This is not a radial nearest-hit
classification. The flat interval supplies the two exact truth hooks, and the
input event's `GsfSimTrackerHitG4StepLinks` supplies each sensitive
traversal's first/hook/last step. The eBrem process is located at the Geant4
post-step point. The normalized interval coordinate is zero at the inner hit
hook and one at the outer hit hook.

The runtime interval remains midpoint-to-midpoint: downstream half of the
inner sensitive traversal, intervening support/gap, then upstream half of the
outer sensitive traversal. The exact results were:

```text
event   interval/radii [mm]       eBrem u   exact region                 Reverse local id/truth    CMS F/B id/truth
63/62   VXD0->VXD1 11.08->16.58   0.99775   outer sensor before hit       1.1362 / 1.1206           unavailable
98/15   VXD S04->S05 40.20->44.16 0.00509   inner sensor after hit        5.3077 / 5.3074            8.4777 /   8.7007
59/72   VXD->ITKB0 43.83->234.67  0.99819   gap just before outer hit     0.5423 / 0.1214            9.2562 /  11.9663
41/72   ITKB0->ITKB1 236.22->345.88 0.00204 gap just after inner hit      1.2391 / 1.7594            2.0722 /   8.0216
12/2    ITKB1->ITKB2 344.57->556.70 0.99378 gap just before outer hit     0.3824 / 1.9210            1.6879 /  17.3373
32/31   ITKB2->TPC 555.22->637.50 0.54424   middle gap                    2.5454 / 3.0122           23.4787 /   7.8625
9/57    last TPC->OTK 1747.50->1808.88 0.98925 gap just before outer hit   5.1390 / 5.1046            0.0220 / 103.9331
49/25   last TPC->OTK 1747.50->1807.66 0.98860 gap just before outer hit   2.3872 / 2.5012            0.4499 /  90.5648
```

Only event 63/62 has its loss inside the second sensitive half of the runtime
interval, namely before the downstream measurement midpoint. Its truth-like
child is already slightly better by local chi-square; it fails to dominate
because its BH prior is 0.0374% against a 98.85% identity prior, not because
the local hit rejects it. Event 98/15 is the sole loss inside the first
sensitive half and is also a near local tie. The other six losses occur in
non-sensitive support/gap material.

If “second half” means simply `u>0.5`, six events qualify. Their matched child
wins the Reverse local chi-square in three and loses in three. The location
therefore does not explain the panel globally. The geometry trend expected
from the implementation is actually directional: forward filtering convolves
after the inner hit and therefore places all interval loss at the inner
anchor, while reverse filtering convolves at the outer state before inward
propagation. A physical loss near the outer hook is geometrically closer to
the reverse approximation; a loss just after the inner hook is the larger
reverse misplacement. Event 41/72 is consistent with that effect, but event
12/2 is a clear counterexample, and event 32/31 recovers despite a mid-gap
loss. In-interval collapse location can contribute event by event, but it is
not the primary explanation for the persistent identity preference in this
2% panel.

### Unbiased 20-event extension of the location table

Twenty additional topology-clear events were selected solely by having one
positive truth-loss interval with cumulative eBrem loss in 1.898--2.056%.
Selection did not use the GSF endpoint or lineage outcome. The same current
build, BH15 model, `MaxComponents=10`, cutoff `1e-4`, truth override off,
Reverse scale 1, and CMS-like scale 1 were used. For the two intervals with
multiple eBrem steps, the displayed coordinate is the dominant step and the
minor step is noted separately. The CMS F/B column is the raw five-dimensional
`F_updated`-versus-`B_pred` chi-square; lower is preferred.

```text
event   loss     dominant u/region                         Reverse local id/truth     CMS F/B id/truth
70/19   1.997%   0.00025 inner sensor after hit (*)        1.9599 / 1.7396            17.3102 /  17.9282
74/35   1.994%   0.98872 gap near outer hit                4.6466 / 4.7453             1.2414 /  79.2687
67/46   1.985%   0.53857 middle gap                        0.3286 / 0.5030             8.7868 /   3.5281
80/17   1.981%   0.99954 gap near outer hit                0.3217 / 1.3641             9.3478 /   8.2602
78/87   1.980%   0.38639 gap (*)                           0.0791 / 2.5573             1.6312 /   6.4038
28/91   2.021%   0.06002 gap near inner hit                2.5932 / 2.5936             3.6924 /   3.8272
32/89   1.977%   0.99217 gap near outer hit                1.1525 / 3.8383            13.2317 /   9.1501
79/1    1.970%   0.54369 middle gap                        0.1266 / 0.3048            10.4482 /  12.9715
65/66   1.966%   0.54718 middle gap                        0.7426 / 0.9466            13.7209 /   9.4696
92/11   2.038%   0.99979 outer sensor before hit          20.0124 / 29.1997           87.5611 / 128.5085
83/4    2.041%   0.45476 middle gap                        0.7865 / 0.7925             0.5953 /  76.2745
94/34   2.041%   0.46055 middle gap                        0.9762 / 1.0155             0.4481 /  95.4161
59/71   1.952%   0.99998 outer sensor before hit           0.3371 / 0.0326             6.3118 /   6.0136
89/18   2.056%   0.99497 gap near outer hit                5.0150 / 4.9947             0.1553 /  76.2262
23/84   1.935%   0.00192 gap near inner hit                3.0932 / 5.4895             5.2636 /   9.6297
5/83    2.069%   0.99879 gap near outer hit                2.9997 / 0.4986             6.9041 /   1.6621
58/34   1.931%   0.46029 middle gap                        1.3287 / 1.3888             2.4614 / 107.4517
44/73   1.929%   0.99819 gap near outer hit                1.2731 / 3.4105             2.2986 /   4.4332
13/57   1.928%   0.61603 outer TPC sensor before hit       2.3957 / no BH child        unavailable
48/94   1.898%   0.44394 middle gap                        3.3233 / 3.2029             0.2487 /  72.9567
```

`(*)` Event 70/19 also has a 0.099% step at `u=0.53738`; its dominant 1.898%
step is at `u=0.00025`. Event 78/87 also has a 0.041% step at `u=0.07955`;
its dominant 1.939% step is at `u=0.38639`.

Event 13/57 is an internal TPC-row interval. Exact Geant4 and DD4hep thickness
are both `4.26123e-5`; forward/reverse runtime weighted thickness is
`4.33264e-5`, below `BHSplitThreshold=1e-4`. Both directions therefore record
zero above-threshold paths and create no radiative child. This is a gating
case, not a failed choice between identity and a truth-like sibling.

In the 20 new events, the matched child has lower Reverse local chi-square in
5/19 comparable events and lower CMS F/B chi-square in 6/19. Combining the
original eight and the new twenty gives 9/27 local preferences and 7/26 F/B
preferences; event 13/57 has no child, while first-interval event 63/62 has no
interior F/B comparison.

Using the dominant eBrem step, the combined sample has ten early (`u<0.5`)
and eighteen late (`u>=0.5`) events. The local preference rate is 3/10 early
and 6/17 late. The F/B truth-like preference is 0/10 early and 7/16 late.
Thus a late loss does not explain why the F/B score rejects the truth-like
child; it is actually a necessary but insufficient condition for all seven
truth-like F/B preferences in this panel. Nine of sixteen comparable late
losses still prefer identity. The result is consistent with Reverse placing
the collapsed correction at the outer state, but it remains selected-sample,
correlated-state evidence rather than a calibrated location efficiency.

### Adjacent-surface audit of the Reverse local chi-square

The Reverse source and recorded DAG were rechecked after questioning whether
the local chi-square above had been read at the wrong surface. For a truth
interval `i -> i+1`, the loop sets `target=hits[i]`, constructs the material
path from the state at `hits[i+1]` to that target, calls
`split(..., reverse=true)` with `truthRetainedFraction(i,i+1)`, and then calls
`addAndFit(targetHit,...)`. The measurement node records `hit_index=i` and the
exact returned `dchi`. Thus the displayed local value is at the inner bounding
hit `i`. The split node also carries `hit_index=i` as the interval owner/target
label even though the correction is applied to the outer continuation before
inward propagation; that label can visually suggest an off-by-one if read as
a physical process point.

The lineage was traced one hit outward and one hit inward. At outer hit `i+1`
the two children do not exist yet, so only their common pre-split parent's
local chi-square can be shown. At inner hit `i-1`, the table reports the
highest-posterior measurement descendant reachable separately from the
identity and matched-loss nodes at `i`. It is not claimed to be the unchanged
Gaussian: another BH split and/or KL merge can occur between the two hits.
No reported identity/truth descendant pair shared the same downstream node.
`cut` means the selected truth-like node at `i` had no descendant at `i-1`.

The original eight-event panel gives:

```text
event   interval   outer i+1 common   at i identity/truth   at i-1 identity-desc/truth-desc
63/62    0->1           4.6686          1.1362 / 1.1206          no inner hit
98/15    4->5           4.4819          5.3077 / 5.3074          2.7722 / 3.0588
59/72    3->4           1.9520          0.5423 / 0.1214          0.9467 / 1.7160
41/72    6->7           3.5186          1.2391 / 1.7594          0.2804 / 3.6362
12/2     6->7           1.1240          0.3824 / 1.9210          0.6297 / 1.4366
32/31    8->9           0.4218          2.5454 / 3.0122          4.8247 / cut
9/57   226->227         outer seed      5.1390 / 5.1046          0.7372 / 0.6783
49/25  229->230         outer seed      2.3872 / 2.5012          1.6998 / 1.7042
```

The twenty-event extension gives:

```text
event   interval   outer i+1 common   at i identity/truth   at i-1 identity-desc/truth-desc
70/19    9->10          0.3123          1.9599 / 1.7396          2.8347 / 2.9600
74/35  231->232         outer seed      4.6466 / 4.7453          0.0500 / 0.0561
67/46    8->9           0.2514          0.3286 / 0.5030          1.9241 / 0.1453
80/17    5->6           6.4202          0.3217 / 1.3641          0.5093 / 0.5211
78/87    5->6           2.7944          0.0791 / 2.5573          2.6851 / 2.9412
28/91    4->5           0.1029          2.5932 / 2.5936          0.2148 / 0.3892
32/89    7->8           0.3510          1.1525 / 3.8383          2.0872 / 0.2078
79/1     9->10          0.8842          0.1266 / 0.3048          4.5397 / 4.0719
65/66    8->9           4.8905          0.7426 / 0.9466          2.0977 / cut
92/11    5->6           1.4975         20.0124 / 29.1997         70.2500 / cut
83/4   232->233         outer seed      0.7865 / 0.7925          0.8880 / 0.8842
94/34  231->232         outer seed      0.9762 / 1.0155          2.8272 / 2.7700
59/71    3->4           1.4273          0.3371 / 0.0326          5.2153 / 1.5174
89/18  231->232         outer seed      5.0150 / 4.9947          2.9325 / 2.9457
23/84    5->6           5.1537          3.0932 / 5.4895          3.6448 / 4.3502
5/83     6->7           0.2979          2.9997 / 0.4986          2.3655 / 1.8482
58/34  232->233         outer seed      1.3287 / 1.3888          2.3623 / 2.2859
44/73    5->6           0.3155          1.2731 / 3.4105          2.7638 / 3.2091
13/57  155->156         no BH split     no identity/truth pair   no pair
48/94  231->232         outer seed      3.3233 / 3.2029          0.6401 / 0.7196
```

The matched-loss descendant has lower local chi-square at `i-1` in 9/23
comparable events. Several preferences reverse between adjacent hits: events
67/46, 32/89, 79/1, 83/4, 94/34, and 58/34 favor identity at `i` but the
matched-loss family at `i-1`; events 98/15, 59/72, 70/19, 89/18, and 48/94
move the other way. This demonstrates that one measurement's local ordering
is unstable under the next propagation/update. It does not reveal an
off-by-one readout: the stored `dchi2` at `i` exactly matches the source's
target-hit update. A true shifted-split test would require constructing the
radiative hypothesis at a different interval boundary; the common parent
chi-square at `i+1` cannot serve as that counterfactual.
