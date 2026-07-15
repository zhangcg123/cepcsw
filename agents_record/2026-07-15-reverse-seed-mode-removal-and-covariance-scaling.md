# Reverse seed-mode removal and covariance scaling

Date: 2026-07-15

## Reason and decision

The CMSSW/ACTS source audit exposed an unnecessary difference between the
default CEPC reverse filter and ACTS: ACTS scales the covariance of every
component in the full final forward mixture before starting the backward pass,
whereas CEPC normally copied those covariances unchanged. CEPC's
`ReverseKappaSeedCov` applied only to two non-default single-component
diagnostics.

The user directed that the non-default `BestBroad` and `IdentityBroad` seed
modes be removed entirely and that `ReverseKappaSeedCov` be used for backward
seed scaling.

The removed modes were:

- `BestBroad`: retain only the highest-weight final forward component, discard
  its full covariance, and replace it with a hand-constructed broad diagonal
  covariance whose curvature entry was `ReverseKappaSeedCov`;
- `IdentityBroad`: do the same with the highest-weight exact no-radiation
  component.

Neither mode was the default. The default `ForwardMixture` path used every
final forward component with its forward posterior weight and unchanged
covariance.

## Implementation

`ReverseSeedMode` and all `BestBroad`/`IdentityBroad` branches were removed
from `GsfAlgorithm.h`, `GsfAlgorithm.cpp`, the reverse template, and
`scripts/run_bestbranch_no_ebrem_sample.py`. The associated
`GSF_REVERSE_SEED_MODE` environment variable and survey CLI switch were also
removed.

The reverse seed now has one invariant construction:

1. use every component of the filtered final forward mixture;
2. retain its forward posterior weight unless the separate diagnostic
   `ReverseInitialWeightMode=Uniform` is requested;
3. multiply the authoritative EDM4hep seed covariance, including correlations,
   by `ReverseKappaSeedCov` before storing the reverse continuation state;
4. convert that full scaled covariance to KalTest helix coordinates;
5. transport it to the outer hit pivot and initialize the reverse track. Every
   subsequent MarlinTrk `initialise` call therefore receives the scaled
   continuation covariance.

`ReverseKappaSeedCov` is retained as a legacy public property name, but its
semantics are now a dimensionless full-covariance multiplier rather than an
absolute curvature variance. Its default and reverse-template fallback are
100.0, matching the reviewed ACTS default scaling. Initialization rejects
non-finite or non-positive values. A value of 1.0 reproduces the former
full-mixture seed covariance semantics; it does not restore the removed
single-component modes.

## Validation

The configured EL9/LCG 105 `RecGsfTracking` target built and installed
successfully. Existing external-package compiler warnings were unchanged.

The first comprehensive run exposed an important wiring error during the
same-code A/B: scaling only the dummy KalTest history site left the
authoritative `continuationState` covariance unchanged, so scale 1 and 100
were exactly identical. This was corrected by scaling the EDM4hep `finalState`
before assigning both the dummy site and `continuationState`. The target built
and installed again successfully.

The final comprehensive same-code A/B used:

- input `/tmp/gsf-match-tracks.root`;
- `SelectedEventIndices=[11]`, `EvtMax=20`;
- five-component `CEPC2GeV85StepConditioned` BH model;
- `MaxComponents=24`, `ReverseKappaSeedCov=1` and 100;
- logs `/tmp/gsf-reverse-seedscale1-fixed-event11.log` and
  `/tmp/gsf-reverse-seedscale100-fixed-event11.log` with corresponding
  `*-fixed-event11.root` and flat tuple outputs in `/tmp`.

The normal Gaudi scheduled stop returned code 4 in both runs. Both completed
234/234 hits with 6,032 accepted reverse updates, zero reverse rejections, four
Jacobian failures recorded by the existing diagnostic, and finite output.
Truth and LCIO pT were 2.0004 and 1.7938 GeV. Scale 1 selected reverse ID 743
with weight 0.835630, 19 final components, chi2 483.2/460, and pT 1.98304 GeV.
Scale 100 selected ID 758 with weight 0.840833, 20 final components, chi2
481.1/460, and pT 1.98302 GeV. Thus the property now demonstrably affects the
reverse state and lineage, although event-11 pT changes by only about
-0.00002 GeV and both round to 1.9830 GeV.

The change is not physics validated. The x100 covariance scaling alters the
reverse measurement leverage and must be tested on the 19 overshoots/18
matched controls, ordinary light representatives, clean 62/9, hard 1/3, and
events 16/17 before population conclusions. The full category and transfer
ladder remains mandatory.
