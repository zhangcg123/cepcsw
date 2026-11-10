# GSF flat tuple: all published tracks' AtIP endpoints

## Authority and defect

After the user confirmed the Condor jobs were finished, they requested fixing
the GSF writer's single-track bug. No additional idle-job confirmation is
needed for this change. This does not authorize new batch submission. The
previous project status is preserved verbatim in
`2026-11-10-agents-before-gsf-track-endpoints.md`.

GSF already computes and publishes independent AtIP states for every successful
track. RecGsfFlatTuple's legacy scalar endpoint families read only collection
element0, omitting the other selected electron's direction. Component vectors
retain both pT values but not the missing angles. This is a tuple-writing
omission, not an algorithm limitation. Current old-campaign recoil plots still
use the explicitly labelled GSF-pT plus Baseline-KF-direction approximation.

## Implementation

Only the tuple writer/header, its README and a focused native-output checker
change. No fitter, selector, configurable property, material/BH model, shared
tracking/simulation/PFA source or run-card steering changes. The existing43
RecGsfTracking properties remain unchanged; the option-change sub-agent audit
rule does not apply.

Each endpoint prefix (`gsf`, `bestbranch_gsf`, `weighted_gsf`,
`fullmixture_gsf`, `ecal_gsf`) gains eight vector branches:
`_track_input_index`, `_track_valid`, `_track_d0`, `_track_phi`,
`_track_omega`, `_track_z0`, `_track_tanl`, `_track_pT`.
All published tracks are retained in their output order and keyed by original
CompleteTracks index. Failed fits do not shift the association to the surviving
selected electron. Absent collections produce empty vectors. Every event clears
all vectors. Each row copies the native AtIP state, without a KF direction or
other-surface fallback. Missing AtIP states are NaN/invalid; unusable pT is NaN.
Inconsistent collection/map lengths, duplicate or out-of-range input indices
fail rather than writing guessed ownership. Historical EDM without output
mapping cannot supply the new identified endpoint schema.

Legacy scalar branch names/types/first-track semantics remain compatible.
The automatic40-vector extension needs no run-card switch. Existing files do
not gain the omitted data; rerun only GSF from the existing REC to produce
exact two-GSF-track recoil. Photon truth still joins the matching REC.

## Verification artifacts

Artifacts remain uncommitted under
`build.gsf-ecal-tuple-dev/validation/per-track-endpoints-20261110/`.
The new writer is first built and installed to a separate `new-install` there.
Baseline/new cards use the same fitter and REC job4 with selected source events
0,2,11,16,17, comprehensive component/split/lineage dumps, and18 event rows.
Event2 exercises a failed first selected track with only the other published;
skipped event rows exercise empty-vector reset. A forward control uses events
0,2 over3 rows. Test-only PODIO outputs retain native states and input indices
for independent comparison to all new flat values.

The new durable test is
`Reconstruction/RecGsfTracking/tests/check_track_endpoints.py EDM TUPLE
[--baseline OLD_TUPLE]`. It compares every endpoint/parameter to the same run's
native TrackState and input map, checks validity/pT/scalar closure and empty
views, and optionally compares every old branch exactly. This is mechanical
persistence validation, not physics validation or a population study.

## Results

Both old/new reverse executions exit0. New direct-EDM checks pass18 event rows,
27 endpoint records (9 published tracks x3 reverse views), output multiplicities
0/1/2, all five native parameters, derived pT, validity, original input maps,
and first-entry scalar closure. Every257 pre-existing baseline fields agrees
exactly, including complete component/lineage data. The fresh baseline used the
normal installed legacy writer (257 branches); the new writer has309 branches,
adding the prior12 passive ECAL fields plus40 per-track vectors. The separate
REC comparison verifies all876 ECAL clusters and876 PFO/ECAL links exactly.
The existing production-selector/lineage checker also passes all five events.

| Source event | Selected original tracks | Published tracks |
|---|---|---|
|0|9,18|9,18|
|2|1,3|3|
|11|2,8|2,8|
|16|3,5|3,5|
|17|22,23|22,23|

For event0 the independent saved BestBranch directions are
phi=(3.1379079818725586,0.20362697541713715) rad and
tanLambda=(-0.11271366477012634,0.347546249628067).
The one-output event2 correctly saves input3 with phi=-2.0277202129364014,
tanLambda=-2.0314788818359375; it is not labelled as failed input1.
The pre-existing one-fit failure is preserved, not repaired by this writer fix.
Forward control exits0 and passes3 event rows,4 native endpoint states,
output multiplicities0/2, and empty inactive paired views. The unchanged
legacy scalar behavior is intentional; multi-track consumers use the vectors.

Checker commands are in the README. Native cards/logs, EDM/flat outputs,
endpoint-check/ecal-check/selection-check logs and hashes are in the artifact
directory above. An initial checker attempt used uproot's ignored file_handler
option and stalled; it was stopped. The maintained checker uses the established
MemmapSource handler and trivial executors; both corrected checks pass.

## Build and installation

The tuple module is built in both the private standalone C++17 build and the
normal configured build. Only the writer DSO is installed into the normal
InstallArea, using copy_if_different from its normal build; no full-repository
install or fitter/shared-package replacement is performed. Private overlay
installation uses its dedicated standalone cmake install. No CMake, options or
workflow-card source changes are needed. Installed module hashes:

- batch overlay `build.gsf-ecal-tuple-dev/install/lib/libRecGsfFlatTuple.so`:
  c26c2618652d6b8ca7fba4696055701dc0c298a422c9ad2219b8f94612a29678,
  exactly the candidate library used by the successful native checks;
- normal `InstallArea/x86_64-el9-gcc11-opt/lib/libRecGsfFlatTuple.so`:
  c50b44dfd5869f9fa64312894eff421b787c8ca94cbd1591ce8446131fdc00ea,
  exactly matching the normal-build module;
- unchanged fitter `libRecGsfTracking.so`:
  6a37fc59ba7b069ceb1dfe8bb0d8f26cb90e843d806af6888268abd57c4eecad.

No new campaign is submitted and no existing physics files are rewritten.
Frozen campaigns intentionally retain their old hashes; do not edit hashes to
reuse them after this installation change. Prepare a fresh campaign when the
user requests regeneration. No population validation, BH/model study, old-file
angle repair or plot regeneration is included.
