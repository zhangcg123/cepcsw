# GSF-only batch from completed TPC-fixed REC, with passive ECAL tuples

## Scope and input status

The user requested preparing a new GSF batch using the completed normal-ISR/FSR
REC production, after establishing that pre-reclustering ECAL measurements
must be included. All 500 REC jobs in
`higgs_eeHX_ISRFSRON_REC_TPCFIX_500jobs_200evt_20261110/` have submitted,
started and completed markers and corresponding ROOT files. Each manifest and
completion record matches 200 requested events and the expected REC input.
The earlier missing-htcondor preparation status is superseded by this observed
completion; the user submitted the REC campaign outside the agent session.

Prepared only:
`higgs_eeHX_ISRFSRON_GSF_TPCFIX_500jobs_200evt_20261110/`.
500 jobs x200 events, target100,000, group cms, memory8000MB. Each job reads
its matching completed `rec-higgs-N.root` directly and writes
`gsf_flat-higgs-N.root`. No simulation, tracking, calorimeter reconstruction,
breakpoint, input cleanup, BH-model change or batch submission is included.

## Passive ECAL export

Changes are limited to `RecGsfTracking/src/GsfFlatTuple.h/.cpp`, its README
and a new source-comparison test. No GSF fitter/configurable property changed;
the existing 43-property surface and maintained-card physics are unchanged.
No configurable option was added, so the property-change sub-agent audit law
does not apply. The new fields are automatic when both snapshot collections
are loaded and optional for tracker-only inputs.

The 12 `pre_ecal_*` branches store availability, original cluster index,
energy/position/hit count, PFO charge, and original PFO-to-CompleteTracks and
PFO-to-ECAL-cluster edges. All ECAL clusters are retained once per event,
including track-unassociated ones. No calorimeter hits are duplicated into the
flat tuple, no new matching is inferred, and no charged-PFO energy substitutes
for measured cluster energy. Values are the stored pre-reclustering measurements
before final PFO-dependent export calibration. ECAL constraints remain off.

The generated cards load both PFO/ECAL and their ECAL/HCAL companion snapshots
to resolve existing relations. A partial snapshot, unresolved cluster, or track
link outside CompleteTracks fails the tuple instead of guessing a relation.
The README defines every field. Existing scalar track fields retain their
historical first-track convention; this is not a per-track endpoint schema
redesign. Selection/output/component maps retain original track indices.

## Isolated build and native checks

Private tuple-writer-only build/install:
`build.gsf-ecal-tuple-dev/{package,build,install}`.
Its setup is `source build.gsf-ecal-tuple-dev/setup-dev.sh` in a fresh shell.
Only RecGsfFlatTuple is built; normal installed libraries, GSF fit code,
simulation, tracking, PFA and REC inputs are unchanged. The standard GSF
installation supplies the unchanged fitter/dependencies. Do not remove this
private install or replace its dependencies while submitted jobs use them.

Native artifacts are under `build.gsf-ecal-tuple-dev/validation/`:

- `baseline-event0`: original installed writer, unchanged maintained physics.
- `new-event0`: private writer plus passive pre-reclustering inputs. All46
  clusters and their relations match REC exactly; all257 pre-existing tuple
  fields agree exactly with the same-event baseline.
- `hard-events`: reads18 events from the NEW campaign's REC job4 and refits
  only source event indices0,11,16,17. Comprehensive verbose split/component/
  lineage dumps are saved in run.log. All876 clusters and PFO/ECAL relations
  across the18 rows reproduce REC exactly, including skipped-fit event rows.
  The existing selection/component/lineage checker passes all four events:

| Source event | Selected original CompleteTracks indices | Published |
| --- | --- | --- |
| 0 |9,18|9,18|
| 11 |2,8|2,8|
| 16 |3,5|3,5|
| 17 |22,23|22,23|

Thus8/8 selected endpoints are mechanically published in this focused test.
This is not population efficiency or momentum-resolution validation. The
passive ECAL test is `tests/check_pre_ecal_tuple.py`; it compares every value
and association to source REC and optionally all old fields to a baseline.
The actual generated worker's post-run output-verification block also passes
on the native event0 tuple. No 200-event GSF worker or scheduler job was run.

## Frozen campaign and command

The generated campaign contains500 checksum-verified, syntax-checked cards,
per-job metadata, worker.py/worker.sh, submit.py/submit.sh, runtime-sha256.json,
campaign-summary.json, README.md and submission-dry-run.log. All are uncommitted
batch artifacts. Preparation used the current maintained card with only input,
output, seed/event substitutions and passive snapshot inputs; physics remains:

- reverse, production-pair selector on, fresh inward scale-1, directional seeds1;
- both BH gates on, CEPCRuntimeCategoryAligned9Clear, DD4hepBetweenSurfaces;
- look-ahead0, LocalMeasurement, MaxComponents10, cutoff1e-4;
- beam constraint on, ECAL constraint off, oracle/material recorder off;
- warning-level production logs; final components and lineage remain automatic.

Dry run validates all500 cards/runtime hashes and prints500 hep_sub commands.
No submitted/started/completed markers exist in the new GSF campaign. Submit
from the usual batch-login shell, from the repository root:

```bash
bash higgs_eeHX_ISRFSRON_GSF_TPCFIX_500jobs_200evt_20261110/submit.sh --submit
```

Omit --submit to print commands only. The submitter requires the scheduler's
cluster ID rather than trusting the hep wrapper's zero return code. Existing
submitted/started jobs or outputs are refused. Workers verify frozen runtime
and card hashes, input/output event counts, passive ECAL-vector integrity and
production-pair output maps before writing completed.json. Partial failures
retain their output and write failed.json. Input REC files are never deleted.

The four selected fit events produced about34MiB including18 event rows.
Extrapolation is roughly0.8TiB for100,000 fitted events, dominated by the already
automatic component/lineage payload; this is not a full-campaign size forecast.
No 200-event runtime/resource benchmark or population validation was added.
At preparation the mounted project filesystem reports1.8TB available; this is
not a separate user-quota measurement. Keep normal/private runtime paths frozen
after actual submission; use another isolated build for further development.

## Memory-request correction before submission

The user questioned the inherited8000MB request because it reduces scheduling
flexibility. It was carried over from the REC production, not established by
a measured GSF requirement. All500 still-unsubmitted job manifests, campaign
summary, README and the current AGENTS focus now use4000MB. Cards, fit settings
and runtime hashes are unchanged. Time flavour remains the cms default10hours.

Earlier focused GSF measurements are:

| Native test | Peak RSS KiB | Peak GiB |
| --- | --- | --- |
| build.gsf-selection-dev/validation/gsf-card-defaults-20261110/focused/run-result.json | 1864356 | 1.778 |
| build.gsf-selection-dev/validation/gsf-card-defaults-20261110/regression/run-result.json | 1987084 | 1.895 |
| build.gsf-selection-dev/validation/new-production-comparison-sf3tovpa/gsf/run-result.json | 2175640 | 2.075 |

These are prior comparable GSF tests, not measurements of the exact new ECAL
writer over200 events. The previous new-writer validation did not record peak
RSS.4000MB is a headroom-based request, not a proven200-event upper bound.
No claim is made about the site priority formula; a larger memory request can
reduce eligible slots. No new jobs were submitted for this correction.
