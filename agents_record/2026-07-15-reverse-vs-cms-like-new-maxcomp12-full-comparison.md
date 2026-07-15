# Reverse versus CMSSW-like new MaxComponents=12 full comparison (2026-07-15)

## Inputs and audit

The new MaxComponents=12 productions are the repository-root flat tuples
`gsf_flat_reverse_<seed>.root` and `gsf_flat_cms-like_<seed>.root`. Each set has
500 files. Exactly 499 files per workflow contain ten `gsf_tuple` entries;
seed 464 is a 943/947-byte missing-tree file in both. Exact `(seed, entry)`
pairing therefore gives 4,990 common finite events. LCIO and truth quantities
agree between the paired tuples.

The comparison uses the established reconstruction-aligned Geant4 categories
and the stable secondary-tracker-activity exclusion. It reports quantile core
widths, RMS, threshold populations, direct eventwise wins, tail counts, and an
inclusive Gaussian core fit. The Gaussian fit is a Poisson-weighted binned fit
over residuals `[-2%, +2%]`, using 80 bins; it is a compact core descriptor,
not an adequate global probability model.

## Inclusive result

For all 4,990 events, reverse versus CMSSW-like gives median residual
-0.06086% versus +0.03733%, width68 0.47342% versus 0.50602%, RMS 20.1687%
versus 19.9374%, and 4,059 versus 4,037 events inside 1%. CMS-like has the
smaller absolute residual in 2,512 events, reverse in 2,475, with three ties;
their mean absolute residuals are 4.2746% and 4.3304%, respectively. Reverse
has 436 events beyond 10% and 151 beyond 50%; CMS-like has 420 and 148.

The inclusive core Gaussian fits give:

| algorithm | fitted mean | fitted sigma | chi2/ndf | entries in fit range |
|---|---:|---:|---:|---:|
| LCIO | -0.03959 +/- 0.00249% | 0.14342 +/- 0.00198% | 10.61 | 3979 |
| reverse | -0.03001 +/- 0.00265% | 0.15663 +/- 0.00225% | 10.58 | 4320 |
| CMSSW-like | +0.06488 +/- 0.00343% | 0.20396 +/- 0.00303% | 10.17 | 4311 |

All three fits have chi2/ndf around 10, directly demonstrating non-Gaussian
core structure even inside the fixed fit window. The fitted sigma must not
replace width68 or tail metrics in physics conclusions. It favors reverse over
CMSSW-like for core width and bias, while the full RMS and paired absolute
error weakly favor CMSSW-like.

## Topology-clean categories

| category | N | reverse/CMS median | reverse/CMS width68 | reverse/CMS RMS | reverse/CMS inside 1% | CMS/reverse wins |
|---|---:|---:|---:|---:|---:|---:|
| no-eBrem | 2032 | -0.0052% / +0.0910% | 0.1397% / 0.1940% | 22.382% / 22.325% | 1921 / 1896 | 780 / 1251 |
| light-eBrem | 2132 | -0.0961% / +0.0020% | 0.4479% / 0.4609% | 5.114% / 5.039% | 1774 / 1776 | 1217 / 914 |
| hard-eBrem | 694 | -0.8283% / -0.6719% | 23.1667% / 22.8289% | 32.319% / 31.439% | 310 / 309 | 432 / 262 |

The new direct comparison confirms the core/tail tradeoff seen in the earlier
12-component production. Reverse clearly preserves the no-eBrem core better. CMSSW-like
wins most light and hard events and reduces their RMS and extreme-tail counts,
but does not improve the hard inside-1% population and slightly broadens the
light core. Across all 4,858 topology-clean events, the eventwise result is
essentially tied (2,429 CMS-like wins, 2,427 reverse wins, two ties), while
reverse keeps the narrower width68 (0.4205% versus 0.4384%) and CMS-like the
lower RMS (18.995% versus 19.241%). The 132-event secondary-activity control is
reported separately and is not used for single-track optimization counts.

## Reproducible outputs

The comparison code is
`Reconstruction/RecGsfTracking/scripts/compare_maxcomponents_12_24_pt.py`;
despite its historical filename, its input paths and labels are configurable.
It now writes `inclusive_gaussian_core_fits.csv` and
`paired_comparison_summary.csv` in addition to the established matched tables
and plots. `python -m py_compile` passes.

All generated tables and plots are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/reverse_vs_cms_like_new_maxcomp12_2026-07-15/`.
The individually audited source summaries are under the sibling
`reverse_new_maxcomp12_2026-07-15/` and `cms_like_new_maxcomp12_2026-07-15/`
directories. These results compare two research workflows on the same sample;
they do not validate either workflow for production.
