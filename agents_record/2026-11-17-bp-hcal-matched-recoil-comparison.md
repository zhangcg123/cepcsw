# Matched ECAL-only / ECAL+HCAL recoil test — 2026-11-17

## Decision and scope

The user requests more events and a solid comparison before a new production.
The current recipe does **not demonstrate improved recoil core resolution**
and increases the low-mass tail. Do not promote it to a full HCAL-enabled
production based on this evidence. Keep the implemented option for further
joint response/association studies. This is not a claim that HCAL can never help.

The maintained card/batch default remains enabled from implementation commit
3e04301; BP_INCLUDE_HCAL=0 explicitly selects ECAL-only. No C++, card, default,
build or installation changes were made during this comparison. No production
was submitted. Existing production files remain untouched.

## Frozen matched experiment

Current REC_ECALTRUTH_500jobs_200evt_20261112 files1–4, zero-based events100–199
in each:400unique inputs. Both modes freshly rerun the same installed plugin,
SHA256685501046a02cb1c1f72ddd9c4fca3e3bc1471f71902b736a571fb549e550163.
This avoids the four recent implementation-test events; it is not claimed to
be untouched by every historical BP study. Selection was fixed before reading
the paired mass results. Eight local workers execute32tasks of25events each,
800event-fit evaluations. Every task succeeds; runtime min486.4/median663.9/
max865.9seconds. Frozen script/plugin and REC size/mtime fingerprints checked.

Controls match the current normal truth-location BP workflow:production-pair
selector, no location scans, beam/charged110 on, separate-neutral111 off,
floating FSR on, loss prior sigma0.001, free-loss maxcalls180. Theta/phi
half-windows25/200mrad. Existing ECAL per-cluster sigma=.011sqrt(E)+.004E.
HCAL uses the user-approved calibrated E_H=4E_raw and per-cluster
sigma_H=.30sqrt(E_H)+.07E_H, terms linear; variances independently add.
Only BP_INCLUDE_HCAL differs between modes. No calorimeter/track re-reconstruction.

Per-leg fallback is110+FSR→ordinary110→beam100→tracker000→BaselineKF. Native
statuses/masks/states choose the route; recoil mass/window and truth category
never choose it. Only a successful FSR family adds its own electron+photon once.
All396baseline-valid pairs remain, including failures/fallbacks; four incomplete
selected pairs excluded. No all-four-hypothesis-valid or calo-valid gate.
Both plotted modes use the same category pre-window pair denominator, preserving
window efficiency rather than independently normalizing the window areas.

Input baseline masses and fresh off/on upstream tracker/beam endpoints/raw ECAL
observations pass strict identity checks. New tuple photon truth independently
reproduces the old category definitions. Historical ECAL-only production masses
agree exactly except tiny numerical differences:maximum1.9057MeV(source3/event170),
one difference above0.1MeV; another source3/event109difference0.92keV. No finite/
undefined changes. These historical results are QA only, never the control curve,
and no offsets are applied. The origin of the historical numerical drift was not
established. Same-code fresh pairing avoids relying on stored optimizer outputs.

## Inclusive results

| Metric | ECAL only | ECAL+HCAL |
|---|---:|---:|
| Pairs |396|396|
|120–130GeV |260|260|
|120–140GeV |316|311|
|Below120GeV |18|28|
|Above140GeV |62|57|
|Undefined recoil |0|0|
|RMS in123.5–128GeV |1.015128|1.081094|
|Entries in that RMS window |205|166|
|Fitted DSCB location |125.109414|124.540718|
|Full fitted-density FWHM |1.304810|1.416799|

Core-region RMS on/off ratio1.064982, paired-event bootstrap95%CI
[0.957487,1.179177],10000draws,seed20261117. This is a conditional fixed-window
standard deviation, not the DSCB Gaussian sigma. Window migration is retained
in each draw, and tail counts always use all396pairs. The low-tail increase is
2.525percentage points, paired95%CI[1.010,4.040]; all10changes are new low-tail
events. The high-tail decrease is1.263points,CI[−2.525,−0.253].120–130yield
change0,CI[−2.273,+2.273]points, with20oppositely directed migrations.

Mass shifts downward in392/396pairs; median−0.733916GeV, mean−1.359033GeV.
Full finite-spectrum standard deviation14.145620→14.295710GeV, ratio95%CI
[0.982341,1.032902]. RMSE relative to nominal125GeV15.530572→15.162092GeV,
ratio95%CI[0.947688,0.997598], reflecting reduced positive bias/high tail.
This post-analysis tail-sensitive diagnostic is distinct from core resolution.

## DSCB identification warning

The maintained ROOT six-parameter DSCB helper uses0.1GeVbins and the unchanged
123.5–128GeVfit range/three starts. Nominal fits pass status0/covariance3, but
Gaussian sigma0.550179±0.218631→0.131735±0.083422GeV is misleading as a visible
peak-width comparison. On-mode alpha_left=.101329,alpha_right=.252846 give a
Gaussian-region span0.046657GeV, below one histogram bin; broad power-law tails
describe most of the visible peak. Off-mode n_left reaches its lower bound.

The prespecified300paired-event bootstrap refits vary all six parameters and
repeat the same three starts/quality gates.280paired fits succeed, meeting the
predeclared90%technical threshold;20pairs fail. The sigma-ratio95%interval is
[0.045618,7.413905], exposing weak identification despite nominal covariance
quality. No resolution improvement is claimed from the small nominal sigma.

Plots therefore label the full fitted-density FWHM instead. It is interpolated
from the unchanged ROOT fit-density grid and has a known Gaussian self-check.
This FWHM was added after observing the sub-bin Gaussian region and is a
descriptive diagnostic, not a separate prespecified significance test. All raw
parameters, starts, covariance/status diagnostics and failures are retained.

## Subsets and tracking controls

Both-leg final-FSR validity is309pairs off,320on; symmetric common subset305.
Within that common subset, low tails16→26 and high tails33→28;120–130counts
215→212. All10new low-tail events remain, so changed fallback selection does
not generate that effect. Core RMS1.004380→1.025649, ratio95%CI[.906912,1.147890].

Selected-leg routes off/on:FSR701/712,ordinary11028/17,beam10033/33,
tracker0002/2,KF28/28. There are792selected legs per mode. No upstream energy
constraint is silently introduced into000/100.

| Event category (>1GeVper-leg birth sums) | Pairs | Below120off→on | Core RMS ratio95%CI |
|---|---:|---:|---|
|Neither |82|3→5|[.976,1.354]|
|eBrem-only |174|13→18|[.839,1.128]|
|FSR-only |50|1→3|Insufficient core entries22/17|
|Both AND |90|1→2|[.836,1.320]|

Flags are event unions over the two selected legs; Both can occur on different
legs. Soft radiation of the other type is allowed in the thresholded only groups.
Category results are exploratory, not independently powered validation claims.

Exploratory available-HCAL-fraction bins<.5%,.5–1%,>=1%contain122/100/145pairs;
29have undefined fractions but remain in the primary sample. All10new low-tail
events occur in the>=1%group. No cut is tuned/validated from this observation.
No HCAL cluster is shared across the two selected legs in these396pairs.

Truth-pT mean absolute residual over792legs5.740790%→5.762326%;>10%outliers
remain89. Neither164legs improve MAE.346253%→.329037%, with>1%outliers12→9.
These are residual diagnostics, not fitted momentum-resolution validation.

## Interpretation, artifacts and publication

The main observed effect is a downward mass shift, reduced high tail and larger
low tail, without demonstrated core narrowing or120–130yield gain. ECAL source
energies already include response corrections. The joint ECAL+HCAL calibration,
associated HCAL truth origin and ECAL/HCAL covariance remain unvalidated.
Energy addition and uncertainty broadening change together; their causal effects
are not separated here. Do not infer that a plain PFO-calibrated sum is already
an unbiased electron/photon measurement or that HCAL is intrinsically useless.

Uncommitted study:TrackingPerformanceStudies/bp_hcal_ab_20261117/, including
plan.json/frozen card,32task directories with native outputs/logs/fingerprints,
run.py/analyze.py/fit_plot.py/fit_bootstrap.py/peak_width.py,events.csv/tracks.csv,
summary.json/fits.json/fit_bootstrap.json/full_spectrum_diagnostic.json/result.json,
five PNG/PDF pairs and complete README. No generated outputs are committed.

Personal plot-gallery workflow delegated to hcal_ab_gallery_20261117 using
gpt-6-luna/high. New Nov17daily page and study page each show all five previews
with descriptions; inclusive first then Neither/eBrem-only/FSR-only/Both.
ServerPID1404151 onailogin002 remains healthy atlocalhost8000. Existing37study
entries/128preview hashes across three old days are preserved. New totals:
fourdays,38entries,133previews. All five new PNG routes and both pages HTTP200;
served PNG hashes match local outputs. Main reviewed final inclusive/FSR-only
renders and all10PNG/PDFhashes. Before-state and verification:
/tmp/bp_hcal_ab_20261117_gallery_review/. New gallery descriptions were corrected
to distinguish RMS values from their ratio and preserve the strict>1GeVcategory
definitions; soft other-type radiation is not silently excluded.

Complete outgoing AGENTS.md is preserved in
2026-11-17-agents-before-hcal-comparison.md. Only current focus changes; the
global-status/laws/compile/run prefix remains byte-identical. No history
directory move or old plot relocation occurs.
