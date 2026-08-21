# Passive truth-material recorder default promotion

Date: 2026-08-22

## Decision

After the passive interval recorder was implemented and its on/off
non-interference gate passed, the user explicitly promoted
`RecordTruthMaterialIntervals` from false to true by default.

The resulting contract is:

- compiled `RecGsfTracking` default: `true`;
- effective active reverse-template default: `true` through the compiled
  default and an explicit assignment; the template requests both embedded
  provenance collections;
- maintained `DumpGsfTrks/gsf.py.bk` value: explicitly `true`;
- truth BH-loss steering remains independently controlled by
  `TruthBHLossOverride`, whose default remains false;
- the recorder remains output-only and cannot alter propagation, material
  paths, BH responses or split gating, component weights, reduction,
  measurement updates, reverse selection, or the published track.

Inputs carrying `GsfG4MaterialSteps` and
`GsfSimTrackerHitG4StepLinks` can therefore populate the final
`GSFTruthMaterialIntervals`, `GSFTruthMaterialRecordStatus`, and 50
`truth_material_*` flat branches without another property assignment. If the
embedded truth collections or a valid association scope are unavailable, the
ordinary GSF still runs unchanged and the recording status identifies the
unavailable scope.

The first active-template smoke exposed that the event-data preparation had
treated an unrequested, unused detector-region association collection as a
fatal event error. Preparation now consumes every available standard tracker
association collection and ignores an unrequested collection until track
matching. The selected track remains all-or-nothing: if one of its accepted
hits needs an unavailable association, that track's recording scope is
invalid. This changes only diagnostic availability.

A final one-event active reverse-template smoke, without overriding the
recorder property, completed with status `1` and wrote 231 interval objects to
both the EDM and flat tuple.

This decision changes recording availability, not the production physics
configuration and not the evidentiary status of any material/BH hypothesis.
No physics-impact audit accompanied the default promotion.

The original implementation contract and exact recorder-on/off validation are
preserved in
`agents_record/2026-08-22-passive-truth-material-interval-final-tuple.md`.
