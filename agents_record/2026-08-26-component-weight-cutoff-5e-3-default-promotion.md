# Component-weight cutoff 5e-3 default promotion

Date: 2026-08-26

## Decision and boundary

The user explicitly promoted `ComponentWeightCutoff=5e-3` from an experimental
finer-BH reverse-K1 campaign to the compiled and active reverse-template
default. The previous default was `1e-4`. This is a steering decision made
with the measured core/tail tradeoff visible; it is not a claim that `5e-3`
optimizes population momentum resolution.

This decision changes only the normalized posterior-component cutoff.
`BHSplitThreshold` remains `1e-4`, `ReductionMergeCost` remains
`SymmetricKL`, and the compiled/active reverse-template `MaxComponents`
remains 12. The maintained `DumpGsfTrks/gsf.py.bk` explicitly steers the
promoted cutoff. Any campaign-level `MaxComponents` override is a separate
steering decision and is not promoted to the compiled or active
reverse-template default by this decision.

## Evidence retained with the decision

The strict same-row comparison used the finer nine-component category-aligned
BH model, `MaxComponents=30`, reverse covariance scale 1, and truth override
off. It matched 6,547 topology-clear events between cutoff `1e-4` and `5e-3`:
2,834 no-eBrem, 3,114 light-eBrem, and 599 hard-eBrem events. Secondary
tracker activity was excluded from the primary study and retained separately.

For FullMixtureMode, inclusive width68 was nearly unchanged
(`0.5193% -> 0.5199%`). The count above 100% fell from 11 to 3 and RMS fell
from `24.83%` to `15.18%`, showing substantial catastrophic-tail suppression.
The tradeoff was worse light-eBrem width68 (`0.6400% -> 0.6541%`) and
hard-eBrem width68 (`9.47% -> 11.13%`), with their >3% counts increasing from
250 to 282 and 224 to 232. The promotion therefore deliberately favors
stronger low-weight-branch suppression despite central eBrem losses observed
in this sample.

Generated plots, exact tables, the file audit, and the full interpretation are
kept as uncommitted analysis products under
`TrackingPerformanceStudies/finerbh_weightcutoff5e-3_reverseK1_topology_clear_2026-08-26/`.

## Reduction semantics

`ComponentWeightCutoff` is applied after target-measurement posterior weights
are normalized and before component-count reduction. It directly uses the
posterior weight.

`ReductionMergeCost="SymmetricKL"` chooses the next pair using the unweighted
symmetric Gaussian-to-Gaussian KL distance. The subsequent moment merge still
uses the two component weights. The available weight-aware pair-ranking
alternative is configured literally as:

```python
gsf.ReductionMergeCost = "Runnalls"
```

Runnalls evaluates an upper-bound cost for replacing the two weighted
components by their moment-matched Gaussian. It is a default-off control and
was not promoted by the cutoff decision. Historical 100-event evidence and
the prior rejection of Runnalls promotion remain in
`agents_record/2026-07-16-runnalls-reduction-100-event-test.md`.

## Synchronized live surfaces

- Compiled property default: `Reconstruction/RecGsfTracking/src/GsfAlgorithm.h`
- Active reverse template: `Reconstruction/RecGsfTracking/options/run_gsf_reverse_template.py`
- Comprehensive property reference: `Reconstruction/RecGsfTracking/README.md`
- Maintained explicit campaign card: `DumpGsfTrks/gsf.py.bk`
- Workflow record: `DumpGsfTrks/README.md`
- Live project baseline: `AGENTS.md`

Historical experiment records retain their original `1e-4` steering and must
not be rewritten as if those runs used the promoted cutoff.

## Mechanical validation

- The dedicated read-only configurable-property audit confirmed all 42
  properties remain represented and every live cutoff surface listed above is
  synchronized. It also confirmed that the only accepted reduction values are
  `SymmetricKL` and `Runnalls`, and that historical `1e-4` experiment records
  must remain unchanged.
- Python syntax compilation passed for the active reverse template and the
  maintained `gsf.py.bk` card.
- `RecGsfTracking` rebuilt successfully in the maintained EL9/LCG 105 tree and
  the configured build installed successfully.
- A comprehensive verbose reverse run selected file-1 events 11, 16, and 17
  from `trk_large_20260823/trk-e--2.0-85-1.root`. It completed all 18 input
  event slots and finalized successfully. Each selected event completed the
  inward component evolution and published BestBranch, WeightedMean, and
  FullMixtureMode. Event 11 retained 234/234 hits, event 16 processed both
  reconstructed tracks, and event 17 retained 232/232 hits. This is a
  mechanical default-promotion gate, not new momentum-validation evidence.
