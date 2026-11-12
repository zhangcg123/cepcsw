# Category distributions of ordinary/floating BP110 fit scores

The user requested performance and score distributions in the radiation
categories after discussing a hypothesis-selection criterion. This is a saved
tuple analysis, not implementation of a selector.

## Inputs and definition

Read all 500 completed current BP_FSRFLOAT_NOSCAN files (100,000 generated
inputs). Join their saved scores to the 67,950 common valid-fit event pairs
in `TrackingPerformanceStudies/bp_fsrfloat_recoil_categories_20261113/`.
That parent requires Baseline KF, GSF, ordinary BP110 and floating BP110 for
both selected production electrons; it is already a selected population.
All BP file size/mtime identities and selected track/MC indices pass.
No removed old REC input is accessed.

Ordinary material fits use `charged_ecal_free_loss_optimizer_nll2`, checked
against `optimizer_objective_nll2`; never substitute the fixed-prior score.
For a null material breakpoint only, the free-loss endpoint aliases the 5D
ordinary fit and uses `charged_ecal_ordinary_likelihood_nll2`.
The alternative uses `charged_ecal_free_loss_fsr_nll2`.
Both are normalized -2 ln L, including covariance and dimensional terms.
Define event Q as the sum over the two electrons of ordinary minus floating
score. Positive Q favors floating FSR. The sum neglects any cross-track
correlation, including shared ECAL clusters.

Exclude 8,790 events where a null-material leg has a collected neutral:
ordinary BP110 uses C alone, floating uses C+N. Fourteen events also have a
missing ordinary score (`Measurement-score native projection failed`),
although their track fits remain usable; twelve overlap the preceding
exclusion, leaving two additional exclusions. Thus 59,158 comparable-score
events and all 135,900 parent track rows are retained in the respective CSVs.
Availability and observation-match flags are independent and saved.

Categories are unchanged EVENT categories: each radiation type is present if
either selected electron's summed photon birth energy is strictly >1 GeV.
Both means AND; sub-threshold radiation is allowed.

## Results on identical retained events

| Category | Comparable events | Median Q | Q > 4 | Ordinary -> floating fraction in 120–130 GeV |
|---|---:|---:|---:|---:|
| Neither | 16,211 | 0.176 | 23.47% | 85.71% -> 84.14% |
| eBrem only | 26,064 | 1.597 | 40.55% | 76.43% -> 78.58% |
| FSR only | 6,364 | 15.995 | 61.82% | 40.37% -> 56.76% |
| Both | 10,519 | 30.011 | 67.36% | 40.82% -> 57.12% |

The central fractions use ALL retained pre-window events as denominators,
not only the visible recoil window. These are peak-population changes, not
fitted resolution measurements. Score >4 is illustrative, not a recommended
or calibrated significance cut. There is useful category separation and
substantial overlap: events with no FSR above threshold also favor floating.
Neither's below-120 population grows 127 -> 498 on this same sample.

Negative Q is not clipped. Counts below -0.001 are 10/303/5/80 in the table's
order; counts below -1 are 1/53/1/19 (74 total). The extremes reach approximately
-45,288 and +53,874. Separate local Gaussian linearizations and independent
material optimizations do not establish an exact globally nested nonlinear
likelihood comparison. The origin of these negative outliers was not debugged
in this plotting task. Do not apply a Wilks/chi-square calibration or claim a
validated selector from these distributions.

## Artifacts and checks

`TrackingPerformanceStudies/bp_fsrfloat_scores_20261113/` contains the analysis
script, README, tracks/events/histogram CSVs, summary, source/plot hashes, four
category raw-score PNG/PDF pairs, a category Q overlay and cut-efficiency
curves. Full finite score tails are kept, with symlog x axes. Histogram bins
have varying widths and show fractions per bin; efficiency curves avoid this
bin-width dependence. Each full histogram sums to its sample count.
All six plots were visually checked, with crowded ticks/text corrected.

Six PNGs are added in a new section of the existing port-8000 gallery;
earlier plots, production data, fitter sources, cards and installation remain
unchanged. No simulation, reconstruction, fit, build, install or production
submission is performed. Analysis and gallery artifacts remain uncommitted.
