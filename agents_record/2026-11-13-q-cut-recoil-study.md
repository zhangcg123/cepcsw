# Exploratory per-track Q threshold and recoil-mass comparison

User requested choosing a threshold for the two existing hypotheses and making
a new recoil plot set. No fitter/default is changed. This is an offline test
of H_beta (ordinary free-loss110) versus H_betaF (floating-FSR110), not H0.

## Frozen threshold and independent job split

Read all500 existing BP_FSRFLOAT_NOSCAN files with their saved native p4 and
scores. Reuse98437 truth-classifiable pairs and98302 valid opposite-charge KF
pairs; selected native track/MC identities and parent BP masses agree. All
file identities/card hashes remain checked. No GSF-success gate is imposed,
unlike the older four-method/Q-plot cohort. There are196604 track rows.

Odd job seeds calibrate; even seeds evaluate. The strict cut is
**Q >126.17379302276795**, selected as the higher95th percentile of67347
eligible calibration tracks whose OWN truth FSR birth-energy sum is <=1GeV.
Calibration selects3367/67347=4.99948% of this control. Evaluation selects
3355/67362=4.98055%. No recoil-mass distribution is used to retune the cut.
This controls a truth-label selection fraction, not a significance or a
guaranteed false energy correction. Soft FSR and contamination remain possible.

Each leg independently chooses its OWN H_betaF electron+photon p4 above the
cut, otherwise its H_beta p4. No extra BP neutral p4 is added. Missing Q or
different ECAL observations conservatively keeps H_beta. The ordinary
null-material C-only / floating C+N mismatch remains ineligible; this study
does not use the new H0 values. Both hypotheses must be valid on both legs
for the same-event recoil comparison. Mixed pairs recompute recoil from the
chosen four-vectors; neither event-mass interpolation nor a truth-category
choice is made.

## Evaluation sample and results

There are37485 even-job common pairs: Neither9013/eBrem-only15802/FSR-only4782/
Both7888. Event categories retain the existing either-leg >1GeV definition;
Both is AND and can involve different legs. They differ from the per-track
calibration labels. Zero/one/two legs choose floating in31747/5504/234 pairs.
5202 pairs have at least one unavailable-Q leg, which keeps H_beta.

| Method | Raw120–130 events | Raw below120 events |
|---|---:|---:|
| H_beta | 22831 | 605 |
| Always H_betaF | 27035 | 1748 |
| Q-selected | 24325 | 1372 |

Peak population rises6.54% over H_beta, less than always-floating's18.41%.
The selected low-mass population remains more than twice H_beta's. Neither
still degrades: peak7694 ->7566 and below12067 ->211. Thus high Q is not a
sufficient clean-track safety criterion; this is not a demonstrated optimum.
No fitted mass-resolution or population physics-validation claim is made.

Eligible evaluation track selection fractions are Neither1.644%, eBrem-only
10.227%, FSR-only25.265%, Both31.406%. The pooled5% no-hard-FSR control therefore
hides category dependence and rejects many genuine >1GeV FSR tracks.

## Plots, normalization and checks

Artifacts: `TrackingPerformanceStudies/bp_q_selected_recoil_20261113/`.
Native tracks/events/histogram CSVs, threshold/summary JSONs, input/artifact/PNG
hashes, README, analysis script and five PNG/PDF pairs are retained. Every
plot compares H_beta, always-floating and Q-selected, plus KF/muon references.
The four category plots independently normalize each curve in120–140GeV,
with0.1GeV bins and Baseline KF+the existing fresh neutral union; muon dashed.
The inclusive plot uses bare KF on the same BP keys and absolute10ab^-1:
electron weight1.408 from ALL50000 generated even-job inputs, muon1.354 from
ALL50000 original inputs. No success/window renormalization. Inclusive electron
yields describe the common valid-fit selection, not full standalone acceptance.
The inherited published7.04/6.77fb reference and forced-invisible muon tracking
normalization convention remain explicit limits.

Native ordinary/floating masses reproduce the parent cache; pure-choice
limits reproduce each original hypothesis. All electron curves share exact
keys; category raw histograms partition the inclusive BP histograms per bin.
All source/PNG hashes, shape integrals and full-input luminosity weights pass.
All five figures are visually checked. No generator recoil or new peak fit.

The gallery adds one section after the Q plots:51PNGs/13sections, exact-once.
All54 earlier PNG/PDF images stay unchanged. Sameailogin002PID443262/port8000,
no restart; page/allfivePNGsHTTP200 and source/local/served hashes match.

This is a job-separated internal check on a previously studied sample, not
an untouched external validation. Material locations remain truth-assisted.
No refits, source build, installation or production submission occurs. The
threshold is analysis-only; do not silently promote it to production.
The complete outgoing AGENTS.md is preserved in
2026-11-13-agents-before-q-cut-recoil.md. Current focus is replaced; active
laws/commands are unchanged. H0 availability remains current global status.
