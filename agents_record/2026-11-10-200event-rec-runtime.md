# 200-event REC campaign runtime inspection (2026-11-10)

Read-only inspection of
`higgs_eeHX_ISRFSRON_REC_500jobs_200evt_20261109` in response to the user's
slow-production question. No jobs were submitted, stopped or modified; no
builds/installations or run-card changes were made.

## Observed completion and timing

All 500 jobs have submitted/started markers; 497 have completed.json, which
the worker writes after checking the final REC and its requested event count.
No fresh full-ROOT-file audit was performed for this inspection.
Runtime below is completed-marker mtime minus started-marker mtime; queue
delay is started-marker minus submitted-marker mtime. It includes worker
execution/verification, not just algorithm CPU time.

| Metric | Minimum | Median | 90th percentile | Maximum |
| --- | ---: | ---: | ---: | ---: |
| Completed-job runtime (hours) | 2.656 | 5.307 | 6.081 | 6.724 |
| Marker-based queue delay (hours) | 0.018 | 0.702 | 1.897 | 2.334 |

167 completed jobs finished within 4 hours, 174 within 5 hours, 429 within
6 hours, and all 497 within 7 hours. Median runtime corresponds to about
95.5 seconds/event including initialization/verification. Earlier killed
1000-event jobs reached median 415 events in 10 hours: linear extrapolation
to 200 events is 4.82 hours. The current median is about 10% longer, with
substantial host-dependent variation. The earlier comparison is approximate
and not a same-event/same-host performance measurement.

## Strong host dependence

Each source seed differs, but every job uses the same frozen workflow and
200-event count. Completed jobs cluster tightly by worker host:

| Host | Completed jobs | Median hours |
| --- | ---: | ---: |
| hxmt129.ihep.ac.cn | 4 | 2.87 |
| bws0988.ihep.ac.cn | 19 | 3.07 |
| hxmt127.ihep.ac.cn | 14 | 3.09 |
| bws0986.ihep.ac.cn | 30 | 3.13 |
| bws0970.ihep.ac.cn | 8 | 5.25 |
| herdws003.ihep.ac.cn | 13 | 5.93 |
| awsbm001.ihep.ac.cn | 6 | 6.45 |
| hxmt119.ihep.ac.cn | 3 | 6.54 |

There are 65 completed-job hosts. Pearson correlation of runtime with final
REC size is about 0.014, and with start-marker time about -0.022. The first
100 starts have median runtime 5.320 hours; the last 100 have 5.350 hours.
This strongly identifies worker environment as the dominant observed source
of spread, rather than a universal regression or the campaign start wave.
It does not identify CPU model versus contention, CPU quota, memory pressure
or I/O as the underlying host-specific cause; no live CPU/RSS accounting is
available in these logs.

## Three unfinished jobs

Jobs 69,311,442 all started on aliws002.ihep.ac.cn. Their latest complete
event indices at the inspection are respectively 133,170,155 (zero based);
subsequent Processing event lines exist. Job 69 and 442 logs grew between
short inspection reads. None has a completion marker, so retain partial REC
files as unverified, not analysis inputs. No fatal/time-limit/OOM signature
was found in the last 4096 bytes of any of the 500 stdout logs; the three
unfinished stderr files contain environment setup messages only. Lack of a
kill message is not authoritative queue status. No queue tool is available
in the present shell, and no live scheduler query was obtained.

These jobs run sim,trk,calodigi,rec only, in one Gaudi event loop. They do
not run GSF, breakpoint minimization or a breakpoint location scan. Merging
stages removes intermediate writes/reads, but preserves full simulation and
reconstruction work per event; reducing events/job does not accelerate each
event. The frozen cards do not enable per-algorithm timing, so no measured
stage CPU breakdown is available. Absolute log-banner clocks and filesystem
marker clocks differ; the reported durations use marker mtimes consistently.
