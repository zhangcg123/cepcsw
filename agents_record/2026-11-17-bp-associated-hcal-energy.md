# Associated pre-HCAL energy in BP — 2026-11-17

## User-approved definition

Use pre-ECAL and pre-HCAL from the same selected PFOs for the BP calorimeter
observation. Charged: exact selected-track PFO association. Neutral: trackless,
zero-charge PFO with at least one eligible ECAL cluster in the existing window.
Do not newly select HCAL-only PFOs. Add each linked pre-HCAL cluster once,
calibrated by4; never add the saved PFO energy on top of its ECAL energy.
The user specifies sigma_H/E_H=0.30/sqrt(E_H)+0.07 with GeV units, terms linear.
The implementation evaluates that model per calibrated HCAL cluster, matching
the existing per-cluster ECAL convention. Variances add independently.

## Implementation and compatibility

New inline HcalEnergyCandidate helper unions HCAL cluster identity across selected
owners/links and retains raw/calibrated energies, uncertainties and IDs. Existing
charged/neutral ECAL selectors additionally expose selected PFO indices without
changing their energies/statuses. Neutral PFO ownership is retained even if its
selected ECAL cluster was already counted through another owner.

RecBreakpoint adds IncludeHcalEnergy (compiled false), HcalEnergyScale=4,
HcalStochasticError=.30 and HcalConstantError=.07. The maintained card and batch
helper explicitly enable HCAL, freeze all four controls, and preserve overrides.
BP_INCLUDE_HCAL=0 is the ECAL-only control. Compiled false deliberately preserves
already frozen old cards. Enabled HCAL requires pre-reclustering input and
charged ECAL from that snapshot. Missing HCAL collection or invalid associated
HCAL energy fails explicitly. No geometry-based HCAL selection is invented.

Combined charged/neutral observations enter existing shared paths for H0/HF,
110/material-FSR and optional111/scans. No fitter parametrization, tracking,
truth matching, original PFA algorithm, GSF card or source changes. HCAL used in
the charged observation is excluded from the neutral observation of that track.
Different track windows can share physical observations, as before: there is no
new correlated joint-track likelihood or online event-level photon ownership.
Energy remains an effective calorimeter observation at the existing fit site.

Raw ECAL measurement branches remain ECAL-only. New branches record selected PFO
IDs; charged_hcal_/collected_neutral_hcal_ energy/error/IDs/raw-energy/calibrated-
energy/per-cluster-error; charged_calo_/collected_neutral_calo_ energy/error;
and enabled/scale/resolution metadata. Historical fit-internal `ecal` names refer
to the combined observation when enabled. No raw hits are added to the tuple.
No HCAL retains original observation/error arithmetic exactly.

## Verification

Artifacts `/tmp/bp-hcal-20261117/` contain old/off/on cards, outputs and verbose
native trajectory dumps, check.py and validation.json, package rollback files
and install fingerprints. Logs `/tmp/bp-hcal-{build,final-build,unit-tests,ctest,
batch-tests,batch-final,install}.log` retain command results.

- Build succeeds; all19BP CTests pass. Candidate tests cover calibration once,
  duplicate relations, multiple ECAL clusters/owners sharing HCAL, no-owner
  and cross-electron exclusion, explicit charged/neutral exclusions and invalid
  calibration. Existing ECAL-only selector checks still pass.
- All67working-tree batch tests pass, including the new freezing/disabled/invalid
  HCAL tests. Existing unrelated batch/test changes are preserved separately.
- Old installed binary versus new-code HCAL-off: all99,204pre-existing branch-row
  values agree exactly, NaN-aware, across94track rows and event-side rows.
- New same-code HCAL-off/on:26,414tracker-only, beam-only and raw-ECAL branch-row
  values agree exactly. All saved HCAL calibration, individual and summed errors,
  combined observations and duplicate/exclusion checks pass. Independent HF
  observed energies/errors agree with the same combined input.
- Current REC source1 events3/11/16/17 contain8selected legs. Their charged-HCAL
  energies are0.766172,0.037637,0.095223,0.775326,0.734644,0.104789,0.606297,
  0.071985GeV. Two legs have collected neutral-HCAL0.019833/0.020855GeV.
- HF status is2for7legs; event11/track12 retains status-2. Material-FSR is2for6,
  -2for11/12 and0for17/6, identical old/off/on. No existing failure is hidden.
- These are four current Higgs events, not the unavailable historical gun
  hard-loss benchmark, and not population/recoil-performance validation.

RecBreakpoint-only installation refreshed after numerical checks. Five previous
package files are backed up in previous-install. Installed plugin SHA256
685501046a02cb1c1f72ddd9c4fca3e3bc1471f71902b736a571fb549e550163 matches the
tested build. Compiled configurable still defaults HCAL off; new maintained
card/batch explicitly enables it. Existing production manifests/cards/ROOT files
are unchanged. Installed-path source1/event3 rerun with111enabled covers two
selected tracks. Track25has111constraint_status2/mask7andfloatingFSRstatus2;
track5has no resolved neutral and correctly retains110(mask3). All5920
measurement/H0/HF/110branch-row values agree with the same event in the
110-only run. installed111_validation.json retains this check.

## Evidence motivating the change and limits

TrackingPerformanceStudies/hcal_omission_20261117/ contains the independent first
25currentRECfiles/5000event measurement:9938selected tracks,2876collected neutral
PFOs; charged-associated HCAL mean0.818GeV and15.73%above1GeV, neutral-associated
mean0.0382GeV and0.10%above1GeV. These are reconstructed associations, not proven
electron leakage. Twenty-one charged ECAL sums are nonfinite, explicitly reported.

Direct REC source1/200event check of8154finite neutral PFOs (5870withHCAL) verifies
saved pre-PFO energy=sum(preECAL)+4sum(preHCAL), maximum absolute floating-point
difference2.73e-6GeV. All59913pre-HCAL clusters in that file have energyError=0;
the BP resolution is user-specified, not taken from these zero placeholders.
ECAL/HCAL covariance, calibration uncertainties, contamination and heavy tails
remain unvalidated. Neither additional collected energy nor finite fits prove
physics improvement. A larger direct categorized A/B remains future work.

The outgoing AGENTS.md is preserved completely in
2026-11-17-agents-before-bp-hcal.md. Global status/laws/commands prefix remains
byte-identical; only current focus is replaced. No history-directory move occurs.
