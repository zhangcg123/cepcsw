# Purity gallery cleanup and intended photon-identification objective

The user requested removal of exactly these four sections from the webpage:

- Truth-selected >95% single-photon neutrals: isolation and electron distance (4).
- Single-photon ≥95% purity: individual identities and selection (4).
- Neutral-cluster purity versus distance by event radiation category (6).
- Conditional purity of FSR/eBrem-associated neutral clusters (11).

Remove the 25 served PNGs and matching PDFs from
`TrackingPerformanceStudies/bp_matched_recoil_pt_20261110/`, preserving original
study outputs and archiving served copies outside the gallery under
`TrackingPerformanceStudies/neutral_purity_gallery_cleanup_20261114/`.
Save the pre-removal manifest/hash snapshot and verify retained images and
15 remaining sections unchanged, exact-once PNG coverage, no removed section
or uncategorized reappearance, and healthy HTTP page/retained PNG. Expected
PNG change87->62, sections19->15; actual PDF count is recorded in verification.
Completed:25PNGs+25PDFs archived;62PNGs/15sections/14PDFs remain,76retainedimage
hashes and15retainedsections unchanged. All four titles are absent, no
uncategorized plots, page/retainedPNG HTTP200. Reused ailogin002PID443262.
The existing localhost8000 server can be reused if healthy. The cleanup does
not retract historical numerical evidence or authorize a new analysis/cut.

## Understanding of the scientific objective

Find a usable reconstructed-level selection for clean neutral PHOTON clusters
associated with the selected production Z->ee electrons. Truth is used first
to identify the desired population and evaluate candidates. Each cluster's
purity uses its FULL measured energy as denominator, with native saved MC
weights providing normalized response attribution. One original photon and
its shower descendants must contribute >95%; distinct photons remain separate.
Summing FSR/eBrem contributions, or multiple photons of the same origin, must
not certify purity. Electron-dominated neutral PFO showers are background.
Attribution weights remain proxies, not exact deposited-energy measurements.

Then inspect reconstructed isolation, electron distance and other actually
available measurements as discriminants. Truth labels must not be a deployed
selection feature. Clean-versus-contaminated comparisons should expose what
these observables distinguish. A useful selection needs both low contamination
and adequate retention of clean photons; neither a large mean purity nor
rejecting almost every clean photon settles this task. Report per-cluster
purity, arithmetic mean, fraction individually passing95% and retention as
different quantities. FSR/eBrem origins can be diagnosed separately without
adding them to meet the single-photon threshold. Ultimate relevance is photon
recovery without reusing electron-shower energy; no recoil benefit is claimed.

No new plots, classifiers, reconstruction or source/card/install changes are
part of this request. The full outgoing AGENTS focus is preserved in
`2026-11-14-agents-before-purity-gallery-cleanup.md`. Earlier study records,
selected-cluster IDs, CSVs and plot originals remain intact. Current gallery
verification belongs to the archive's `gallery_verification.json`.
