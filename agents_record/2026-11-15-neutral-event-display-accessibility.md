# Accessible neutral event displays — 2026-11-15

User reports difficulty distinguishing red and blue and asks to optimize the
plot colours. Both just-created current REC event displays (job369/event82/
track6 and job34/event52/track8) were updated together, including PNG and PDF.
This preference is now an active plotting law in AGENTS.md for future work.
The existing event facts, component definitions and limitations remain in
`2026-11-15-current-rec-neutral-event-displays.md`.

## Presentation

- Matched charged hits: charcoal circles; neutral-cluster hits: gold triangles.
- Original FSR ECAL entrance: gold star with black outline. Original eBrem
  entrance: grey diamond with black outline. Root IDs have black direct labels.
- Electron truth entrance: black x; Baseline KF ECAL reference: black +.
- Component bars: plain grey main electron, gold diagonal stripes FSR,
  pale dotted first eBrem root, grey cross-hatched second eBrem root,
  solid black Others. Root identities remain separate.
- Percentage labels are black on light fills and white on black, with small
  matching-fill text backgrounds to keep hatching away from the letters.
- Plot geometry, selections, values, energies, window, projection and viewport
  are unchanged. Display marker shape/opacity changed; hit marker areas retain
  the same energy scaling and display-only cap.

Distinction is encoded in shapes, hatches and text as well as colour; the
presentation does not rely on red/blue discrimination. Both complete renders
were visually reviewed. This does not claim a clinical colour-vision simulation
or user-confirmed perception test.

## Evidence and scope

Artifacts remain in `TrackingPerformanceStudies/neutral_event_displays_20261115/`.
Original script, README, metadata and four image files are in
`accessibility_before/`. `accessibility_checks.json` confirms identical event
payload hashes and all non-style plot metadata; plot fraction-closure and
projection checks pass. The changed plotting script compiles. No native REC
file was read again, and no physics analysis, fit, build or installation was run.
All analysis scripts, images and manifests remain uncommitted.

The dedicated gallery agent publishes only these two PNG/PDF replacements,
updates only their existing final section's colour/marker explanation, and
verifies the healthy localhost8000 gallery. The unrelated100 images and19
sections must remain byte/object identical. The outgoing gallery snapshot and
hashes are in `accessibility_gallery_before/`; live publication proof is in
`accessibility_gallery_verification.json`. Existing event-description physics
facts and native-MC spatial-purity caveat remain unchanged.

AGENTS.md only gains the user's accessibility law, current-display presentation
notes, and a link here. No outgoing unique status is removed; existing global
laws and historical records are retained.
