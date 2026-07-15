# CMSSW and ACTS GSF backward-stage source validation

Date: 2026-07-15

## Scope and pinned revisions

This audit resolves the preliminary conclusions in
`2026-07-15-cmssw-acts-gsf-smoothing-comparison-handoff.md`. No CEPCSW source
or configuration was changed.

The inspected upstream revisions were:

- CMSSW `master` commit `d58a3bbcbae2d3a107f43ab08fe06599f68629e5`;
- ACTS `main` commit `40c9a031ca5ba15010c1e30329078d394b4bd3b4`.

The audit used shallow sparse checkouts in `/tmp/cmssw-gsf-audit` and
`/tmp/acts-gsf-audit`. Line numbers below refer to those exact revisions.

## Corrected CMSSW conclusion

CMSSW does run a dedicated backward multi-component GSF, but the preliminary
description "closer to a two-filter-style GSF smoother" needs an important
qualification: it does **not** form and retain all forward/backward Gaussian
component products at each surface.

`GsfTrajectorySmoother.cc` establishes the following algorithm:

1. The backward seed is the forward-predicted mixture at the last measurement,
   with each component covariance rescaled by `ErrorRescaling` (lines 69--70).
   The production default is 100 (`GsfTrajectorySmootherESProducer.cc`, lines
   81--90).
2. The last hit is applied to that inflated seed (smoother lines 78--103).
3. At each earlier hit, the current backward filtered mixture is propagated in
   the opposite direction and merged before the update (lines 121--140). The
   production constructor uses backward material propagation and fixes
   `materialBeforeUpdate=true` (ESProducer lines 50--78).
4. The backward **prediction**, which excludes the current hit, is combined
   with the stored forward **updated** state, which includes the current hit
   (smoother line 160). Thus the current measurement enters the smoothed state
   once. The separately computed forward-prediction/backward-prediction
   combination at line 150 is used only for the hit chi-square at line 172.
5. The backward prediction is then updated with the hit for continuation
   inward (line 138), and that backward filtered mixture is merged after the
   smoothed state has been stored (lines 208--209).

The decisive detail is `TrajectoryStateCombiner.cc`, lines 6--32. Although its
inputs can be multi-component TSOS objects, it reads only each TSOS's exposed
5D local parameters and covariance. `BasicMultiTrajectoryState.cc`, lines
47--101, shows that these exposed quantities are already the moment-matched
mean and covariance of the mixture. The combiner applies the ordinary
information combination to those two moments and returns a single TSOS. It
does not enumerate forward/backward component pairs, compute pair weights, or
reduce a product mixture.

Therefore the precise classification is:

- backward stage: genuine multi-component reverse GSF with electron material;
- per-surface published smoothed state: single-Gaussian two-filter combination
  of the moment-collapsed forward-updated and backward-predicted mixtures;
- not a retained-lineage RTS Gaussian-sum smoother;
- not a full Frühwirth-style forward/backward component-product smoother.

The final `reco::GsfTrack` is built by projecting the geometrically inner
trajectory `updatedState()` to the beam line (`TrackProducerAlgorithm.cc`,
lines 326--384). In a smoothed trajectory that `updatedState()` is the stored
single-Gaussian smoothed state at interior hits. The `GsfTrackExtra` can retain
inner/outer multi-state information elsewhere, and mode utilities exist, but
the base track momentum constructed here is the mean/covariance state projected
to the PCA, not a highest-weight component or a 1D marginal mode.

CMSSW component updates use the full innovation likelihood, including the
inverse square-root innovation determinant
(`PosteriorWeightsCalculator.cc`, lines 87--125). Its close-component reducer
does not implement a global closest-pair KL loop: the current implementation
repeatedly takes the lowest-weight active component and merges it with its
nearest active component according to the configured distance
(`CloseComponentsMerger.icc`, lines 27--124). The standard 5D merger is
configured externally; this audit does not equate its distance with CEPC's KL
reducer.

## Source-validated ACTS conclusion

The preliminary ACTS interpretation is confirmed and can now be stated more
strongly.

`GaussianSumFitter.hpp` explicitly says that individual component states are
not exported to the returned `MultiTrajectory` and that no dedicated component
smoothing in the Frühwirth sense is performed (lines 49--54).

The actual backward algorithm is:

1. Save the filtered component mixture at the last forward measurement
   (`GsfActor.hpp`, lines 450--463).
2. Initialize the reverse pass directly from that same forward posterior,
   multiplying each component covariance by
   `reverseFilteringCovarianceScaling`, default 100
   (`GaussianSumFitter.hpp`, lines 383--405; `GsfOptions.hpp`, lines 127--130).
3. Mark the last forward filtered combined state as smoothed, then run the same
   actor in reverse over the measurements (`GaussianSumFitter.hpp`, lines
   407--427).
4. At every revisited measurement, perform a fresh component Kalman update and
   posterior-likelihood reweighting (`GsfActor.hpp`, lines 344--419). There is
   no multiplication by the stored forward predicted or filtered state.
5. Replace the returned state's smoothed slot with a collapse of the reverse
   **filtered** mixture (`GsfActor.hpp`, lines 594--613). The collapse method is
   configurable as moment mean or maximum-weight component and defaults to
   maximum weight (`GsfOptions.hpp`, lines 27--30 and 139--140).
6. The optional reference-surface track parameters come from the reverse-pass
   end mixture collapsed with the same configured method. The full final
   multi-component state can also be stored in an optional column
   (`GaussianSumFitter.hpp`, lines 498--515).

ACTS is consequently a forward GSF followed by a second reverse
multi-component filter whose seed is the inflated final forward posterior. It
is not an independent backward likelihood, not a two-filter product smoother,
and not an RTS/retained-lineage component smoother. Because the reverse seed
already contains all forward measurements and the reverse pass updates those
measurements again, it deliberately reuses measurement information; it should
not be described as a statistically independent two-filter smoother.

At material surfaces ACTS first performs the measurement/no-measurement update,
then convolves each filtered component with the BH mixture, reduces, applies a
low-weight cut, and updates the stepper (`GsfActor.hpp`, lines 246--313).
Backward BH transport reverses the fractional energy-loss mapping through
`p -> p/mean` rather than applying forward loss again (`GsfUtils.cpp`, lines
122--146). Multiple scattering is split into pre/post updates on measurement
surfaces (`GsfActor.hpp`, lines 224--243 and 315--324). The example integration
selects largest-weight or symmetric-KL reduction; KL is a closest-pair loop
with moment merging (`GsfFitterFunction.cpp`, lines 114--127;
`GsfMixtureReduction.cpp`, lines 23--53 and 73--80). Defaults are four
components and a `1e-4` weight cutoff (`GsfOptions.hpp`, lines 115--119), while
the actor also respects the stepper's component capacity.

## CEPC implication

The three workflows are now cleanly separated:

- CEPC optional smoother: retained-forward-graph, KL-aware RTS-style
  component smoothing; it generally stays LCIO-like in the tested sample.
- CMSSW: reverse multi-component filter plus a per-hit single-Gaussian
  two-filter combination of collapsed forward/backward moments.
- ACTS: reverse multi-component refilter seeded from the inflated forward
  posterior, with no forward/backward state combination; returned per-hit
  states are collapses of the reverse mixture.

The CEPC reverse filter is architecturally much closer to ACTS than to CMSSW's
published smoothed-state construction, but it is not claimed equivalent until
its seed, surface ownership, material ordering, and final collapse are mapped
field by field. CMSSW does not provide evidence that CEPC should implement an
all-pairs product smoother; its current source does not do that. No new CEPC
smoother implementation is justified merely by this package comparison.

The next useful work remains the active positive-amplification state audit and
its physics discriminator. If a package-faithful smoother control is later
wanted, the smallest source-faithful controls are (a) an ACTS-like reverse
refilter with explicitly forward-posterior seed inflation and configurable
mean/max collapse, and (b) a CMSSW-like collapsed-moment combination of the
stored forward-updated and backward-predicted states. Both must remain
default-off and pass the existing overshoot/control and hard-loss validation
ladder.
