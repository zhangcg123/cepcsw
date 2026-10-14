# Full-context track/interval eBrem sequence model (2026-10-15)

## Outgoing focus and request

The preceding 99-feature, finite-receptive-field convolutional model and
its exact results are preserved in
`2026-10-15-hierarchical-sequence-ebrem-model.md`; the earlier two-BDT
track-first pipeline remains in
`2026-10-15-track-first-ebrem-classification.md`. The user correctly noted
that the training example is a **complete ordered track**, so hand-built
neighbourhood summaries and duplicated adjacent-hit fields are unnecessary.
They requested implementation of a model able to relate hits across the
full track while retaining track and interval output levels.

## Implemented analysis model

`TrackingPerformanceStudies/interval_bdt_20261011/full_context_sequence_model.py`
is an uncommitted analysis artifact. It reuses the truth-isolated 99-column
cache, but selects only **24 local diagnostics per hit** and **five local or
track-context values per interval**. It constructs N distinct hit rows from
the N-1 cached interval rows, using the `+1` fields only for the final hit.
The 24 hit values are forward/backward innovation pulls, logged local chi2
and update status, native-smoother residual pulls/chi2/status, five signed
F/B state differences and full five-dimensional compatibility, local
curvature update fractions, detector system/layer, and radius. The five
interval values are reconstructed log t/X0, transformed chord length,
material-valid flag, position fraction along the track and baseline pT.
Missingness masks double these scalar inputs inside the network. No
`pre5/post5/pre10/post10` feature, sampled neighbouring hit, ECAL quantity,
breakpoint refit, or truth feature enters training. Geant4 truth supplies
the >1%-per-interval and any-positive-track labels and evaluation only.

A shared hit embedder feeds one bidirectional GRU. At interval i -> i+1,
its interval head compares the forward GRU state after hit i (all preceding
hits) with the backward GRU state starting at hit i+1 (all subsequent hits),
plus that interval's local values. Thus the candidate score has full-track
before/after context without engineered windows. The same interval logits
are pooled across VXD/ITK/TPC, with learned region-count correction, to
make the track logit. One model jointly trains track binary cross entropy
and positive-track interval-ranking loss. Negative tracks influence the
shared interval scorer through track loss; there is no explicit
negative-interval loss or multiplicity head yet.

The underlying baseline KF's direction-local prefits use hits at both ends,
and its native smoother uses all hits. Therefore the input diagnostics are
**not statistically independent left/right measurements**, even though
the recurrent encoder separates the ordered hit rows at each candidate
boundary. This model is a learned change-point proposal, not a calibrated
likelihood or production eBrem detector.

## Same-split test

The same whole-seed development split has 8,069 training, 2,559 validation
and 2,554 test tracks, with 2,464/736/758 positive tracks and
2,818/850/871 qualifying intervals. The old 99-feature convolution and
the new full-context model use the same track/interval labels and joint
training objective. The validation checkpoint rule was average precision
plus unrestricted exact top-1 interval recall, with five-epoch early stop;
epoch 10 was selected from 15 trained epochs. The saved checkpoint was
reloaded and reproduced all scores on three test tracks, including every
interval score. Test IDs and track labels were asserted equal to those of
the preceding convolutional study.

| Metric | Two-model BDT | 99-feature CNN | Local-only full-context GRU |
|---|---:|---:|---:|
| Track AP | 0.736 | 0.737 | 0.746 |
| Track ROC AUC | 0.775 | 0.778 | 0.786 |
| Unrestricted exact top-1 / 871 | 349 | 359 | 372 |
| Unrestricted exact top-2 / 871 | 509 | 495 | 508 |
| Unrestricted exact top-5 / 871 | 697 | 667 | 687 |
| At validation 5%-FPR cut: positive tracks / 758 | 400 | 406 | 414 |
| At that cut: false-positive tracks / 1,796 | 91 | 98 | 93 |
| At that cut: end-to-end exact top-1 / 871 | 176 | 189 | 201 |
| At that cut: end-to-end exact top-2 / 871 | 284 | 281 | 301 |

The full-context model improves modestly over the CNN at both hierarchy
levels. It does not decisively improve top-2/top-5 over the older interval
BDT. At the validation 99%-recall cut, the test set has 739/758 detected
positive tracks and 1,696/1,796 falsely flagged negative tracks (30.3%
track purity). There remains no high-efficiency, high-purity detector and
no 99%-correct interval locator. The test split has been reused throughout
feature development; these numbers are development evidence only.

## Reproduction and continuation

From the repository root, after the old `hierarchical_sequence_model.py
extract` has generated its local cache:

```bash
source setup.sh
OPENBLAS_NUM_THREADS=1 OMP_NUM_THREADS=4 build.105.0.0.x86_64-el9-gcc11-opt/run python3 -u TrackingPerformanceStudies/interval_bdt_20261011/full_context_sequence_model.py train
```

The uncommitted local outputs are `full_context_sequence_model.pt`,
`full_context_normalization.npz`, `full_context_test_scores.npz` and
`report_full_context.json` in that analysis directory. The maintained
package sources, shared KF, cards and installed runtime were untouched.

Next, audit track-level false positives and missed 1--2% losses by geometry
and loss location; compare candidate rank and boundary ambiguity. Test
explicit interval-negative training and a one/two/rare-three multiplicity
head, but keep every internal TPC interval eligible. Final physics claims
require independent simulation seeds, clean-track safety and endcap/geometry
transfer checks.
