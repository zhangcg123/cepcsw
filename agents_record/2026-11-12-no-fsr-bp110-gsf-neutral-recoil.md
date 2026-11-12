# Matched no-FSR recoil: BP110 and GSF + neutrals, 2026-11-12

User requested comparison on events with no FSR. Both selected production
Z->ee electrons must have known has_fsr=0; unknown excluded. All eBrem categories,
including946events with zero eBrem on both, no positive-eBrem gate. No BP100 gate.
Both BP110 and GSF BestBranch endpoints valid, baseline state/charge consistency.
15645no-FSR candidates;24same baseline charge and3871unavailable-fit rejections;
11750identical common events before either mass validity/window cut.

BP110 kept as fitted, truth-selected eligible location or valid null, fitted
loss and tracker+beam+ECAL constraint. Collected neutral energy can already be
in its ECAL sum; do not add photon p4 again. GSF uses native per-track IP states
plus existing10/200mrad neutral-cluster p4 union, masslessfromnominalIP, once per
cluster.2376events/2788clusters,9374zero-additionevents. No new purity/energy cut.
Use frozen neutral observations from matching BP flat tuples, not the later PFA
replay REC cluster study. Exact cache source/REC/track/MC/G4/baseline/radiation
identity joins and cached bare/recovered GSF mass closures pass.

Input496GSFsources/99200represented events, matching BP cache. Current page
normalization retained:10ab^-1 *7.04fb/99600parent generated inputs from498BP
sources, beforeFSR/endpoints/mass/recovery cuts. Both curves have SAME pool and
weight; finite or mass-window outcomes never renormalize the comparison.

120--130GeV in0.1GeVbins; only BP110 and GSF+neutrals. Square, no title, AGORA
simulation bold outside axes,10ab^-1inside upperleft, transparent grid. DSCB
using existing ROOT.RooCrystalBall helper fit123.5--128GeV, status0/cov3 both.
BP110sigma0.3337786884+/-0.0192887455GeV; GSF+neutral0.3179883771+/-0.0145102990GeV.
Rawwindowcounts8970/8735; undefined2/139; expected6340.240964/6174.136546 at10ab.
Similar core widths, no significant-improvement claim. Sigma is spectrum core,
not detector-only resolution. BP110 has truth-location/ECAL input advantage;
GSF plus blind neutral addition can double-count material radiation.

Artifacts:TrackingPerformanceStudies/recoil_no_fsr_bp110_gsf_neutrals_20261112/:
analyze.py,run.log,pairs.csv,histogram.csv,summary.json,plot_files.json,
analysis_code_hashes.json,PNG/PDF. Only cached results read; no new ROOT-event,
PFA/tracking/BP fit, batch or installation change. New DSCB histogram fits only.
Allsource hashes, same-event/no-FSR/endpoint gates, unique cluster union, vector
closure, zero-addition invariance, hist/normalization closure and fit gates pass;
layout viewed. Gallery published with15PNGs/7sections/exact-once, all14prior
image hashes unchanged. New no-FSR section follows energy and precedes existing
recoil sections. Page and newPNGHTTP200, sourcehashmatches. Main independently
checked category coverage/sourcehash/description. Same ailogin002 PID2122174
localhost8000, no restart. gallery_verification.json preserves results.

Outgoing AGENTS status preserved in full in
2026-11-12-agents-before-no-fsr-recoil-comparison.md before replacing focus.
Global introduction/laws/commands byte-identical. Prior photon/neutral figure
status remains recoverable through snapshot and linked dated study records.

## Follow-up: add bare electron and inclusive muon Baseline KF

Added Baseline KF electron on the SAME11750no-FSR events, and the maintained
INCLUSIVE bare muon Baseline KF48636pair reference, no new muonFSRveto. Same
10ab^-1luminosity, distinct channel weights: electron10000*7.04/99600=0.7068273092,
muon10000*6.77/50000=1.354. Not common-area/window normalization. Inclusive muon
yield is therefore much taller than the FSR-vetoed/common-valid electron sample.
Legend explicitly says inclusive; all3electron curves share their event pool.

Rawwindowcounts muon36918,eKF7526,BP8970,GSF8735. Expected49986.972/5319.582329/
6340.240964/6174.136546. NewKFcorewidths muon0.2726570511+/-0.0036108043GeV,
electron0.2805035134+/-0.0106012144. BP/GSFhistograms and sigmas unchanged; muon
histogram/sigma match the maintained inclusive baseline to1e-6GeV. Allfits0/cov3.
Plot square120--130GeV,0.1bins,blacksolidmuon,grayeKF,greenBP,orangeGSF.
Large legend/notes retained; final layout rerendered from cached fit PDFs,
no extra fit optimization for cosmetic changes. No native reconstruction run.
Prior2-curvePNG/PDF/code/data/notes in before_baselines/. Gallery verified
15PNGs/7sections/exact-once; page+replacementPNG200/sourcehashmatch, other14
images unchanged. Same ailogin002 PID2122174 localhost8000, no restart. Prior
servedPNG+sectionJSON in before_baselines/gallery/. Main reviewed coverage,
source/copiedhash and explicit selection/weight definitions. Verification:
baseline_gallery_verification.json in source-study directory.
