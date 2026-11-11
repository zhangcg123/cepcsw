# 5,000-event electron-centered truth ECAL photon distributions

User requested5000events, electron at center, x=deltaTheta and y=deltaPhi,
with FSR/eBrem shown separately. Inputs are25current REC_ECALTRUTH files,
200events each, evenly spread through the500completed jobs. Seed list and
source identities are frozen in summary.json. No native software rerun.

Each plotted point is an ORIGINAL photon with both its own and its selected
production electron's first saved G4 ECAL entry. Form rays from the same
selected electron truth production vertex to both entries, then plot
x=theta_gamma-theta_e and y=wrapped(phi_gamma-phi_e), in mrad. The electron is
implicitly at(0,0); no electron marker is drawn. These are impact-position
angles, not birth momentum angles or reconstructed cluster centroids.

FSR identity follows the saved unique generator-history link to the final
stable production electron. Material eBrem follows the exact emitting G4
parent trackID. Selected MC-to-G4 identity is resolved through saved complete
silicon SimTrackerHit provenance and MC relations, checked for uniqueness and
against known FSR parentG4 IDs. No angular truth match or momentum inference.
All9916selected production tracks have unique G4 identities in this sample.

Both photon types include all recorded birth energies. NO neutral/charged
cluster selection, fit-success gate or photon energy cut. The previous
upper2% reconstructed-cluster energy trim is not applied to truth photons.
Thus this sample includes photons merged into charged clusters and must not
be confused with the conditional resolved-neutral acceptance denominator.
Original photons without ECAL entries (including upstream conversions) are
excluded, not extrapolated or replaced by daughters.

| Type | Selected-owner records | Missing electron entry | Missing photon entry (electron valid) | Plotted pairs |
|---|---:|---:|---:|---:|
|FSR|9067|16|2805|6246|
|Material eBrem|17686|538|979|16169|

Full figure uses +/-3200mrad on both axes, covering every valid pair. Zoom
uses deltaTheta +/-150mrad and deltaPhi +/-400mrad;1590FSR and242eBrem points
lie outside this zoom and are labelled explicitly. The dashed reference
rectangle is |deltaTheta|<=50mrad and |deltaPhi|<=200mrad. Its truth-pair
geometric counts are3951FSR/15825eBrem; these are NOT neutral collection
energy/count efficiencies. The full valid pair counts are always shown.

Validation:5000events/25unique sources,22415unique photon-pair keys, complete
population closure, all source size/mtime identities, valid original photon
relations, MC/G4parent agreement and no ambiguous selected electron G4 IDs.
Synthetic checks independently confirm theta displacements go to x, phi to y,
zero centers and phi wrapping. Compressed CSV was read back and populations
checked; PNG/PDF hashes and visual layout verified. Input simulation/tracking,
PFA, fitter sources, run cards, shared installation and jobs are unchanged.

Artifacts: analyze.py, summary.json, validation.json, analysis_code_hashes.json,
truth_entry_pairs.csv.gz, run.log, plot_files.json, plots/ full/zoom PNG/PDFs.

## Project-memory update

Outgoing complete AGENTS is preserved in
2026-11-12-agents-before-centered-5k-photons.md. Only current focus is replaced;
active scope/laws remain intact. No history-directory migration. Previous
1000-event FSR diagnostic and full100000-event neutral acceptances remain
unchanged; this5kplot uses all valid original truth photons instead. The
main session explicitly stated this definition before plotting. Gallery
publication is delegated to the plot-gallery skill's dedicated agent.

As a separate read-only input-integrity control, actual top-level ROOT branch
names (not only PODIO metadata IDs) were compared between original REC_TPCFIX
seed1 and REC_ECALTRUTH seed1. No old branch is lost. Added branches are exactly
the new cluster-particle association and its rec/sim relation banks. This
confirms earlier full-REC preservation for that directly checked file.

## Gallery publication

Dedicated plot-gallery agent added full/zoom figures as a new first section.
Gallery validates16PNGs/7sections with each image covered exactly once. Live
page and both PNGsHTTP200 with final source hashes matching copies, independently
reviewed by main. Existing ailogin002 localhost8000 server PID2122174 updated
live without restart; all previous plots/sections preserved. Manifest and
copied analysis figures remain uncommitted. No analysis rerun for publication.
