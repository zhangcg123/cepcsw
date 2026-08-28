# Standard-KF-style common GSF initialization

Date: 2026-08-28

## Decision and scope

All `RecGsfTracking` workflows now share one fresh forward initialization.
This includes ordinary forward GSF and the forward pass subsequently consumed
by the reverse, KL-smoother, and CMS-like workflows. The backward algorithms
remain mechanically unchanged: this change does not turn reverse or CMS-like
into an RTS smoother and does not alter their endpoint selectors.

The implementation is isolated in the dedicated `GsfTrackInitializer` class.
`GsfAlgorithm.cpp` only supplies the ordered matched hits, consumes the
returned first-hit-filtered site, and starts the remaining forward recursion at
hit 1. No parallel Kalman measurement formula was added.

## Baseline-compatible mechanics

For each input track the initializer:

1. passes the radius-ordered matched EDM4hep hits to
   `MarlinTrk::createPrefit`, which selects the first, middle, and last
   available two-dimensional hits;
2. assigns the same loose covariance defaults used by `FullLDCTracking`:
   `Var(d0)=1e6`, `Var(phi)=1e2`, `Var(omega)=1e-4`, `Var(z0)=1e6`, and
   `Var(tanLambda)=1e2`;
3. creates a temporary track from the configured GSF MarlinTrk system and
   executes `addHit(first) -> initialise(prefit) -> addAndFit(first)`;
4. retrieves the exact accepted first-hit filtered state and converts it to
   the initial KalTest site used by every forward GSF component.

The accepted first-hit `dchi2` is included in component fit chi-square. The
first measurement's dimension is also added to published NDF metadata because
the accepted state is transplanted as the GSF track's initial site rather than
stored as a second KalTest site. This preserves the existing site-index
contract while accounting for the newly consumed measurement.

## `KappaSeedCov` compatibility contract

`KappaSeedCov` remains available for old diagnostic cards, but it no longer
selects a `CompleteTracks`-anchored forward seed:

- the compiled and maintained-card default is `-1`;
- every finite value at or below zero uses the exact standard
  `Var(omega)=1e-4` prefit entry;
- a finite positive value replaces only that curvature entry with
  `Var(omega)=KappaSeedCov * alpha^2`, where `kappa=omega/alpha`;
- the geometric prefit, other four loose covariance entries, and explicit
  first-hit update remain active for every setting.

`ReverseKappaSeedCov` and `CmsErrorRescaling` retain their separate meanings
after the common forward pass.

## Focused gate

The EL9/LCG-105 `RecGsfTracking` target built and installed successfully. A
same-build verbose gate used the five-component production BH model,
`MaxComponents=12`, `ComponentWeightCutoff=5e-3`, standard
`KappaSeedCov=-1`, and events 11, 16, and 17 from
`trk_large_20260823/trk-e--2.0-85-1.root`. Reverse, KL-smoother, and CMS-like
all completed, retained all input-track hits, wrote 18-row flat tuples, and
reported identical initialization records for a given input track.
An ordinary-forward event-11 smoke also completed with 234/234 hits,
`pT=40.7718` GeV, and `chi2/NDF=452.8/462`, confirming that the same class is
used when all backward-workflow switches are off.

The leading-track pT values in GeV were:

| event | truth | CompleteTracks | reverse | smoother | CMS-like |
|---:|---:|---:|---:|---:|---:|
| 11 | 40.7316 | 40.9120 | 40.9037 | 41.0056 | 40.8933 |
| 16 | 37.8940 | 18.3050 | 18.3188 | 18.2975 | 18.2871 |
| 17 | 18.7970 | 14.7890 | 18.6130 | 18.9837 | 18.6124 |

Event 16 is not a clean single-track control: it also contains a nine-hit
secondary reconstructed track. That secondary track was separately fitted by
all three workflows (`CompleteTracks=38.5915`, reverse `38.9140`, smoother
`38.7745`, CMS-like `38.5922` GeV) and must not be pooled into topology-clear
performance claims.

The first-hit updates had the expected very small local chi-square under the
loose covariance. For the leading tracks, the first-hit records were:

| event | 2D hits | prefit omega | first-hit dchi2 | prefit Var(omega) |
|---:|---:|---:|---:|---:|
| 11 | 234 | -2.20683851e-5 | 2.81956341e-14 | 1e-4 |
| 16 | 224 | -5.00737478e-5 | 2.46036093e-18 | 1e-4 |
| 17 | 232 | -6.00232124e-5 | 6.34623138e-14 | 1e-4 |

This is a mechanical initialization and hard-event gate, not population
validation. The new seed deliberately changes the historical forward prior,
so stored old-method tuples are not same-code controls. Clean-track safety and
categorized held-out performance require fresh population reruns.

## Reproduction boundary

Generated EDM, flat ROOT, and verbose logs were kept under `/tmp` and are not
project records. The gate used
`Reconstruction/RecGsfTracking/options/run_gsf_reverse_template.py` with
`GSF_SELECTED_EVENT_INDICES=11,16,17`; the three mutually exclusive workflow
switches selected reverse, smoother, or CMS-like. Final performance claims
must rerun both sides with the same committed source.
