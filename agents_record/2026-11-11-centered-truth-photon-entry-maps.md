# Electron-centered truth photon ECAL-entry maps (2026-11-11)

The user noted the current10mrad theta/200mrad phi neutral window was designed
for eBrem and requested truth photon entry positions before choosing an FSR
window: each electron at the center, no electron entrance marker, photons only.
Two new scientific PNG/PDFs implement this; no recovery window or fit is changed.

## Input and coordinates

Reuse the frozen compressed coordinate CSV from the preceding truth-geometry
study:498 completed BP sources x200=99,600 events,197,637 selected production
tracks, no BP/GSF fit-success selection. Usable pairs are123,581 FSR and317,862
material eBrem,441,443 total. FSR uses saved generator-origin status1 with
final-electron MC identity and known G4-parent consistency; eBrem uses exact
emitting-electron G4 ID. No angular criterion defines photon identity.
All photon birth energies and both electron charges are pooled, no charge
folding, energy threshold, weighting or random subsampling. Each marker is one
original photon/electron entry pair.

For each pair subtract the main electron's original first truth ECAL entry
from the photon's original first truth ECAL entry. Rotate delta x into the
local azimuthal, polar and radial unit axes of the electron-entry ray from
its saved truth production vertex. The mm plot is an exact3D Cartesian
rotation, not an assumed-radius angle conversion or surface arc length.
Retaining the radial component prevents opposite-surface entries from folding
onto a false near-center2D tangential projection. The electron is implicitly
(0,0,0), with no electron marker. Core cube±200mm; wide cube±7000mm.

The angular companion plots photon-minus-electron phi(wrapped to[-pi,pi])
and theta of those truth entry rays, in mrad. This is entry-position geometry,
not momentum/emission angles. Core square±200mrad; wide square±pi*1000mrad.
Each figure has FSR left/eBrem right, core top/wide bottom. Only photon scatter
artists appear, no electron marker or existing/proposed window overlay.

## Populations and limits

| Display | FSR shown/full | eBrem shown/full |
| --- | ---: | ---: |
| Local3D core cube | 79,048/123,581 (64.0%) | 309,643/317,862 (97.4%) |
| Angular core square | 88,596/123,581 (71.7%) | 311,468/317,862 (98.0%) |
| Wide views | 123,581/123,581 | 317,862/317,862 |

These displayed fractions are not collection/recovery efficiencies. Both
original photon and electron entries must exist. Omitted FSR rows:390 missing
electron entry and55,384 missing photon entry with valid electron entry.
eBrem:9,059 and19,258. Upstream conversion daughters/shower centroids are not
substituted; missing entry alone does not establish its cause. No reconstructed
cluster or PFO position is used. This conditional original-photon truth sample
cannot by itself establish a calorimeter FSR recovery efficiency or select a
validated window. No new FSR recovery steering has been chosen or implemented.

## Checks and artifacts

Synthetic identity,3-4-5, common translation, azimuthal rotation and antipodal
cases pass. All441,443 exact local-frame distance checks agree within1.82e-12mm.
441 deterministic independent orthonormal-basis/inverse/angle checks agree,
maximum inverse error9.10e-13mm. Photon keys are unique, original population
counts and source SHA256/stat unchanged. Both wide displays include every
valid photon; electron marker count0. Both layouts visually inspected.
Rasterized point layers keep all points while PNG/PDF totals remain~1.6MB.
All46 preceding PNG/PDF artifacts are byte-identical. No ROOT file is reread
or duplicate coordinate cache written. No source tuple, library, recorder,
tracking/PFA/fitter implementation, original card or batch state changes.

Uncommitted study artifacts in
TrackingPerformanceStudies/centered_truth_photon_entries_20261111/: analyze.py,
analysis.log, README, summary.json, validation.json, prior-plot SHA256 manifest,
PLOTS_READY, truth_photon_entries_electron_centered_{mm,angles}.png/pdf. Copies
join the same bp_matched_recoil_pt_20261110 gallery. Personal plot-gallery agent
extends the existing geometry section with these two maps first and moves that
section first, preserving all other section relative order;25PNG/9sections.
The agent verified live definitions/counts and both served PNG hashes on
ailogin002, PID2122174, http://127.0.0.1:8000/. Parent reviewed the manifest,
25-PNG exact-once coverage and all46 old/four new artifact hashes. Prior source
definitions/coverage:
[the original geometry record](2026-11-11-ecal-photon-electron-angles.md).
Complete outgoing focus is preserved in
[the snapshot](2026-11-11-agents-before-centered-photon-maps.md).
