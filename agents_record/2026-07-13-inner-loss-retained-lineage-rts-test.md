# Inner-loss retained-lineage RTS test (2026-07-13)

## Question and method

The user asked whether a real smoother could recover light-eBrem events with
few inner transitions.  The available mathematically valid first test is the
opt-in RTS smoother on every retained forward BH lineage.  Each branch stores
the exact filtered state, accepted MarlinTrk prediction, transport Jacobian,
process Jacobian, and process noise at every surface.  RTS is then run backward
along that discrete process history and its smoothed innermost state is used
for IP extrapolation.  This is a Gaussian smoother conditional on each
retained BH history; it is not a two-filter cross-lineage mixture recombination.

The required `TopN` reduction was used so no merged component represents
multiple incompatible discrete histories. Reverse filtering was disabled.
All tests used complete verbose component dumps and retained every hit.

## Focused results

| event | dominant loss surface | dominant loss | truth pT | LCIO pT | RTS GSF pT | smoothed branches |
|---|---:|---:|---:|---:|---:|---:|
| 433/6 | 0 | 3.8069% | 2.0004 | 1.9226 | 1.9224 | 11/11 |
| 302/9 | 4 | 4.2579% | 2.0004 | 1.9126 | 1.9126 | 12/12 |
| 342/8 | 3 | 4.7831% | 2.0004 | 1.8998 | 1.8997 | 12/12 |

All RTS transitions completed, all tracks were finite, and there were no
measurement-update rejections.  None of the three inward losses was recovered.

The forward filter starts from the LCIO IP estimate, which is already close to
the lower post-loss momentum.  Forward BH convolution creates still lower
outgoing momenta; it cannot create the missing higher pre-loss seed mean.  A
control on 302/9 broadened the initial kappa covariance from `1e-7` to `1e-3`.
It also completed cleanly but produced 1.9125 GeV and left the identity-like
branch at weight 0.945, so covariance broadening alone does not repair the
missing mean support.

At the user's request the 302/9 covariance control was extended through
`KappaSeedCov = 1e-2`, `1e-1`, and `1`.  The corresponding smoothed pT values
were 1.9125, 1.9126, and 1.9126 GeV.  Every setting accepted all 2,766 component
updates, smoothed all 12 final lineages, and produced finite output.  The best
identity-like weight changes only from 0.9450 at `1e-3` to 0.9470 at `1`.
Thus the null result is not due to insufficient covariance magnitude or a
numerical rejection.  The first measurements collapse the single broad
Gaussian around the same biased-low mean; increasing its width does not create
a distinct high-momentum seed mode.

## Interpretation

This test rejects the idea that ordinary within-lineage RTS smoothing can
recover these few-inner-transition events. A full Gaussian-sum two-filter
smoother would need independently constructed forward and backward messages
and surface-wise mixture multiplication with explicit removal of shared
measurement information. The current reverse refit is not such a message: it
starts from forward information and would double-count measurements if simply
multiplied by the forward mixture.

For transition 0 in particular, a two-filter smoother cannot manufacture
pre-loss curvature measurements that do not exist. It may still change the
transition-3--4 boundary cases through a better independent prior/message
construction, but implementing it is a new algorithmic workflow rather than
enabling an existing option. The focused negative result should be retained as
the baseline and no performance claim should be made from it.

## Explicit TopN RTS covariance scan

After the user explicitly authorized TopN RTS, the same three reverse-missed
events were evaluated over the covariance range used in the historical hard
event scan. These results use `RetainedLineageSmoothing=True`,
`ReductionMode=TopN`, and `ReverseFiltering=False`; they do not change normal
defaults.

| KappaSeedCov | 433/6 pT [GeV] | 302/9 pT [GeV] | 342/8 pT [GeV] |
|---:|---:|---:|---:|
| 1e-7 | 1.922399 | 1.912559 | 1.899737 |
| 1e-5 | 1.922046 | 1.912538 | 1.899682 |
| 1e-4 | 1.922037 | 1.912526 | 1.899672 |
| 3e-4 | 1.922045 | 1.912524 | 1.899667 |
| 1e-3 | 1.922080 | 1.912526 | 1.899665 |
| 1e-2 | 1.922594 | 1.912539 | 1.899647 |

Truth is 2.000359 GeV. All 18 jobs terminated successfully and retained the
complete 232/233/233 hits. Comprehensive nominal dumps smooth 11/11, 12/12,
and 12/12 retained lineages with no measurement-update rejection. Thus the
historical hard 1/3 recovery as covariance grows is not reproduced for losses
at transitions 0, 4, and 3. The maximum change is only about 0.00055 GeV in
433/6; 302/9 and 342/8 are essentially invariant or slightly worse.

Outputs are under `/tmp/gsf-rts-covscan-inner`.

## Middle-transition TopN RTS scan

The scan was extended to ordinary topology-clean reverse misses at successive
middle surfaces: 230/7 has 4.5285% dominant loss at transition 5, 95/5 has
3.8782% at transition 6, and 45/3 has 3.5578% at transition 7. Their reverse
residuals are -4.693%, -3.973%, and -3.259%.

| KappaSeedCov | 230/7 pT [GeV] | 95/5 pT [GeV] | 45/3 pT [GeV] |
|---:|---:|---:|---:|
| 1e-7 | 1.907063 | 1.920857 | 1.935063 |
| 1e-5 | 1.907049 | 1.920865 | 1.935082 |
| 1e-4 | 1.907042 | 1.920863 | 1.935067 |
| 3e-4 | 1.907041 | 1.920851 | 1.935066 |
| 1e-3 | 1.907043 | 1.920816 | 1.935059 |
| 1e-2 | 1.907052 | 1.920652 | 1.935040 |

Truth is 2.000359 GeV. All 18 jobs completed with full 233/232/234 hits. The
comprehensive nominal 230/7 run accepted all 2,766 component updates and
smoothed 12/12 final lineages. No event recovers and the total covariance
dependence is below 0.00021 GeV. Thus merely reaching transition 5--7 is not
sufficient for forward TopN RTS recovery in the selected missed population;
the historical hard 1/3 result is not explained by surface index alone.

Outputs are under `/tmp/gsf-rts-covscan-middle`.
