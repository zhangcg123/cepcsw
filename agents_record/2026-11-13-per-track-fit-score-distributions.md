# BP110 score plots changed to per-track distributions

The user requested per-track plots instead of event-score sums. The six
score PNGs on the current gallery are replaced with per-track versions;
all recoil and other plots remain unchanged. Original event-score outputs
remain in `TrackingPerformanceStudies/bp_fsrfloat_scores_20261113/`.

New artifacts: `TrackingPerformanceStudies/bp_fsrfloat_track_scores_20261113/`
(analysis script, README, tracks/histogram CSVs, summary and PNG/PDFs).
Read the preceding checked 135,900-track cache and the parent's per-electron
truth energies, joined by job/event/track/MC/leg; no ROOT reread or refit.
Source hashes, identities, finite scores, unique keys, threshold boundaries,
category partition and histogram-count closure are checked.

Each track now uses its OWN summed FSR and eBrem photon birth energies,
strictly >1 GeV for each type. Both is AND. The old pair category remains in
the CSV as `event_category`; it no longer classifies these plots.
The score difference is ordinary BP110 minus floating-FSR BP110 for ONE
track, with no two-leg sum. The normalized likelihood definitions and
observation-comparability rules are unchanged.

Apply score gates independently: 9,070 tracks have different ECAL observations;
14 have missing ordinary scores, twelve overlapping that exclusion. Thus
135,900 - 9,070 - 2 = 126,828 comparable tracks. This includes 8,512 usable
tracks with a noncomparable partner, which the old event-level gate excluded.
The inherited common valid-fit/GSF/opposite-charge event selection remains;
this is not all production tracks.

| Own-track category | Tracks | Median score difference | Fraction >4 |
|---|---:|---:|---:|
| Neither | 65,985 | approximately 0 (7.54e-8) | 12.95% |
| eBrem only | 41,209 | 0.245 | 27.31% |
| FSR only | 11,856 | 7.639 | 54.07% |
| Both | 7,778 | 12.670 | 56.67% |

Raw-score and difference histograms retain all finite tails with symlog x
axes and varying bin widths (fraction per bin). The efficiency curve is
bin-independent. Positive differences favor floating FSR. No significance
calibration, recommended threshold or automatic selector is introduced.
Negative outliers remain: 3/95/0/15 tracks have differences below -1 in the
table's order. Their cause is not investigated by this plotting update.
No fitter, card, installation, production data or batch job changes.
