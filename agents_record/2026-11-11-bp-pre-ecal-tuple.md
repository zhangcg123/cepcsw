# BP pre-reclustering ECAL snapshot

## Authority and scope

The user requested `pre_ecal_cluster_energy/x/y/z` in the BP flat tuple,
matching the GSF exporter. Implementation is confined to
`Reconstruction/RecBreakpoint`: the existing association-tuple helper,
algorithm booking/filling, existing truth/ECAL tuple test and README.
There are no configurable-property changes or new dependencies. Existing run
cards already load the snapshot for the default PreReclustering constraint.
No GSF, shared KF, simulation, tracking or PFA code/card is changed. The branch
remains `breakpoint-delayed-neutral`; no branch operation or batch submission.
The complete outgoing AGENTS is preserved in
[the snapshot](2026-11-11-agents-before-bp-pre-ecal-tuple.md).

## Schema and behavior

All 12 GSF snapshot branches are written once per selected event in the
existing `neutral_pfos` tree, joined to per-track `breakpoint` rows by
`event_index`:

- `pre_ecal_available` (int).
- `pre_ecal_cluster_index`, `pre_ecal_cluster_hit_count` (vector<int>).
- `pre_ecal_cluster_energy`, `pre_ecal_cluster_x/y/z` (vector<double>).
- `pre_ecal_pfo_charge` (vector<double>, original pre-PFO order).
- `pre_ecal_pfo_track_pfo_index`, `pre_ecal_pfo_track_input_index` (vector<int>).
- `pre_ecal_pfo_cluster_pfo_index`, `pre_ecal_pfo_cluster_index` (vector<int>).

The cluster arrays copy the complete original pre-ECAL collection without
measurement filtering; energy is GeV, position mm. Link arrays identify charged
and trackless-neutral ownership through original pre-PFO/CompleteTracks
indices. HCAL relations are skipped. Individual hits are not duplicated;
only their count is recorded. No generator/shower truth membership is inferred.
Existing final-PFO cluster fields remain final-PFO diagnostics.

The helper mirrors GSF validation: both snapshots absent gives availability 0
and empty vectors; both present, even empty, gives 1; a partial snapshot,
unavailable relation, foreign/out-of-range track, or out-of-range ECAL link
fails explicitly. The default PreReclustering input requirement remains strict.
Legacy FinalPFO cards may omit pre-collections and then record availability 0;
loading pre-collections independently makes the snapshot available without
changing the selected constraint source. The fields are passive.

## Verification and installation

`RecBreakpoint` and `RecBreakpointTruthEcalTupleTest` build successfully with
EL9/LCG105. The existing test executable passes extended cases for raw cluster
order/E/xyz/hit counts, charged and neutral links, HCAL exclusion, state reset,
missing versus empty snapshots, partial snapshots, foreign/out-of-range tracks
and unavailable track/cluster links. An initial test-only compile failure used
an EDM4hep API absent from this release; it was corrected to the supported
unavailable-object constructor before the successful build/run.

Direct old/new native Gaudi reruns use current TPC-fix REC job11, entries
0, 11, 16 and 17 (EvtMax18), including all eight selected electrons. Both
runs use the same frozen campaign steering, except LocationScanSetups=[]
for a bounded writer regression, with verbose state/covariance dumps.
Truth-location ordinary/free-loss/truth-override fits and beam/charged ECAL
constraints remain enabled. The baseline explicitly loads a private copy of
the old installed module; the other run loads the rebuilt module.
Both terminate successfully.

- All **917 old branches across 151 per-track rows** agree exactly, including
  NaNs, fit outputs and diagnostics.
- All **114 old branches across four event rows** agree exactly.
- Only the event tree gains the **12 pre_ecal fields**.
- All 12 values and ROOT types agree exactly with matching GSF event rows;
  all values also agree with the original REC collections and links:
  **248 clusters and 248 PFO/ECAL links**.

The initial full-location-scan comparisons were stopped by this task because
that unchanged scan dominated verification time. Their `*-scan-aborted.*`
artifacts are retained and are not counted as successful checks. Full location
scan/population regression was not repeated for this passive schema addition.
This is a writer/mechanical gate, not physics validation.

Only the RecBreakpoint package was installed, including its generated Gaudi
metadata. Installed module SHA256 matches the tested build. The recorded GSF
tracking/flat-writer library checksums are unchanged. Existing REC, BP and GSF
production files/cards are untouched. Existing flat files require regeneration
to gain these fields; no campaign was rerun or submitted.

Artifacts (uncommitted):
`TrackingPerformanceStudies/bp_pre_ecal_tuple_20261111/`, including README,
old/new cards/ROOT/logs, comparison script, validation.json, unit/build/install
logs and runtime-sha256.json. Durable schema is in
`Reconstruction/RecBreakpoint/README.md`.

## Prior studies retained

The completed generator FSR membership survey and its unresolved replay limits
remain in [the FSR record](2026-11-10-fsr-cluster-membership.md).
Existing comparison plots and all plotting choices remain in
[the comparison record](2026-11-10-matched-gsf-bp-overlays.md).
This snapshot adds measurement geometry/ownership only; it does not add the
photon-to-cluster MC contribution links required for that FSR attribution.
