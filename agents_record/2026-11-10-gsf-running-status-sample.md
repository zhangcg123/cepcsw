# Running GSF campaign: bounded status inspection

The user confirmed the GSF tuples are running and prohibited submitting BP
jobs before they finish. Only five job outputs/logs plus the three existing
failure records were inspected in detail. No scheduler query is available in
this session; marker states are not an authoritative queue inventory.

Campaign: `higgs_eeHX_ISRFSRON_GSF_TPCFIX_500jobs_200evt_20261110`. Snapshot time: 2026-11-10T10:12:36.339305+00:00.
500 submitted/started markers;0 verified completion markers;3 failure markers.
The other497 jobs are not individually confirmed live through the scheduler.

| Job | Worker | Minutes since recorded start | Latest logged event index (0-based) | Output MiB |
| --- | --- | --- | --- | --- |
|1|ams019.ihep.ac.cn|34.7|69|591.2|
|2|ams017.ihep.ac.cn|34.6|71|599.2|
|4|aws004.ihep.ac.cn|35.1|118|1069.5|
|10|lhws310.ihep.ac.cn|35.1|120|977.6|
|50|lhws279.ihep.ac.cn|35.0|82|616.0|

These are lower bounds on progress, not exact completed-event counts.
All preceding event indices have passed through the sequential loop, while
the last logged event can still be in progress. Warning-level logs do not log
every event. All five ROOT files grew between two reads, supporting ongoing
activity. Read-only uproot opening found no saved gsf_tuple tree key in these
live files; no ROOT recovery, forced autosave or file mutation was attempted.
Absolute filesystem mtimes can be offset; elapsed values use the recorded
worker start timestamp and host clock rather than log-banner/mtime differences.

Failed jobs87,144,195 each report native exit139 and segmentation violation.
Latest warning-event indices are74,14,2; recorded runtimes before failure are
14.7,10.6,2.1minutes respectively. These indices do not identify the crashing
event. All three stacks enter podio::ObjBase::release during event-object
cleanup: SimTrackerHit/MCParticle destruction for144/195 and tracker-association
destruction for87. This is an observed crash site, not an established cause or
proof of a PODIO defect. No memory-limit or walltime-kill diagnosis is established.
Other sampled logs contain non-positive-definite matrix messages and individual
track/component rejections while jobs continue; these are distinct from a
whole-job crash. Fit efficiency and live RSS were not measured.

No source, runtime, cards, libraries or production outputs changed. No jobs
were stopped, retried or submitted. Keep the normal installation and private
GSF ECAL tuple overlay frozen while these jobs run. BP submissions remain
explicitly deferred until GSF finishes. JSON details are an uncommitted campaign
artifact: sample-status-20261110.json.
