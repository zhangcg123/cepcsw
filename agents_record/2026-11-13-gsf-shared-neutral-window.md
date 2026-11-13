# GSF recovery shares the new BP neutral window — 2026-11-13

User requested the same window for GSF after setting the BP card/batch baseline
to a theta half-window of 25 mrad (total width 50 mrad), phi half-window 200 mrad.

Reviewed the maintained GSF card, native flat writer documentation and current
comparison reader `TrackingPerformanceStudies/bp_fsrfloat_recoil_categories_20261113/analyze.py`.
Its `neutral_sum` reads BP's `collected_neutral_ecal_cluster_indices` for both
selected tracks, verifies the native pre-ECAL PFO/cluster links and energies,
deduplicates their union and builds massless four-momenta from cluster energies
and positions relative to nominal IP. That same addition is used with the KF
and GSF endpoint pairs. It does not repeat an independent hardcoded angular
selection. Consequently new BP tuples generated with the updated collector
already provide the new window to GSF + neutrals recovery.

GSF flat tuples retain all pre-reclustering ECAL clusters/links, not a windowed
subset, so no new GSF fit or flat export is needed solely to change recovery.
The historical reader intentionally validates its frozen input's old 10 mrad
theta half-window; it and existing cached masses/figures remain unchanged.
A new comparison must check its new manifest window and reuse its actual IDs,
rather than reinterpret the old cache as 25 mrad.

The separate default-off `EcalComponentConstraint` re-ranking prototype has
different semantics (all-cluster sum around a forward-GSF extrapolation). Its
properties/card settings remain unchanged and it is not enabled. No GSF
configurable property, fitter, card, library, batch job or plot was changed.
Updated the GSF and maintained-card READMEs and live baseline note to make the
shared recovery baseline explicit. This was a source/path review; no new
reconstruction, plot production or performance validation was performed.
