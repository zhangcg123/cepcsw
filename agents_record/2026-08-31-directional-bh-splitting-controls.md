# Directional BH-splitting controls

Date: 2026-08-31

## Motivation and decision

The shared outward filter and the independent reverse inward filter previously
executed Bethe-Heitler component splitting whenever the electron/material
threshold conditions were satisfied. This made it impossible to isolate a
backward-only radiative-hypothesis campaign without changing source code.

`RecGsfTracking` now exposes two independent boolean properties:

| Property | Compiled default | Active reverse-template value | Maintained `DumpGsfTrks/gsf.py.bk` reverse value |
|---|---:|---:|---:|
| `ForwardBHSplitting` | `true` | inherited `true` | `false` |
| `InwardBHSplitting` | `true` | inherited `true` | `true` |

The compiled and active-template defaults preserve prior behavior exactly.
The maintained card deliberately selects the experimental backward-only BH
configuration. Its filenames do not encode these gates, so campaigns with
different values require distinct output tuple paths.

## Exact scope

`ForwardBHSplitting` gates both outward splitter call sites: the initial
hit-0-to-hit-1 interval and every subsequent accepted-hit outgoing interval.
It therefore governs the shared outward pass used by forward, retained-graph
smoother, and reverse workflows.

`InwardBHSplitting` gates the reverse outer-to-inner splitter call site. It is
inert unless `ReverseFiltering=true`; it does not govern
`GaussianSumSmoothing`, which only consumes the retained forward graph.

Both properties suppress BH child creation only. They do not disable:

- DD4hep/current-surface path evaluation;
- passive truth/runtime material recording;
- deterministic `ElossOn` treatment;
- `MSOn` process noise;
- propagation or measurement updates;
- same-surface `F_updated x B_predicted` product diagnostics.

With `InwardSeedCovarianceScale>0`, disabling inward splitting does not
collapse components copied from the terminal forward mixture. With the
maintained fresh seed (`InwardSeedCovarianceScale=-1`), disabling inward
splitting leaves one live inward state. `SmoothedMarginal` then has an
effectively unit marginal if its products are valid.

Passive `truth_material_*_above_threshold_count` values count
thickness-eligible paths, not executed splits, and remain populated when a
directional gate is false. Actual splitting must be audited with lineage
source 1/2 operation 2 or the detailed split record. The truth BH-loss oracle
also replaces only enabled, executed splitter calls. The independent
`RecGsfGlobalLossRefitter` is unaffected.

## Configuration audit

A dedicated option-surface audit verified all three splitter call sites and
made no source edits. `RecGsfTracking` now exposes 43 properties.
`DumpGsfTrks/gsf.py.bk` explicitly steers 42 and deliberately inherits only
`RecordTruthMaterialIntervals=true`. The authoritative package README and
workflow README record the new semantics and the maintained-card difference.
The active reverse template is intentionally unchanged because it inherits
the compatible `true/true` compiled defaults.

## Mechanical validation

The EL9/LCG-105 `RecGsfTracking` and `RecGsfFlatTuple` targets built and the
configured tree installed successfully. The focused matrix used
`trk_large_20260823/trk-e--2.0-85-1.root`, event 11, the production
`CEPC2GeV85StepConditioned` model, `DD4hepBetweenSurfaces`, fresh inward seed
`-1`, `MaxComponents=10`, cutoff `1e-4`, `SymmetricKL`, identity protection,
and `LocalMeasurement`.

| Forward | Inward | Forward split nodes | Inward split nodes | Final components | BestBranch pT [GeV] | WeightedMean pT [GeV] | FullMixtureMode pT [GeV] |
|---:|---:|---:|---:|---:|---:|---:|---:|
| true | true | 460 | 480 | 10 | 40.8956874601 | 41.3707442285 | 40.9034281818 |
| false | true | 0 | 480 | 10 | 40.8956874601 | 41.3707442285 | 40.9034281818 |
| true | false | 460 | 0 | 1 | 40.8956874601 | 40.8956874601 | 40.8956874601 |
| false | false | 0 | 0 | 1 | 40.8956874601 | 40.8956874601 | 40.8956874601 |

Truth and LCIO pT for this row are `40.7315673828` and `40.8954540661 GeV`.
Material summaries stayed populated in all cases. In forward/inward order,
valid-path aggregate counts were 1757/1206, 233/1206, 1757/233, and 233/233;
above-threshold counts were 92/96, 11/96, 92/11, and 11/11. These counts vary
with the number of live parents and are not split counts.

The requested `false/true` configuration then completed required events 11,
16, and 17. Against the stored pre-change `true/true` LocalMeasurement result,
all 192 common BestBranch, WeightedMean, FullMixtureMode, and final-component
scalars/vectors matched exactly. This is expected with a fresh inward seed and
local-measurement weighting: the live reverse result does not consume the
forward mixture.

One maintained-card smoke run combined `false/true` with
`SmoothedMarginal`. Event 11 completed with ten final components and
BestBranch/WeightedMean/FullMixtureMode pT of `40.9122483259`,
`41.5489877611`, and `40.9111819897 GeV`. This differs slightly from the
previous `true/true` SmoothedMarginal endpoint because the same-surface
products now have only the unsplit outward state as their forward partner. It
is mechanism evidence only.

## Disposition of the preceding inward-weight focus

The complete BH15 `SmoothedMarginal` population analysis is now available for
8,206 matched events: 7,310 topology-clear and 896 secondary-activity control
events. In the topology-clear population, LocalMeasurement versus
SmoothedMarginal FullMixtureMode had inclusive width68 `0.534%` versus
`0.535%`, abs-q68 `0.347%` versus `0.483%`, and 9 versus 37 residuals above
100%. No-eBrem width68 worsened from `0.207%` to `0.248%`; light-eBrem
abs-q68 worsened from `0.515%` to `0.629%`; hard-eBrem width68 improved from
`7.744%` to `7.183%` while abs-q68 worsened from `5.001%` to `5.398%` and
the above-100% tail count rose from 4 to 23. Therefore
`SmoothedMarginal` remains an unsafe diagnostic, not a default candidate.

## Next gate

Run direct same-code population comparisons with distinct output paths. First
establish the expected endpoint equivalence of `true/true` and `false/true`
under fresh-seed `LocalMeasurement`, while measuring graph size and memory.
Then compare those two split configurations under `SmoothedMarginal`, where
the outward gate changes the live overlap weighting. Split topology-clear
results into no/light/hard and early-transition categories; audit endpoint
failures and catastrophic tails; and report the stable secondary-activity
control separately. No compiled-default change is justified by the present
mechanical evidence.
