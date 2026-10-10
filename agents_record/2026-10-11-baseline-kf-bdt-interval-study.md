# Baseline-KF BDT interval study (2026-10-11)

## Scope and outgoing focus

The user explicitly ruled out breakpoint refits for this identification task.
The previous current focus proposed disjoint inner/outer segment refits and a
profiled loss to identify intervals. That proposal is historical, not active.
Its complete evidence and rationale remain in
`2026-10-11-interval-identification-batch-audit.md`. The identification tuple
itself is unchanged: it reads REC, records baseline forward, native-smoothed
and backward KF diagnostics, hits/geometry and reconstructed DD4hep chord
t/X0, and adds Geant4 labels after feature production. Its direction-local
prefits use both track ends and are not strictly independent evidence. The
dataset has barrel-radius ordering only; an endcap ordering remains unvalidated.
No breakpoint recovery refit, ECAL constraint, truth steering, shared KF edit,
package installation, or batch job occurred in this BDT test.

## Dataset and method

The analysis reads all 72 completed
`breakpoint_interval_barrel_20261010/interval_identification-barrel-*.root`
files. The established strict single-track, >=100-hit, successful unique
truth match and purity >=0.9 selection yields 13,182 tracks. Seeds, not rows
or events, are partitioned 3:1:1 by their position in the sorted seed list:
8,069 train, 2,559 validation and 2,554 untouched test tracks. The test has
871 truth intervals with eBrem momentum loss >=1% of the generator momentum,
on 758 tracks. The remaining 1,796 tracks have no in-span >=1% interval;
1,371 of these also have <0.2% total out-of-span loss and <0.2% maximum
in-span loss, making a stricter clean control. The training set retains every
positive interval and a seeded 5.5% random sample of negative intervals;
validation and test score **all** intervals. The resulting XGBoost scores are
not calibrated probabilities. The validation set alone sets the per-track
maximum-score threshold corresponding to 99% sensitivity.

Features contain reconstructed material/geometry and baseline-KF diagnostics
only: chord t/X0/length/coverage, detector labels and radius, forward and
backward local chi2 with discarded-site sentinels masked, rejection flags,
curvature/covariance comparisons, and native-smoothed comparisons. The second
version additionally uses 5- and 10-hit window averages of chi2 and curvature
patterns. Neither truth t/X0, truth loss/interval, Geant4 photon information,
nor any breakpoint-refit output enters the features. Truth enters the >=1%
labels, strict evaluation cohort and final scoring only. Because the existing
KF seeds use both track ends, forward/backward features must not be
interpreted as independent likelihood factors.

Model: XGBoost histogram-tree binary classifier, 350-tree cap, max depth 4,
learning rate 0.055, subsample 0.85, column subsample 0.9,
min_child_weight 10 and L2 regularization 10, early-stopped on validation
interval AUC-PR. The local and multiscale passes compare geometry-only,
KF-only and combined variants on the same seed split. Analysis code and
machine-readable model/reports are uncommitted experiment artifacts under
`TrackingPerformanceStudies/interval_bdt_20261011/`. The maintained source
and card are not changed.

## Held-out result

Exact recall is the fraction of the 871 labelled intervals appearing in the
highest-scoring K candidates on their own track. The raw material top-K row
comes from the earlier full audit filtered to the identical test seeds.

| Candidate ranking | Top 1 | Top 2 | Top 5 | Top 20 |
|---|---:|---:|---:|---:|
| Raw reconstructed t/X0 | 228/871 (26.2%) | 377/871 (43.3%) | 649/871 (74.5%) | 759/871 (87.1%) |
| BDT, geometry only | 230/871 (26.4%) | 379/871 (43.5%) | 655/871 (75.2%) | 759/871 (87.1%) |
| BDT, local KF only | 291/871 (33.4%) | 437/871 (50.2%) | 624/871 (71.6%) | 746/871 (85.6%) |
| BDT, local KF + geometry | 342/871 (39.3%) | 489/871 (56.1%) | 676/871 (77.6%) | 773/871 (88.7%) |
| BDT, multiscale KF + geometry | 341/871 (39.2%) | 485/871 (55.7%) | 681/871 (78.2%) | 772/871 (88.6%) |

The multiscale combined BDT's top-5 exact recall is 157/204 VXD,
311/335 ITK and 213/332 TPC intervals. It improves ITK top-1 ranking
substantially over material alone but does not solve the TPC ambiguity.
Only 411/748 (54.9%) test tracks with one or two >=1% intervals have *all*
their true intervals in the top two multiscale candidates. Ten further
positive test tracks have >=3 such intervals and cannot be fully represented
by an output capped at two.

The multiscale combined event threshold chosen on validation detects 752/758
(99.2%) test tracks with >=1% loss, but falsely flags 1,751/1,796 (97.5%)
tracks without a >=1% loss. It also falsely flags 1,336/1,371 (97.4%) of the
strict clean controls. Of 2,503 tracks flagged at that threshold, only 339
(13.5%) have the true >=1% interval ranked first. The local combined model
is similar: 753/758 detected, 1,752/1,796 falsely flagged, 342/871 top-1
exact. At high recall, this is not a usable loss/no-loss decision.

The local combined BDT's largest feature importances are log t/X0 (0.310),
chord length (0.303), downstream layer (0.085), upstream detector (0.076),
then KF curvature separation and smoother differences. These are model
importance scores, not causal attributions. Adding 5-/10-hit averages changes
top-5 recall by only five intervals and does not materially reduce clean-track
false positives. About 7% of true >=1% test intervals have a truth boundary
ambiguity flag (64/871); exact and neighbour-tolerant targets must stay
separate. The 714 truth-invalid rows in the full dataset were excluded rather
than treated as negatives.

## Interpretation and next direction

The BDT does extract some localization information beyond raw material, but
the current per-interval binary classifier is dominated by detector/material
peaks and cannot decide whether eBrem occurred. The lack of improvement with
short multihit averages suggests that a larger BDT on the same local inputs
is unlikely by itself to reach the requested 99% efficiency **and** correctness.
This is an inference from this barrel, seed-held-out study, not an impossibility
proof or a cross-geometry validation.

Stay entirely within reconstruction-only identification: next compare a
track-level no-loss discriminator and a track-grouped interval-ranking BDT,
with explicit one/two/rare-three multiplicity decisions and harder negatives
sampled from material peaks. Construct *signed, sustained change-point*
features from the already saved baseline forward/backward innovation and
curvature sequences, including covariance/rejection reliability, rather than
only window averages. Scan every internal TPC interval. Evaluate exact and
neighbour-tolerant locations, false positives on clean tracks, boundary-
ambiguous labels, and held-out seeds before trying endcap or new geometry.
Do not use a breakpoint refit, ECAL information or truth in classifier inputs
for this task.
