# Track-first eBrem classification and interval location (2026-10-15)

## Decision and preceding focus

The user fixed the target: a track is positive when **at least one** of its
truth intervals has eBrem momentum loss divided by the primary truth momentum
**greater than 1%**. The procedure must decide whether the whole track is
positive before locating its interval. Negative intervals from no-loss tracks
remain valid locator training examples. The preceding focus was a direct
interval BDT with 49, 99 and 264 reconstructed-only feature variants. Its
ordered-neighborhood variant reached 349/871 exact top-1, 509/871 top-2 and
697/871 top-5 interval recall but, when its maximum interval score was used
as a track decision, still falsely flagged about 98% of negative tracks at a
high-sensitivity validation cut. Full prior evidence and the feature-extractor
qualification remain in `2026-10-11-ordered-neighborhood-bdt.md`; the
barrel-dataset cohort definition remains in
`2026-10-11-interval-identification-batch-audit.md`.

## Integrated procedure

`TrackingPerformanceStudies/interval_bdt_20261011/track_first_pipeline.py` is
an **uncommitted analysis artifact**, not a CEPCSW algorithm. It reads the
existing baseline-KF interval-identification ROOT tuples, aggregates complete
tracks into 320 track-level features and trains an XGBoost track classifier.
Features summarize native forward/backward/smoothed KF diagnostics, signed
F/B compatibility, reconstructed hit-to-hit material and detector regions.
The record uses the existing seed-held-out split. The training extractor reads
every interval, because the earlier interval cache kept only 5.5% of training
negative intervals. It asserts the first reconstructed feature array is
exactly unchanged when all truth fields are removed.

At inference the track score is cut first. Only accepted tracks are passed to
the existing ordered-neighborhood interval classifier, whose intervals are
ranked by score. That locator was already trained on all positive intervals
plus sampled negative intervals from **all** selected tracks, including tracks
with no qualifying loss. Its earlier `>=1%` training label and the current
strict `>1%` definition have no exact-boundary conflict in the saved interval
cache: zero losses equal exactly 0.01. Model features use no truth, ECAL or
breakpoint refits. There was no C++ source, production card, shared-KF or
installed-runtime change.

The selected diagnostic cohort is still truth-filtered for evaluation:
single reconstructed track per event, at least 100 hits, valid truth label and
truth-match purity at least 0.9. This is **not** a deployable event selection
without truth. Whole simulation seeds separate the 8,069 training, 2,559
validation and 2,554 test tracks. Their positive-track counts are 2,464,
736 and 758; positive-interval counts are 2,818, 850 and 871. The existing
test seeds were used repeatedly during earlier feature development, so this
is development evidence, not an independent final test.

## Track detection

The trained track classifier stops at tree iteration 107. On the test split,
average precision is 0.736 and ROC AUC is 0.775; the positive prevalence is
758/2,554 = 29.7%. Thresholds below were set using **validation** tracks.

| Validation target | Test TP / 758 | Test FP / 1,796 | Track purity | Strict clean FP / 1,371 |
|---|---:|---:|---:|---:|
| 1% negative FPR | 338 (44.6%) | 19 (1.1%) | 94.7% | 10 |
| 5% negative FPR | 400 (52.8%) | 91 (5.1%) | 81.5% | 57 |
| 10% negative FPR | 440 (58.0%) | 178 (9.9%) | 71.2% | 126 |
| 99% positive recall | 737 (97.2%) | 1,698 (94.5%) | 30.3% | 1,289 |

At the corresponding validation-FPR targets, the previous maximum-interval-
score decision detected 282/758, 345/758 and 390/758 positives with 17, 105
and 194 false positives. Direct track classification improves this tradeoff,
particularly at the strict cuts. It does **not** satisfy a high-efficiency,
high-purity detector. At the 5% FPR target, it finds only 28/122 tracks whose
largest interval loss is 1--2%, 83/178 at 2--5%, 102/157 at 5--10%, and
187/301 at at least 10%. These are same-development-split observations, not
a diagnosed physical information limit.

## Interval location after the track gate

The locator receives only accepted tracks. An exact top-1 match is a
qualifying truth interval at the locator's highest-ranked interval; top-2
counts qualifying intervals in its first two candidates. The **end-to-end**
denominator remains all 871 true test intervals, including tracks rejected by
the first stage.

| Validation target | Selected tracks | Exact top-1 / 871 | Exact top-2 / 871 | Top-1 / selected true tracks |
|---|---:|---:|---:|---:|
| 1% negative FPR | 357 | 158 (18.1%) | 254 (29.2%) | 158/338 (46.7%) |
| 5% negative FPR | 491 | 176 (20.2%) | 284 (32.6%) | 176/400 (44.0%) |
| 99% positive recall | 2,435 | 339 (38.9%) | 496 (56.9%) | 339/737 (46.0%) |

The locator is unchanged in this study. The track gate raises purity among
selected tracks but loses true intervals when positive tracks are rejected.
For the same selected tracks, the prior 49-input interval model scores
150/172/331 exact top-1 at the three targets above; the ordered-neighborhood
locator scores 158/176/339. Do not describe track purity as exact interval
purity or imply that a top-2 list is an identified multiplicity.

## Reproduction and continuation

Run from the repository root:

```bash
OPENBLAS_NUM_THREADS=1 python3 -u TrackingPerformanceStudies/interval_bdt_20261011/track_first_pipeline.py extract
source setup.sh
OPENBLAS_NUM_THREADS=1 OMP_NUM_THREADS=4 build.105.0.0.x86_64-el9-gcc11-opt/run python3 -u TrackingPerformanceStudies/interval_bdt_20261011/track_first_pipeline.py train
```

The first command uses the ordinary Python environment with modern uproot;
the second uses the LCG environment's XGBoost. The uncommitted local outputs
are `track_first_cache/`, `track_classifier.json`,
`track_classifier_test_scores.npz` and `report_track_first.json` under the
same analysis directory. Extraction matched prior per-split track and interval
counts; test IDs were asserted to align with the stored locator scores.

Next, improve the whole-track loss decision and diagnose missed tracks,
especially the 1--2% class and strict-clean false positives. Then train
hard-negative interval examples and a multiplicity decision, retaining every
internal TPC interval as a candidate. Require independent data, clean-track
safety and endcap/geometry transfer checks before a physics-performance claim.
