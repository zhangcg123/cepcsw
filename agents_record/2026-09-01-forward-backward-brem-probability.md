# Forward/backward brem-probability diagnostic

Date: 2026-09-01

## Purpose

The reverse GSF already saves the direct same-surface comparison between
`F_updated[i]` and `B_predicted[i]`.  Its signed pT difference can indicate
that the outer hits prefer a lower momentum than the inner hits, but the raw
number is not comparable across surfaces or events with different covariance.
This change adds a dimensionless, one-sided Gaussian probability diagnostic.
It remains passive and does not alter component weights, cutoff, KL reduction,
inward propagation, or endpoint publication.

## Definition and interpretation

For every direct source-3, operation-5 pair, use the existing definitions

```text
delta_pT = pT(B_predicted) - pT(F_updated)
sigma_delta^2 = Var(delta_pT)
```

and save

```text
P_brem = Phi(-delta_pT / sigma_delta)
       = 0.5 * erfc(delta_pT / sqrt(2 * sigma_delta^2)).
```

Consequently, `P_brem=0.5` is directionally neutral, a value approaching one
means the backward message has significantly lower pT, and a value approaching
zero means the messages disagree in the opposite direction.  Mechanically,
`P_brem>0.97725` is one-sided 2-sigma evidence and `P_brem>0.99865` is
one-sided 3-sigma evidence.

The value is useful as a brem discriminator only on the exact no-radiation
pair.  A radiative pair is expected to move back toward `P_brem=0.5` when its
loss hypothesis reconciles the messages.  The probability is an uncalibrated
algorithmic score, not `P(brem | hits)`: it inherits the first-order pT
variance and zero forward/backward cross-covariance approximation, and it has
no point-mass no-brem prior or radiative-loss prior.  Population calibration
against independent no-brem and truth-brem samples remains required.

Finite zero variance maps negative, zero, and positive `delta_pT` to one,
one-half, and zero respectively.  Invalid inputs remain NaN, as do all nodes
other than direct source-3 operation-5 pairs and the two boundary mixtures.

## Persisted schema

The row-aligned EDM collection is

```text
GSFLineageNodeFBBremProbability
```

and `RecGsfFlatTuple` writes

```text
lineage_node_fb_brem_probability
```

An older EDM without this collection keeps any existing F/B delta fields and
receives row-aligned NaNs only in the new probability branch.

## Validation

The focused EL9/LCG-105 build and install of `RecGsfTracking` and
`RecGsfFlatTuple` completed successfully.  A verbose event-11 run and focused
events 11, 16, and 17 completed with the expected three endpoint collections
and populated lineage records.

The ten-event BH5 negative-peak sample was rerun from the same input events and
configuration.  All 247 pre-existing flat-tuple branches were identical to the
pre-change output at every selected row; the probability branch was the only
new branch.  Across 117,540 finite direct-product nodes, the persisted value
agreed exactly with the defining `erfc` expression.  No finite value occurred
on a non-direct node, and every finite value was within `[0,1]`.

For the identity pair at the truth-dominant loss interval, only two of the ten
selected 1--2% negative-peak events had `P_brem>0.95`; one exceeded the
one-sided two-sigma value `0.97725`, and none exceeded the three-sigma value
`0.99865`.  Those two events were also the only cases whose F/B loss estimate
lay within 30% of the Geant4 truth loss.  This is useful focused evidence that
the score expresses the intended directional separation, but it is neither a
population efficiency measurement nor a calibrated decision threshold.  The
complete validation table is a generated result at
`/tmp/gsf-negative-fb-probability/negative_peak_fb_probability_summary.csv`
and is intentionally not tracked.
