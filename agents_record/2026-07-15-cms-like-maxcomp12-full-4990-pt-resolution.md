# CMSSW-like MaxComponents=12 full 4,990-event pT resolution (2026-07-15)

The new `gsf_flat_cms-like_1.root` through `_500.root` MaxComponents=12 production was audited
and plotted with the established Geant4 surface-owned categories and secondary
tracker-activity exclusion. There are 500 input files, 499 usable files, and
4,990 matched valid events. Seed 464 is the sole broken file and has the known
missing-tree/947-byte failure.

For the topology-clean populations:

| category | N | LCIO median | CMS median | LCIO width68 | CMS width68 | LCIO RMS | CMS RMS | LCIO/CMS inside 1% |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| no-eBrem | 2032 | -0.0190% | +0.0910% | 0.1321% | 0.1940% | 2.911% | 22.325% | 1977/1896 |
| light-eBrem | 2132 | -0.2088% | +0.0020% | 1.2575% | 0.4609% | 4.253% | 5.039% | 1549/1776 |
| hard-eBrem | 694 | -13.381% | -0.6719% | 25.0007% | 22.8289% | 33.505% | 31.439% | 205/309 |

Across all 4,990 valid events, LCIO versus CMS-like gives median -0.1103%
versus +0.0373%, width68 2.0710% versus 0.5060%, and 3,764 versus 4,037 events
inside 1%. Full RMS worsens from 14.901% to 19.937%, driven especially by new
extreme no-eBrem failures. Thus the light/hard central recovery is substantial,
but clean-track degradation and catastrophic tails prohibit a production
claim.

PNG/PDF plots, matched residuals, file audit, and summary CSV are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/cms_like_maxcomp12_2026-07-15/`.
The generic plotter's seed parser was extended to accept both dash- and
underscore-delimited filenames.
