# Weighted Runnalls reduction: focused validation and 100-event test

Date: 2026-07-16

## Question and bounded change

This test asks whether the pair-ranking cost used by mixture reduction offers
an immediate optimization. It does not change the moment merge, posterior
cutoff, identity-lineage protection, measurement update, process model, or
final selection. Established `SymmetricKL` remains default. A default-off
`ReductionMergeCost=Runnalls` choice was added inside `RecGsfTracking` only.

For normalized weights and covariances, the tested Runnalls bound is:

```text
B(i,j) = 1/2 [(wi+wj) log|Vij| - wi log|Vi| - wj log|Vj|]
```

`Vij` is the covariance from the existing moment match. The five-dimensional
helix mean uses the same phi wrapping as that merge. Non-positive/non-finite
determinants receive a sentinel cost; weights are normalized before ranking.
The initialized property accepts only `SymmetricKL` or `Runnalls`; steering is
exposed as `GSF_REDUCTION_MERGE_COST` and runner option
`--reduction-merge-cost`.

## Build and focused gate

The focused build and install completed successfully. Topology-clean light
event 284/1 was then run with reverse filtering, `MaxComponents=12`, aggregate
weight, Runnalls, and comprehensive component/reduction dumps. Outputs are in
`/tmp/gsf-runnalls-focused-284-1`. It retained 232/232 hits, had zero reverse
update rejection, made seven reductions, ended with 12 components, and
published pT 2.0275 GeV. Inspected determinants and merge costs were finite;
no sentinel, NaN, or infinity was selected. This is mechanical validation.

## Matched 100-event A/B

The durable input list was
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/random_light_100_seed20260713.csv`:
100 topology-clean light-eBrem events in 87 files. Both sides used identical
current code, input entries, reverse workflow, `MaxComponents=12`, aggregate
selection, and identity protection. Only merge ranking differed.

- Symmetric KL: `/tmp/gsf-random-light100-symkl12-20260716`
- Runnalls: `/tmp/gsf-random-light100-runnalls12-20260716`
- eventwise table: `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/random_light100_symkl12_vs_runnalls12_20260716.csv`
- summary: adjacent `_summary.json`

The reused comparator calls baseline columns `reverse` and candidate columns
`cms`; here those aliases mean SymmetricKL and Runnalls. All 100 pairs
completed and had matching LCIO/GSF hit counts.

| metric | LCIO | Symmetric KL | Runnalls |
|---|---:|---:|---:|
| median residual [%] | -0.21381 | -0.04080 | -0.07541 |
| mean absolute residual [%] | 1.12543 | 0.61251 | 0.61940 |
| RMS residual [%] | 2.26054 | 1.22469 | 1.23068 |
| central-68 half-width [%] | 1.20580 | 0.41924 | 0.37602 |
| inside 1% | 73 | 83 | 82 |
| inside 2% | 81 | 93 | 93 |

Runnalls changed pT in 31/100 events: it won absolute truth error in 13, lost
in 18, and tied in 69. It lowered pT in 20 and raised it in 11. The largest
improvement was 457/7 (+0.37061% to -0.02795% residual); the largest worsening
was 107/1 (+0.27501% to -0.74119%).

## Decision

The narrower central 68% comes with slightly worse mean absolute error, RMS,
inside-1% count, and eventwise truth error. Keep `SymmetricKL` as default and
Runnalls as a controlled diagnostic. Promotion requires the established
overshoot/matched-control ladder plus no-eBrem and hard-eBrem populations;
this 100-event light sample is not sufficient for tuning or validation.
