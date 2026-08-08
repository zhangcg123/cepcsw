# Fresh broad-electron 1,000-event survey

Date: 2026-08-08

The fresh batch contains 50 seeds and the reverse-GSF jobs process the first
20 entries per seed, for 1,000 analyzed events. All 50 jobs finalized and all
1,000 flat-tuple entries are present. GSF fitted 1,068 of 1,069 input tracks;
the sole non-fitted track is one of five tracks in seed 50, entry 6. The
primary event output is present. That event has 455 secondary tracker SimHits
and belongs to the excluded secondary-topology control population.

The legacy filenames contain `2.0-85`, but the generated simulation cards do
not describe a fixed 2 GeV-pT, theta-85-degree sample. They explicitly use
`EnergyMins=[10]`, `EnergyMaxs=[50]`, `ThetaMins=[40]`, and
`ThetaMaxs=[140]`. In the analyzed subset, generator momentum spans
10.07--49.97 GeV, pT spans 6.85--49.46 GeV, and theta spans approximately
40.1--140.0 degrees. Treat this batch as a broad energy/angle transfer and
safety sample, not as a direct continuation of the historical fixed
2 GeV/85-degree optimization population.

The simulation and raw material-step files each contain 100 entries, whereas
the reverse-GSF flat tuples contain 20. The fresh join therefore explicitly
uses entries 0--19 of every seed. Geant4 `event_id` is zero-based; flat-tuple
`iev` is the one-based event count. All 1,000 entry identities pass this
explicit alignment check and have finite truth, LCIO, and GSF primary-track
momenta.

`GsfMaterialStepRecorderAnaElemTool` ran with `TrackerOnly=True`; consequently
the primary classification uses every recorded primary-electron eBrem step.
A volume-token-only cross-check undercounts losses in tracker support volumes
and is not used for the physics categories. The authoritative fresh counts
are 303 no-eBrem, 520 light-eBrem, and 177 hard-eBrem events.

The objective SimHit topology classification finds 121 events with at least
one non-primary tracker SimHit, including 17 with at least 20. These are kept
as a separate control population. The topology-clean populations are 303
no-eBrem, 480 light-eBrem, and 96 hard-eBrem events.

For the topology-clean populations, LCIO versus reverse GSF gives:

| category | sample | median residual | central-68 half-width | inside 1% | inside 5% |
|---|---|---:|---:|---:|---:|
| no-eBrem | LCIO | +0.0178% | 0.1750% | 303/303 | 303/303 |
| no-eBrem | GSF | +0.0545% | 0.2346% | 294/303 | 302/303 |
| light-eBrem | LCIO | -0.1634% | 1.0703% | 368/480 | 446/480 |
| light-eBrem | GSF | -0.0971% | 0.5126% | 393/480 | 461/480 |
| hard-eBrem | LCIO | -14.2358% | 20.2436% | 20/96 | 24/96 |
| hard-eBrem | GSF | -0.8669% | 17.4907% | 42/96 | 54/96 |

The same tradeoff remains visible: central light/hard performance improves,
while clean tracks broaden and new extreme positive tails appear. Among the
879 topology-clean events, GSF has 33 residuals above +1% and eight above
+5%; LCIO has none. Eight events have a positive LCIO residual that GSF
amplifies by more than one percentage point. The most extreme is seed 7,
entry 8: no tracker eBrem, LCIO +0.033%, GSF +288.136%. Seed 14, entry 1 is a
near-zero-loss light event with LCIO -0.020% and GSF +203.272%. These are
priority verbose-dump candidates. This survey is not a same-code
`ComponentWeightCutoff=1e-8` versus `1e-4` A/B and does not validate the broad
transfer performance of the conditioned model.

Generated, intentionally uncommitted analysis outputs are under:

- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/new_1000_sample_2026-08-08/`
- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-08-08/`

The main event catalogue is `fresh_event_catalog.csv`; categorized statistics
are in `performance_summary.csv`; topology rows are in
`secondary_tracker_activity_event_ids.csv`; and focused candidates are in
`focused_event_candidates.csv`.
