# Foreground x/+ and ECAL truth-energy labels — 2026-11-15

User asks for the cross and plus to appear on top of solid markers, photon truth
energy after each truth label, and electron truth energy as well. All eight
current event displays are refreshed, including PNG and PDF plus the two
reference copies retained at the root gallery.

The shared analysis renderer now assigns Baseline KF + / electron-truth x
zorders20/21, compared with at most10 for other axes artists. Both use a white
4.5-point outline around2.2-point black strokes and slightly larger symbol areas.
An explicit assertion in every render requires both marker layers to exceed
all other axes artists, including filled photon/cluster markers and annotations.
The accepted charcoal/gold/grey palette, distinct shapes and hatches remain.

Each visible photon-root label appends `entry_energy`, the photon postEnergy at
its saved first ECAL entrance, in GeV. A separate `e truth` label shows the main
electron's first-entry **total energy**, sourced from the existing verified
truth cohort `main_truth_GeV`. The cohort reader defines this as G4
postKineticEnergy + electron mass at the first ECAL entry:
`TrackingPerformanceStudies/ecal_truth_collection_widths_20261113/analyze.py`.
No generator/IP energy or reco cluster energy is substituted for this label.

Component legends retain explicit photon **birth** energies, a distinct
quantity. Converted originals have no invented ECAL-entry marker; their missing
entry notes now include their explicitly identified birth energy. Photon/electron
label placements were adjusted where needed to prevent overlap. No particle,
entry position, measured energy, native truth fraction, display window, event
selection, distance category or physics result changed.

Artifacts are in `TrackingPerformanceStudies/neutral_event_distance_examples_20261115/`:

- `truth_labels_before/`: prior scripts, metadata, images and event-payload hashes.
- `truth_labels_checks.json`:8 electron labels,18 photon labels, matching saved
  energies, foreground-layer assertions, unchanged payload hashes and unchanged
  geometry/selection fields. All8 complete renders reviewed; one long photon
  label in472/121 was moved off the filled photon markers and re-reviewed.
- Updated original and distance `plot_summary.json` files include truth-entry
  energy fields and marker zorders. Changed plotting scripts compile; existing
  projection and component-stack checks pass. No native REC read or refit.

Gallery publication replaces16 child files and4 original root reference files,
keeping the remaining100 images unchanged. Root/child membership and counts
remain84PNG/36PDF/120images,21 root sections,8PNG/8PDF/4 child sections. The
same healthy localhost8000 gallery serves the update; existing URLs stay valid.
The gallery agent's outgoing snapshot and proof are
`truth_labels_gallery_before/` and `truth_labels_gallery_verification.json`.

Only passive analysis rendering, documentation and gallery artifacts change;
no CEPCSW reconstruction/simulation implementation, card, build, install,
default or production job changes. Scripts/images/manifests stay uncommitted.
Only this record, AGENTS focus and its complete outgoing snapshot are checkpointed.
The frozen global status, laws, accessibility preference and commands remain
byte-identical. Outgoing status is preserved in
`2026-11-15-agents-before-truth-label-energies.md`.
