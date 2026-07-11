# 2 GeV, 85-degree transition pipeline and state diagnosis — 2026-07-12

## Scope decision

The first CEPC step-conditioned mixture is deliberately restricted to primary
electrons with 2 GeV transverse momentum at 85 degrees. It is an execution
proof, not a general CEPC model. The user explicitly deferred independent
training/held-out separation: the same production sample may be fitted and
inspected. The model must be named `CEPC2GeV85StepConditioned` so its limited
scope cannot be mistaken for general validation.

## Production status

The Geant4 step producer records primary-electron pre/post position and
momentum, step `t/X0`, process name/subtype, material, sensitive flags, and
touchable paths. The transition builder assigns every recorded step exactly
once to the half-open interval between consecutive reconstruction-equivalent
sensitive-surface entries. Each output row contains `p_before`, `p_after`,
`z=p_after/p_before`, `-log(z)`, total and eBrem-attributed loss, eBrem step
count, path length, total `t/X0`, material/process breakdown, and bounding
surface identities.

The recorder is integrated into the existing simulation template. Batch files
are:

- `DumpGsfTrks/sim.py.bk`;
- `dump_gsf_material_steps.sh`;
- `sub_gsf_material_step_jobs.sh`.

The submission driver requests ten seeds of 1000 events each at 2 GeV pT and
85 degrees. The user submitted these jobs on 2026-07-12. All simulations
completed; seeds 3-9 required local transition conversion after the original
batch worker's relative build-runner path was mangled. The worker now uses an
absolute build runner. The per-seed ROOT outputs were archived after fitting
under `BHModelComparisonStudies/CEPC2GeV85StepConditioned/production/` as
`gsf_material_steps-e--2.0-85-SEED.root` and
`sim-material-e--2.0-85-SEED.root`, together with the derived
`gsf_material_transitions-e--2.0-85-SEED.csv` files and audit JSON. The ten
seeds contain 2,574,697 transitions with finite `z`, no `z>1`, and complete
two-anchor event coverage.

The first extraction uses five fixed `1-z` strata (negligible `<1e-4`, small
`<0.01`, moderate `<0.05`, large `<0.2`, and extreme) within eight `t/X0` bins
from zero to 0.03. It accepts 2,573,914 rows, including 9,528 eBrem rows, and
leaves 783 higher-thickness transitions outside the reconstruction-range
table. The reproducible extractor is
`Reconstruction/RecGsfTracking/scripts/extract_cepc_step_conditioned_mixture.py`;
the current generated artifact is `/tmp/cepc2gev85-step-mixture-10seeds.csv`.

There are many more transitions than events because a transition is one
surface-to-surface material interval, not one primary electron. The 2,574,697
rows from 10,000 events correspond to a mean of 257.47 owned intervals per
event. Intervals from the same event are correlated and must not be described
as statistically independent events.

The full transition sample contains 9,985 intervals with at least one eBrem
step; 9,528 are within the first model range `t/X0 < 0.03`. Across the complete
10,000-event sample, the eBrem-attributed loss sums to 1311.52 GeV, or
0.131152 GeV per event (6.56% of the 2 GeV input momentum). There are 5,873
events with at least one recorded eBrem step; conditional on those events, the
mean eBrem-attributed loss is 0.223313 GeV (11.17% of 2 GeV). The mean retained
fraction of eBrem-containing intervals is 0.914017 over the full thickness
range and 0.922355 for `t/X0 < 0.03`.

The diagnostic spectrum is most legible as fractional loss `1-z` on a
logarithmic axis, but the GSF model interface remains `(weight, mean_z,
variance_z)` with `z=p_after/p_before`. The change of plotted variable is only
a linear reflection and does not change the model semantics. The spectrum has
a dominant near-zero-loss peak and a long eBrem tail. The last two thickness
bins have only 2,667 and 577 transitions, so their unsmoothed component
parameters must not be used as discontinuous lookup values without inspection.
For this phase, analyze this new transition dataset on its own; comparison to
the earlier global eBrem study was explicitly stopped by the user.

## Constrained interpolation artifact

The inspected table is now converted into a reproducible execution artifact by
`Reconstruction/RecGsfTracking/scripts/fit_cepc_step_conditioned_model.py`.
The chosen representation is an eight-knot, five-component constrained
interpolation rather than a high-order global polynomial:

- an explicit `t/X0=0` anchor has unit negligible-loss weight and zero tail
  probability;
- interpolation from zero to the first knot is linear in physical parameters;
- subsequent interpolation is linear in `log(t/X0)`;
- weights use additive-log-ratio coordinates and are renormalized;
- retained-fraction means use logit coordinates;
- positive conditional variances use logarithmic coordinates;
- values above the last fitted knot are held constant for this limited
  execution artifact.

The stable outputs are under
`Reconstruction/RecGsfTracking/data/CEPC2GeV85StepConditioned/`:

- `cepc2gev85_step_conditioned.json` — source-integration artifact and knots;
- `cepc2gev85_step_conditioned_dense.csv` — 241-point evaluated diagnostic;
- `cepc2gev85_step_conditioned.png` — weights, means, and conditional widths.

Validation over 2,001 thickness values from zero through 0.03 found finite
parameters, nonnegative weights, means in `(0,1]`, positive variances, and a
maximum weight-normalization error of `4.44e-16`. The summed non-negligible
weight at `t/X0=1e-12` is `6.98e-11`, demonstrating the explicit vanishing-tail
limit. The curves pass through the extracted knots and retain their strong
non-monotonic changes across thickness bins; this is intentionally visible
rather than hidden by an oscillatory or unjustified global polynomial. The
artifact is suitable for the requested same-sample execution proof but is not
an independently validated or general CEPC physics model.

## BetheHeitlerSplitter integration

`BetheHeitlerSplitter::{Model,modelFromName,modelName,split}` now supports
`CEPC2GeV85StepConditioned`. The C++ implementation embeds the artifact's eight
knots and reproduces its zero-to-first-knot physical interpolation, subsequent
log-thickness transformed interpolation, normalization, and constant upper
extrapolation. The explicit name and comments retain the 2 GeV pT, 85-degree
scope.

`RecGsfTracking` built and installed successfully. A comprehensive forward
event-11 run using `DD4hepBetweenSurfaces`, `ElossOn=false`, KL reduction, and
the new model is reproducible with
`options/run_gsf_cepc2gev85_step_conditioned_event11.py`; its log is
`/tmp/gsf-cepc2gev85-conditioned-event11.log`. The first split at
`t/X0=0.00502495` produced artifact-matching weights
`(0.031229, 0.900903, 0.035445, 0.018643, 0.013781)` and retained fractions
`(0.999918, 0.999534, 0.976293, 0.898761, 0.540722)`.

The event completed with 234/234 hits, 1047 accepted component updates, zero
recovery, zero rejection, four splits, three reductions, a peak of 25
components, and 11 final components. The selected forward IP result remained
at pT 1.7934 GeV, essentially LCIO rather than 2.0004 GeV truth. This proves
finite execution and correct artifact wiring only; it does not demonstrate
hard-loss recovery. The separate `BHSplitThreshold=1e-4` policy remains
unchanged and suppresses model evaluation below that thickness in ordinary
configuration despite the artifact's explicit zero-material limit.

## Invalidated event-11 cross-file diagnosis

**Do not use the conclusions in this subsection.** A direct PODIO comparison
on 2026-07-12 showed that the truth comparison and refit run were not the same
simulated detector event. The preserved truth belongs to event index 11 of
`/tmp/gsf-match-tracks.root`, whereas the new-model refit used event index 11
of `tuples/trk-e--2.0-85-1.root`. Their four generated MC particles are exactly
equal, which made the nominal event appear consistent, but their 234 tracker
hits and fitted `CompleteTracks` states differ starting at hit 0. For example,
hit 0 is `(-10.69594,2.90166,0.96894) mm` in the matched file and
`(-10.69653,2.89947,0.97174) mm` in the refit input.

The numerical observations below describe two different detector realizations
and therefore cannot establish truth-to-child support, split timing, posterior
selection, or lineage survival for one event. They are retained only as an
explicit audit trail of the provenance error. A valid diagnosis must rerun
`CEPC2GeV85StepConditioned` on `/tmp/gsf-match-tracks.root` and use its paired
`/tmp/gsf-match-material-interval-comparison-event11.csv` truth.

The authoritative matched source is the preserved comparison
`/tmp/gsf-match-material-interval-comparison-event11.csv`, not the later
independent ten-seed production's nominal event ID. It identifies two physical
eBrem intervals in matched event 11.

The hard eBrem is transition 6, owned by the ITK layer-0 to layer-1 interval
from `r=235.153` to `344.395 mm`. Geant4 gives `t/X0=0.00719995`,
`p=2.006653 -> 1.797001 GeV`, and true `z=0.895522`. DD4hep component paths
after hit 6 are `0.0072029-0.0072094 X0`, confirming correct material and
ownership. At this thickness the conditioned hypotheses are:

| component | prior weight | mean z | sigma(z) | truth pull |
|---:|---:|---:|---:|---:|
| 0 | 0.000916 | 0.999946 | 0.000009 | -11608 |
| 1 | 0.938610 | 0.999527 | 0.001038 | -100.2 |
| 2 | 0.031622 | 0.976708 | 0.010902 | -7.45 |
| 3 | 0.016757 | 0.898370 | 0.040718 | -0.070 |
| 4 | 0.012094 | 0.539098 | 0.213125 | +1.67 |

Component 3 covers the true loss extremely well, but it is not created on the
owning transition. After the hit-6 measurement there are exactly 12
components, while the split gate requires `components.size() < MaxComponents`.
The material is computed but splitting is skipped. The hit-6 likelihood has
selected a pre-loss state near pT `2.0132 GeV` with normalized weight
`0.998671`. Cutoff then reduces 12 components to 5, and the next split occurs
after hit 7 at `t/X0 ~= 0.006862`, one surface late.

The delayed component-3 child starts at pT `1.77893 GeV` and weight `0.016640`.
It survives immediate KL reduction, remains after hit 8, and becomes dominant
by hit 9 with normalized weight `0.995177` and pT `1.78788 GeV`. It remains
dominant through the TPC (`0.996977` before hit 150). The final selected
KL-merged history contains this delayed component-3 lineage, but the published
forward IP state remains at LCIO pT because the loss was applied at the wrong
transition and ordinary forward output preserves the filtered inner history.

The second eBrem is transition 150, owned by TPC row 140 to 141 from
`r=1337.5` to `1342.5 mm`. Geant4 gives `t/X0=0.000045`,
`p=1.795729 -> 1.785157 GeV`, and `z=0.994113`. The closest available support
is the moderate component at mean `0.978562`, sigma `0.004340`, weight
`0.000110`, or `3.58 sigma` from truth; the small component is `8.89 sigma`
away. No children are created because the DD4hep path `4.24996e-05` is below
`BHSplitThreshold=1e-4`. The dominant branch remains essentially unchanged:
weight `0.996977 -> 0.996978` and pT `1.78608 -> 1.78600 GeV` at hit 150.

The invalidated localized conclusion was that support is adequate for the hard ITK loss,
but two workflow gates prevent correctly timed hypotheses: the hard loss is
blocked by the pre-split component-count condition, and the thin loss is
blocked by the fixed thickness threshold. Likelihood and reduction are not the
primary hard-transition failure because the correct child is never created
there; when created one surface late, it is strongly selected and survives.

### Validated paired rerun

The new model was then rerun on event index 11 of the correct
`/tmp/gsf-match-tracks.root`; the log is
`/tmp/gsf-cepc2gev85-conditioned-matched-event11.log`. Direct PODIO comparison
establishes the provenance: this file has 12 events, the selected event has the
same generated MC record used by the truth production, and its hit-6/7 cell IDs
and positions match the preserved reconstruction-transition audit. The paired
run again computes `0.0072037-0.0072095 X0` for the owning hard interval after
hit 6. It has 12 components there, does not split, cuts 12 to 4, and splits
after hit 7 instead. At transition 150 it computes `4.24996e-05 X0` and does
not split because of the `1e-4` threshold. Thus the two gate observations are
confirmed by the paired run; detailed weights from the earlier mismatched run
remain invalid and must not be reused. The paired run finishes with 234/234
hits, 929/0/0 accepted/recovered/rejected updates, and pT `1.7938 GeV` versus
truth `2.0004 GeV`.

## State-by-state diagnostic capability

Geant4 now provides the physical location of every eBrem step and identifies
the two measurement surfaces that own it. The GSF intentionally lumps process
convolution over that surface-to-surface interval; no GSF state is expected at
the internal physical step unless a material surface is introduced there.

For each hard-loss transition, compare:

1. Geant4 and DD4hep interval `t/X0`, location, and exactly-once ownership.
2. True retained fraction `z` with every BH child mean and conditional width.
3. Child weights immediately after splitting and after the next measurement's
   exact innovation likelihood.
4. Weight evolution through later measurements and any low-weight cutoff or
   current-surface KL reduction.
5. The retained lineage through reverse filtering/smoothing to the IP.

The diagnosis is then localized:

- no child covers true `z`: mixture support/parameterization failure;
- a suitable child is created but the next update suppresses it: prediction or
  likelihood/state-consistency problem;
- a supported child disappears at reduction: retention-policy problem;
- the lineage survives filtering but gives the wrong IP state: reverse
  transport/smoothing problem.

Event 11 remains the first detailed hard-transition case, followed by complete
verbose runs of events 11, 16, and 17.

## Remaining work

1. Change split/reduction ordering so an eligible owned transition is not
   skipped merely because the pre-split mixture is at `MaxComponents`; validate
   the hard transition before considering any threshold change.
2. Audit `BHSplitThreshold` against the fitted low-thickness tail and decide
   how to represent sub-threshold eBrem without uncontrolled repeated splitting.
3. Require finite 234-hit output without covariance failure or measurement
   rejection. Any momentum improvement is useful evidence but is not an
   independent performance validation.

## Independent paired 10-event-file diagnosis

The first ten newly produced electron seed trios each contain 10 events. A
combined extraction from their material-step files contains 23,021 owned
transitions in 100 events, including 114 eBrem transitions in 58 events. For a
moderate diagnostic rather than an extreme tail, seed 1 event 3 was selected.
It has one 233-hit `CompleteTrack` and an eBrem transition from ITK layer 0 to
layer 1 (hit 6 to hit 7): true `z=0.91027597`, loss `0.180094 GeV`, and
Geant4 `t/X0=0.00720654`.

The event pairing is exact rather than inferred from seed and event number.
The `sim` and `trk` copies have identical signatures for all four MC particles
and for the 6 VXD, 3 ITK-barrel, and 223 TPC simulated hits, including cell ID,
position, deposited energy, and time. The generated electron record is also
identical.

The conditioned forward GSF run is preserved in
`/tmp/gsf-conditioned-seed1-event3.log`. DD4hep assigns
`0.00720772-0.00721433 X0` to the owning hit-6 transition, agreeing with
Geant4. Nevertheless, the mixture begins hit 6 with 12 components and creates
no children on the outgoing transition. The hit-6 measurement instead
reweights previously created histories: two old low-retained-fraction
lineages acquire weights `0.510525` and `0.488446`, with pT `2.00543` and
`2.00875 GeV`; cutoff reduces 12 components to 8. The next split occurs after
hit 7 on the non-eBrem layer-1-to-layer-2 interval (`~0.014461 X0`), confirming
the same one-surface-late capacity-gate behavior seen in the matched event-11
rerun.

The run finishes successfully with 233/233 hits, 2,757 accepted updates, zero
recovery, two rejected prediction attempts that did not reject measurements,
four splits, three reductions, peak 50 and final 12 components. The selected
branch has weight `0.967847`; published forward IP pT is `1.7980 GeV`, equal
to LCIO within the printed precision and below `2.0004 GeV` truth. This is a
paired execution and workflow diagnostic, not physics validation.

### Split-before-budget correction

The forward workflow was changed so every eligible owned transition is split
before enforcing `MaxComponents`; a just-created over-budget mixture is
allowed to reduce on the same surface regardless of the ordinary post-split
age delay. The change is confined to `RecGsfTracking/src/GsfAlgorithm.cpp`.
It builds and installs successfully.

The identical seed-1 event-3 rerun is preserved in
`/tmp/gsf-conditioned-split-first-seed1-event3.log`. On the true hit-6 eBrem
interval, 12 parents now produce 60 children. Low-weight cutoff leaves 21 and
same-surface KL reduction returns the mixture to 12. For the dominant upstream
parent at pT `2.00728 GeV`, the five continuation states have pT approximately
`2.0073`, `2.0064`, `1.9606`, `1.8034`, and `1.0822 GeV`. The component with
retained fraction near `0.8990`, which covers true `z=0.910276`, survives KL
reduction with weight `0.0165144`.

At the next measurement, that component has the smallest relevant innovation
penalty (`dchi2=4.82`), updates from pT `1.80346` to `1.76697 GeV`, and becomes
dominant with normalized weight `0.917160`. Thus creation timing, truth support,
same-surface retention, and next-measurement selection all work in this paired
case. The event finishes with 233/233 hits, eight splits, seven reductions,
peak 60 and final 10 components. The published forward IP pT nevertheless
remains `1.7980 GeV`, because the forward output retains the inner filtered
history rather than transporting the selected downstream loss information
back to the IP. This is now the localized limitation; the result is not a
physics-performance validation.

### Retained-lineage RTS smoother rerun

The same paired event was rerun with `RetainedLineageSmoothing=True`,
`ReverseFiltering=False`, `ReductionMode="TopN"`, and material IP
extrapolation disabled. The dedicated option is
`options/run_gsf_cepc2gev85_step_conditioned_smoother.py`; the full log is
`/tmp/gsf-conditioned-smoother-seed1-event3.log`.

All 12 final retained lineages have 232 smoothing steps and complete
successfully. A final component containing the correctly timed hit-6 `g3`
loss remains present with weight `0.107465`, so failure is not simply absence
of that lineage. Nevertheless, every smoothed inner pT stays near the common
filtered value `1.7980208 GeV`; representative results are `1.79808714 GeV`
for the selected component and `1.79809333 GeV` for the correctly timed-loss
component. The published IP pT is consequently `1.7981 GeV`, still LCIO-like
and far from `2.0004 GeV` truth.

TopN retention also changes the eventual winner relative to the immediate
post-hit-7 posterior. The final best component has weight `0.342735` and its
history uses the low-loss `g1` child on hit 6; correctly timed `g3` histories
survive among other endpoints but are not the final maximum-weight branch.
The RTS transition dump shows finite covariance closure and finite gains, but
the process uncertainties strongly limit backward momentum information. This
run demonstrates operational smoothing, not IP recovery.

### ACTS-strategy audit and first backward-GSF attempt

ACTS source inspection corrected the working interpretation. ACTS explicitly
does not perform dedicated component RTS smoothing. It initializes a second
multi-component pass from the complete forward mixture at the last measurement,
propagates in the opposite direction while revisiting measurements and material,
and moment-matches the final backward mixture at the reference surface. The
possible reuse of forward measurement information is deliberately set aside
for this execution phase, as authorized by the user.

The local reverse workflow was moved toward those mechanics: reverse material
uses direction-aware DD4hep integration between the current outer and target
inner measurement surfaces; splitting occurs before the reverse component
budget; interval material is applied before the target measurement; and the IP
output moment-matches the complete reverse mixture instead of publishing only
its best branch. A dedicated card is
`options/run_gsf_cepc2gev85_step_conditioned_reverse.py`.

The first direction-unaware run performed zero reverse splits and is invalid as
an ACTS-strategy test. After adding inward intersection direction, reverse
material and splitting become active. The reverse measurement states around
the hard interval are finite and include hit-6 pT candidates from approximately
`1.79` to `1.91 GeV`. However, the corrected pre-measurement-order run terminates
immediately after hit 6 while entering the following reverse material
split/reduction, before producing an output track. The current blocker is
therefore reverse BH convolution or reduction stability after the hard
interval. No physics conclusion should be drawn from the incomplete run
`/tmp/gsf-conditioned-acts-reverse-v3-seed1-event3.log`.

#### Termination repair and completed backward mixture

The termination was caused by diagnostic ancestry strings rather than reverse
filter mathematics. KL merging recursively concatenated complete histories;
reverse splitting then cloned the already enormous strings, producing
exponential memory growth. Diagnostic histories are now bounded to 4096
characters while preserving their beginning and end. This changes no state,
weight, covariance, likelihood, split, or reduction calculation.

After rebuilding, the identical ACTS-style run completes in
`/tmp/gsf-conditioned-acts-reverse-v4-seed1-event3.log`. It retains 233/233
hits with 2,457 accepted and zero rejected reverse component updates, seven
reverse splits, seven reverse reductions, and 11 final reverse components. The
moment-matched reverse IP mixture has pT `1.99286 GeV`, p `2.0005 GeV`, phi
`2.66431`, d0 `0.003826 mm`, and z0 `-0.002020 mm`, compared with truth pT
`2.0004 GeV` and LCIO pT `1.7980 GeV`. The highest reverse endpoint weight is
`0.956086`, but the published parameters and covariance are the complete
mixture moment match. This is strong paired execution evidence for the ACTS
strategy, not yet statistical validation; the forward-information reuse caveat
remains intentionally set aside for this phase.

### Three additional paired eBrem events

Three exact `sim`/`trk` pairs with one 233-hit track each were run through the
completed ACTS-style backward mixture. Pairing was verified from MC and
VXD/ITK/TPC hit signatures, not filenames alone.

| seed/event | principal true loss | LCIO pT | reverse-mixture pT | truth pT | result |
|---|---:|---:|---:|---:|---|
| 1/5 | `z=0.95929`, 0.0817 GeV at transition 6 | 1.9234 | 2.0302 | 2.0004 | recovers, overshoots 0.0298 GeV |
| 2/7 | `z=0.92603`, 0.1485 GeV at transition 4 | 1.8521 | 1.8538 | 2.0004 | does not recover |
| 7/9 | `z=0.86623`, 0.2684 GeV at transition 8, plus 0.1147 GeV at 231 | 1.7496 | 2.0047 | 2.0004 | recovers within 0.0043 GeV |

All runs finish with 233/233 hits. Reverse accepted/rejected update totals are
2690/0, 2481/0, and 2584/3; reverse split/reduction counts are 7/7 in every
case. Logs are `/tmp/gsf-acts-reverse-s1-e5.log`,
`/tmp/gsf-acts-reverse-s2-e7.log`, and `/tmp/gsf-acts-reverse-s7-e9.log`.

The seed-2 event-7 failure is localized. The reverse split creates candidates
near pT `2.06` and `2.08 GeV` on the correct transition, so support exists.
At hit 4 their innovation penalties (`dchi2~=0.88-1.32`) are nearly
indistinguishable from the no-loss candidates (`~0.82-1.25`). With little
inner lever arm after this early loss, the BH prior dominates and the no-loss
endpoint finishes at weight `0.977291`. The failure is therefore early-loss
observability/posterior weighting, not absent support or wrong interval
material. These four paired events are execution diagnostics, not a performance
efficiency measurement.

### Twenty additional long-track eBrem events

The 20 strongest remaining eBrem cases with at least 232 `CompleteTrack` hits
were selected after excluding the four previously diagnosed events. They span
principal retained fractions `0.555-0.991`, with 16 inner (`transition < 20`)
and four late transitions. All ACTS-style backward-mixture jobs complete and
retain 232-235 hits. This is a deliberately selected same-production sample,
not an unbiased efficiency or held-out validation sample.

Mean absolute pT error changes from `77.3` to `32.8 MeV`, median from `36.7`
to `15.3 MeV`, and 19/20 events improve. Reverse pT is within 20 MeV for 13/20
and within 50 MeV for 17/20. For the 16 inner cases, mean absolute error changes
from `79.1` to `27.1 MeV` and all 16 improve. The dominant outlier is seed/event
9/9: a `1.022 GeV` summed loss dominated by `z=0.555` at penultimate transition
231 leaves insufficient downstream lever arm, and reverse pT remains `190.8
MeV` low. Seed/event 3/3 is the sole formal non-improvement (`1.6` to `2.8
MeV` absolute error), with both estimates already close to truth.

Across the sample, maximum absolute reverse-minus-truth differences are about
`0.00207 rad` in phi and `0.00095` in eta; absolute d0 and z0 remain below
`0.0272 mm` and `0.0160 mm`. Reverse-output chi2/ndf ranges from `0.902` to
`1.203`. The EDM outputs and flat tuples use `/tmp/gsf-batch20-s*-e*.root` and
`/tmp/gsf-flat-batch20-s*-e*.root`. These selected recovery rates and chi2
values do not establish calibration or production validity.
