# Direct tracker-to-GSF workflow

Date: 2026-08-22

## Decision

The maintained `dump_gsftrk.sh` worker now runs the three-stage chain

```text
simulation -> tracker digitization/reconstruction -> GSF refit
```

and passes `trk-<particle>-<pT>-<theta>-<seed>.root` directly to the generated
GSF card. Calorimeter digitization and reconstruction are deliberately skipped
while the ECAL component-selection study remains paused.

## Superseded workflow

Immediately before this decision, the worker ran

```text
simulation -> tracking -> calorimeter digitization -> calorimeter reconstruction -> GSF
```

and the generated GSF card consumed `rec-<sample>.root`. The optional
`gsf_only=true` control likewise reused that reconstructed-event file. The
calorimeter cards are retained outside the active worker for possible future
studies; this change does not delete their historical outputs or records.

## Active contract

- `gsf_only=false` runs simulation, tracking, and GSF in that order.
- `gsf_only=true` requires and reuses the sample-qualified tracker output.
- The generated GSF card substitutes `trk-<sample>.root` for its maintained
  `trk_v01.root` placeholder.
- The maintained GSF card does not request `EcalCluster`; its explicit
  `EcalComponentConstraint=false` steering remains unchanged.
- Embedded `GsfG4MaterialSteps` and `GsfSimTrackerHitG4StepLinks` are created by
  simulation and preserved by the tracking card's `keep *`, so the passive
  material recorder and default-off truth oracle do not require a calorimeter
  stage.
- Truth-on/truth-off filename tagging and the ordinary BH fallback contract are
  unchanged.

This is workflow plumbing only. It changes neither the GSF physics defaults nor
the material/BH investigation focus.

## Verification

`bash -n dump_gsftrk.sh` and Python parsing of the maintained GSF card passed.
The worker's exact card substitutions were then applied to a temporary card and
run for one event directly from the tracker-stage
`/tmp/gsf_embedded_truth_test/trk_embedded.root`. The log selected
`CEPCRuntimeGenericGrid5Clear`, reported `ecalConstraint=0` and
`truthBHLossOverride=0`, fitted `1 / 1` track, wrote one flat-tuple entry, and
terminated successfully without an `EcalCluster` input. The paired truth-on
card also fitted `1 / 1`, matched one selected input track from the embedded
EventData, replaced all 18 executed BH responses, and reported zero invalid-
event, endpoint-distance, or interval-map fallbacks. This confirms that the
tracker output preserves the provenance needed by both current diagnostic
consumers without the calorimeter stages.
