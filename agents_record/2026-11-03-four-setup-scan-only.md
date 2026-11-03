# Four-setup location FIT scan, without online ranking

## User clarification and scope

The user explicitly rejected the proposed online ranking: scan every location
with 000/100/110/111, save the optimized fits, analyze/select later. This
supersedes the earlier ranking-based campaign, not its historical results.
The user reconfirmed no active jobs and authorized install/refreeze after
validation. Branch remains breakpoint-delayed-neutral; no remote changes.

Added opt-in IntervalSelectionMode=HitScanAll, not another fitter. The new
LocationFitScan record/orchestrator lives beside HitLocationScan and calls the
existing BreakpointFitter/FreeLossFitter. There is no new KF, RTS, loss map,
propagator, variance treatment, optimizer or acceptance rule. Existing Truth,
Manual and ranked HitScan remain available; compiled/card default remains Truth.
No changes to shared KF, GSF, simulation, PFA or rec data products.

## Workflow

1. Read CompleteTracks plus existing matched truth/PFO/ECAL inputs.
2. Plan points: a separate 5D no-breakpoint control, then every reconstructed
   VXD/ITK hit (IDs1/2/3), excluding the last hit. All TPC/OTK measurements remain.
3. A truth owner already in silicon references that point. An eligible
   non-silicon owner adds one reference-only point at the end. Unavailable truth
   does not alter the silicon list. No truth loss amount is supplied to a fit.
4. At each point independently fit 000, then 100, 110, and 111. Every new fit
   starts from scratch with the accepted observation set and the ORIGINAL
   prior settings. No earlier optimized prior center, state or covariance is
   passed as new prior information. Gaussian SigmaLogLoss remains0.001, with
   prior-center optimization over[0,1]. Existing optimizer acceptance applies;
   this is not a guarantee of the global mathematical optimum.
5. Missing/failed new observations inherit the preceding accepted setup at
   that SAME point. Saved source, actual mask, attempt status and error make
   this explicit. Missing neutral copies110; failed110 may retain100.
6. Record all fits. NO location ranking, gain cut, winning location or
   selected-site track publication. Choose locations later in analysis.

HitScanAll requires FreeLossFit=True, all three input switches ON and
TruthOverride=False. ExperimentalDelayedNeutral steers111 and is ON in the
dedicated batch launcher. Existing charged ElectronPhoton/Only routing is
unchanged. The input switches enable observations for this four-setup study;
individual tracks may still lack those measurements.

The no-breakpoint control has NO loss coordinate. Its111 entry therefore
inherits110 even when a neutral cluster exists. No zero-photon likelihood or
extra pseudo-measurement is invented. Do not calculate a neutral-constrained
null-versus-loss NLL gain from that inherited control. The requested task is
recording fits, not deciding whether the track has eBrem.

## Flat data contract

One main breakpoint-tree row per input track, as before. The location_scan_*
vectors are aligned, one entry per `(point,setup)`, four consecutive entries
with setup labels `[0,100,110,111]` at each point.

| Field | Meaning |
|---|---|
| point, kind, hit, system, layer | Point ID, kind0 null/1 silicon/2 truth-only, ordered-hit index and cellID system/layer |
| setup | Requested conventional setup label, not a rank |
| actual_constraint_mask | Actually used beam=1, charged=2, neutral=4 |
| source_setup | Setup that actually supplied the accepted fit |
| fit_status | 2 new fit,1 inherited valid fit,-2 no valid fit |
| attempt_status | 2 new fit succeeded,0 no new fit,-2 attempted fit failed |
| minuit_status, attempt_minuit_status | Saved fit's optimizer status versus the new attempt's status; -99 if no optimizer |
| nll2, chi2, log_determinant, measurement_dimensions | Normalized -2logL and its existing full-likelihood components |
| pt, pt_error | IP pT and uncertainty from that same fit |
| prior_center | Accepted optimized Gaussian prior center, not a fixed physical b |
| fitted_log_loss, fitted_variance | Posterior b and variance; null has no b |
| error | Failed-attempt/unavailable-observation or inherited-fit explanation |

Track-level location_scan_status=1 means recorded, not physics correctness or
all optimizations succeeded. point_count counts UNIQUE points, not vector
entries. truth_point identifies the shared point for four truth references;
truth_hit and truth_status/error retain association status. Truth status1 means
matched but no eligible owner (null reference),2 means eligible owner,-1 means
unavailable. It is not a count of emitted photons or a truth no-eBrem classifier.

Main status=2 and interval_selection_status=6 denote scan-only rows. Selected
intervals are empty. Normal refit collections are empty and their endpoint
branches remain NaN/inactive. Input KF, matched MC, per-hit geometry and PFO/
cluster observations remain. No interpretation of rts_pt/free_loss_rts_pt as
the best scan fit is permitted in this mode. Use location_scan_pt instead.
Legacy hit_scan_* ranking fields remain booked but inactive (status0).

For later analysis, compare like actual observation sets. A requested110 with
actual mask1 after a fit failure does NOT have the same measurement likelihood
as a successful110 with mask3. All inheritances are retained as data, not used
to rank locations. The previous 2-sigma charged-energy compatibility fallback
remains an analysis-level choice, not an online rejection rule in this scanner.

## Tests and evidence

- Module freshly built and staged under /tmp; existing external-header and
  filesystem clock-skew warnings retained, no workaround in shared code.
- 56 Python batch/card tests passed, including the new mode, required schema,
  MC association loading and a 50-job scan-only planning/dry-submit fixture.
- 2 freshly built standalone CTests passed. Pure point-planning tests cover
  silicon scope, last-hit guard, truth reuse/outside reference and reset.
- Existing Truth-mode events1:11,16,17,38 reproduce all962 previous branches
  across both trees exactly, NaN-aware. The shared row-finalization refactor
  and new inactive branches do not perturb existing physics endpoints.
- Event1:5 (collected neutral) records9 points/36 setup entries. Its truth
  owner is point4, hit3. A separate Manual hit3 run matches ALL FOUR setups
  exactly in pT, pT error, optimized center and posterior b/variance, and NLL2
  within1e-7. Delayed111 is exercised with actual neutral measurement.
- A separate Manual no-breakpoint run for1:5 matches all controls. The111
  control is explicitly an inherited110, not an artificial neutral-null model.
- Event1:38 (no collected neutral) records10 points/40 entries, truth point6
  at hit5. Every000 and110 silicon NLL reproduces the earlier single-setup scan
  exactly. All ten111 entries reuse110 and carry inherited status/actual mask.
- Both cases have main status2, no selected interval, inactive ranking fields
  and NaN ordinary/free refit endpoints. NLL decomposition, vector alignment,
  no-pruning point coverage and batch schema checks pass.
- Known failure event2:45 exercises100 failure at h0 (retains000, actual mask0)
  and110 failure at h1 (retains100, actual mask1). Both retain the SAME point
  with explicit errors; saved NLL, pT, errors and loss statistics match their
  stated source exactly. After a failed earlier constraint, later stages use
  only the previously accepted observations, as in the maintained workflow.

- A real Higgs event records3 independent track rows with10/1/8 points
  (40/4/32 entries). The no-silicon track is retained with its null controls.
  Per-track point numbering/vectors reset correctly, no online interval is
  selected, and every row retains the input-track index. This is a topology
  control, not a single-track optimization/performance sample.

No efficiency, purity, global-optimum or population pT validation follows from
these mechanism checks. All local Gaudi applications terminated successfully.

## Batch cards and deployment

Batch layout was subsequently changed by the user to **200 jobs x50 events,
group cms**. Current details/command are in
`2026-11-03-hit-scan-batch-200x50-cms.md`. The 50x200 preparation below is
historical and archived; scan-only implementation/data contract are unchanged.

The existing50 unstarted/unsubmitted ranked-scan cards were checksum/marker
checked and MOVED recoverably to
higgs_silicon_scan_20261103/runcards_before_scan_only_20261103/.
Archive README warns that their absolute paths are historical and they are
not directly runnable. No ROOT input/output was deleted. Regenerated50 current
card/manifest pairs using the maintained helper; all checksums and controls
match HitScanAll,111 inputs/delayed neutral, no truth loss override/backwards.
Same output directory, 50x200 existing Higgs events, group higgs, no scheduler
submission. Dry submit validated. External rec files are retained.

The validated package is installed in the project-default runtime. Submit the
refreshed cards explicitly:

```bash
./subbreakpointhiggsscanjobs.sh submit-prepared
```

Test area /tmp/bp-scan-only-9xx5YM contains staged runtime, build/test logs,
all5.root/log, all38.root/log, manual5.root/log, null5.root/log, legacy.root/log,
failure45.root/log, Higgs smoke, run_case.py, validate.py, validation.json and
extra-validation files. Temporary data may expire; no generated ROOT files,
cards or logs are committed.

Tested library SHA256:
f16257eb68ad4eb40aa8c6e3dff3b91c20296fd5cd21e9ea2f8f15722c7b9383.
Previous installed package:
75c5edca3a7510e80b99721b3ebd98fd27efa6e05b81dce25324aba168b9b0ae.
Installed package is byte-identical to the tested staged library above. Only
the RecBreakpoint package library, configurable and discovery metadata were
installed; no /tmp runtime override is used by the batch worker. A fresh
project-default EvtMax0 probe loads the actual frozen HitScanAll card with an
isolated output path; it is configuration/schema verification, not a physics
test. No Condor jobs were submitted and the full10k batch was not run locally.

## Memory preservation

Complete outgoing AGENTS preserved in
2026-11-03-agents-before-scan-only.md. Heading map: section1 unchanged;
section2 replaced by scan-only focus, with its entire previous text in the
snapshot. Both substantive sections remain. All active laws, scope, compile/
run instructions are unchanged; ownership guards and the open acceptance-audit
prohibition remain explicit. No history directory moved, no migration manifest
required, no history deleted. Previous ranking/batch/111 steering records remain
historical evidence, not the active scan-only contract.
