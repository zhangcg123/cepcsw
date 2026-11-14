# Four more event displays, including two >95% single-photon neutrals — 2026-11-15

User asks for more displays in the accepted style and two clear neutrals with
purity>95%. Four new distinct current-REC events are added to the dedicated
gallery, preserving the previous eight images and current style.

## Highlighted examples and strict definition

| Job/event index/track | Focus/charged | Measured GeV | 3D centre distance mm | Single-photon native fraction | Dominant original photon |
|---|---|---:|---:|---:|---|
|70/140/7|N44/C0|7.252163887|335.020492|1.0|FSR gamma16|
|92/84/26|N20/C9|25.150384903|381.633399|0.9973967731|eBrem gamma248|
|21/3/17|N55/C1|1.670076013|63.430559|0.5823534995|eBrem gamma363|
|287/20/24|N25/C1|18.007097244|373.330669|0.5628992804|eBrem gamma392|

All indices are zero-based. The first two focus clusters satisfy the **strict**
P_gamma>0.95 condition before rounding. Each dominant photon belongs to the same
selected Z electron and has an actual original-photon ECAL entrance. Their saved
entry energies are7.267171GeV (gamma16) and32.554375GeV (gamma248).

P_gamma=max over individual original photon roots of the sum of their unique
shower-descendant native weights, divided by the cluster's full native MC-weight
sum. Electron and other response remain in the denominator. It is equivalently
that assigned photon energy component divided by measured cluster energy; it
is not FSR+eBrem summed or the sum of multiple same-origin photons. Ambiguous
ancestry stays outside the photon numerator. The figures state the maximum
single-photon purity in the header and retain every displayed root separately.

The initial92/84 candidate N46 is a4.209GeV fragment with100% native attribution
to the same gamma248. The final highlighted focus is the larger25.150GeV N20
cluster, whose centre is near that photon's actual entrance. Both clusters are
shown; they are not two independent photons. High purity does not mean complete
photon-energy recovery. For70/140, the7.252GeV neutral response is close to the
7.267GeV truth entrance energy; that observation is example-specific.

The two mixed examples contrast close and separated clusters. N55 in21/3 is
41.6395%FSR gamma14 and58.2353%eBrem gamma363; charged C1 has substantial hard
photon response, and the main-electron entry energy is only0.111048GeV. N25
in287/20 is43.7099%FSR gamma14 and56.2899%eBrem gamma392 despite373mm distance
from the selected charged cluster. No population efficiency or separation cut
is inferred from these selected illustrations.

## Current source and rendering

Sources are existing `higgs_eeHX_ISRFSRON_REC_ECALTRUTH_500jobs_200evt_20261112`
files, with exact identities/owners/cluster IDs in
`TrackingPerformanceStudies/neutral_event_more_20261115/selected_events.json`.
The existing55391-neutral cache supplied candidates; clear-candidate search
required a charged reference,3–30GeV energy,200–650mm distance and individual
photon fraction>95%. Native reads establish geometry and actual entrances.

Six distinct full native event entries were inspected; four accepted.161/155
and200/125 are endcap views and were not retained for this barrel-reference
projection.208/117 was additionally inspected with track-reference branches only
and rejected as endcap. All accepted event payloads contain all event clusters;
N20's energy/individual-root weights/distances were independently checked after
changing the initially considered N46 focus. No simulation or reconstruction ran.

`run.py` reuses the existing reader and accessible renderer, adding passive
caller support to omit repeated reference copies, append the single-photon
fraction and set case-specific label positions. The renderer's existing default
caller behaviour remains. No existing plot is regenerated. Accepted style:
charcoal/gold/grey, charged circles/neutral triangles, independent FSR stars/eBrem
diamonds, foreground white-outlined electron x and Baseline KF +, individual-root
bar hatches and black labels. Photon/electron map labels use saved ECAL-entry
energies; photon legends retain explicit birth energies. Projection/window/
marker sizing are unchanged; native MC weights remain unvalidated as physical
3D deposit purity, and reconstructed hits have no individual-photon truth links.

## Verification and artifacts

Study: `TrackingPerformanceStudies/neutral_event_more_20261115/`.
The shared reader checks current source size/mtime, IDs/indices, hit links,
normalized fractions, energies and cached radiation/isolation fields. Independent
`verify.py` passes7 individual-root fraction checks,4 measured energies and
3D distances, current source identities, same-electron ownership, exactly two
highlighted strict>95% cases and their saved original entrances. Full weight
closure, projection, foreground-layer and truth-energy assertions pass. Both
analysis wrappers compile. All4 renders reviewed, with3 label layouts adjusted
and final renders reviewed. `verification.json`, `render_checks.json`, logs and
`plot_summary.json` retain exact evidence, hashes and render configuration.

No CEPC reconstruction/simulation source, algorithm, card, build, install, default
or production changes. Scripts/data/images/manifests remain uncommitted. Only
this durable record, AGENTS current focus and its complete outgoing snapshot
are checkpointed. All global status/laws/commands remain byte-identical. The
outgoing focus is preserved in
`2026-11-15-agents-before-more-clear-neutrals.md`.

## Gallery publication

The dedicated plot-gallery workflow adds only4PNG/4PDF files to the existing
`bp_matched_recoil_pt_20261110/neutral_event_displays/` child. A clear>95% section
highlights the first two; the mixed additions join Near/Far groups. Existing
120 image hashes and all root section objects/order remain fixed. Child grows
to12PNG/12PDF/5sections; root recursive88PNG/40PDF/128images and21sections.
Outgoing manifests/hashes are in `gallery_before/`; live source-copy/HTTP and
preservation checks are in `gallery_verification.json` under the new study.


Publication verified at localhost8000/neutral_event_displays/ on ailogin002,
existing PID3994352 (no restart). All120 previous image hashes and the root
manifest remain exact; only8 new images were added. Child12PNG/12PDF/5sections,
root88PNG/40PDF/128images/21sections; everyPNG matches one category. Both clear
example PNGs returnHTTP200 and their served hashes match the reviewed sources.
The new clear section is first; prior child titles/descriptions/relative order
remain intact and the two new mixed patterns join Near/Far.
