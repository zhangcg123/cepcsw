# BP plotting reset: per-track pT by no/light/hard eBrem

The user requested cleaning the plotting directory and beginning comparisons
with single-track pT resolution split into no/light/hard eBrem. The previous
complete BP analysis directory was archived outside the served root at
`.analysis_archive/bp_before_ebrem_pt_20261110T130402Z/`, preserving old plots,
CSV/JSON data, README, manifest and scripts. The active directory remains
`TrackingPerformanceStudies/bp_matched_recoil_pt_20261110/`.
Only cached tracks.csv/snapshot.json were copied back. A focused renderer and
tracks_ebrem.csv now save the energy-based classification. There are exactly
two PNG/PDF pairs, pt_ebrem_core and pt_ebrem_wide, each with three panels.
No recoil, old radiation split, extra control, tail-survival or quantile plot
remains in the gallery. All previous evidence remains in the archive and
`2026-11-10-bp-matched-recoil-pt-plots.md`.

## Definitions and scope

Material-eBrem fraction is the sum of recorded primary-electron tracker
brems-photon birth energies divided by the matched stable generator electron
energy after FSR/before material. Denominator is read exactly from matching REC
MCParticle momentum/mass, with original MC index, PDG/status and pT closure
checks. Cached BP estimates are not refitted. No: fraction0; light:0<f<0.01;
hard:f>=0.01. The1% boundary belongs to hard. No extra photon-energy threshold.
FSR is retained in all bins and excluded from the eBrem numerator. No recorded
eBrem is not proof of zero physical radiation outside recorder coverage.
All19,855 selected tracks are kept, with no pair/opposite-charge requirement.
Comparisons remain Baseline KF versus existing BP scan110 plus per-track calo
fallback, on identical tracks. No estimator/ranking, reconstruction source,
card, installation, production file or job changes.

Residual100*(pT_reco/pT_gen-1). Core[-5,5]% and wide[-100,200]%log-y use full
category denominators. Panel annotations give in-window counts. All numerical
statistics use full tails; central68% is(q84-q16)/2, not a Gaussian sigma fit.
One common legend is placed outside the panels to preserve the peak shapes.

## Results

Central68% half-widths use all tracks within each category; no range clipping.

| Category | Tracks | With FSR | Complete scans | Baseline KF width [%] | BP + fallback width [%] |
| --- | ---: | ---: | ---: | ---: | ---: |
| no | 5,180 | 3,065 | 4,784 | 0.2075 | 1.6020 |
| light | 5,044 | 2,957 | 4,582 | 0.2985 | 1.9755 |
| hard | 9,631 | 5,840 | 7,934 | 15.4058 | 3.9189 |

All19,855 estimates and fallback routes are unchanged from the previous
analysis. Exact generator-energy joins, pT closure, threshold boundary checks,
all400 BP/REC source identities, and category/FSR/scan accounting pass.
Both figures were visually inspected. FSR is retained; these categories do
not represent FSR-free controls. No new tracking fit or job is run.

## Gallery refresh

The dedicated plot-gallery agent replaced the manifest with exactly2 sections
covering the2 PNGs once each. It reused healthy PID384311 on
ailogin002.ihep.ac.cn,localhost8000; no restart was needed. HTTP page200 lists
only pt_ebrem_core.png and pt_ebrem_wide.png, with none of the previous16 names.
The core PNG returns200 image/png. The main agent reviewed the manifest's
threshold/population/method definitions and figure layouts. Old plots and
analysis artifacts remain archived outside the served root. URL remains
http://127.0.0.1:8000/ on the same host/forwarding setup.

## Future GSF comparison: setup100

The user selected100 as the fair basis for the future GSF comparison. Current
GSF has beam constraint on and ECAL constraint off. BP and GSF beam centers
and widths agree:0,0 mm and0.0145,3.6e-5 mm. Main comparison should therefore
use tracker+beam100 without calorimeter fallback, location selected using100
fit quality rather than truth or110-calorimeter-ranked locations, on identical
valid tracks with failures reported separately. The current BP campaign has
LocationScanSetups=[110] and IntervalSelectionMode=Truth; a native job4 check
confirms only110 entries and the existing beam_spot_free_loss_rts_* endpoint
is at the truth-selected location. It is an oracle-location control, not the
blind100 result. New100 location scans require rerunning only BP from the same
REC events. The already-running matched GSF settings need no change. No source,
card, job, current plot or installed library is changed for this decision.

The user subsequently approved existing truth-location BP100 histograms for
the immediate/future benchmark. That supersedes the preceding requirement for
a blind100 rerun for the current plots. The110 figures above are archived;
active100 definitions/results and the remaining information advantage are in
`2026-11-10-bp100-truth-location-pt.md`. No blind100 scan is claimed.
