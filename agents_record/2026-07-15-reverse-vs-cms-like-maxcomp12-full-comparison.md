# Reverse versus CMSSW-like MaxComponents=12 full comparison (2026-07-15)

The ordinary reverse-filter MaxComponents=12 production in
`tuples285maxcomp12/` was paired by exact `(seed, entry)` with the new
CMSSW-like MaxComponents=12 production. Both have 499 usable seeds and the same
known broken seed 464, giving 4,990 common events. LCIO values agree and are
the shared baseline.

Across all 4,990 events, reverse versus CMSSW-like gives median residual
-0.0615% versus +0.0373%, width68 0.4670% versus 0.5060%, RMS 20.801% versus
19.937%, and 4,055 versus 4,037 events inside 1%. CMSSW-like has smaller
absolute error in 2,539 events, reverse in 2,450, with one tie.

For topology-clean categories:

| category | N | reverse median | CMS median | reverse width68 | CMS width68 | reverse RMS | CMS RMS | reverse/CMS inside 1% | CMS/reverse eventwise wins |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| no-eBrem | 2032 | -0.0068% | +0.0910% | 0.1435% | 0.1940% | 22.720% | 22.325% | 1915/1896 | 782/1250 |
| light-eBrem | 2132 | -0.0943% | +0.0020% | 0.4366% | 0.4609% | 5.093% | 5.039% | 1769/1776 | 1242/890 |
| hard-eBrem | 694 | -0.6633% | -0.6719% | 22.8740% | 22.8289% | 34.502% | 31.439% | 315/309 | 434/260 |

The interpretation is mixed but stable: ordinary reverse better preserves the
clean core and wins most no-eBrem events; CMSSW-like wins most light/hard events
and reduces full-tail RMS, especially hard-eBrem, while giving up a small
number of central-count events. Neither controls the extreme clean tail.

Plots, matched residuals, and summaries are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/reverse_vs_cms_like_maxcomp12_2026-07-15/`.
