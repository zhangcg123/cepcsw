# Fresh broad-electron 4,800-event GSF survey

Date: 2026-08-08

## Job and output integrity

After the cloned-state ownership memory fix, 48 of 50 reverse-GSF jobs
completed all 100 requested events.  Their 48 EDM files and 48 flat tuples
are readable and contain exactly 4,800 entries each.  Across these events,
GSF fitted 5,122 of 5,129 input tracks.  The seven non-fitted secondary tracks
occur in six multi-track events; every primary flat-tuple row is finite and
aligned (`event_id == entry`, `iev == entry + 1`).

Seeds 32 and 36 did not fail from memory exhaustion.  Seed 32 completed entries
0--3 and seed 36 completed entries 0--7 before deterministic-looking
segmentation faults during podio event cleanup.  Both stacks enter
`podio::ObjBase::release` from destruction of an
`MCRecoTrackerAssociationCollection`; their flat tuples have zero committed
entries and are excluded in full.  The usable survey therefore has 4,800,
not 5,000, events.

The filenames retain the historical `2.0-85` token, but this remains the fresh
broad generator sample: truth momentum spans 10.01--49.97 GeV and theta spans
40.02--140.00 degrees.  It is a transfer/safety sample, not a fixed 2 GeV-pT,
85-degree optimization sample.

## Truth categories and topology

The authoritative primary-electron category uses all recorded eBrem steps,
because the material-step recorder was run tracker-only.  The 4,800 usable
events contain 1,537 no-eBrem, 2,325 light-eBrem, and 938 hard-eBrem events.

There are 545 events with non-primary tracker SimHits, including 53 with at
least 20.  Reporting those separately leaves topology-clean populations of
1,534 no-eBrem, 2,182 light-eBrem, and 539 hard-eBrem events.

## Topology-clean LCIO versus reverse GSF

| category | sample | median residual | central-68 half-width | inside 1% | inside 5% |
|---|---|---:|---:|---:|---:|
| no-eBrem | LCIO | -0.0007% | 0.1717% | 1529/1534 | 1534/1534 |
| no-eBrem | GSF | +0.0163% | 0.2172% | 1491/1534 | 1525/1534 |
| light-eBrem | LCIO | -0.1873% | 1.0083% | 1657/2182 | 2043/2182 |
| light-eBrem | GSF | -0.1067% | 0.5585% | 1734/2182 | 2109/2182 |
| hard-eBrem | LCIO | -14.3401% | 22.3531% | 118/539 | 159/539 |
| hard-eBrem | GSF | -1.2115% | 20.2545% | 201/539 | 287/539 |

The larger transfer sample confirms the established tradeoff.  GSF improves
the light and hard central distributions and hard-event containment, while it
broadens the clean core and produces unacceptable positive tails.  Among the
4,255 topology-clean events, LCIO has six residuals above +1% and none above
+5%; GSF has 230 above +1% and 63 above +5%.  By category the GSF counts above
+1%/+5% are 39/7 no-eBrem, 129/24 light-eBrem, and 62/32 hard-eBrem.  There are
35 events with positive LCIO residuals amplified by GSF by more than one
percentage point.

The most extreme topology-clean positive modes include seed 34/entry 72
(hard, +1787.1%), seed 18/entry 73 (light, +1304.8%), and seed 23/entry 43
(no-eBrem, +528.8%).  Previously identified seed 7/entry 8 remains extreme at
+288.1%.  These tails preclude a broad-transfer validation claim despite the
central improvements.

Generated, intentionally uncommitted analysis outputs are under:

- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/new_4800_sample_2026-08-08/`
- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_4800_2026-08-08/`

The main files are `fresh_event_catalog.csv`, `performance_summary.csv`,
`focused_event_candidates.csv`, `secondary_tracker_activity_event_ids.csv`,
and `topology_clean_event_outcomes.csv`.
