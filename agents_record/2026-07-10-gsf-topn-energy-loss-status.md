---
name: gsf-topn-energy-loss-status
description: Multi-component update validation and why immediate TopN=1 does not recover hard electron energy loss
metadata:
  type: status
  date: 2026-07-10
---

# GSF TopN Energy-Loss Status

## What is now working

The baseline-style per-component update is operational with BH splitting and
reduction.  A `MaxComponents=9`, `ReductionTargetComponents=3`, `TopN` test on
events 10, 12, 14, and 15 completed with zero recovery shortcuts.  The selected
last-site transverse momenta were 1.999, 1.959, 1.996, and 2.001 GeV.  Some
low-weight, low-momentum children were rejected at early hits, while viable
siblings continued normally.  The former catastrophic high-momentum smoothing
failure was not reproduced.

## TopN=1 finding

A quiet run of events 10 through 19 used:

```python
gsf.MaxComponents = 2
gsf.ReductionTargetComponents = 1
gsf.ReductionMode = "TopN"
gsf.BHModel = "GlobalSim2GeV85"
```

The GSF output stayed close to LCIO, including the known hard-loss events:

| event | truth pT [GeV] | LCIO pT [GeV] | GSF pT [GeV] | LCIO chi2/ndf | GSF chi2/ndf |
|---:|---:|---:|---:|---:|---:|
| 11 | 2.000 | 1.793 | 1.793 | 855.9/462 | 500.9/460 |
| 16 | 2.000 | 1.812 | 1.812 | 1004.6/462 | 628.7/460 |
| 17 | 2.000 | 1.579 | 1.579 | 491.9/462 | 465.7/460 |

Thus the baseline-style update improves chi2 and stability but immediate
TopN=1 does not repair the generated IP momentum.

## Why energy loss is not recovered

`GlobalSim2GeV85` always returns the same five retained-fraction hypotheses:

```text
z=0.3658  weight=0.0242
z=0.6785  weight=0.0345
z=0.9750  weight=0.2027
z=0.9950  weight=0.1593
z=0.99995 weight=0.5793
```

After a split, only the immediately following hit contributes likelihood before
TopN=1 reduction.  Early/local hit likelihoods are usually too similar to
overcome the prior weights, so the `z=0.99995` near-no-loss child wins.  The
hard-loss children are deleted before a sequence of outer hits can accumulate
enough curvature evidence.  Once deleted, they cannot become favored later.

Additional limitations:

- The seed is the LCIO track, which already reflects the compromised global fit
  in hard-loss events.
- `GlobalSim2GeV85` ignores the passed step `t/X0`; it is a global tracker eBrem
  distribution applied independently at every qualifying layer.
- Splitting is represented by changing curvature on the component's last site
  before the next measurement, rather than by an explicit pre-material to
  post-material transition object.
- `MaxComponents` gates whether a split starts; it does not cap the number of
  children produced by that split.  For example, `MaxComponents=2` can still
  produce `1 -> 5 -> 1` at a split.

## Next implementation direction

Do not use `TopN=1` as a test of GSF energy-loss recovery.  The next focused
test should retain 3-5 children for several hits after a split, then reduce only
after enough measurement leverage has accumulated.  Inspect hard-loss events
11, 16, and 17 to see whether a lower-retained-fraction branch becomes favored.

After that workflow test, replace the global model with a step-`t/X0`-
conditioned BH mixture and represent material loss as a distinct transition
between pre- and post-material states.  Only then return to broad GSF-vs-LCIO
performance claims.

## Configuration cleanup made at this stage

The algorithm now has one active GSF workflow.  Removed debugging scaffolding:

- `FitterMode` and the alternate `KalTestTool` KF path
- `KFFitterTool`, `KFFitBackward`, `KFMaxChi2PerHit`, and five KF seed-error properties
- unused `KFRecoveryMode`
- `GSFInitialisationMode` and `GSFInitialisationFitHits`

The baseline early-fit modes and pure-KF path remain documented as historical
root-cause experiments.  They are no longer runtime configuration choices.
