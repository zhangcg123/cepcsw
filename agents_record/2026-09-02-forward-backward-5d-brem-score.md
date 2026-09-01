# Direction-signed five-dimensional F/B brem score

Date: 2026-09-02

## Decision and definition

The passive `lineage_node_fb_brem_probability` diagnostic now uses the full
five-dimensional same-surface difference between `F_updated[i]` and
`B_predicted[i]`, rather than only the first-order pT difference and its
variance. No fit weight, state, cutoff, KL reduction, propagation, or endpoint
publication uses this value.

For the wrapped state difference and independence-approximation covariance,

```text
delta_x = x_Bpred - x_Fupdated
S       = C_F + C_B
q       = delta_x^T S^-1 delta_x
R       = F_chi2,5(q)
```

the persisted score is

```text
P_brem = 0.5 * (1 + R),  delta_pT < 0
         0.5,            delta_pT = 0
         0.5 * (1 - R),  delta_pT > 0
```

The five-dimensional chi-square CDF supplies the incompatibility magnitude;
the pT sign supplies the physical energy-loss direction. The score approaches
one for a large incompatibility in the loss direction and zero for a large
opposite-direction incompatibility. In one dimension this signed-radial
construction reduces exactly to the former
`Phi(-delta_pT/sigma_delta_pT)` score.

This is not `P(brem | hits)`. It still assumes zero F/B cross-covariance, has
no explicit no-brem or radiative prior, and can be raised by discrepancies in
non-curvature parameters. Under ideal independent calibrated Gaussian
messages it has a uniform null distribution, but the current messages do not
justify that calibration. It remains meaningful as a brem diagnostic only on
the exact no-radiation pair.

The EDM collection and flat branch names are unchanged. Tuples made before
this change store the former one-dimensional definition under
`lineage_node_fb_brem_probability`; their raw `lineage_node_fb_delta_pT`,
`lineage_node_fb_delta_pT_variance`, and `lineage_node_dchi2` fields permit
either score to be reconstructed and identify the tuple semantics.

## Mechanical validation

The EL9/LCG-105 `RecGsfTracking` and `RecGsfFlatTuple` targets built and
installed. A verbose compiled-double-off event-11 run retained the intended
direction-local prefits (`0,1,2` outward and `231,232,233` inward) and produced
the complete passive product graph without BH children.

The same compiled-double-off focused run completed for events 11, 16, and 17:

| Event | Truth pT [GeV] | CompleteTracks pT [GeV] | GSF pT [GeV] |
|---:|---:|---:|---:|
| 11 | 40.7315674 | 40.8954541 | 40.8924439 |
| 16 | 37.8940163 | 18.2928319 | 18.2873231 |
| 17 | 18.7969780 | 14.8066689 | 14.8178930 |

Against the immediately preceding directional-prefit outputs, 738 common
branches other than the deliberately changed probability and the unrelated
hit-detector-tag vector were identical over the three selected rows. All 691
finite direct-pair probabilities agreed exactly with the defining ROOT
chi-square-CDF expression, and no finite probability appeared on another node
type.

## Same-population discrimination gate

The new score was reconstructed exactly from the saved full 5D overlap
quadratic and signed pT difference for the preceding 199 topology-clear-event
gate (100 no-brem controls and 99 brem events). This is exact for the derived
diagnostic because no upstream state or covariance changed. The test retains
the 0.2% truth-loss floor and accepts a score at the truth interval or either
adjacent recorded interval. Truth-window intervals are excluded from the
45,415-interval Type-I denominator.

| Threshold | Score | Type I | Type II, loss >=0.2% | Type II, loss >2% | Type II, loss >5% | Type II, loss >10% |
|---:|:---|---:|---:|---:|---:|---:|
| 0.95 | former kappa/pT 1D | 5,244/45,415 = 11.55% | 56/96 = 58.33% | 10/32 = 31.25% | 4/16 = 25.00% | 2/11 = 18.18% |
| 0.95 | direction-signed 5D | 4,813/45,415 = 10.60% | 61/96 = 63.54% | 12/32 = 37.50% | 5/16 = 31.25% | 4/11 = 36.36% |
| 0.977249868 | former kappa/pT 1D | 3,782/45,415 = 8.33% | 61/96 = 63.54% | 12/32 = 37.50% | 4/16 = 25.00% | 2/11 = 18.18% |
| 0.977249868 | direction-signed 5D | 3,057/45,415 = 6.73% | 66/96 = 68.75% | 13/32 = 40.62% | 5/16 = 31.25% | 4/11 = 36.36% |
| 0.998650102 | former kappa/pT 1D | 2,143/45,415 = 4.72% | 68/96 = 70.83% | 15/32 = 46.88% | 6/16 = 37.50% | 4/11 = 36.36% |
| 0.998650102 | direction-signed 5D | 1,751/45,415 = 3.86% | 79/96 = 82.29% | 18/32 = 56.25% | 7/16 = 43.75% | 5/11 = 45.45% |

At equal Type-I count to the former 0.95 gate, the 5D threshold is 0.94331.
It misses 59/96 eligible losses instead of 56/96, 11/32 losses above 2%
instead of 10/32, and 3/11 losses above 10% instead of 2/11. Thus the extra
dimensions modestly suppress false alarms but do not improve separation of
truth-brem intervals.

The stable 133-event secondary-tracker-activity population remains excluded
from these single-track counts and was not rerun for this derived passive-score
gate. Generated ROOT files, logs, and CSV tables are under `/tmp` and are not
tracked.

## Conclusion

The requested five-dimensional score is implemented and mechanically sound,
but it does not rescue local eBrem identification. It must remain passive and
must not trigger the proposed hit-0 hypothesis bank. A useful decision still
requires coherent path evidence and a better-defined independent outer-hit
message or cross-covariance treatment, rather than a larger local mismatch
quadratic alone.
