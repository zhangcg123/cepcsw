# GSF rerun on the same REC events as the BP 200 x 50 campaign

The user requested regenerating GSF with the repaired multi-track tuple writer,
using exactly the REC events used by the BP flats:200 jobs x50 events. They
selected short walltime. Prepared a fresh, separate campaign:
`higgs_eeHX_ISRFSRON_GSF_MATCHBP_200jobs_50evt_20261110/`.
No batch submission, fit, source/card/library modification or input deletion
is performed by this preparation. Prior project focus is preserved in
`2026-11-10-agents-before-matched-gsf-batch.md`.

All200 BP manifests and checksummed cards match corresponding source REC
manifests:jobs1--200, entries0--49, each REC containing200 events. BP cards
have no selected-event restriction or skip. Both REC and existing BP output
paths exist for all200 jobs. Six samples(1,4,50,100,150,200) directly verify
REC200-entry counts and BP neutral_pfos.event_index=0..49. An initially
exhaustive ROOT metadata scan was stopped after about40 files to avoid needless
metadata I/O; complete manifest/card matching plus bounded ROOT checks supplies
the exact input/range proof. No running Condor job was stopped or touched.

Physics copies the previous GSF cards exactly, apart from output paths and
EvtMax200->50:reverse, production-pair selector on, fresh inward seed-1,
directional seeds1, both BH gates on, CEPC BH model, DD4hepBetweenSurfaces,
look-ahead0, LocalMeasurement, MaxComponents10, cutoff1e-4, beam on, ECAL off,
oracle/material recorder off. No new fitter property or steering option.

Resources:cms group,4000MB, explicit hep_sub `-wt short`. The installed local
hepjob CLI defines that option and its cms_short configuration is1800seconds
(30minutes). Five completed old200-event jobs(1,2,4,10,50) took91.8,91.4,54.4,
61.1,85.8minutes. A linear50-event estimate is roughly14--23minutes; this is
not a50-event benchmark or guarantee of the30-minute limit.

The repaired automatic *_track_* vectors retain both electrons' independent
AtIP directions, five parameters, pT and original CompleteTracks index. Existing
pre_ecal_* snapshot fields retain pre-reclustering energies/positions, hit counts,
and PFO charge/associations; no calorimeter hits are duplicated. Full component
and lineage payload remains unchanged. Physical-event join is GSF iev-1 = BP
event_index; track join uses original input index. BP final-PFO cluster indices
must not be equated with pre-reclustering indices.

Fresh hashes pin the installed tested writer(c26c261...), unchanged fitter,
truth libraries/dictionaries, tuple metadata, setup script and new worker/
submission scripts. Old frozen campaigns and their hashes are unchanged. The
worker accepts200 source rows but processes/verifies50 output rows. It requires
all40 new endpoint-vector branches plus pre_ecal_* and selection/output maps,
checks ownership/lengths/finite valid states and inactive views, and writes a
completion marker only after verification. Existing outputs are never overwritten.

## Verification and command

All200 generated cards compile, preserve prior physics source text exactly
after normalizing only output paths/EvtMax, and match BP input path/count.
All runtime/card/BP-provenance hashes pass. Dry run prints exactly200 commands,
each with cms,4000MB and -wt short. No submitted/started markers or new GSF
outputs exist. The exact new worker output-check block passes on18 native
fixed-writer rows:10 selected tracks,9 published,876 ECAL clusters, all40
endpoint vectors. This is not a fresh50-event runtime/resource pilot.
Local Python has no htcondor module, so no submission is attempted locally.
Run in the usual batch-login shell from the repository root:

```bash
bash higgs_eeHX_ISRFSRON_GSF_MATCHBP_200jobs_50evt_20261110/submit.sh --submit
```

Omit --submit for frozen validation and commands only. After actual submission,
freeze the normal installed dependencies and batch-overlay writer paths; use
isolated locations for separately authorized development. No old batch helper,
card, product, source or installed library is altered by this preparation.
Generated cards/scripts/check logs/ROOT fixtures remain uncommitted artifacts.
