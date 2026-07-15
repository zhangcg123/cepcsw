# New MaxComponents=12 tuples and matched comparison with 24

## Input audit and standalone products

The new `tuples285maxcomp12/` set contains 500 flat tuple files. Of these, 499
have a complete 10-entry `gsf_tuple`; only seed 464 is unusable. The standalone
MaxComponents=12 analysis therefore contains 4,990 finite matched events. Its
inclusive, surface-owned eBrem category, topology-clean category, and dominant
transition-location plots and tables are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/maxcomp12_new_tuples_2026-07-14/`.

The topology-clean light-eBrem transition-location sample contains 2,132
events: 73 at transitions 0--2, 142 at 3--4, 499 at 5--6, 522 at 7--8, 84 at
9--11, and 812 above 11.

## Direct matched comparison

The MaxComponents=12 and MaxComponents=24 event tables were joined by exact
`(seed, entry)`. Since the 24 set has 14 broken seeds, the common comparison
contains 4,860 events. Products are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/maxcomp12_vs_24_matched_2026-07-14/`.

Across all common events, MaxComponents=12 versus 24 has median residual
-0.06184% versus -0.06665%, central-68 half-width 0.46096% versus 0.46264%,
3,955 versus 3,948 events inside 1%, and full RMS 20.724% versus 20.191%.
Inside the displayed +/-5% zoom window, both have 4,375 events; the recomputed
central-68 half-width is 0.24277% for 12 versus 0.24015% for 24.

On the common topology-clean categories:

- no-eBrem width68 is 0.14403% for 12 versus 0.13879% for 24, with 1,866 events
  inside 1% for both;
- light-eBrem width68 is 0.43533% versus 0.45167%, with 1,723 versus 1,719
  inside 1%, but RMS is 5.151% versus 5.068%;
- hard-eBrem width68 is 21.844% versus 22.143%, with 310 versus 309 inside 1%,
  but RMS is 34.110% versus 31.802%.

Thus 12 gives tiny central-count/quantile gains on this surviving subset but
worse full-tail behavior and a slightly broader no-eBrem core. This is a
tradeoff, not evidence to reverse the selected MaxComponents=24 default. It is
also consistent with the focused 30-event rejection in showing that capacity
changes are event-dependent.

Reproducible scripts:

- `Reconstruction/RecGsfTracking/scripts/compare_new_gsf_flat_pt_resolution.py`
- `Reconstruction/RecGsfTracking/scripts/plot_new_gsf_pt_by_transition_location.py`
- `Reconstruction/RecGsfTracking/scripts/compare_maxcomponents_12_24_pt.py`
