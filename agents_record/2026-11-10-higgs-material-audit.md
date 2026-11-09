# Higgs REC material truth and BH-range audit (2026-11-10)

The user questioned whether compact REC truth is sufficient for the GSF passive
material recorder, why it is restricted to original input index0, and whether
material/BH controls work for actual Higgs electrons, especially endcaps.
This is a read-only investigation with temporary diagnostic cards. No source,
installed library or maintained default changed; no simulation was rerun.
All inputs are the new normal-ISR/FSR production's rec-higgs-4.root (200 events).

## Stored information and current scope

The simulation writer retains every SimTrackerHit first/last/resolved-hook
checkpoint, every eBrem process step and photon-emission parent checkpoint,
track endpoints and ECAL boundary checkpoints. Omitted non-radiative interior
steps are represented by GsfG4MaterialStepSpans with exact step ranges/counts
and integrated t/X0. TruthBHLossEventData uses those spans for interior sums
and individual hook/radiative steps for positions, momentum and eBrem loss.
This supports its hook-to-hook material integrals without resimulation; it
does not reproduce arbitrary omitted step-by-step material composition.
The normal card discovers and reads the span collection. The retained compact
reader test covers full/compact equivalence and skipped intermediate hits.

Current RecordTruthMaterialIntervals is still scoped by
TruthBHLossInputTrackIndex, default0, and filtered by the production-electron
selector. It does not automatically cover the selected pair. In this file
only27 of397 selected reconstructed electron tracks have original index0,
before any exact-hook guard. This is legacy single-track/gun steering.
Its purpose is validation: distinguish geometry/material assignment from BH
response and inference failures. It never steers ordinary GSF fitting.

A suitable future scope change is to record every processed selected track,
keep interval records indexed by original input/output track, and expose
per-track recording statuses in the flat tuple. The existing EDM status
collection is already per input track; its current flat view is scalar.
Leave TruthBHLossInputTrackIndex as the explicit oracle-only scope. This
proposal is NOT implemented by this audit and adds no new steering requirement.

## Raw reconstructed-hit inventory

The native standalone inspector uses the actual shared TruthElectronSelector
and TruthBHLossEventData implementations, native PODIO relations and all raw
REC track hits sorted by radius. It is not a GSF fitting efficiency study;
GSF's matched hit subset can differ. Endcap means at least one ITKEndcap or
OTKEndcap hit, identified by actual collection IDs.

- 197 events have two unique reconstructed partners;3 have one:397 selected tracks.
- 319/397 pass strict raw-hook matching. Failures:23 status28,41 status30,
  1 distance failure and13 missing provenance-bearing hit associations.
- 85 tracks contain endcap hits;67 pass the raw-hook check. Of those67,
  58 contain a G4 hook-to-hook interval thicker than0.03 X0.
- Positive hemisphere:41 endcap-hit tracks,34 hook-valid,31 above0.03.
  Negative hemisphere:44,33,27 respectively.
- Among252 hook-valid tracks without endcap hits,5 exceed0.03.
- The maximum valid G4 interval is0.2715531892 X0.

These counts are conditional material-coverage/range diagnostics from one
200-event file, not population resolution or calibrated detector efficiencies.
The64 malformed status28/30 ranges reproduce the known TPC producer-state
problem. Missing provenance associations are kept separate; they are not
silently repaired or all attributed to the same recorder bug.

## Native passive recording on actual Higgs electrons

Three temporary cards use maintained reverse settings, depth0 and pair
selection, with TruthBHLossOverride=False. Only the diagnostic index is changed
to the actual selected track. The matched-hook recorder succeeds in all three:

| Region; job4 event/track | theta | Intervals | G4 total X0 | DD4hep same-hook total X0 | Relative total difference |
| --- | ---: | ---: | ---: | ---: | ---: |
| barrel;0/9 | 96.43deg |232|0.07210639137|0.07209896464|-0.01030%|
| positive endcap;17/23 |26.89deg|162|0.26664901436|0.26680564246|+0.05874%|
| negative endcap;24/10 |150.02deg|201|0.36977446838|0.36974574246|-0.00777%|

All595 truth-hook geometry intervals are valid. For the first two cases,
all runtime forward/reverse material candidates are valid too. Maximum
same-hook relative interval differences (G4 t/X0>1e-6) are0.0716% in the
barrel and0.8193% in the positive endcap. G4 curved trajectories and DD4hep
straight chords/endpoints are distinct geometric objects.

The positive-endcap terminal interval is hit161->162, z2754.899->2925.797mm:
G4=0.17221840026, DD4hep same hooks=0.17221501570,
actual fitted forward/reverse path=0.17374897676 X0. The negative endcap
hit200->201, z-2760.766->-2925.797mm, gives G4=0.27155318916,
DD4hep=0.27154369067, actual runtime=0.27206206592 X0.
These thick intervals are not merely inflated DD4hep path estimates.

For the barrel and positive-endcap cases, all205 non-material flat fields
match their existing same-settings runs exactly after changing the recorder
index. Only passive material fields differ: direct non-steering evidence.
This is focused geometry validation, not validation of every endcap topology.
The recorder covers first-hit to last-hit intervals; it does not validate
the extra beam-to-first-hit boundary against G4 truth.

## Concrete BH response limitation

The active CEPCRuntimeCategoryAligned9Clear table has8 thickness knots,
4.6934641432e-5 through0.03 X0, with10 components. In
BetheHeitlerSplitter.cpp::cepcIntervalMixture, lower/upper default to the last
knot and are changed only for x<knots.back(). At or above0.03, the same final
weights/means/variances are returned. Thus actual0.174/0.272 intervals receive
the0.03 response; actual material remains correctly persisted in diagnostics.
This is a model-range saturation, not evidence that the thicker material is
absent. The cap applies per interval, not to the cumulative track material.

ActsAtlas likewise evaluates at min(x,0.2); switching models alone does not
cover the largest observed endcap intervals and is not physics validation.
The BH response has no explicit Higgs/endcap flag: it is conditioned on t/X0;
endcap geometry changes that input and its hit/measurement topology.

Keep DD4hepBetweenSurfaces as the geometry-based candidate; do not substitute
CurrentSurface merely to shrink the BH input. Before claiming endcap GSF
performance, fix/validate thick-interval response (extended fitted range or
validated composition of smaller material responses), retain out-of-range
diagnostics, resolve/label truth-hook failures, and test per-track momentum
against generator truth by angle, hemisphere, momentum and eBrem category.
No such physics/source change is authorized or implemented in this audit.

## Artifacts

build.gsf-selection-dev/validation/higgs-material-audit-20261110/ contains
inventory.py/json/log, three native cards/logs/flat tuples, material.json per
case, summarize.py, summary.json/log, passivity.json and validation.json.
They remain uncommitted analysis outputs. Runtime defaults and software are
unchanged from GSF promotion checkpoint1549ed8. The earlier three named
truth-hook failures and their introduction history remain in
2026-11-10-new-rec-truth-provenance-failures.md.
