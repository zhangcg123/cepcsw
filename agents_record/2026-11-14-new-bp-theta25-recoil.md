# Recoil mass from completed BP H0_F/theta25 production — 2026-11-14

All500jobs completed with markers and no failed marker. Native new flat tuples
contain200event-side rows each:100000inputs. Read fresh from
higgs_eeHX_ISRFSRON_BP_H0F_THETA25_NOSCAN_500jobs_200evt_20261113; no deleted old
BP ROOT or old BP fitted endpoint is used. Matching current REC_ECALTRUTH files
supply original CompleteTracks AtIP direction/charge and ECAL reference states.
All native pre-ECAL energies/positions reproduce REC; independently rebuilt
25/200mrad half-window neutral IDs match196874complete-pair selected-track rows.
Card hashes/frozen controls and all source path/size/mtimes checked before/after.
No tracking, fitting, simulation, PFA, card or install rerun/change.

## Curves

- H0: genuine5D no-breakpoint setup110, null_110_*, successful kind0/mask3.
- H0F: independent no-material-breakpoint floating-FSR fit, fsr_only_110_*.
- H_beta: native ordinary free-loss BP110, truth-selected eligible material
  interval or native valid-null fallback; no scan.
- H_betaF: matching BP110 electron+its own fitted FSR photon p4.
- Baseline KF: fresh native REC AtIP phi/tanLambda and saved KF pT.

H0/H_beta use native fitted electron IP p4 without extra neutrals. H0F/H_betaF
use the electron and photon from that SAME family once; never the ordinary
control electron plus a different fitted FSR. Scalar FSR amplitude is assumed
collinear atIP by the native model. Successful state/mask/finite values and
massless/collinear photon and electron mass-shell closures checked. No KF
fallback substitutes failed BP hypotheses. Original family routing retained;
ordinary null-material BP110 can use charged-only routing whereas H0/H0F use
configured C+N. These plots are not a calibrated four-way likelihood test.

## Six new figures, all120–140GeV with0.1GeV bins

1. Inclusive10ab^-1: bare muon Baseline KF (black solid), bare electron Baseline
   KF (grey fill), BP110+floatingFSR (blue). Electron uses every98302valid
   opposite-charge baseline pair, floating110uses its own75511valid pairs.
   NO H0/H0F/ordinaryBP/GSFsuccess gate or radiation category gate. Weights are
   constant L*sigma/N_generated: electron0.704 (7.04fb/100000), muon1.354
   (6.77fb/50000), L10000fb^-1. Never divide by window/fit-success counts.
   Failures/undefined recoil remain inefficiency in the full input denominator.
   These are the established published channel references, not fresh generator
   header rates; forced-invisible muon sample remains a tracking reference.
2–6. Inclusive and neither/eBrem-only/FSR-only/Both category SHAPES: cached
   inclusive muon Baseline KF reference (black dashed); Baseline KF+neutrals;
   H0,H0F,H_beta,H_betaF (all solid). Every electron curve in a panel shares the
   EXACT SAME pre-window event keys, requiring valid baseline charge pair,
   reconstructed neutral references and ALL FOUR BP hypotheses. No GSF gate.
   Each curve then normalizes by its OWN finite120–140GeV count; raw window N
   shown in legend, common pre-window count shown at left. Separate shape and
   luminosity histogram columns are saved in histogram.csv.

KF+neutrals uses raw hit-supported zero-charge trackless pre-reclustering
clusters in the new25/200mrad window, massless p4 from nominalIP and measured
cluster position; union counted once across both selected legs. NO energy,
purity, truth-origin or isolation cut. The earlier55.757GeV/upper2% trimming
was for the neutral-energy study, not these native fits/recoil predictions.
Only KF+neutrals receives this explicit measured neutral addition. BP energy
constraints already use their native collected observations, so no extra
neutral p4 is added to any BP hypothesis.

## Category and event counts

Truth categories use each selected production electron's SUM of recorded
photon BIRTH energies by type. A type is present if E_sum>1GeV on EITHER leg;
Both=FSR AND eBrem (possibly different legs). Neither means both types fail
that threshold. Only means no >1GeV other type; soft other radiation remains.
FSR uses generator-linked parent ownership; eBrem uses selected main G4 parent.
The98302baseline charge pairs come from98437complete classified pairs;1563
inputs lack a complete selected pair and135have invalid/same reconstructed
baseline charge. The raw classified category counts are unchanged exactly
against the previous full-sample category table (used only as a cross-check).

| Category | All classified pairs | Common shape pairs |
|---|---:|---:|
| Neither |21602|17948|
| eBrem only |43739|31798|
| FSR only |11517|9673|
| Both AND |21579|15649|
| Inclusive |98437|75068|

Per-method valid-pair/window counts and every rejection are in summary.json,
category_counts.csv, and the row-level events.csv. Category raw histograms
partition the common inclusive histogram bin-by-bin for every electron curve.
H0/H0F/H_beta/H_betaF own valid baseline-pair counts are89645/84459/81822/75511.
Do not compare common-category counts directly with the raw truth matrix or
with the independently selected inclusive luminosity curve.

## Verification and limits

Fresh REC baseline states reproduce ALL98302previous bare-KF masses EXACTLY
(maximum difference0GeV). Muon44236window-event histogram is unchanged exactly.
New H0_F branches are read natively from every output. All state/p4 and native
window/measurement links checked, source/card identities preserved. Histograms,
normalization closure, category partitioning, six legend counts and data-derived
y ranges checked. All six figures reviewed before publication. No generator
recoil, peak fits, quantile or extra tail plots. This is a new BP hypothesis
comparison; its common gate differs from the older GSF-matched panels. No
same-code old/new-window A/B or isolated detector-resolution/physics validation
is claimed. These research BP fits still use truth-selected material locations.

Native read and plot scripts,500job CSV/metadata files, source snapshots,
validation, full counts and sixPNG/PDF pairs are uncommitted artifacts here.
New runs reread ROOT to avoid stale caches if production paths are reused.
Existing gallery figures must be preserved; these six form a new section.

Inclusive expected120–140yields at10ab^-1: electronKF46318.272, muonKF59895.544, BP110+FSR45762.112. Floating110has65003raw window events,54824in120–130; these are yields/populations, not a fitted resolution.

## Provenance and preserved project state

Artifact directory: `TrackingPerformanceStudies/bp_h0f_theta25_recoil_20261114/`.
All source snapshots/controls/cards, native and baseline validation, job-level
rows, category counts, failure summaries, histograms and source/plot hashes are
saved there as uncommitted analysis artifacts. This record/AGENTS/outgoing
snapshot are the durable checkpoint. Complete outgoing focus is preserved in
2026-11-14-agents-before-new-bp-theta25-recoil.md. No input/reconstruction-source/card/build/
install or production-submission modification.

## Gallery publication

Appended the sixPNG/PDF pairs as "New BP recoil: theta25mrad window and H0+FSR".
Final81PNGs/17sections/exact-once,33PDFs. All102prior gallery image hashes
unchanged. The delegated plot-gallery workflow verified the page/all6newPNG
HTTP200 and source/local/served SHA equality. Same ailogin002.ihep.ac.cn
PID443262, localhost8000, no restart. Main independently reviewed all6renders,
all102old hashes, all12new copies and manifest coverage. Evidence in study
gallery_verification.json and before_gallery.json. Global AGENTS prefix is
preserved byte-for-byte; full outgoing focus snapshot is retained.

New names (each PNG/PDF): recoil_bp_theta25_h0f_inclusive_10ab_120_140 and
recoil_bp_theta25_h0f_{inclusive,neither,ebrem_only,fsr_only,both}_shapes_120_140.
