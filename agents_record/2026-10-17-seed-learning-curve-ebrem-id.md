# Seed-count learning curve for the full-context eBrem identifier (2026-10-17)

## Question and controlled comparison

The user asked whether insufficient training statistics explain the weak
track-level detection and interval location of >1% tracker eBrem. The existing
bidirectional GRU was retrained on nested 25% and 50% subsets of **whole
training simulation seeds**, twice with different subset selections. The
original saved 100% checkpoint was replayed as the reference. The hit/interval
inputs, architecture, objective, optimizer, maximum 30 epochs, five-epoch
patience, and validation checkpoint rule (average precision plus exact top-one
interval recall) remained the same. Each subset computed its own normalization
from its training tracks only. Every model was evaluated on the same 2,559
validation tracks from 14 held-out validation seeds (736 positive tracks, 850
qualifying intervals); the 2,554-track test split was not used for this study.

The 5%-false-positive-rate (FPR) threshold was selected separately from the
same validation negatives for each model. All models therefore have 92/1,823
false-positive validation tracks. The comparison is diagnostic, not an
independent physics estimate: the original checkpoint was selected using this
validation set, this set has informed prior development, and there is only one
100% training initialization.

| Training seeds/tracks | Positive training tracks | Validation AP | Detected positive tracks / 736 at ~5% FPR | Exact top-one intervals / 850, unrestricted |
|---|---:|---:|---:|---:|
| 11 / 2,008 (25%, repeat 0) | 587 | 0.723 | 364 | 360 |
| 11 / 2,021 (25%, repeat 1) | 623 | 0.734 | 369 | 361 |
| 22 / 4,028 (50%, repeat 0) | 1,183 | 0.726 | 363 | 361 |
| 22 / 4,030 (50%, repeat 1) | 1,243 | 0.728 | 376 | 358 |
| 44 / 8,069 (100%, saved model) | 2,464 | 0.738 | 379 | 348 |

For the 120 validation tracks whose largest interval loss is 1--2%, the
numbers detected at ~5% FPR are respectively 21, 24, 19, 27 and 24 in the
same row order. Thus the full sample does not improve detection of this
near-threshold class in this comparison. The corresponding counts for 275
tracks with >=10% loss are 161, 165, 167, 167 and 171.

Using paired resampling of the 14 validation seeds with each model's original
threshold held fixed, the full model's track-efficiency advantage over each
subset is 0.4--2.2 percentage points; the 95% bootstrap interval for the
second 50% subset includes zero (-0.3 to +1.2 points). These intervals describe
paired validation-sample variation only. They do not include training subset
or model-initialization uncertainty, nor the effect of selecting thresholds
and checkpoints on the validation set.

## Interpretation and continuation

More same-distribution training seeds give a **small** track-classification
benefit in these runs: 379/736 positives at full size versus 363--376/736
with half and 364--369/736 with a quarter. Average precision increases to
0.738 at full size, but it is not monotonic across the subset repeats.
Unrestricted exact interval top-one is actually 348/850 at full size versus
358--361/850 in the four reduced runs. These observations do not support
training count as the principal explanation of weak interval identification
within the current 2k--8k-track range. They do not rule out benefits from
larger, more varied independent samples or from a different objective.

The next diagnostic should target information and model structure: the 24
selected per-hit summaries omit full covariances and transport, the baseline
directional prefits use hits from both ends, and the model does not yet receive
withheld-hit even/odd filter predictions. Evaluate those changes at a fixed
FPR and on genuinely new seeds, with particular attention to 1--2% losses,
clean-track safety and internal TPC intervals. No reconstruction source,
maintained cards, installed runtime or shared KF package changed for this
learning curve.

Reproduction (analysis files remain uncommitted under the project Git law):

```bash
source setup.sh
OPENBLAS_NUM_THREADS=1 OMP_NUM_THREADS=4 build.105.0.0.x86_64-el9-gcc11-opt/run \
  python3 TrackingPerformanceStudies/interval_bdt_20261011/learning_curve.py --replicates 2
OPENBLAS_NUM_THREADS=1 OMP_NUM_THREADS=1 build.105.0.0.x86_64-el9-gcc11-opt/run \
  python3 TrackingPerformanceStudies/interval_bdt_20261011/summarize_learning_curve.py
```

The local analysis directory is
`TrackingPerformanceStudies/interval_bdt_20261011/learning_curve_20261017/`.
It contains the exact chosen seeds and checkpoint histories, validation scores,
`summary.json` and `learning_curve.png`.
