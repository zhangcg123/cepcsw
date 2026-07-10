# Reverse-GSF muon control

Date: 2026-07-10

This record preserves the five-event muon control performed after the focused
reverse-filtering milestone. The purpose was to test the hypothesis that the
reverse workflow mechanically raises every reconstructed transverse momentum.

## Proper muon hypothesis

Five 2 GeV, 85-degree muon events from `trk-mu--2.0-85-1.root` were processed
with:

```text
ElectronHypothesis = false
ReverseFiltering = true
MaxComponents = 12
ComponentWeightCutoff = 1e-8
ReductionMode = KL
```

With the electron hypothesis disabled there were no BH splits, no reductions,
and exactly one component. All reverse measurement updates were accepted.

| event | truth pT [GeV] | LCIO pT [GeV] | reverse pT [GeV] |
|---:|---:|---:|---:|
| 0 | 2.0004 | 1.9997 | 1.99968 |
| 1 | 2.0004 | 1.9968 | 1.99671 |
| 2 | 2.0004 | 1.9979 | 1.99788 |
| 3 | 2.0004 | 2.0017 | 2.00168 |
| 4 | 2.0004 | 1.9951 | 1.99516 |

The reverse result is effectively identical to LCIO and is not systematically
higher. Reverse traversal and repeated measurement updating alone do not create
the electron-scale momentum increase.

## Forced electron BH hypothesis on muons

The same events were deliberately processed incorrectly with
`ElectronHypothesis=true` and `BHModel=GlobalSim2GeV85`. This used exactly the
same split/reduce strategy as the electron validation:

```text
measurement update and full innovation likelihood
  -> five-component GlobalSim2GeV85 split when eligible
  -> normalize and remove weights below 1e-8
  -> current-surface KL reduction to 12
  -> continue
```

The reverse pass used the same operations with inverse transition semantics.

| event | LCIO pT [GeV] | forced-BH reverse pT [GeV] | reverse splits | reverse reductions | final components | rejected updates |
|---:|---:|---:|---:|---:|---:|---:|
| 0 | 1.9997 | 1.9821 | 3 | 3 | 11 | 0 |
| 1 | 1.9968 | 1.9924 | 3 | 3 | 10 | 0 |
| 2 | 1.9979 | 1.9983 | 0 | 0 | 12 | 0 |
| 3 | 2.0017 | 1.9976 | 4 | 4 | 12 | 2 |
| 4 | 1.9951 | 1.9952 | 0 | 0 | 12 | 0 |

Three events decreased and two were nearly unchanged. Even an inappropriate
electron BH hypothesis did not raise every muon pT. The electron hard-loss
recovery is therefore associated with competition among process histories and
their measurement likelihoods, not a universal reverse-filter bias.

## Split-at-cap caveat

The strategy is identical for electrons and forced-BH muons, including an
implementation artifact: splitting occurs only while the current component
count is below `MaxComponents`. If a mixture already contains 12 components,
the split is skipped rather than temporarily expanding and then reducing. This
explains the zero reverse splits for forced-BH muon events 2 and 4 and applies
equally to the electron runs. It remains relevant when the final 4-5 component
policy is designed.

The option files, ROOT outputs, and logs used for this control were temporary
artifacts and are not project-status sources.
