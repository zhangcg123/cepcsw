# CMS identity-compatibility chi-square schema simplification

Date: 2026-08-28

## Decision

The live CMS-like lineage schema retains one CMS-specific local diagnostic:

```text
lineage_node_cms_smooth_identity_compatibility_dchi2
```

Its EDM source is
`GSFLineageNodeCmsSmoothIdentityCompatibilityDChi2`. On every accepted
interior CMS backward measurement node, the same surviving forward-updated
identity state at that hit is compared with the node's backward-predicted
candidate. For forward mean/covariance `(mu_F,C_F)` and backward
mean/covariance `(mu_B,C_B)`, the recorded value is

```text
(mu_F-mu_B)^T (C_F+C_B)^-1 (mu_F-mu_B).
```

A finite value is the availability tag. NaN means the comparison was not
applicable or its full-state covariance operation failed. The value excludes
the backward prior and never changes a component state, weight, reduction, or
published endpoint.

The common `lineage_node_dchi2` on the same node is retained unchanged. In
CMS-like and reverse workflows it is the exact live
backward-predicted-state-versus-current-measurement chi-square returned by the
MarlinTrk update. The two diagnostics answer different questions:

- `lineage_node_dchi2`: compatibility with the current detector measurement;
- `lineage_node_cms_smooth_identity_compatibility_dchi2`: compatibility with
  a fixed forward identity reference that summarizes the inward-side fit.

The first has the detector measurement dimension and the second has five
track-state dimensions. Their absolute values must not be compared as if they
were the same test statistic. The intended comparison is whether identity and
radiative backward siblings at one hit receive the same or opposite ordering
under the two scores.

## Removed live diagnostics

The earlier broad, default-on per-layer CMS study was deliberately removed
from the algorithm EDM and flat tuple. Removed groups comprise:

- validity and forward-identity metadata;
- moment-matched forward predicted/updated kappa and variance;
- all-other/all-hit kappa, variance, and derived pT;
- all-other/all-hit overlap, compatibility log determinant, and log overlap;
- re-evaluated local chi-square, innovation determinant, and local
  likelihood;
- evidence and with-prior scores plus both within-hit normalizations;
- identity-compatibility log determinant and log overlap.

The complete superseded field names, formulas, and focused evidence remain in
`agents_record/2026-08-27-cms-like-per-layer-smoothing-diagnostic.md` and
`agents_record/2026-08-28-cms-like-forward-identity-compatibility.md`. Those
records describe historical files only and no longer define the live EDM or
flat-tuple interface.

## Implementation boundary

The active hit-1 CMS forward-by-backward product mixture and its three endpoint
publications are unchanged. The retained identity comparison reuses the
already required forward identity snapshots and backward predicted component
snapshots. Removed moment matches and local re-evaluation machinery are no
longer computed. No Gaudi configurable property was added, removed, renamed,
or changed, so `DumpGsfTrks/gsf.py.bk` requires no steering change.

## Validation gate

The focused gate for this schema change must establish all of the following:

1. `RecGsfTracking` and `RecGsfFlatTuple` build and install.
2. A CMS-like event contains exactly one flat branch matching
   `lineage_node_cms_smooth_*`.
3. That vector is row-aligned with `lineage_node_n`; accepted interior CMS
   backward nodes can carry both finite retained compatibility chi-square and
   finite common `lineage_node_dchi2`.
4. A reverse event keeps the same schema but records NaN in the CMS-specific
   vector.
5. CMS-like endpoint tracks and common lineage values are unchanged against a
   same-input pre-change reference, apart from the intentionally removed
   collections/branches and diagnostic-only log messages.

## Completed gate

Both `RecGsfTracking` and `RecGsfFlatTuple` built and installed in the focused
EL9/LCG-105 build. A same-input rerun of job 98 entry 15 used
`CEPCRuntimeCategoryAligned15Clear`, `MaxComponents=10`, cutoff `1e-4`, the
standard initializer, and CMS covariance scale 100. Its pre-change reference
was produced earlier on the same installed-code baseline before this passive
schema simplification.

- The new flat file contains exactly one `lineage_node_cms_smooth_*` branch;
  29 historical CMS-specific branches were removed.
- All 106 endpoint, final-mixture, and common-lineage branches compared exactly
  at entry 15, including NaN placement; there were zero mismatches.
- The event contains 9,902 lineage nodes. The retained vector has the same
  length, with 2,592 finite values over 231 hits. Every finite value is on an
  accepted source-2 measurement node whose common `lineage_node_dchi2` is also
  finite.
- A reverse-only rerun of the same event contains 8,496 lineage nodes and zero
  finite CMS-specific values, as required.
- CMS-like maintained-baseline reruns of job 1 entries 11, 16, and 17 all
  published endpoints. Their node/finite-retained counts were respectively
  `5670/2314`, `2815/1561`, and `3295/1619`; every finite retained value had a
  finite common backward measurement chi-square on the same node.

Passing this gate establishes mechanical schema simplification and
non-interference only. It does not calibrate either chi-square or validate a
physics-selection rule.
