# Final BP recoil with per-track upstream fallback

User requests a new section ABOVE the existing BP_H0F/theta25 section on the
November15review page, removing the common-four-hypothesis validity gate and
showing only the final hypothesis, bare electronBaselineKF and muon reference.
User explicitly selects fallback order BP+FSR→ordinaryBP110→beam100→tracker000
→inputKF, and random Gaussian MASS smearing calibrated to a25%larger fitted
muon core. H0/H0F are parallel models, not this upstream chain.

## Saved native states, no refits

Read all500current BP_H0F_THETA25_NOSCAN ROOT files and matching REC_ECALTRUTH
files,100000inputs. Thin native branches supply state/status/mask and p4;
REC provides both input-track directions. Frozen source/card identities and
selected MC/input-track joins checked. Native KF masses reproduce all98302
baseline-valid pairs;75511both-leg successfulHbetaF masses reproduce their
previous cache. All198430selected native track rows are accounted for.

Retain the original baseline-pair charge/state requirement:98437classified
pairs minus135baseline failures leaves98302. The1563incomplete pairs still
cannot form the selected Z recoil. No H0/H0F/GSF success or additional neutral
reference gate. Fallback is independent per electron, selected from saved fit
status, applied constraint masks and finite physical p4, never recoil mass,
Higgs proximity or the display window. Invalid composite recoil is recorded,
not used to choose a different result.

Only successful110+FSR uses its own electron+its own photon p4 once. All other
routes use their native electron alone; no extra measured neutral p4. Copied
upstream states in failed110/100 are not mistaken for new fits. Native000
status3meansKFcopy, explicitly counted asKF; statuses1/2are eligible000results.

| Selected leg stage | Tracks |
|---|---:|
|110+FSR|172290|
|ordinary110|7503|
|beam100|7349|
|tracker000|1016|
|inputKF|8446|

Total196604legs. Fallback occurs in22791pairs:21268ononeleg,1523onboth.
The old75068common-four-hypothesis events remain;23234additional baseline-valid
pairs are restored.186native000KFcopies are recognized. This is an offline
analysis ensemble, not a newly installed native fallback or model selector.

## Muon broadening

Reuse all48636muon pairs from the maintained50000generated-input sample,
keeping48625finite physical masses. Generate one standard-normal draw per
stored row with fixed seed20261116, apply it to every finite mass BEFORE any
window cut, and tune one Gaussian width against the existing same-code DSCB
helper (123.5–128GeV,0.1GeVbins, three starts, status0/cov3 required).

Unsmeared sigma0.2726239541GeV; target0.3407799426GeV. Added Gaussian sigma
0.1597405981GeV gives fitted sigma0.3403688221GeV, ratio1.248491987 (+24.8492%),
within0.25%relative target tolerance. ROOT6.40.02. The convolution's fitted
DSCB core is calibrated explicitly, not equated to simple Gaussian quadrature.
Fixed draws, original/smeared masses, fit trials/source hashes retained.
This synthetic MASS-reference broadening is not a detector/muon-momentum rerun
or a physical-resolution validation. The original muon data remain unchanged.

## Six new plots and normalization

Six120–140GeV panels with0.1GeVbins: inclusive10ab^-1yield; inclusive shape;
Neither,eBrem-only,FSR-only,Both shapes. Exactly3curves: bare electronKF grayfill,
finalBP+FSRwithfallback goldsolid, smeared inclusive muon blackdashed. Large
fonts/grid/AGORA simulation outside axes preserved; no extra fit curves.
Category shapes independently normalize each curve's own finite window entries.
Muon is a separate inclusive reference repeated across electron categories.
Truth categories retain strict>1GeV per-electron birth-energy sums/EITHERleg.

Pre-window category counts: Neither21571,eBrem-only43687,FSR-only11502,Both21542;
sum98302. These differ from the raw matrix only by135baseline failures. Both
electron curves share every pre-window key. Full-input luminosity denominators
remain100000electron/50000muon, weights0.704/1.354; no success/window renormalization.

| Method |120–140 raw|120–130 raw|Below120|Above140|Undefined|
|---|---:|---:|---:|---:|---:|
|Bare electronKF|65793|42808|98|32382|29|
|Final with fallback|79120|64633|4173|14914|95|
|Smeared muon|44243|36908|47|4335|11|

Expected120–140yields at10ab^-1: KF46318.272, final55700.48, muon59905.022.
Category counts/histograms partition inclusive; all under/overflow/undefined
counts close. Source/plot hashes and all6renders reviewed. Improvements here
are spectrum population/shape diagnostics; native truth-assisted material
locations and existing nonlinear/response limitations remain.

Uncommitted study: TrackingPerformanceStudies/bp_final_fallback_recoil_20261116/.
read.py, smear_muon.py, plot.py, README,500job metadata/event/track CSVs,
events.csv, tracks.csv, histograms/counts, read_summary.json, summary.json,
muon_smearing.json/muon_smeared.csv, validation.json, logs and6PNG/PDF pairs.
No simulation, tracking, PFA, BP refit, source/card/default/build/install change.
No generated ROOT file. A review contact sheet is under/tmp only, not served.

The user-requested review placement isNov15despite the new study dateNov16.
Preserve existing sections and originals. Full outgoing project state is saved
in2026-11-16-agents-before-final-fallback-recoil.md before focus replacement.

## Publication and final preservation

New daily-catalog row is first onNov15, with source_date2026-11-16 and
review_date2026-11-15. Old H0F/theta25 row is second, population matrices third.
Nov15now13entries/47previews, otherdays12/38and12/43;37catalogentries/128refs.
Rootvalidation2471PNGs/11sections (6new source PNGs, no copies). All128PNG
routes return200;6new downloads hash-match summary/source files. Server
PID1404151 remains healthy at localhost8000 with no restart.

Main compares to `/tmp/gallery_daily_review_final_fallback_before_publish/`
daily_review.json: every old row is identical and in the same relative order,
both other day objects are identical, and exactly the6approved new imagepaths
are added. Source/review dates are explicit. Snapshot was initially reported
without its `_publish` suffix; main located the actual saved before-state.
Live verification and main_review_final_fallback_publication.json are retained
in the daily-review artifact directory. Existing128refguard includes per-image
archive suppression; no historical retired figures are restored.

AGENTS global prefix remains byte-identical; current focus is replaced with
this completed study and outgoing full status retained. Only durable records
and AGENTS are committed. Analysis scripts/data/plots/catalogs remain uncommitted.
