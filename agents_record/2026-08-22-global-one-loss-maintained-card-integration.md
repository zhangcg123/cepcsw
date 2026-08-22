# Global one-loss maintained-card integration

Date: 2026-08-22
Scope: `Reconstruction/RecGsfTracking`, the maintained
`DumpGsfTrks/gsf.py.bk`, and live project documentation

## Decision and boundary

The user requested that the experimental global one-loss method remain
parallel to the established GSF workflows and become selectable through the
maintained GSF card. This change provides that mechanical selection without
turning the method into the production default or a validated replacement.

The four explicit card values are now:

```text
smoother
reverse
cms-like
global-loss
```

`method="reverse"` remains the source default. Selecting `global-loss`
schedules `RecGsfGlobalLossRefitter` instead of `RecGsfTracking`; it does not
enable a mode inside the existing algorithm. The established methods still
schedule `RecGsfTracking` and write `GSFTracks`.

No batch submission script was changed, so no campaign silently starts using
the experimental method. No source outside `Reconstruction/RecGsfTracking`
was changed. The user-edited `DumpGsfTrks/README.md` was deliberately left
untouched; the authoritative algorithm/options explanation and live status
were synchronized instead.

## Explicit global-loss steering

The maintained card explicitly assigns all 14 algorithm-specific properties,
rather than inheriting future compiled behavior:

| Property | Card value |
|---|---:|
| `ElectronHypothesis` | `true` |
| `BHModel` | `CEPC2GeV85StepConditioned` |
| `BHSplitThreshold` | `1e-4` |
| `MSOn` | `true` |
| `ElossOn` | `false` |
| `OuterSeedCovarianceScale` | `100` |
| `ProcessSigmaWindow` | `3` |
| `ProfileGridPoints` | `9` |
| `ProfileRefinementIterations` | `6` |
| `MinimumRetainedFraction` | `0.05` |
| `MinimumRadiativeLogBayesFactor` | `3` |
| `CandidateIntervalIndices` | empty |
| `SelectedEventIndices` | empty |
| `VerboseDump` | `false` |

The global method has no truth-BH oracle or passive runtime-GSF material
recorder. Its input collection list therefore does not request
`GsfG4MaterialSteps` or `GsfSimTrackerHitG4StepLinks`. Its EDM and flat
filenames use the unambiguous `global-loss` tag. The ordinary three workflows
retain their existing truth-BH on/off filename tags and their complete
43-property steering.

## Stable flat-tuple adapter

`RecGsfFlatTuple` gained one default-off property:

```text
UseGlobalLossTracks = false
```

With the default value, it reads `GSFTracks` exactly as before. With the
property true, it reads `GlobalLossTracks` into the existing `gsf_*` scalar
and hit branches. The maintained card sets it true only for
`method="global-loss"`. This avoids a second analysis schema while preserving
the collection-level separation in the EDM.

The optional `GSFTracksEcalConstrained` path remains associated with ordinary
GSF jobs. The global method does not schedule or claim an ECAL-constrained
result.

## Mechanical validation

The scoped EL9/LCG 105 build and install completed for
`RecGsfTracking` and `RecGsfFlatTuple`. Their generated component and
configurable catalogs expose `RecGsfGlobalLossRefitter` and
`UseGlobalLossTracks`. The repository-wide catalog was merged directly from
its already generated fragments; a full unrelated-package rebuild was not
used as a validation claim.

A one-event same-input reverse regression used seed 2 from
`trk-e--2.0-85-2.root`. Before/after flat files contained one entry and the
same 131 branches. Every Awkward buffer, including dtype, shape, and raw bytes,
was identical. The post-change log selected `RecGsfTracking` and reported
`trackSource=GSFTracks`.

A one-event global-loss smoke test used the same input and a temporary
`CandidateIntervalIndices=[5,6]` allow-list solely to bound runtime. It
completed successfully, selected identity for that event, and wrote:

- one `GlobalLossTracks` track;
- the complete global-loss diagnostic collection set;
- one 131-branch flat entry with `trackSource=GlobalLossTracks`.

The smoke tuple reported truth pT 34.8016396 GeV, LCIO pT 34.9000966 GeV, and
global-loss pT 34.9047186 GeV. These numbers establish persistence and wiring,
not physics performance.

## Physics status and next diagnostic

Workflow availability does not supersede the frozen 30-event evidence. The
global method still selects adjacent wrong intervals in important hard-loss
cases and can profile to the retained-fraction floor, including the recorded
catastrophic file-66 entry-66 tail. It remains a diagnostic instrument.

Do not add multi-loss histories or tune
`MinimumRadiativeLogBayesFactor`/`MinimumRetainedFraction` next. First
decompose the per-hit likelihood crossover between selected interval 4 and
truth-compatible interval 5 in file 66 entry 66, using file 19 entry 4 and
file 66 entry 22 as correct-history controls. The main material/BH consistency
focus and frozen reverse production baseline remain unchanged.
