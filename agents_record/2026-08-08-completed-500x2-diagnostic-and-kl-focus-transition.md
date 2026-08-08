# Completed 500x2 state diagnosis and transition to KL fine-tuning

Date: 2026-08-08

## Purpose

This record preserves the outgoing live focus from `AGENTS.md` and the
evidence obtained before the project focus changed to hypothesis-driven
fine-tuning of the `SymmetricKL` mixture reduction. The new focus does not
claim that KL reduction is the sole origin of either clean-track degradation
or missed light-eBrem recovery.

## Superseded live question and baseline

The outgoing question was why a minority of topology-clean no-eBrem and
light-eBrem tracks selected discrete radiative modes and developed momentum
tails, especially around surfaces 5--8, despite favorable population-level
light/hard recovery.

The baseline was and remains the five-component
`CEPC2GeV85StepConditioned` BH model with `MaxComponents=12`,
`ReductionTargetComponents=0` (therefore 12), `SymmetricKL`, aggregate-weight
final selection, identity-lineage protection, `ComponentWeightCutoff=1e-4`,
and the independent reverse refit. The complete effective configuration is
explicit in `DumpGsfTrks/gsf.py.bk` and preserved by tag
`gsf-memory-leak-fixed-2026-08-08`.

The previous immediate action order was:

1. Freeze deterministic 500-event topology-clean no-eBrem and 500-event
   topology-clean light-eBrem cohorts from the fresh 4,800-event catalogue,
   excluding seeds 32/36 and all secondary-tracker-activity events.
2. Capture compact surface-by-surface forward and reverse component records,
   including innovation quantities, process histories, weights, reduction
   ancestry, and final selection scores.
3. Diagnose clean LCIO-preserved tracks against GSF degradation and positive
   tails.
4. Diagnose light-eBrem good/partial/missed recovery, degradation, and
   overshoot with the same schema.
5. Form a candidate only if both cohorts pointed to the same implementable
   mechanism, then apply focused-event, hard-loss 11/16/17, and held-out
   population gates.

The pre-execution form of that action order is also preserved in
`agents_record/2026-08-08-pre-500x2-state-diagnostic-action-order.md`.

## Population evidence carried into that diagnosis

- The fresh broad-electron transfer sample had 4,800 usable events from 48
  completed 100-event GSF jobs. Seeds 32 and 36 failed during podio
  association cleanup and were excluded in full. The topology-clean
  populations were 1,534 no-eBrem, 2,182 light-eBrem, and 539 hard-eBrem.
- GSF changed global +-1% containment from 71.5% to 75.0%, driven by
  light/hard recovery, while losing 39 clean no-eBrem events from the core and
  producing 230 clean-topology residuals above +1% and 63 above +5%.
- The cloned `TKalTrackState` ownership leak was fixed. Peak RSS was 1.66 GB
  for five events and 1.74 GB for 20 events, and successful production jobs
  completed 100 events. The seed-32/36 failures were podio cleanup failures,
  not memory exhaustion.
- The expanded survey had 4,990 matched events from 499 usable seed files;
  seed 464 lacked its `gsf_tuple`. Before topology exclusion it contained
  2,045 no-eBrem, 2,148 light-eBrem, and 797 hard-eBrem events; the active
  single-track populations were 2,032, 2,132, and 694.
- Recovery was strongly layer dependent: transitions 0--4 were mostly
  information limited, 5--6 formed the boundary, and 7--11 showed strong
  central recovery.
- The earlier 19-overshoot/18-control and transition-7--8 studies showed
  heterogeneous prior/likelihood decisions and a recurring coupled
  surface/mode mismatch. KL reduction was not a recurring local cause.
- Global process-prior reweighting, rank publication, dominant-unmerged
  lineage selection, g3 splitting, bounded noisy-OR surface scoring, a simple
  added 5--8% truth-surface component, the six-component conditioned model,
  and promotion of Runnalls ranking all failed their gates.
- A capacity of 24 could preserve both real recovery and false radiative
  modes. It did not solve selection and remained only a comparison control;
  the user retained 12 as the baseline.

## Completed state-diagnostic cohorts

The first campaign completed 500 topology-clean no-eBrem and 500
topology-clean light-eBrem state-by-state diagnoses. It was deliberately
outcome enriched rather than a population-rate estimate.

In the first no-eBrem cohort, 499 tracks began with LCIO inside +-1%; 489 were
preserved and ten were degraded by GSF. All ten selected strong g2/g4 modes at
hits 6--9. Eight were reverse-only takeovers. Two were already locked into a
false forward radiative seed. The exact identity branch would have restored
the +-1% core for all ten. Dominance crossed before reduction in eight; two
needed reduction amplification.

In the first light-eBrem cohort the outcome counts were: 100 good recovery, 51
partial recovery, seven near recovery, 43 overshoot, 80 missed recovery, 26
degradation, 57 truth-like-LCIO degradation, nine other truth-like cases, and
127 preserved. Identity would repair 50/57 truth-like degradations but was
better in only 3/100 good recoveries and inside +-1% in only eight. Global
identity publication would therefore destroy genuine recovery.

A second zero-overlap holdout contained 250 no-eBrem and 250 light-eBrem
events. The catalogue reconstruction exactly reproduced the recorded
topology-clean counts of 1,534/2,182/539. All 48 jobs and all 500 requested
events completed.

The held-out no-eBrem cohort deliberately included the 25 identified
LCIO-core-to-GSF-tail cases plus controls:

- All 25 selected strong radiative modes at surfaces 5--9: 21 selected g2 and
  four selected g4.
- Twenty were reverse-only takeovers and five inherited radiative forward
  support.
- For the reverse-only cases the median radiative/identity weight ratio at
  branch birth was 0.037, with range 0.0015--0.348. The branch overtook
  identity after one to five inward measurement updates.
- The median accumulated exact measurement log-likelihood gain was 3.89; the
  strongest individual evidence most often appeared at surface 5.
- The no-radiation GSF branch was better in 24/25 but inside +-1% in only
  23/25. The original LCIO state was inside +-1% in all 25, proving that an
  independent baseline anchor is not equivalent to protecting the GSF
  identity lineage.
- Four of 25 first crossed above identity immediately after reduction; the
  other 21 crossed through forward prior or measurement likelihood before or
  without the decisive merge. KL can amplify a failure but is not its sole
  origin.
- Among 223 preserved controls, 212 selected reverse identity, ten selected
  weak g1 branches, and one deterministically failed reverse-IP publication
  and used the forward fallback.

The held-out light-eBrem cohort contained all 52 remaining good recoveries,
100 missed recoveries, and 98 preserved tracks:

- 50/52 good recoveries selected a radiative reverse branch. Thirty had
  radiative forward support and twenty were reverse-only. Identity was better
  in only 1/52 and inside +-1% in only 4/52.
- 92/100 missed recoveries selected identity; the other eight selected only
  weak g1 modes. The identity state was inside +-1% in none of them.
- 93/98 preserved tracks selected identity and five selected weak g1 modes;
  the identity state was inside +-1% for all 98.
- For the twenty reverse-only genuine recoveries the median branch-birth odds
  were 0.041 and the median accumulated measurement log-likelihood gain was
  3.41. These overlap the false reverse-only values of 0.037 and 3.89. Both
  cohorts most often received their strongest evidence near surface 5.

One held-out event, seed 1/event 65, reproducibly differed from its stored
tuple under the current same-code rerun but remained a clean degradation under
both results. Seed 43/event 87 reproducibly failed to publish a reverse IP
state and used the forward fallback. The other 499 reruns matched stored pT to
the verbose output precision.

## Interpretation at focus transition

The completed diagnosis rejected a simple mode, layer, evidence-magnitude,
identity-publication, forward-confirmation, capacity, or one-hit threshold.
False and genuine modes overlap too strongly. Requiring forward radiative
confirmation would remove many false reverse-only modes but also remove 20/52
held-out genuine recoveries.

The remaining reduction-specific hypothesis is narrower: local moment merging
may pool weight and state information from distinct process-surface/mode
histories into a Gaussian child that no real trajectory followed. Subsequent
nonlinear transport and measurement updates then treat that centroid as a
physical branch, while `AggregateWeight` publishes its pooled normalized
weight. This could amplify ambiguous false modes even though measurement
likelihood created them initially. The hypothesis must be evaluated by merge
causality, not by another broad option sweep.

The rare light-eBrem degradation/overshoot classes in the fresh 4,800-event
batch were exhausted by the first enriched cohort. A genuinely independent
final light-tail validation still requires new simulation or another held-out
dataset.
