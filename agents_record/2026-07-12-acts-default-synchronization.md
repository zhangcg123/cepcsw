# Synchronization of the ActsAtlas reference model

## Reason

Direct plotting of the former embedded `ActsAtlas` tables exposed unphysical
means above one and enormous variances in the polynomial regime. Historical
records contained contradictory claims about the coefficient order. The
implementation was therefore compared directly with the current official ACTS
source rather than inferred from the older local tables.

## Authoritative source and implementation

The synchronization used ACTS `main` commit
`900c9e5e7709af4001aa4f24056d54f548616bab` on 2026-07-12:

- `Core/include/Acts/TrackFitting/BetheHeitlerApprox.hpp`
- `Core/src/TrackFitting/BetheHeitlerApprox.cpp`

The current default differs structurally from the former local implementation.
It has one six-component, fifth-degree table over `[0, 0.2]`, uses transformed
weight/mean/variance values for all components, evaluates coefficients by
highest-order-first Horner evaluation, and uses:

```text
t/X0 < 0.0001       no change
t/X0 < 0.002        exact-moment single Gaussian
t/X0 >= 0.002       six transformed polynomial components
t/X0 > 0.2          fallback evaluation at 0.2
```

The obsolete local low/high tables and untransformed high regime were removed.
All 108 embedded coefficients compare exactly with the official source. The
CEPC implementation retains its existing splitter interface and curvature/
covariance application; only the `ActsAtlas` mixture provider was synchronized.
`CEPC2GeV85StepConditioned` remains the default and is unaffected.

## Validation

`RecGsfTracking` builds and installs. Regenerated parameter plots are physical.
Complete verbose reverse-filter checks give:

| Focus | Result | Hits | Rejected updates |
|---|---:|---:|---:|
| clean-like 62/9 | pT 2.07425 GeV | 236/236 | 0 |
| light good-recovery 369/1 | pT 2.01715 GeV | 233/233 | 0 |
| hard 1/3 | pT 1.98463 GeV | 233/233 | 0 |
| exact-pair event 11 | pT 1.98627 GeV | 234/234 | 1 |

The available `/tmp/gsf-match-tracks.root` still ends after event 11, so exact-
pair events 16 and 17 cannot be rerun from that file. The one event-11 rejected
component update does not prevent a complete finite track, but it means the
zero-rejection focused gate is not satisfied for this optional reference.

The synchronized ACTS model remains an ATLAS reference, not CEPC validation.
Its overlay against CEPC eBrem-attributed transition truth visibly mismatches
the CEPC spectra, especially the no-eBrem atom and thin-step tail probabilities.
