# Recoil spectra from completed shared-pass ECAL/HCAL jobs — 2026-11-18

## Request and fixed source snapshot

User reports the new production is running and requests recoil spectra from its
finished flat tuples. All500jobs had submitted and started markers at inspection;
375verified completions were frozen at2026-11-17T16:56:43Z (Nov18local).
Read all375current same-file outputs,75000inputevents, from
`higgs_eeHX_ISRFSRON_BP_ECAL_HCAL_PAIRED_500jobs_200evt_20261117/` and corresponding
`REC_ECALTRUTH_500jobs_200evt_20261112` REC files. Jobs finishing later are outside
this snapshot. Frozen cards/manifest/completion hashes and ROOT/REC identities
are saved in the study snapshot; file identities are checked before/after reads.

Artifacts (uncommitted):
`TrackingPerformanceStudies/bp_ecal_hcal_recoil_20261118/`.
No ROOT inputs copied, no native reconstruction/refit, no physics-source/card/
default/build/install change and no new production submission. Running libraries
remain untouched. The prior unsubmitted status is now superseded: campaign is
active. The native implementation/gates remain those of36c494f, documented in
`2026-11-17-bp-shared-calo-comparison.md`.

## Common event selection and reconstruction

There are73741baseline-valid opposite-charge production-electron pairs after
1156incomplete selected pairs and103BaselineKF charge/state failures. Both BP
curves retain exactly these pre-window event IDs, without an all-hypothesis-valid,
GSF, neutral-reference or fit-success gate. Current REC supplies actual bare KF
IP directions and momenta; all73741recoil values reproduce the cached baseline
exactly, including NaNs. The prior event cache supplies cross-check identities,
categories and bare KF masses only. Every plotted BP result is freshly read
from the NEW same-file production, rather than old fitted BP files.

For each leg/mode choose first available saved result in the user-agreed chain:
110+FSR → actual-applied110 → shared100 → genuine000 → BaselineKF.
Only successful110+FSR adds its OWN fitted photon once. Statuses, masks, saved
states and physical per-leg p4 determine availability, never recoil mass/window
or truth radiation type. Native000status3 is counted asKF. ECAL-only110 has no
EDM index; its saved state/status/masks are the availability contract. Shared
upstream and primary states keep normal native index checks. Undefined combined
recoils remain outcomes, not triggers for another mass-based fallback. No extra
measured-neutral p4 is added to any electron curve. H0/HF remain parallel models,
not steps of the upstream chain; no new online selector is introduced.

Independent native photon truth checks reproduce the prior event categories.
For each selected electron sum photon BIRTH energies by type; a type is present
if either electron's sum is strictly>1GeV. Both meansAND, with opposite legs
allowed. No ECAL MC-weight/purity inference defines these tags. Category pairs:
Neither16188,eBrem-only32798,FSR-only8592,Both16163;sum73741.

## Figures, normalization and observed counts

SixPNG/PDF pairs use120–140GeV,0.1GeV bins: inclusive10ab^-1yield; inclusive
shapes; four category shapes. Gray-filled bare electronBaselineKF, black-dashed
synthetic muon reference, gold solid/circle ECAL-only BP110+FSR and green
solid/triangle ECAL+HCAL. AGORA simulation outside axes, square panels, transparent
grid, large text, no titles/fit lines/generator recoil. All six renders reviewed.

Muon reference is the previously user-requested fixed-seed Gaussian MASS smear
calibrated to1.25times the fitted DSCB core width (achieved1.248492); same48636
selected muon pairs and50000generated-input denominator as before. Label says
core×1.25; this is not a new muon reconstruction/detector-resolution result.

Inclusive weights use all represented generated inputs before selection/window:
10ab^-1 ×7.04fb /75000 =0.9386666667per electron event;
10ab^-1 ×6.77fb /50000 =1.354per muon event. Established channel reference cross
sections are reused without extra branching factors. Category/inclusive shape
curves independently normalize to their OWN finite120–140window counts. Legends
show raw windowN for shapes and expected yields for luminosity. The inclusive
muon reference is repeated unchanged across categories.

| Curve |120–140|120–130|Undefined|Expected120–140at10ab^-1|
|---|---:|---:|---:|---:|
|Electron BaselineKF|49351|32138|23|46324.139|
|ECAL-only BP110+FSR fallback|59312|48466|70|55674.197|
|ECAL+HCAL BP110+FSR fallback|56933|47663|70|53441.109|
|Synthetic muon reference(core×1.25)|44243|36908|11|59905.022|

The HCAL spectrum visibly has more low-side response in this snapshot. No DSCB
core-width estimate or resolution-improvement claim is made. Each mode's route
composition also differs on these same events:

|Mode|110+FSR|110|100|000|KF|
|---|---:|---:|---:|---:|---:|
|ECAL-only|129183|5620|5575|752|6352|
|ECAL+HCAL|132057|2747|5558|767|6353|

Both totals147482legs. The comparison includes both observation changes and
changed fallback availability. Truth-assisted material locations, no scans,
window±25/±200mrad, HCAL×4 and sigma_H=.30√E_H+.07E_H remain native settings.
A completion-selected subset may depend on runtime, so results are provisional
until the full production completes. Fixed-input normalization does not remove
that potential population bias. HCAL response/covariance limitations remain.

## Verification and gallery

Self-check explicitly covers failed FSR, absent secondary EDM index, independent
110 validity and shared KF fallback. All375files passed source identity, native
mode, paired identity, physicalp4, REC baseline and photon-truth checks. Category
raw histograms partition inclusive exactly; all curves share pre-window keys;
full-input weight identities and source hashes pass. `events.csv`, `tracks.csv`,
per-job results, `histogram.csv`, `category_counts.csv`, `read_summary.json`,
`summary.json`, `validation.json`, `main_review.json` retain the evidence.

The complete gallery workflow is delegated through the personal plot-gallery
skill to one dedicated agent. Publication adds one new Nov18daily study section,
with notes and6previews together, preserving all existing39studies/135explicit
image entries, day ordering, retired-preview flags and original URLs. Actual
HTTP200 verified for root,Nov18page and all6newPNGs. Live galleryPID715687 on
ailogin002.ihep.ac.cn already served the correct root and was retained. Launcher
status has a stale PID record; host ps/ss and HTTP verify the actual process.
Final catalog:5days,40studies,141explicitPNGentries;old39/135 intact. Root validates
2484PNGs/12sections. URL:http://127.0.0.1:8000/daily/2026-11-18/.
Study gallery_verification.json and delegated report retain the proof.

## Knowledge preservation

Only Current focus is replaced. Full outgoing AGENTS is preserved in
`2026-11-18-agents-before-completed-calo-recoil.md`. Active global status/laws/
commands remain byte-identical. No historical record or old plot is removed.

## User follow-up: left-shift interpretation

User notes that the HCAL recoil peak moves left. Matched saved4p check confirms
median fitted visible-energy increase+0.497930GeV across73741pairs and median
finite-pair recoil shift−0.624821GeV. Restricting only this diagnostic to56106
events with both modes/legs successful110+FSR preserves the effect: median
ΔEvis+0.538921GeV (99.9786%positive);56077finite paired recoils have median
ΔM−0.677938GeV (99.9822%left). These are paired shifts, not fitted peak positions.
Main six spectra still retain all73741baselinepairs. Full data in
paired_energy_shift.json; no extra plots, fits or source changes.

This shows an upward fitted visible-energy pull, and that it persists with
identical successful-fit routes. It does not identify the standalone HCAL scale
as excessive. Existing ECAL response corrections and joint calorimeter response/
correction overlap require checking before calibration claims.
