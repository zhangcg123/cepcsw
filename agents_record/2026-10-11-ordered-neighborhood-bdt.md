# Ordered neighborhood BDT study (2026-10-11)

## Scope and preserved previous focus

The user asked whether neighboring hits could improve eBrem interval
identification. The preceding focus and full numerical evidence are preserved
in `2026-10-11-baseline-kf-bdt-interval-study.md`: baseline-KF/material BDTs,
whole simulation seeds separated into train/validation/test, >=1% truth-loss
labels, no breakpoint recovery fit, no ECAL inputs and no truth features.
Its proposed next work was a no-loss discriminator, interval ranking and signed
sequence features. Five-/ten-hit averages alone had given little improvement.
The current work tests ordered neighborhoods and one ranking-objective trial.
The no-loss discriminator remains unimplemented.

No reconstruction C++ source, production card, installed runtime or batch
workflow was changed. All new code is uncommitted analysis under
`TrackingPerformanceStudies/interval_bdt_20261011/`.

## Features, controls and validation

The original 49 features are retained as an exact reference. Additional
features use hit offsets -3,-2,-1,0,1,2,3,4 around candidate interval i -> i+1:

- signed native forward/backward innovation residuals divided by the square
  root of their respective residual-covariance diagonal, and native innovation
  chi2;
- native-smoothed included-hit residual pulls and residual chi2, with their
  own valid-status requirements;
- five signed coordinates of B_predicted-F_updated, with phi wrapped and
  each divided by its zero-cross-covariance difference uncertainty;
- full five-dimensional compatibility using P_F_updated+P_B_predicted when
  positive definite and both states have the same pivot; this is a correlated
  diagnostic feature, not an independent likelihood or eBrem probability;
- curvature changes in forward, backward and smoothed updates, statuses,
  detector/layer labels, same-detector flags, radial distances, neighboring
  reconstructed t/X0, and signed before/after contrasts over 3/10 hits.

The ordered model has 264 inputs. A 99-input central-enriched control adds
only the new diagnostics at boundary hits i and i+1 to the old 49 features.
It therefore separates the effect of richer local diagnostics from the wider
ordered context. Both use the same XGBoost classifier settings and seed split
as the previous comparison. The prior combined model reproduces its exact
positive/top1/top2/top5/top20 counts when evaluated on the cached old 49 inputs.
Removing all truth fields from a focused row leaves its reconstructed feature
matrix exactly identical, including NaNs. Features are cached per seed to
avoid repeated ROOT reads. Full test predictions, track identifiers and labels
are retained for subsequent diagnosis.

One flaw in the earlier analysis extractor was discovered: its state-array
helper returned an entirely missing track array if any hit had the wrong
vector length. This affected 722 selected tracks with partially missing
backward-predicted parameters/covariance, and 200 with partially missing
forward-updated or smoothed states/covariance. These are per-field counts,
not a disjoint union. The new diagnostic features pad invalid hits individually
and retain valid hits. The original 49 reference features deliberately retain
their exact historical behavior for reproducibility; this is not a C++ tuple
producer defect. Hence gains over the old BDT cannot be attributed solely to
neighborhood size. The central-enriched control is essential.

All 72 files yielded the same 13,182 selected tracks: 8,069 train, 2,559
validation, 2,554 test. All training-positive intervals and 5.5% of training
negatives were retained, exactly as before. Every validation/test interval is
scored. The test contains 758 tracks with 871 >=1% loss intervals, 1,796
tracks without an in-span >=1% loss and 1,371 stricter clean controls. The
selection is the documented single-track/>=100-hit/truth-match cohort, not the
old certified topology-clear catalogue. The same test seeds have now been
inspected during several feature-development iterations; these comparisons
are development evidence, not a new independent final validation.

## Exact interval results

Each count below is the number of the 871 true loss intervals appearing in
the track's top K candidates, with exact index matching.

| Classifier | Top 1 | Top 2 | Top 5 | Top 20 |
|---|---:|---:|---:|---:|
| Previous combined, 49 inputs | 341 (39.2%) | 485 (55.7%) | 681 (78.2%) | 772 (88.6%) |
| Central-enriched, 99 inputs | 359 (41.2%) | 517 (59.4%) | 692 (79.4%) | 803 (92.2%) |
| Ordered neighborhood, 264 inputs | 349 (40.1%) | 509 (58.4%) | 697 (80.0%) | 803 (92.2%) |

The neighborhood model's top-5 exact coverage is 160/204 VXD, 308/335 ITK
and 229/332 TPC. Top-20 TPC coverage improves from 234/332 to 265/332;
the central-enriched model already reaches the same total top-20 coverage.
Of 748 test tracks with one or two qualifying intervals, 430 have all their
true intervals inside the neighborhood model's top two candidates (57.5%).
This does not establish the output multiplicity or reject additional false
candidates. Ten other positive test tracks have at least three such intervals.

| Truth interval loss | Number | Previous top 5 | Central-enriched top 5 | Neighborhood top 5 |
|---|---:|---:|---:|---:|
| 1% to <5% | 380 | 290 (76.3%) | 290 (76.3%) | 284 (74.7%) |
| >=5% | 491 | 391 (79.6%) | 402 (81.9%) | 413 (84.1%) |

The improvement is mostly in harder-loss candidate coverage. In 2,000 paired
bootstrap resamples of entire test tracks, neighborhood-minus-previous exact
recall differences have 95% intervals (percentage points): top1 [-1.27,3.17],
top2 [0.68,4.83], top5 [0.11,3.54], top20 [2.06,5.11]. These uncertainties
describe resampling this dataset, not geometry transfer or model-selection
uncertainty. There is no convincing top1 improvement.

## Loss/no-loss and ranking-objective checks

At the validation threshold targeting 99% detection of >=1% loss tracks,
the neighborhood BDT detects 746/758 test positives (98.4%) and falsely flags
1,752/1,796 negatives (97.6%). It also flags 1,340/1,371 stricter clean
controls. Thus the high-sensitivity decision remains unusable.

At thresholds fixed using 5% validation false-positive rate:

| Classifier | Test loss-track detection | Test false positives |
|---|---:|---:|
| Previous combined | 375/758 (49.5%) | 87/1,796 (4.8%) |
| Central-enriched | 360/758 (47.5%) | 100/1,796 (5.6%) |
| Ordered neighborhood | 345/758 (45.5%) | 105/1,796 (5.8%) |

These are track-detection metrics, not correct interval-location efficiency.
Other 1% and 10% validation false-positive operating points are preserved in
`report_neighborhood.json`. Wider context does not yield a better clean-track
tradeoff in this trial.

A separate XGBoost `rank:pairwise` trial used track groups, the same sampled
training candidates, and validation NDCG@2. Only groups with a positive label
contribute to this candidate-ranker training/evaluation objective; inference
still ranks all intervals of every test track. The old-49-input ranker reaches
338/487/678/769 top1/top2/top5/top20 counts. The neighborhood ranker reaches
300/474/687/801; validation early stopping selects iteration 4. Neither
beats the classifier. These rankers do not make a no-loss decision and their
scores must not be treated as eBrem probabilities. In particular the sparse
uniform training negatives remain a limitation; no hard-negative study has
yet been done.

## Conclusion and continuation

Ordered neighbors carry some useful information, including signed F/B
curvature differences, but adding them does not solve precise selection of
one or two intervals. Richer central diagnostics account for much of the
candidate-list gain. The earlier extractor flaw further weakens any claim
that the first BDT established a fundamental information limit.

Next diagnose candidate confusions on individual tracks and develop hard
negatives from competing material peaks and adjacent intervals. A separate
track-level no-loss/multiplicity decision remains needed. Preserve signed
residual ordering, valid masks and geometry dependence; explicit measurement
surface axes are still absent, so cross-geometry transfer is not established.
Continue using baseline KF information only, with no breakpoint refits or ECAL
inputs. A new independent sample will be needed for final performance claims.

Reproduction from repository root:

```bash
OPENBLAS_NUM_THREADS=1 python3 TrackingPerformanceStudies/interval_bdt_20261011/neighborhood_bdt.py extract
source setup.sh
OPENBLAS_NUM_THREADS=1 OMP_NUM_THREADS=4 build.105.0.0.x86_64-el9-gcc11-opt/run python3 TrackingPerformanceStudies/interval_bdt_20261011/neighborhood_bdt.py train
OPENBLAS_NUM_THREADS=1 OMP_NUM_THREADS=4 build.105.0.0.x86_64-el9-gcc11-opt/run python3 TrackingPerformanceStudies/interval_bdt_20261011/neighborhood_ranker.py
```

Extraction used the ordinary Python environment with modern uproot; training
used the LCG environment's XGBoost 2.0.2. Cache/manifest, models and all reports
are uncommitted local analysis artifacts in the directory above.
