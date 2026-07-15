# Topology-clean surface and process-core audit (2026-07-13)

## Scope

This record continues the fresh topology-clean light-eBrem survey after the
dominant-unmerged-lineage final selector was rejected.  All population counts
below exclude events with non-primary tracker SimHits.  No new simulation or
reconstruction production was required for the process-core comparison.

## Outcome population versus dominant truth-loss surface

The 2,132 topology-clean light events were joined to their dominant
reconstruction-owned Geant4 loss transition.  The complete table is under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/`.

The outcome distribution depends strongly on where the dominant loss occurs:

| dominant transition | events | missed | good | partial | overshoot | truth-like preserved |
|---|---:|---:|---:|---:|---:|---:|
| 0--2 | 73 | 28 | 0 | 2 | 0 | 42 |
| 3--4 | 142 | 55 | 1 | 3 | 0 | 83 |
| 5--6 | 499 | 70 | 64 | 16 | 10 | 335 |
| 7--8 | 522 | 20 | 160 | 22 | 8 | 305 |
| 9--11 | 84 | 3 | 27 | 1 | 0 | 53 |
| 12--49 | 58 | 3 | 10 | 1 | 0 | 44 |
| 50--99 | 79 | 0 | 9 | 1 | 0 | 69 |
| 100--149 | 77 | 1 | 14 | 1 | 0 | 61 |
| 150--199 | 86 | 7 | 5 | 2 | 0 | 72 |
| 200+ | 512 | 18 | 1 | 1 | 2 | 465 |

Representative truth locations agree with the state audits: missed 2/7 has a
7.40% dominant loss at transition 4; good 234/4 has 8.51% at transition 8;
missed 166/6 has 3.83% at transition 5; good 399/8 has 3.60% at transition 9;
and overshoot 26/9 has 5.53% at transition 7 before the wrong branch emerges
at hit 5.  False correction 463/7 has only a 0.146% loss at transition 69 and
flips at hit 1.

The dominant axis is therefore inward measurement leverage and process-loss
placement, not a single global prior defect.  Raising the radiative prior
globally would be especially unsafe because it would act on the large
truth-like populations as well as the missed inner-loss cases.

## Expanded process-mixture comparison

The model-training source consists of ten production transition CSVs with
2,574,697 input rows, 2,573,914 accepted rows, 783 rows outside the configured
t/X0 range, and 9,528 eBrem rows.  The expanded 499-seed surface-owned sample
has 1,324,057 input rows, 1,323,548 accepted rows, 509 outside-range rows, and
4,852 eBrem rows.  Both were extracted with the same five-component,
eBrem-attributed definition.

The radiative-tail probability comparison by t/X0 bin is:

| t/X0 center | training N | expanded N | training tail | expanded tail | difference significance |
|---:|---:|---:|---:|---:|---:|
| 0.000050 | 2,425,350 | 1,243,254 | 0.07838% | 0.07810% | -0.09 sigma |
| 0.000224 | 41,064 | 25,764 | 0.30197% | 0.27558% | -0.62 sigma |
| 0.001000 | 40,321 | 20,422 | 0.74403% | 0.78347% | +0.53 sigma |
| 0.003162 | 10,038 | 5,034 | 4.48296% | 4.33055% | -0.43 sigma |
| 0.007071 | 31,683 | 16,056 | 8.94486% | 9.13677% | +0.69 sigma |
| 0.012247 | 22,214 | 11,181 | 14.73845% | 14.38154% | -0.87 sigma |
| 0.017321 | 2,667 | 1,493 | 18.71016% | 18.01742% | -0.56 sigma |
| 0.024495 | 577 | 344 | 25.30329% | 25.58140% | +0.09 sigma |

No total-tail difference reaches one standard deviation.  Across 40 component
weights, the largest absolute shift is 2.09 sigma: the extreme component in
the 0.002--0.005 bin has 70 versus 22 entries and changes by -0.260 percentage
points.  The largest mean-z difference is a 3.25-sigma shift in an extreme
thin-material cell with only 10 versus 8 entries; it is not a stable basis for
changing the model.  The few populated-cell mean shifts are modest and do not
form a coherent direction across bins.

The comparison is reproducible with
`Reconstruction/RecGsfTracking/scripts/compare_step_mixture_samples.py`; its
complete component table is
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/step_mixture_resurvey_2026-07-13/training_vs_expanded_mixture.csv`.

## Decision

Reject global non-radiative/process-weight recalibration as the next
optimization change.  The expanded truth sample is statistically compatible
with the existing process weights in every t/X0 bin, while reconstruction
failures divide between prior-limited inner losses and strongly
measurement-supported wrong branches.  Continue with a surface-conditioned
state audit of how available post-loss measurement leverage and later
reduction determine the final branch; do not introduce a global prior scale,
an ad hoc measurement threshold, or covariance tuning.

## First matched inner-loss trace

A complete verbose comparison used missed 433/6 and good 369/1.  Their
dominant losses are closely matched: 3.8069% at transition 0 and 3.8266% at
transition 6, respectively.  Both tracks retain all 232 hits without update
rejection.  The reverse filter creates truth-compatible components in both
events, so absence of a process child is not the failure.

For good 369/1, the summed weight within 1% of truth pT is 0.0916% after hit 6,
then jumps to 38.55% after hit 5, 73.28% after hit 4, and finishes at 81.46%.
The measurement sequence therefore supplies decisive evidence immediately
inside the loss.  For missed 433/6, the compatible weight is 0.0650% after hit
6, 0.0168% after hit 5, 0.0067% after hit 4, and only 0.00031% at hit 0; the
identity-like component finishes at 98.73%.  The truth-compatible branch
survives to the end, but no inner measurement ever makes it competitive.

The trace is summarized reproducibly in
`matched_pair_433_6_369_1_inner_branch_weights.csv` in the topology-clean
survey directory by
`Reconstruction/RecGsfTracking/scripts/summarize_reverse_truth_branch.py`.
This rules out subsequent KL deletion for this matched failure and strengthens
the interpretation that very inward losses have insufficient post-loss
curvature lever arm.  Such events should remain in inclusive performance and
safety accounting, but they are not appropriate drivers for a prior or
reduction adjustment unless another reconstruction observable can supply the
missing information.
