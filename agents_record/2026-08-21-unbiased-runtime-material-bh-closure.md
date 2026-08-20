# Unbiased runtime material/BH closure (2026-08-21)

## Scope and steering

This record evaluates the new `MaterialBHAuditCSV` campaign against the paired
Geant4/DD4hep material-recorder tuples. It is analysis only: no source, model,
default, or run-card setting was changed.

The audited runtime steering is the frozen production candidate:

- `MaterialPathMode=DD4hepBetweenSurfaces` in both directions;
- `BHSplitThreshold=1e-4`;
- five-component `CEPC2GeV85StepConditioned`;
- `MaxComponents=12`, `SymmetricKL`, identity-lineage protection, and
  `ComponentWeightCutoff=1e-4`;
- reverse `BestBranch` publication and ECAL off.

Although the filenames retain `e--2.0-85`, the material-recorder campaign is
the broad 10--50 GeV, theta 40--140 degree electron sample. Of 411 nonempty
audit files, 390 contained 100 audited events, nine contained 99, and twelve
contained 2--44. The usable audit therefore contains 40,040 events, of which
35,582 have no secondary tracker SimHit activity and 4,458 are the separately
reported topology/control population. Fewer than 100 audited events does not
by itself distinguish an incomplete job from an event without an auditable
input track; twelve of the low-count seeds also lacked a normal `gsf_tuple`
tree and were recovered only for material analysis.

## Exact comparison population

The CSV component rows were grouped by seed, event, input track zero,
direction, seed flag, and consecutive runtime hit pair. Within every group:

- parent weights summed to one within `6.67e-16`;
- the largest component-to-component `path_t_over_x0` spread was
  `7.30e-13`;
- no group mixed split-threshold or BH-execution decisions.

This justifies one authoritative runtime path per hit pair while retaining the
full parent population in the audit. The 411 files contain 18,422,697 such
runtime groups: 9,211,554 forward, 9,211,143 reverse, and 767,674 executed BH
groups across both directions.

Runtime endpoints were matched spatially to the primary truth-side sensitive
midpoint anchors. The runtime pair remains authoritative. When it skipped a
truth anchor, the recorder intervals between the matched endpoints were
summed. A five-millimetre endpoint gate rejects remote nearest-neighbour
assignments; the normal endpoint offset has median about 0.405 mm and 95th
percentile about 1.22 mm. BH response is counted once with the complete
forward sequence (including its separately recorded seed path); reverse rows
are used for direction closure.

The Python reproduction of the compiled CEPC interpolation was checked
against audit child weights. For example, the first seed-1 call at
`0.0008551722942523852 X0` reproduced identity weight
`0.99325270028046397` exactly.

## Material closure when the runtime path is valid

For 9,195,374 spatially matched endpoint pairs valid in both directions, the
largest forward/reverse relative difference is `4.21e-10`; none exceeds
`1e-9`. The prior canonical-direction correction therefore closes the valid
population. The 88 apparent direction outliers in an ungated comparison all
involve at least one invalid coverage evaluation.

For topology-clear, spatially matched, valid forward paths, the summed runtime
minus truth-recorder DD4hep thickness is:

| runtime region | paths | summed relative difference |
|---|---:|---:|
| VXD | 169,526 | +0.0055% |
| VXD -> ITK | 34,508 | -0.0083% |
| ITK | 70,045 | -0.0070% |
| ITK -> TPC | 34,925 | -0.0292% |
| TPC | 7,869,675 | +3.978% |
| TPC -> OTK | 34,660 | +0.0225% |
| all regions | 8,213,360 | +0.571% |

The same comparison to summed Geant4 `t/X0` is nearly identical. The TPC
offset is the known truth-midpoint versus digitized/reconstructed-endpoint
effect amplified by very thin gas intervals: its per-interval relative
16--84% range is -4.24% to +6.63%. It is below the split threshold for the
ordinary internal-TPC population. Re-evaluating the current BH model with the
matched truth DD4hep or Geant4 thickness instead of the exact runtime
thickness changes the inclusive predicted radiative probability only from
`0.0775244` to `0.0773576`/`0.0773606`, and the predicted mean loss from
`0.00656680` to `0.00655244`/`0.00655270`. Thus the normal endpoint-level
material difference is not the cause of the observed BH-response mismatch.

## Newly exposed invalid coverage population

The unbiased audit contains 11,175 runtime groups with `path_valid=0`. Unlike
the focused 30-event sample, the production-scale sample therefore does not
support a global statement that all DD4hep paths are valid. In the
topology-clear forward physical population there are 4,790 invalid paths in
223 events:

- 4,512 internal TPC paths, normally returned with zero usable path;
- 209 TPC -> OTK paths;
- 24 ITK -> TPC paths;
- 45 paths in VXD/ITK categories.

They never execute BH, even when the returned `path_t_over_x0` is above
`1e-4`. The invalid topology-clear paths contain 34 eBrem intervals and
52.16 GeV of Geant4 eBrem loss; six lose more than 5%, five more than 10%, and
three more than 20%. Representative missed hard losses are:

| seed:event | runtime hit pair | region | runtime `t/X0` | truth loss fraction |
|---|---:|---|---:|---:|
| 60:9 | 230 -> 231 | TPC -> OTK | 0.009364 | 81.39% |
| 444:99 | 230 -> 231 | TPC -> OTK | 0.007830 | 28.22% |
| 149:57 | 232 -> 233 | TPC -> OTK | 0.007846 | 25.27% |
| 214:27 | 231 -> 232 | TPC -> OTK | 0.007792 | 16.56% |

The failure is the full-distance coverage invariant, not an empty material
query. For seed 104 event 14, hit 231 -> 232, the endpoint distance is
61.3319 mm while the audited segment lengths sum to 53.9215 mm: 7.4105 mm is
missing and coverage is 87.92%. The returned thickness is nevertheless
`0.0079784 X0`, close to truth DD4hep `0.0079924 X0`, but `valid=0` makes
`above_split_threshold=0` and suppresses BH. The bounded one-micron leading
nudge cannot repair a multi-millimetre missing segment. This is an unresolved
material-path/coverage defect and must be closed before retuning BH.

## Production split-threshold population

After removing invalid and spatially unmatched rows, every valid
topology-clear forward path below the production split threshold is internal
TPC gas. There are 7,867,363 such intervals. Geant4 records eBrem in 6,080 of
them, including 792 losses above 5%, 525 above 10%, and 271 above 20%. They
contain 5,440.96 GeV of eBrem loss: 20.89% of all eBrem-bearing clear forward
intervals and 16.01% of their total eBrem loss.

If the current BH mixture is evaluated counterfactually at these exact
sub-threshold thicknesses, its radiative probability agrees closely with
truth (`0.00077072` predicted versus `0.00077281` observed), but it predicts
too much loss magnitude (`3.9205e-5` mean fractional loss versus
`2.3807e-5`). Production executes no split there. This identifies a real
threshold/gating limitation, but the active non-goal remains threshold tuning
before invalid-path closure and branch-local diagnosis.

## BH-response closure on actual valid calls

For 383,128 inclusive valid, spatially matched, executed forward calls:

| quantity | observed | current BH prediction |
|---|---:|---:|
| eBrem probability | 0.072743 | 0.077524 |
| mean fractional eBrem loss | 0.004522 | 0.006567 |
| `P(loss > 5%)` | 0.017819 | 0.019689 |
| `P(loss > 20%)` | 0.006640 | 0.009648 |

The inclusive response is closer than either topology partition, but the
current model still overpredicts the mean and extreme tail. The apparent
inclusive agreement also hides a strong selection dependence:

| population | calls | observed/predicted eBrem probability | observed/predicted mean loss | observed/predicted `P(loss>20%)` |
|---|---:|---:|---:|---:|
| topology clear | 345,997 | 0.066463 / 0.077205 | 0.002958 / 0.006535 | 0.004052 / 0.009596 |
| secondary activity | 37,131 | 0.131265 / 0.080505 | 0.019101 / 0.006863 | 0.030756 / 0.010131 |

Secondary tracker activity is correlated with the hard-radiation outcome and
is not a truth-blind runtime conditioner, so this split must not become model
logic. It shows why an inclusive scalar calibration can conceal opposite
clean/control biases. In the clear set, VXD radiative probability is close
(`0.016493` observed versus `0.017205` predicted), while ITK, detector bridges,
and executed long TPC intervals are more over-radiative and especially
over-heavy-tailed. The mean-loss excess is persistent from 10 to 50 GeV and
changes sign below 10 GeV, supporting an energy/region/angle response study
rather than a thickness-only retune.

There are 1,533 inclusive calls above the last current BH knot
`0.0244948974 X0` (0.400% of executed calls): 567 reach 0.03, 49 reach 0.05,
26 reach 0.1, and the maximum is 0.647534. Of these, 313 span more than one
truth sensitive interval. The current model returns the same last-knot mixture
for all of them, so this is a rare but explicit interval-collapse/saturation
limitation that can matter branch-locally.

## Interpretation and next actions

The unbiased result separates four mechanisms:

1. Valid forward/reverse DD4hep paths close and replacing their exact runtime
   thickness with matched truth thickness does not repair BH calibration.
2. An unresolved coverage-validity defect suppresses BH on some real paths,
   including several hard TPC -> OTK losses.
3. The `1e-4` split gate deliberately suppresses all ordinary internal-TPC BH
   response, including rare hard losses that the model probability would
   otherwise describe.
4. On actual valid calls, the thickness-only current BH response is
   population-, region-, energy-, and tail-miscalibrated; rare large collapsed
   intervals also saturate.

Proceed by reproducing the invalid seed 60:9, 104:14, 149:57, and 444:99 paths
with explicit requested/covered-length diagnostics and matched valid controls.
Review a focused coverage repair before changing source. After same-code
material A/B closure, find the first truth-compatible lineage rank loss in
those and matched good events. Only then separate a threshold control from an
energy/angle/region-aware BH candidate and apply the focused verbose,
hard-event 11/16/17, clean-track, and held-out population gates.
