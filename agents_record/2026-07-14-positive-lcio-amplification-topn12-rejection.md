# Positive-LCIO amplification: TopN at MaxComponents 12

Date: 2026-07-14

## Configuration and outputs

The fixed 30-ID set was rerun with `MaxComponents=12`,
`ReductionMode=TopN`, `AggregateWeight`, and the same comprehensive dumps and
all other settings used by the KL-12 comparison. The log configuration line
confirms `reductionMode=TopN`. All 28 seed jobs terminated successfully.

Disposable logs and ROOT outputs are under
`/tmp/gsf-positive-lcio-amplified-30-verbose-topn12`. Durable tables are:

- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/positive_lcio_amplified_30_topn12_diagnostics.csv`
- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/positive_lcio_amplified_30_topn12_vs_kl12.csv`
- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/positive_lcio_amplified_30_topn12_vs_kl24.csv`

The diagnostic analyzer was corrected to reset component collection at every
unrelated `MIX` block and to label a missing reverse output explicitly. This
fix changes no published pT, selected signature, or capacity outcome; it
prevents the later forward final-mixture dump from contaminating the reverse
hit-0 identity row.

## Failure mechanism

Unlike KL reduction, `reduceTopN` sorts by current component weight, deletes
everything below rank 12, and renormalizes. It neither merges discarded mass
nor protects the no-radiation identity lineage. Identity is absent from the
final reverse mixture in 9/30 events.

Two events fail the required reverse-output gate. Light event 193/9 loses
identity at the hit-9 TopN reduction and all surviving branches are rejected
by hit 7. Light event 233/4 retains identity through hit 5, but TopN has
discarded the alternative support and all 12 survivors are rejected at the
next inward measurement. Their logs end with `finalComps=0`; there is no
`REVERSE IP output`. Their flat tuples contain the LCIO/non-reverse fallback,
so their near-zero residuals must not be counted as TopN improvements.

The run has 79,360 accepted and 29 rejected component updates, 28 valid
reverse outputs, and two zero-component failures. On the 28 common valid
events, TopN-12 versus KL-24 has median absolute residual 1.5364% versus
1.1173%, mean absolute residual 1.4882% versus 1.3966%, and 12 versus 14 events
inside 1%. Eventwise it improves 15, worsens 6, and leaves 7 unchanged versus
KL-24, but that selected-set count does not override the worse aggregate
metrics, identity loss, or two hard failures. Versus KL-12 on the same valid
events, it improves 17, worsens 9, and leaves 2 unchanged, while median
absolute residual worsens from 1.1245% to 1.5364%.

## Decision

Reject TopN-12. It suppresses some positive-amplification cases by deleting
radiative branches, but it can also delete identity and the alternative
support needed to survive later measurements. It fails the mandatory complete
finite reverse-track gate and is not a safe solution to aggregate-weight
representation dependence. Keep KL with `MaxComponents=24` as the baseline.

