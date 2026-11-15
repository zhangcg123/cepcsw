# One review page per day

User asks to organize plots and descriptions by day, showing that day's studies
on one webpage without opening study/asset subdirectories. Continue the recent
November13–15scope:35study roots,120current previews and7historical/retired
study notes. Use the recorded study dates, not mutable file modification times.

| Day | Studies, including historical | Current plot previews | Historical studies |
|---|---:|---:|---:|
|2026-11-15|10|33|1|
|2026-11-14|13|44|5|
|2026-11-13|12|43|1|

The personal gallery uses a declarative catalog pointed to by the root
`gallery.json` field `daily_review_manifest`. Current catalog:
`TrackingPerformanceStudies/gallery_daily_review_20261115/daily_review.json`.
Each `/daily/YYYY-MM-DD/` page groups its studies and plot sections, with visible
descriptions, all current thumbnails and full README notes expandable in-place.
Root day links appear newest-first. Original study pages and direct PNG routes
remain available. No PNG file copying, moving or regeneration, no analysis,
reconstruction/refit, CEPCSW card/build/installation change.

Catalog images are the120existing paths actually shown on the verified current
study pages. Historical studies retain notes/status without previews. Both
study-level and per-image ancestor `show_previews=false` controls remain binding;
neither accidental catalog inclusion nor a childtrue flag restores old variants.
The corrected photon-ID/correlation views remain enabled. Day headers/root link
counts use effective visible images, not raw catalog lengths.

Loader validation covers schema, real ISO dates/newest-first unique days,
unique resolved study/image paths, contained directories, contained Markdown
notes and PNG files, and images belonging to their declared study. Metadata
paths remain inside the served root. Unknown/bad day routes return404. Markdown
and plot cards reuse existing safe renderers; no physics-name logic in server.

Main independently checks every catalog study against the prior35-page audit:
exact35study/date assignments, all expected canonical120current image paths,
no duplicates/archive expansion, and existing descriptions/README paths.
Focused fixtures cover root day ordering, descriptions, stable URLs, inherited
and per-image preview suppression, effective counts, notes and unknown-day miss.
New day folder links carry the data-name required by the existing root filter.

All catalog/inventory/verification artifacts remain uncommitted under
`TrackingPerformanceStudies/gallery_daily_review_20261115/`. Personal renderer,
launcher checks and SKILL.md maintain the presentation contract; the historical
CEPCSW server copy and all scientific plot-generation sources remain unchanged.
Update the catalog when adding/moving studies or current plots; dates derive
from recorded study provenance, never metadata-edit time.

## Live result and final checks

All three day pages are HTTP200 at http://127.0.0.1:8000/daily/YYYY-MM-DD/,
with10/13/12studies and33/44/43previews for Nov15/14/13. All120unique current
PNG links return image/png HTTP200. Root day links are newest-first and preserve
the existing directory filter's data-name contract. Unknown day returns404.
Converted, BPtheta25, older BP aggregate and event-display study pages, plus
original image routes, remain available. The managed server now runs as
PID1404151 on ailogin002.ihep.ac.cn, with the same TrackingPerformanceStudies
root and localhost8000 port.

Launcher validation passes2465rootPNGs/11sections exactly once and3days/35studies/
120daily image references. Focused fixture and main review cover inherited and
per-image suppression, effective counts, contained references, date/section
ordering, descriptions and original image URLs. Main rechecks every one of the
304scoped PNG hashes after the update; all remain unchanged. Existing root
category sections are preserved. Artifacts README, daily_inventory.json,
daily_review.json, live_verification.json and main_review.json persist in the
uncommitted artifact directory.

Complete outgoing AGENTS is preserved in
`2026-11-15-agents-before-daily-plot-review.md`. The new focus replaces the old
one; global laws/commands remain intact, with only the requested daily-review
maintenance preference added. Previous converted-photon/recoil evidence remains
in its original dated records and outgoing snapshot.
