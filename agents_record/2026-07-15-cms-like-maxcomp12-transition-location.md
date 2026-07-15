# CMSSW-like MaxComponents=12 transition-location categorization (2026-07-15)

The 2,132 topology-clean light-eBrem events in the full CMSSW-like
MaxComponents=12 production were categorized by the established dominant
Geant4 surface-owned loss transition. Counts are 73 at 0--2, 142 at 3--4, 499
at 5--6, 522 at 7--8, 84 at 9--11, and 812 above 11.

| transition | N | LCIO median | CMS median | LCIO width68 | CMS width68 | LCIO/CMS inside 1% | LCIO RMS | CMS RMS |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| 0--2 | 73 | -0.4587% | -0.2945% | 1.6841% | 1.4330% | 43/43 | 2.703% | 2.520% |
| 3--4 | 142 | -0.5446% | -0.3274% | 2.4684% | 2.2927% | 82/85 | 4.862% | 4.697% |
| 5--6 | 499 | -0.3487% | -0.0699% | 1.9454% | 0.6376% | 332/388 | 5.015% | 5.236% |
| 7--8 | 522 | -0.5433% | -0.0116% | 1.7786% | 0.4633% | 306/451 | 3.435% | 5.266% |
| 9--11 | 84 | -0.5090% | +0.0312% | 1.6543% | 0.4497% | 50/76 | 3.065% | 1.310% |
| >11 | 812 | -0.0728% | +0.0657% | 0.2395% | 0.2542% | 736/733 | 4.324% | 5.234% |

The layer boundary remains clear. Transitions 0--4 improve only modestly;
5--11 show strong central recovery, strongest at 7--11. Losses above 11 are
already mostly LCIO-like and CMS-like slightly broadens their core. Full RMS
worsens at 5--8 despite the strong central gain, demonstrating new tails.

Plots, eventwise residuals, and the summary CSV are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/cms_like_maxcomp12_2026-07-15/transition_location/`.
