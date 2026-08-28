# CMS-like workflow retirement

Date: 2026-08-29

## Decision

Retire the active `CmsGsfSmoothing` workflow alias and its steering surfaces.
The alias no longer represented a distinct fit: after the common inward-filter
consolidation, both it and reverse used the same outward population, inward
recursion, terminal `B_updated[0]` mixture, and three endpoint publications.
The only CMS-associated product left was the passive per-surface
`B_smoothed[i] = F_updated[i] x B_predicted[i]` diagnostic, which reverse now
records unconditionally.

The maintained workflows are therefore `smoother`, `reverse`, and the separate
`global-loss` algorithm. Reverse remains the default and production candidate.
This interface cleanup makes no physics claim and does not promote the passive
smoothed mixture into propagation or publication.

## Removed active interface

- The `RecGsfTracking.CmsGsfSmoothing` Gaudi property and C++ dispatch branches.
- `method="cms-like"`, `"cms"`, and `"cms_like"` from
  `DumpGsfTrks/gsf.py.bk`.
- `GSF_CMS_GSF_SMOOTHING` from the reverse template.
- The `cms` choice from `run_reverse_selection_sample.py`.

The compiled property inventory changes from the actual pre-removal 41 to 40.
The package README's former count of 42 was already stale. The maintained card
explicitly steers 39 of the 40 properties and deliberately inherits only the
compiled `RecordTruthMaterialIntervals=true` default.

## Preserved contract and migration

No EDM collection, flat-tuple branch, numeric lineage code, or endpoint schema
was removed. Reverse continues to write `GSFTracksBestBranch`,
`GSFTracksWeightedMean`, and `GSFTracksFullMixtureMode`, and to persist the
complete final mixture and component-lineage graph. Source code 3 still means
a passive two-filter smoothed-mixture lineage node. Historical final-component
source codes 3 and 4 remain documented so old CMS-like tuples can be read.

Old generated cards are experiment outputs and are not edited in place. Before
reuse, regenerate any card that assigns `CmsGsfSmoothing`; use
`ReverseFiltering=true` for the maintained inward refit and remove the obsolete
`GSF_CMS_GSF_SMOOTHING` environment setting. Historical CMS-like result files
and dated `agents_record/` evidence remain unchanged.

## Evidence boundary

Immediately before retirement, same-code reverse/CMS-alias runs on hard-loss
events 11, 16, and 17 compared 900 endpoint/status scalars and 882
final-component/lineage vectors with zero mismatches. Each side contained
30,285 source-3 smoothed nodes, 29,088 operation-5 retained smoothing
candidates, 5,657 fate-7 diagnostic survivors, and 58,176 smoothing edges.
No source-3 node was BestBranch or a published final-mixture member; all final
components used reverse source code 2. The pre-retirement details are in
`2026-08-29-smoothed-diagnostic-only-publication.md`.

## Completed validation

The focused EL9/LCG-105 build and install completed for `RecGsfTracking` and
`RecGsfFlatTuple`. The regenerated build and installed Configurables do not
contain `CmsGsfSmoothing`; the header contains 40 algorithm properties. Python
syntax checks passed for the maintained card, reverse template, and focused
sample runner.

With the same input and baseline steering, the post-removal reverse events 11,
16, and 17 matched `/tmp/diagnostic-only-reverse.root` exactly across 900
endpoint/status scalars and 882 final-component/lineage vectors. A fresh audit
reproduced 30,285 source-3 nodes, 29,088 operation-5 nodes, 5,657 fate-7
survivors, and 58,176 smoothing edges, with zero published diagnostic nodes and
zero non-reverse final-component sources. A comprehensive verbose event-11 run
recorded `SMOOTHED MIXTURE` at hits 1 and 0 and published the terminal reverse
mixture. This is a mechanical unchanged-reverse gate, not physics validation.
