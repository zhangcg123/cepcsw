# Reverse-Kappa correlated-prior validation and disconnect handoff

Date: 2026-08-24

## Purpose and boundary

This record preserves the no-eBrem reverse-refit validation completed before
a planned session disconnect. It answers whether
`ReverseKappaSeedCov=1` genuinely improves the reverse GSF or instead keeps the
result close to the already fitted `CompleteTracks`/forward posterior because
the same measurements are reused.

No project source was changed. The production default remains
`ReverseKappaSeedCov=100`, `ReverseInitialWeightMode=ForwardPosterior`,
`ReverseSelectionMode=AggregateWeight`, and `ReverseOutputMode=BestBranch`.
This diagnostic does not authorize Kappa tuning or a default change.

The repository state at the checkpoint was:

- branch: `dev`;
- source HEAD: `e14271f3b877a1100dae5696c3748f60a13098e5`;
- worktree: already very dirty before this diagnostic, with 3,665 status
  entries (139 tracked changes and 3,526 untracked entries);
- project changes made by this validation: documentation only;
- generated validation outputs: `/tmp` only.

The companion record
`agents_record/2026-08-24-no-ebrem-identity-lineage-kf-non-equivalence.md`
establishes the broader mechanism: a no-radiation identity lineage is not an
LCIO-state copy, the forward GSF is already not baseline-KF-equivalent, and the
reverse method is a new correlated inward refit.

## Structural source finding

The current reverse workflow is statistically correlated with its own data:

1. `GsfAlgorithm.cpp` lines 2758--2774 select the complete final forward
   filtered mixture as the independent-reverse seed.
2. Lines 2775--2787 multiply each full covariance by
   `ReverseKappaSeedCov` but retain the forward component mean and, by default,
   the forward posterior component weight.
3. Lines 2842--2844 revisit hits `N-2` through `0`; those hits were already
   consumed by the forward fit that produced the seed.
4. Lines 3083--3094 perform a fresh baseline-compatible
   `initialise -> addAndFit` update on each inward hit.
5. Lines 3101--3112 multiply the inherited weight by another full Gaussian
   innovation likelihood.

The compiled default is 100 in `GsfAlgorithm.h` lines 152--158. Covariance
inflation weakens the numerical influence of the correlated seed, but it does
not make the seed independent: its component means, process histories, and
default weights remain functions of the same measurements.

## Existing 100-event no-eBrem control

The frozen selection and eventwise comparison are:

- selection:
  `/tmp/gsf_truth_override_reached_100_20260823/topology_clear_no_ebrem_100_selection.csv`;
- eventwise table:
  `/tmp/gsf_truth_override_reached_100_20260823/topology_clear_no_ebrem_100_kappa1_comparison_eventwise.csv`;
- summary:
  `/tmp/gsf_truth_override_reached_100_20260823/topology_clear_no_ebrem_100_kappa1_comparison_summary.txt`;
- plot:
  `/tmp/gsf_truth_override_reached_100_20260823/topology_clear_no_ebrem_100_kappa1_comparison.png`.

The sample contains 100 topology-clear events with valid truth/material scope
and zero Geant4 interval eBrem above `1e-9 GeV`; seed counts are 31, 46, and
23. Five pre-existing LCIO reconstruction tails are seed/event `2:2`, `2:5`,
`2:63`, `2:97`, and `3:69`. They are retained in the inclusive table but
excluded from the 95-event clean-core metrics below.

### Central-value results

For the clean core:

| Method | Mean absolute truth residual | Width68 | Maximum absolute residual |
|---|---:|---:|---:|
| LCIO | 0.129591% | 0.145247% | not used as a method gate |
| reverse, Kappa 100 | 0.165241% | 0.186832% | 1.35418% |
| reverse, Kappa 1 | 0.127864% | 0.146329% | 0.533922% |
| GSF smoother | 0.120902% | 0.149400% | 0.451285% |

Across all 100 events, the method-to-LCIO displacement was:

| Method | Mean absolute displacement from LCIO | Maximum | Within 0.01% | Within 0.1% |
|---|---:|---:|---:|---:|
| reverse, Kappa 100 | 0.068137% | 0.786217% | 11/100 | 80/100 |
| reverse, Kappa 1 | 0.005849% | 0.044652% | 84/100 | 100/100 |
| GSF smoother | 0.021132% | 0.328728% | 39/100 | 99/100 |

Eventwise, Kappa 1 was closer to truth than LCIO in 57 events and farther in
43. It was closer to truth than Kappa 100 in 68 and farther in 32. These counts
do not make Kappa 1 an independent improvement: its clean-core mean absolute
displacement from LCIO is only `0.006012%`, about 4.64% of the LCIO mean
absolute truth-residual scale. It primarily preserves the baseline central
value.

## EDM covariance and truth-pull validation

The full EDM files were used, not flat-tuple proxy errors. For input
`CompleteTracks` and output `GSFTracks`, track 0's first state was required to
have `location=1` (`AtIP`). The analysis used:

```text
pT = |(3 * 2.99792458e-4) / omega|
sigma(pT)/pT = sqrt(covMatrix[5]) / |omega|
pull = (pT - truth pT) / sigma(pT)
```

`covMatrix[5]` is the packed `omega-omega` variance. The maximum difference
between pT reconstructed from the EDM curvature and the existing flat-tuple
pT over all four methods and 100 selected events was exactly `0.000e+00 GeV`
at the reported numerical precision. This closes the state mapping before any
covariance interpretation.

The 95-event clean-core result is:

| Method | Median sigma(pT)/pT | Mean sigma(pT)/pT | Pull mean | Pull RMS | Pull width68 | abs(pull)>3 |
|---|---:|---:|---:|---:|---:|---:|
| LCIO | 0.10840% | 0.10879% | +0.196 | 1.519 | 1.477 | 4 |
| reverse, Kappa 100 | 0.17602% | 0.17610% | +0.120 | 1.165 | 1.054 | 2 |
| reverse, Kappa 1 | 0.10678% | 0.10773% | +0.202 | 1.531 | 1.461 | 5 |
| GSF smoother | 0.14176% | 0.14453% | +0.142 | 1.114 | 1.051 | 1 |

Paired relative-uncertainty ratios to LCIO were:

| Method / LCIO | Median | 16% quantile | 84% quantile |
|---|---:|---:|---:|
| reverse Kappa 100 | 1.618 | 1.570 | 1.651 |
| reverse Kappa 1 | 0.985 | 0.977 | 0.996 |
| smoother | 1.312 | 1.302 | 1.326 |

This does **not** show a catastrophic square-root-of-two covariance collapse
at Kappa 1. Instead, Kappa 1 reproduces essentially the LCIO uncertainty and
the same under-covered pull distribution. Its good-looking central values are
therefore not accompanied by a new, independently calibrated determination.
Kappa 100 broadens the reported uncertainty enough to improve pull coverage,
but its central value moves farther in many clean events.

## New five-event Kappa sweep

Five topology-clear, truth-valid no-eBrem seed-1 events were rerun:

```text
10, 22, 65, 87, 88
```

Events 10, 22, 65, and 88 cover progressively displaced reverse-100 cases;
event 87 is a quiet matched control. Existing Kappa 1 and 100 outputs were the
endpoints. New same-code runs used Kappa 3, 10, and 30. All other steering was
unchanged: reverse method, truth BH-loss oracle on, DD4hep-between-surfaces,
the production five-component BH selector, 12 components, symmetric KL,
cutoff/split `1e-4`, identity protection, forward-posterior initial weights,
aggregate-weight selection, best-branch publication, and ECAL off.

The three new jobs exited zero and finalized successfully. The oracle matched
all five selected input tracks and replaced 82 executed BH responses. Event
65 also contains a second reconstructed input track without oracle scope; that
track fell back to ordinary BH and is not the track-0 pT used here.

Each cell below is `truth residual % / reported sigma(pT)/pT % / truth pull`:

| Event | LCIO | K1 | K3 | K10 | K30 | K100 | Smoother |
|---|---|---|---|---|---|---|---|
| 1:10 | -0.0092 / 0.1342 / -0.068 | +0.0030 / 0.1327 / +0.023 | +0.0502 / 0.1724 / +0.291 | +0.1012 / 0.2014 / +0.502 | +0.1310 / 0.2141 / +0.612 | +0.1466 / 0.2194 / +0.668 | -0.0402 / 0.1759 / -0.228 |
| 1:22 | +0.0658 / 0.1125 / +0.585 | +0.0591 / 0.1111 / +0.532 | -0.0073 / 0.1438 / -0.050 | -0.0781 / 0.1681 / -0.464 | -0.1237 / 0.1790 / -0.691 | -0.1446 / 0.1836 / -0.788 | +0.1098 / 0.1483 / +0.740 |
| 1:65 | +0.5680 / 0.2382 / +2.384 | +0.5339 / 0.2731 / +1.955 | +0.7693 / 0.3367 / +2.285 | +1.0690 / 0.3945 / +2.710 | +1.2622 / 0.4256 / +2.966 | +1.3542 / 0.4400 / +3.078 | +0.2392 / 0.4612 / +0.519 |
| 1:87 | -0.0277 / 0.1265 / -0.219 | -0.0220 / 0.1245 / -0.177 | -0.0244 / 0.1606 / -0.152 | -0.0254 / 0.1856 / -0.137 | -0.0252 / 0.1960 / -0.129 | -0.0262 / 0.2003 / -0.131 | -0.0146 / 0.1654 / -0.088 |
| 1:88 | +0.1089 / 0.1040 / +1.047 | +0.0991 / 0.1036 / +0.957 | +0.1576 / 0.1346 / +1.171 | +0.2333 / 0.1582 / +1.475 | +0.2717 / 0.1691 / +1.607 | +0.2990 / 0.1738 / +1.720 | +0.0534 / 0.1377 / +0.388 |

The paired five-event trajectory is:

| Method | Mean abs(method-LCIO) | Mean abs truth residual | Median sigma(pT)/pT |
|---|---:|---:|---:|
| LCIO | 0.00000% | 0.15592% | 0.12649% |
| Kappa 1 | 0.01370% | 0.14343% | 0.12452% |
| Kappa 3 | 0.07715% | 0.20176% | 0.16061% |
| Kappa 10 | 0.17641% | 0.30139% | 0.18565% |
| Kappa 30 | 0.23784% | 0.36277% | 0.19604% |
| Kappa 100 | 0.26879% | 0.39412% | 0.20026% |
| smoother | 0.09448% | 0.09143% | 0.16540% |

For every displaced case, increasing Kappa progressively moves the result away
from LCIO while increasing its reported uncertainty. The quiet event remains
stable. This is the expected signature of a correlated forward-posterior prior
whose influence is being diluted. It is not evidence that Kappa 100 is optimal,
and the deliberately selected five events are not a population validation.

## Temporary artifacts

The new outputs are under:

```text
/tmp/gsf_reverse_kappa_sweep_20260824/seed01/
```

The important files are:

```text
kappa3.log
kappa10.log
kappa30.log
gsf_e-_reverse-truth-bh_89301.root
gsf_e-_reverse-truth-bh_891001.root
gsf_e-_reverse-truth-bh_893001.root
gsf_flat_e-_reverse-truth-bh_89301.root
gsf_flat_e-_reverse-truth-bh_891001.root
gsf_flat_e-_reverse-truth-bh_893001.root
```

The pre-existing temporary card
`/tmp/gsf_truth_override_reached_100_20260823/gsf_reverse_truth_kappa1.py`
was mechanically extended to accept `GSF_SELECTED_EVENTS` and
`GSF_REVERSE_KAPPA_SEED_COV` environment variables. This is not a maintained
project card and is intentionally not committed. Recreate rather than trust it
if `/tmp` is lost.

## Conclusion

The reverse method has a formal repeated-data/correlated-prior problem. Kappa
1 makes that correlation numerically dominant and explains why its clean-track
central value resembles LCIO so closely. Kappa 100 is a heuristic forgetting
factor: it reduces the prior's covariance precision and improves pull coverage,
but it cannot erase the forward-data dependence of the seed means, component
histories, or default posterior weights.

Do not claim Kappa 1 as GSF recovery, and do not solve this by tuning Kappa.
The mathematically clean directions are a measurement-independent outer seed
for a genuinely independent backward refit, or a correlation-aware smoother
that explicitly uses stored forward predicted/filtered states and transport.
Before either becomes a production proposal, first establish a default-off
single-component KF-equivalence control as described in the companion record.

## Restart procedure

After reconnecting:

1. Read `AGENTS.md` for the live project status and frozen production baseline.
2. Read this record and
   `agents_record/2026-08-24-no-ebrem-identity-lineage-kf-non-equivalence.md`.
3. Confirm branch `dev`, source HEAD, and the dirty worktree before any edit;
   preserve all unrelated changes.
4. Do not use `/tmp` outputs for a final claim without a same-code rerun.
5. Keep reverse Kappa 100 unchanged. If the reverse-correlation investigation
   is explicitly resumed, design and review the single-component equivalence
   or independent-prior diagnostic before changing production source.
6. Otherwise return to the ordered material/BH branch-local investigation in
   `AGENTS.md`; the present side diagnostic does not replace that active focus.

No technical implementation remains running at disconnect time.
