# Secondary tracker topology separation

The existing raw simulation files `tuples285/sim-e--2.0-85-*.root` were
surveyed directly; no new simulation or reconstruction job was run. The
classification counts non-primary (`MCParticle` index greater than zero)
SimTrackerHits in VXD, ITK barrel/endcap, TPC, and OTK barrel/endcap. It does
not count calorimeter shower activity and must not be described as an
all-detector secondary-particle classification.

Across 5,000 events, 133 have at least one secondary tracker SimHit, 75 have at
least two, 48 have at least three, 35 have at least five, 25 have at least ten,
and 20 have at least twenty. The last group includes spectacular curling
conversion/delta-electron topologies with 23--11,552 secondary tracker hits.
Eighteen of these twenty contain a stored photon and positron, consistent with
photon conversion; the other two, 19/5 and 229/2, contain additional electrons
without a positron.

The complete stable `seed,entry` table, per-detector counts, total secondary
tracker-hit count, and `has_secondary_tracker_activity`/`complex_topology`
flags are in:

`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/secondary_tracker_activity_2026-07-13/secondary_tracker_activity_event_ids.csv`

The table contains all 5,000 simulated events. The current reconstruction
category table contains 4,990 events: 132 secondary-activity events and 4,858
events without secondary tracker activity. Seed 464/event 2 is the one
secondary-activity event absent from the resolution comparison because seed
464 has no `gsf_tuple`.

The normalized GSF pT residual comparison gives:

| topology | count | median residual | central 68% | inside 1% | inside 5% |
|---|---:|---:|---:|---:|---:|
| no secondary tracker SimHits | 4,858 | -0.0582% | [-0.677%, +0.144%] | 4,002 | 4,420 |
| any secondary tracker activity | 132 | -10.145% | [-69.019%, +0.090%] | 56 | 61 |
| at least 20 secondary tracker hits | 20 | -13.866% | [-64.957%, +0.909%] | 4 | 8 |

The plot and reproducible script are respectively:

- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/secondary_tracker_activity_2026-07-13/gsf_pt_resolution_by_secondary_tracker_activity.png`
- `Reconstruction/RecGsfTracking/scripts/plot_gsf_pt_by_secondary_tracker_activity.py`

For subsequent single-track GSF optimization, exclude the complete
`has_secondary_tracker_activity` set from objective counts and representative
selection. Always retain and report it as a separate topology/control
population. This exclusion is topology-based, not residual-based. The 20-hit
flag is a nested descriptive complex subset, not the primary exclusion
definition.

## Updated surface-owned category comparison

The no/light/hard LCIO-versus-GSF pT comparison was regenerated after removing
the complete secondary-tracker-activity set. Of the 4,990 reconstruction
events, the exclusion removes 13/2,045 no-eBrem, 16/2,148 light-eBrem, and
103/797 hard-eBrem events. The retained optimization populations are therefore
2,032, 2,132, and 694 events.

For the retained light category, LCIO versus GSF has median residual -0.2088%
versus -0.0969%, central-68 half-width 1.2575% versus 0.4357%, 1,549 versus
1,769 events inside 1%, and 1,971 versus 2,054 inside 5%. For retained hard
events, the median changes from -13.381% to -0.661% and the central-68
half-width from 25.001% to 22.888%. No-eBrem central performance stays close:
LCIO versus GSF central-68 half-width is 0.1321% versus 0.1434%.

Extreme GSF-only tails remain even after this topology exclusion, so full RMS
is not a stable optimization metric and secondary topology is not the sole
cause of the radiative-selection tail. The updated plots and summary are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/secondary_tracker_activity_excluded_2026-07-13/`.
