# New MaxComponents=24 tuple pT-resolution comparison

## Scope and inputs

The newly produced root-level files `gsf_flat-e--2.0-85-1.root` through
`gsf_flat-e--2.0-85-500.root` were compared with LCIO using the established
surface-owned Geant4 eBrem categories. The comparison joins tuples to category
records by exact `(seed, entry)` identity and also reports the topology-clean
population after excluding events with non-primary tracker SimHits.

The reproducible script is
`Reconstruction/RecGsfTracking/scripts/compare_new_gsf_flat_pt_resolution.py`.
Outputs are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/maxcomp24_new_tuples_2026-07-14/`.

## Broken-file audit

Fourteen of 500 flat tuple files are unusable, leaving 486 complete files and
4,860 matched finite events. Seeds 1, 35, 86, 150, 227, 289, 417, 426, 464,
and 490 contain no `gsf_tuple`; seeds 348, 355, 377, and 467 cannot be opened
by ROOT. No partial usable tree was admitted: a file must contain the expected
10 entries. The exact status, size, and ROOT error are in
`input_file_audit.csv`.

## Results

For the topology-clean sample, the category counts are 1,981 no-eBrem, 2,076
light-eBrem, and 680 hard-eBrem events.

- No-eBrem: LCIO versus GSF median is -0.0200% versus -0.0113%, central-68
  half-width is 0.1326% versus 0.1388%, and the population inside 1% is
  1,927 versus 1,866. The GSF RMS is dominated by existing extreme tails
  (22.99%).
- Light-eBrem: median is -0.2111% versus -0.1025%, central-68 half-width is
  1.2504% versus 0.4517%, and the population inside 1% is 1,507 versus 1,719.
  GSF improves the core but has a worse full RMS, 5.068% versus 4.283%.
- Hard-eBrem: median is -13.332% versus -0.665%, central-68 half-width is
  24.458% versus 22.143%, and the population inside 1% is 203 versus 309.
  The full RMS improves from 33.093% to 31.802%.

The corresponding inclusive category counts are 1,994, 2,092, and 774. Exact
inclusive and topology-clean statistics are in `pt_resolution_summary.csv`;
the event-level joined residuals are in `matched_event_residuals.csv`. Both
full-range and +/-5% plots are provided as PNG and PDF.

These results describe the surviving 486-seed subset and must not be compared
to a 499-seed result as if denominators were identical. They reproduce the
established qualitative MaxComponents=24 picture: a strong light/hard core
gain, slight clean-core broadening, and unresolved GSF extreme tails. This is
not an independent validation or production-performance claim.

## Dominant transition-location categories

After the tuples moved to `tuples285maxcomp24/`, the complete file audit was
repeated from that directory and reproduced the same 486 usable files, 14
broken seeds, and 4,860 matched events. The topology-clean light-eBrem subset
was joined by exact `(seed, entry)` to the established dominant surface-owned
loss-transition catalogue and plotted in bins 0--2, 3--4, 5--6, 7--8, 9--11,
and greater than 11. The respective counts are 69, 138, 485, 510, 83, and 791,
for 2,076 events total.

The GSF central-68 half-width versus LCIO is 1.3350% versus 1.5778% at 0--2,
2.1719% versus 2.4276% at 3--4, 0.6475% versus 1.8959% at 5--6, 0.5005% versus
1.7767% at 7--8, 0.4123% versus 1.6976% at 9--11, and 0.2071% versus 0.2493%
above 11. Thus the moved-tuple subset retains the established layer dependence:
small changes at 0--4 and strong core recovery at 5--11. Full RMS still worsens
at 5--6, 7--8, and above 11 because it remains sensitive to extreme GSF tails.

The reproducible plotting script is
`Reconstruction/RecGsfTracking/scripts/plot_new_gsf_pt_by_transition_location.py`.
Plots, exact summaries, and the joined event table are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/maxcomp24_new_tuples_2026-07-14/transition_location/`.
