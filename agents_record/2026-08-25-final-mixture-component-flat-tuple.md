# Default-on final-mixture component flat-tuple record

Date: 2026-08-25

## Purpose and boundary

The final smoother/reverse pT marginal previously required temporary verbose
instrumentation because the persisted outputs contained only BestBranch,
WeightedMean, and FullMixtureMode endpoints. The ordinary output now records
the parameters needed to reconstruct that one-dimensional marginal. This is
automatic/default-on persistence, not a new algorithm property, selection
mode, or fourth track endpoint. It never changes component states, weights,
reduction, selection, or publication.

Ordinary forward, CMS-like, and global-loss methods do not expose the same
final multi-component endpoint and therefore leave these records empty.

## EDM and flat schema

`RecGsfTracking` writes nine aligned PODIO user-data collections:

- `GSFFinalMixtureComponentInputTrackIndex`;
- `GSFFinalMixtureComponentOutputTrackIndex`;
- `GSFFinalMixtureComponentIndex`;
- `GSFFinalMixtureComponentID`;
- `GSFFinalMixtureComponentSource`;
- `GSFFinalMixtureComponentValid`;
- `GSFFinalMixtureComponentWeight`;
- `GSFFinalMixtureComponentKappa`;
- `GSFFinalMixtureComponentKappaVariance`.

Every finite positive-weight survivor is retained. Weights are normalized
independently inside each input/output track group. Source code 1 identifies
the Gaussian-sum smoother and source code 2 identifies ordinary reverse
filtering. The component index is its position in the final internal vector;
the component ID is an event-local lineage/debug identifier. `valid=1`
requires a successful IP extrapolation, finite five-parameter mean, positive
finite kappa variance, and positive-definite full IP covariance. Failed
components retain their mapping and weight with `valid=0`; a failed
extrapolation stores NaNs, while a later covariance-validity failure may retain
diagnostic kappa values. The flat pT is NaN for every invalid entry so an
incomplete mixture cannot be mistaken for a complete one.

`RecGsfFlatTuple` always creates:

- `final_mixture_component_available` and
  `final_mixture_component_n`;
- vectors with the suffixes `input_track_index`, `output_track_index`,
  `index`, `id`, `source`, `valid`, `weight`, `kappa`,
  `kappa_variance`, and `pT`.

The flat pT value is `1/abs(kappa)` for valid components and NaN otherwise.
The track mapping preserves every output track in an event; this is important
because the older scalar endpoint fields describe only the first output
track.

For `pT > 0`, valid component marginals reconstruct the density through

```text
f(pT) = sum_i w_i / pT^2 * [
          N(+1/pT | kappa_i, variance_i)
        + N(-1/pT | kappa_i, variance_i)] .
```

If any component in a track group is invalid, the valid subset is not a
complete persisted mixture and must not be silently renormalized and reported
as such.

## Mechanical validation

The focused EL9/LCG-105 build completed for both `RecGsfTracking` and
`RecGsfFlatTuple`, followed by installation. Verbose same-input gates used
`trk_large_20260823/trk-e--2.0-85-1.root`, the production reverse seed scale
100, truth override off, and the reverse template.

- Focused reverse event 0: 12/12 valid components, source 2, aligned vectors,
  and per-track weight sum 1.
- Focused Gaussian-sum smoother event 0: 12/12 valid components, source 1,
  aligned vectors, and per-track weight sum 1.
- Required hard events 11, 16, and 17: 44/44 valid components. Event 11 had
  12 components; event 16 mapped 9 to input/output track 0 and 12 to track 1;
  event 17 had 11. Every track group summed to one.
- The hard-event BestBranch, WeightedMean, and FullMixtureMode pT values were
  exactly unchanged relative to the immediately preceding same-code output:
  event 11 was `40.9351049739`, `41.3734028959`, `40.9174657114` GeV;
  event 16 track 0 was `18.3188084765`, `18.3189062109`,
  `18.3189062109` GeV; event 17 was `18.6248032091`, `18.9627940575`,
  `18.6309608277` GeV.

These gates validate persistence, track association, and non-interference.
They do not validate the physics performance of any endpoint or the use of a
one-dimensional pT marginal as a final estimator.

No configurable property was added, removed, renamed, or changed, so the
dedicated configurable-property option-surface audit law was not triggered.
The maintained card was updated only to document the automatic output.
