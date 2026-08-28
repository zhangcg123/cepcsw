# Max-10 and 1e-4 posterior-cutoff default promotion

Date: 2026-08-29

## Decision

The compiled `RecGsfTracking` defaults, active reverse template, maintained
`DumpGsfTrks/gsf.py.bk`, and live documentation now use:

```python
MaxComponents = 10
ComponentWeightCutoff = 1.0e-4
```

`MaxComponents=10` is the posterior-reduction trigger and default reduction
target when `ReductionTargetComponents=0`; it is not an instantaneous limit on
the children evaluated at the next measurement. `ComponentWeightCutoff=1e-4`
acts on normalized measurement-posterior weights before component-count
reduction and is independent of the material `BHSplitThreshold=1e-4` despite
the equal numerical values.

## Superseded baseline

Before this promotion, the compiled and active-template defaults were
`MaxComponents=12` and `ComponentWeightCutoff=5e-3`. Historical records and
stored tuples using that baseline remain valid provenance and are not rewritten.
Explicit legacy or comparison cards that assign 12 or 24 continue to override
the new component default and must be labelled accordingly. In particular,
the legacy `run_gsf_e*.py`, `run_gsf_test*.py`, and focused conditioned-event-11
cards explicitly keep 12 while inheriting the newly compiled `1e-4` cutoff;
they are therefore hybrid controls, not reproductions of the old 12/`5e-3`
baseline. Freezing them to the old pair would require a separately authorized
card change.

The superseded default decisions and their original evidence remain unchanged
in `2026-07-16-maxcomponents-12-default-restoration.md` and
`2026-08-26-component-weight-cutoff-5e-3-default-promotion.md`.

The new values also govern the passive interior `B_smoothed` diagnostic. Ten
is its reduction target when the configured target is zero, while `1e-4` cuts
normalized forward/backward Gaussian-product overlap weights. Consequently,
lineage node fates and counts can change even though the smoothed product does
not steer endpoint publication. The separate `RecGsfGlobalLossRefitter` does
not consume either property.

This is a steering/default decision, not evidence that the new combination is
physics-validated. It requires the standard focused component gate and a
same-code topology-clear population validation, including clean-track safety,
no/light/hard-loss categories, tails, early transitions, and the separately
reported secondary-activity control.

## Completed mechanical gate

A dedicated no-edit option audit counted 40 `RecGsfTracking` properties,
confirmed that the maintained card explicitly steers 39 and deliberately
inherits only `RecordTruthMaterialIntervals`, identified the passive-smoothed
semantics above, and found no unit/CTest assertion of these defaults.

The EL9/LCG 105 build and install completed for `RecGsfTracking` and
`RecGsfFlatTuple`. Both the maintained card and active reverse template passed
Python syntax compilation. The installed generated configurable reports:

```text
MaxComponents:          10
ComponentWeightCutoff:  0.0001
```

A same-code fresh-inward run selected events 11, 16, and 17 without explicit
max/cutoff overrides. Runtime configuration reported `maxComponents=10`; all
three events fitted and the job finalized. Event 16 retained its previously
known FullMixtureMode status `-1` fallback to BestBranch.

The comprehensive event-11 trace showed forward and reverse posterior cutoff
lines with `threshold=0.0001`, component reductions with `max=10 target=10`,
and passive interior smoothed mixtures from hit 232 through hit 1 with at most
10 retained components. The fresh inward initializer reported standard
`Var(omega)=0.0001`. The job fitted and finalized successfully. These are
mechanical steering, execution, and lineage gates only; population performance
remains unvalidated.
