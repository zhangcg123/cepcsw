# One hierarchical track/interval eBrem model (2026-10-15)

## Outgoing focus and scope

The preceding two-model track-first BDT and all its counts, thresholds,
limitations, and reproduction commands are preserved in
`2026-10-15-track-first-ebrem-classification.md`. The user requested one
coherent model while retaining the hierarchy: first decide whether a track
has any eBrem interval above 1% of primary truth momentum, then locate its
one or more intervals. This is an analysis experiment, not a reconstruction
algorithm or a production identifier. No breakpoint refit, ECAL, or truth
feature enters the model; truth is used only for cohort selection, labels,
and evaluation. The diagnostic cohort's truth-match purity cut is not a
deployable selection. Every internal TPC interval remains eligible.

## Implementation

`TrackingPerformanceStudies/interval_bdt_20261011/hierarchical_sequence_model.py`
is an uncommitted analysis artifact. It extracts all ordered intervals of
each training track from the baseline-KF identification tuples rather than
sampling negatives. Its 99 per-interval reconstructed inputs are the first
49 established material/KF/geometry features plus detailed diagnostics at
hits i and i+1; the ordered convolutional encoder supplies wider context.
Missing values receive explicit masks. Its extraction asserts that removing
all truth fields leaves the first reconstructed feature array unchanged.

One dilated one-dimensional convolutional encoder outputs a score for every
interval. The **same** interval scores are pooled through VXD, ITK, and TPC
groups, with learned region-count correction, to produce the whole-track
logit. This is a single parameter-sharing model with two output levels, not
two separately trained classifiers. The track decision is thresholded before
the interval ranking is used operationally. Training jointly minimizes track
binary cross entropy and a positive-track interval-ranking loss. All
intervals on no-loss tracks affect the track loss as negative evidence;
positive tracks supply the location loss. Multiple qualifying intervals are
represented in the training labels and evaluation. The model does not yet
predict the number of losses separately.

The established whole-seed train/validation/test split is unchanged:
8,069/2,559/2,554 tracks with 2,464/736/758 positive tracks and
2,818/850/871 qualifying intervals. The first epoch maximized the declared
validation monitor, average precision plus unrestricted exact top-1 interval
recall; training stopped after five subsequent non-improving epochs. A
saved-checkpoint replay reproduced the scores of three test tracks and every
one of their interval scores. The same test split has already informed prior
feature development, so it is development evidence, not an untouched final
test.

## Results

Test track average precision is 0.737 and ROC AUC is 0.778, compared with
0.736 and 0.775 for the preceding direct track BDT. The unrestricted
interval ranking finds 359/871 exact top-1, 495/871 top-2, and 667/871
top-5 qualifying intervals. The earlier ordered-neighborhood interval BDT
found 349/871, 509/871, and 697/871 respectively. The new model therefore
has a small top-1 gain but worse top-2 and top-5 coverage. It is not a
demonstrated location-performance advance.

Thresholds below were fixed from validation tracks; the denominator for
end-to-end interval recall always remains all 871 qualifying test intervals.

| Validation target | Test TP / 758 | Test FP / 1,796 | Track purity | Strict clean FP / 1,371 | Exact top-1 / 871 | Exact top-2 / 871 |
|---|---:|---:|---:|---:|---:|---:|
| 1% negative FPR | 337 | 15 | 95.7% | 10 | 169 | 247 |
| 5% negative FPR | 406 | 98 | 80.6% | 75 | 189 | 281 |
| 10% negative FPR | 435 | 170 | 71.9% | 132 | 196 | 292 |
| 99% positive recall | 749 | 1,746 | 30.0% | 1,338 | 353 | 487 |

The 5%-FPR gate identifies 53.6% of positive tracks; at a high-efficiency
cut, it still falsely flags 97.2% of negative tracks. This does not meet the
desired high-recall/high-purity track decision, nor does the interval ranking
meet 99% exact location recall. These are the limits of this trained model
on this development split, not a proof that the saved KF information is
insufficient.

## Reproduction and next checks

Run from the repository root. Extraction uses ordinary Python with modern
uproot; training uses PyTorch in the installed LCG environment:

```bash
OPENBLAS_NUM_THREADS=1 python3 -u TrackingPerformanceStudies/interval_bdt_20261011/hierarchical_sequence_model.py extract
source setup.sh
OPENBLAS_NUM_THREADS=1 OMP_NUM_THREADS=4 build.105.0.0.x86_64-el9-gcc11-opt/run python3 -u TrackingPerformanceStudies/interval_bdt_20261011/hierarchical_sequence_model.py train
```

The local uncommitted outputs are `hierarchical_sequence_cache/`,
`hierarchical_sequence_model.pt`, `hierarchical_sequence_normalization.npz`,
`hierarchical_sequence_test_scores.npz`, and
`report_hierarchical_sequence.json` under the same analysis directory.

Next, diagnose misses by detector region, loss size, candidate rank, and
strict-clean false positives using consistent track-region definitions.
Examine whether no-loss intervals need an explicit interval-level loss and
whether the shared track pooling is over-sensitive to many TPC candidates.
Then evaluate a one/two/rare-three multiplicity head without removing any
interval candidates. Reserve independent seeds and endcap/geometry samples
for final validation; do not use ECAL, breakpoint refits, or truth as inputs.
