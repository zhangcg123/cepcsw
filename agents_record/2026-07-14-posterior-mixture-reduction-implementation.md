# Posterior mixture-reduction implementation

Date: 2026-07-14

## Requested change

The user requested implementation of the statistically preferable ordering in
which every Bethe-Heitler process child receives the target measurement update
and exact innovation likelihood before low-weight cutoff or KL/TopN mixture
reduction. The implementation is confined to
`Reconstruction/RecGsfTracking/src/GsfAlgorithm.cpp`; no shared package was
modified.

The preceding immediate-reduction ordering and removal of the ineffective
`ReductionMinHitsAfterSplit` control are preserved in
`agents_record/2026-07-14-reduction-min-hits-removal.md`.

## Implemented ordering

Forward transition `i -> i+1`:

```text
posterior at hit i
  -> outgoing BH split
  -> propagate/update every child at hit i+1
  -> exact log likelihood -0.5*(deltaChi2 + logDetInnovation)
  -> normalize
  -> posterior low-weight cutoff
  -> posterior KL/TopN reduction
```

Reverse transition `i+1 -> i`:

```text
posterior at hit i+1
  -> direction-reversed BH split
  -> propagate/update every child at hit i
  -> exact log likelihood -0.5*(deltaChi2 + logDetInnovation)
  -> normalize
  -> posterior low-weight cutoff
  -> posterior KL/TopN reduction
```

The existing `MaxComponents`, `ReductionTargetComponents`, reduction mode,
identity protection, BH model, likelihood, and aggregate-weight publication
semantics are unchanged. `MaxComponents=24` is now a posterior reduction
target rather than a pre-measurement computational ceiling. With five BH modes,
focused runs reach 120 temporary components and update all of them before
reduction.

Verbose stages now identify `posterior-cutoff`, `posterior-pre-reduce`, and
`posterior-post-reduce` in the forward pass, and
`reverse-posterior/norm`, `reverse-pre-reduce`, and
`reverse-post-reduce` in the reverse pass. The existing reverse decisive-odds
stage names remain available, but now refer to target-measurement posteriors.

## Build and installation

The configured EL9/LCG-105 `RecGsfTracking` target built and linked
successfully. The complete configured install also completed successfully.
Only the established external ROOT/KalTest compiler and missing-PCM warnings
were observed.

## Comprehensive focused checks

All checks used the installed package, `MaxComponents=24`,
`CEPC2GeV85StepConditioned`, `DD4hepBetweenSurfaces`, reverse filtering,
aggregate-weight selection, best-branch publication, and comprehensive
component dumps.

| Seed/event | Role | Hits | Forward A/R/J | Reverse accepted/rejected | Peak/final | Truth / LCIO / GSF pT [GeV] | Selected reverse mode |
|---|---|---:|---:|---:|---:|---:|---|
| 284/1 | known false correction | 232/232 | 5564/0/0 | 6216/0 | 120/24 | 2.0004 / 1.9837 / 2.0275 | hit-4 g2 |
| 404/8 | genuine light recovery | 233/233 | 5913/0/0 | 5877/0 | 120/24 | 2.0004 / 1.9708 / 1.9999 | hit-7 g2 |
| 62/9 | clean-like safety case | 236/236 | 1878/0/0 | 6297/0 | 120/23 | 2.0004 / 1.9999 / 2.0678 | hit-5 g2 |

All three tracks are complete and finite with no new update rejection or
reported covariance failure. Gaudi returns scheduled-stop code 4 after the
selected event and successfully finalizes each job.

Logs and generated outputs are under `/tmp`:

```text
/tmp/gsf-posterior-reduction-284-1.log
/tmp/gsf-posterior-reduction-404-8.log
/tmp/gsf-posterior-reduction-62-9.log
```

## Interpretation and validation boundary

The machinery objective is achieved: process children are no longer compressed
before their target hit evaluates them. The focused physics evidence is mixed,
not validating. The strong 404/8 recovery is retained, but 284/1 still selects
a false radiative mode and clean-like 62/9 remains severely overcorrected.
These are not same-code pre/post population comparisons, so no broad performance
claim follows from them.

The required 19-overshoot/18-control population, five ordinary light
representatives, hard 1/3, legacy events 11/16/17, and full category/transfer
ladder have not been run. The local seed-1 input required for hard 1/3 and the
legacy 11/16/17 checks is absent. Do not treat build success, finite output, or
the 404/8 improvement as validation of posterior reduction.

