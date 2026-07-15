# CMSSW and ACTS GSF smoothing comparison: pre-disconnect handoff

Date: 2026-07-15

## Why this record exists

After the KL reduction-aware CEPC smoother remained essentially LCIO-like on
the durable 50-event overshoot sample, the discussion turned to how established
GSF packages perform their backward stage. This record preserves the findings,
the corrections made during the discussion, and the exact questions that must
be answered by a source-level audit after reconnecting.

No implementation change was made during this comparison.

## Terminology correction

There is not one unique algorithm that should be called *the* standard GSF
smoother. At least two established smoothing formulations are relevant:

1. an RTS-like backward recursion extended to a retained Gaussian mixture;
2. a two-filter construction that combines a forward filtering distribution
   with an independent backward likelihood/information mixture.

The optional CEPC smoother is a legitimate KL reduction-aware Gaussian-sum
backward-graph/RTS-style implementation. Its failure to move most selected pT
values does not make it nonstandard or mechanically incorrect. It means that
backward refinement of the retained forward hypothesis graph usually does not
create or select the radiative momentum mode that the reverse refit selects.

The proposed two-filter smoother is also an established statistical method,
not an ad hoc new invention. It is an alternative architecture whose central
requirement is independence of the backward message from the forward posterior
with which it is multiplied. It is not simply further splitting a BH mode.

The copy-friendly conceptual explanation is in
`codex_copies/two_filter_gaussian_sum_smoother.md`.

## Preliminary CMSSW finding

CMSSW has dedicated `GsfTrajectoryFitter`, `GsfTrajectorySmoother`,
`GsfMultiStateUpdator`, electron material-effects/BH machinery, and a
configurable close-component merger. The public CMS GSF guide says that the
fitter and smoother follow the corresponding KF architecture with an added
merging stage, and that the multi-component state is carried in TSOS objects.
The common configuration couples a forward GSF fitter to a backward-material
GSF trajectory smoother.

The working interpretation from the available documentation and historical
configuration is that CMSSW's smoother propagates a backward Gaussian mixture
and combines it with stored forward information at each measurement, making it
closer to a two-filter-style GSF smoother than to the CEPC retained-lineage
backward graph.

This interpretation is **preliminary**. The current CMSSW master implementation
of `GsfTrajectorySmoother` was not successfully retrieved and inspected line by
line during this session. Do not treat the exact combination formula, hit
ownership, component-product construction, or reduction ordering stated in the
conversation as source-verified until that audit is complete.

Primary public documentation identified:

- CMS public GSF fitter guide:
  `https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideGsfFitter`
- CMSSW repository:
  `https://github.com/cms-sw/cmssw`

## Source-confirmed ACTS finding

ACTS implements `Acts::GaussianSumFitter` with a multi-component stepper. At a
material surface, each component is convolved with a BH Gaussian-mixture
approximation. Component multiplication is controlled by KL-based merging and
a configured maximum component count. The default documented BH approximation
uses six Gaussian components with fifth-order polynomials in material
thickness, with special handling for very thin material.

ACTS performs a backward pass, but its API documentation explicitly states
that individual component states are not exported to the returned
`MultiTrajectory`; only combined mixture states are returned. It further states
that **no dedicated component smoothing as described by Frühwirth is
performed**. Therefore ACTS must not be cited as an example of full
component-pair or retained-lineage GSF smoothing.

At the architectural level ACTS currently appears closer to a forward GSF plus
reverse multi-component filtering workflow, followed by mixture reduction for
stored output, than to the optional CEPC smoother. This resemblance is not yet
proof of algorithmic equivalence.

Primary ACTS documentation identified:

- GSF overview:
  `https://acts.readthedocs.io/en/v20.2.0/core/track_fitting.html`
- `GaussianSumFitter` API note (v30.3.2):
  `https://acts.readthedocs.io/en/v30.3.2/api/namespace/namespace_acts.html`
- Current/latest API entry point:
  `https://acts.readthedocs.io/en/latest/api/api.html`
- ACTS repository:
  `https://github.com/acts-project/acts`

Because ACTS evolves, the next audit must record an exact tag or commit rather
than silently mixing documentation versions. The local `ActsAtlas` BH model is
already synchronized separately to official ACTS commit `900c9e5e`; that BH
provenance does not establish equivalence of the fit or backward workflow.

## Current comparison

| Workflow | Forward mixture | Backward stage | Dedicated component smoothing | Present output/selection concern |
|---|---|---|---|---|
| CEPC optional smoother | Yes | KL reduction-aware retained forward graph | Yes, RTS/backward-graph style | Usually remains LCIO-like; cannot reproduce reverse recovery |
| CEPC reverse filter | Yes | Second inward multi-component refit | No | Recovers many hard losses but creates positive tails; seed may reuse forward information |
| CMSSW | Yes | Dedicated GSF trajectory smoother with backward material propagation | Probably two-filter-like; exact current source audit required | Determine mixture combination and final mode/mean semantics |
| ACTS | Yes | Always performs a backward multi-component pass | Explicitly no dedicated component smoothing | Returned trajectory states contain combined mixture information, not exported components |

## Required detailed audit after reconnecting

Inspect exact, pinned source revisions rather than relying on overview pages.

### CMSSW

1. Pin the CMSSW branch/tag/commit and inspect `GsfTrajectorySmoother` plus its
   state combiner, propagator, updater, and merger dependencies.
2. Determine the backward boundary state and covariance inflation/rescaling.
3. Identify whether the stored forward **predicted** or **filtered** state is
   used at each surface.
4. Establish exactly where the current hit enters so it is counted once.
5. Write down the Gaussian-mixture product weights and whether all
   forward/backward component pairs are formed.
6. Establish where material effects are applied in both directions and how
   electron energy loss is interpreted during backward propagation.
7. Record reduction timing, distance measure, maximum component count, and any
   protection or pruning behavior.
8. Trace how the innermost mixture becomes a `GsfTrack`: weighted mean, 1D
   marginal mode, highest-weight component, or another estimator.

### ACTS

1. Pin the current ACTS tag/commit and inspect `GaussianSumFitter.hpp`, its GSF
   actor, multi-stepper, component reduction utilities, and BH updater.
2. Determine how the backward pass is initialized and whether it is seeded by
   a forward posterior.
3. Determine whether backward processing repeats measurement updates, uses
   stored forward states, or replaces their output slots.
4. Audit hit/material ordering and direction-dependent energy-loss treatment.
5. Record the exact KL reduction metric, reduction point, weight cutoff, and
   maximum-component semantics.
6. Determine precisely how the mixture is collapsed for each returned track
   state and reference-surface parameters.
7. Confirm whether newer ACTS revisions still carry the documented limitation
   of no dedicated component smoothing.

### CEPC mapping and tests

1. Map both packages' state definitions and surface ownership onto
   `RecGsfTracking` without changing code first.
2. Compare the backward initialization, BH direction, measurement ownership,
   reduction ordering, and final estimator against the current reverse filter.
3. Identify the smallest faithful CMSSW-like and ACTS-like controls. Keep them
   optional and retain the present reverse filter as a benchmark.
4. Validate any implementation first with comprehensive state dumps and the
   existing 19 overshoots/18 controls, ordinary light representatives, clean
   62/9, hard 1/3, and events 11/16/17 before population tests.
5. Do not infer superiority from architectural pedigree. Require improved
   truth residuals without sacrificing clean or hard-loss performance.

## Resume point

Resume by retrieving the current source files from pinned CMSSW and ACTS
revisions and producing a line-referenced algorithm table. Do not implement a
new smoother or modify the current optional smoother until the source-level
comparison resolves the backward seed, measurement ownership, material
direction, mixture combination, reduction, and publication semantics.

