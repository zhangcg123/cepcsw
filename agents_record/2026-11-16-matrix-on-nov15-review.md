# Add the population matrix to the November15 review

User explicitly requests the existing FSR/eBrem population matrix on the
November15daily page. Append one section/study without replacing or reordering
any previous entry. This is a review-placement override: original study date
2026-11-12, review date2026-11-15; both are recorded in the catalog and visible
caption. The default recorded-study-date rule remains for other entries.

Use only the existing finalAND figure:
`event_radiation_matrix_20261112/event_fsr_ebrem_summary_gt1gev_and.png`.
This is the event-level strict>1GeV matrix, not a1%fractional cut. The summed
truth-photon birth energy is tested separately by type per selected electron;
an event flag passes if either leg passes, and Both can involve different legs.
Counts over98437classifiable events: Neither21602, FSR-only11517, eBrem-only
43739, BothAND21579. There are no fit-success/recoil/neutral-selection cuts.
Earlier OR and unthresholded figures are not added to this day page.

New catalog totals: November15has11study sections/34previews; November14stays
13/44 andNovember13stays12/43. Overall36study sections/121current previews,
with7historical study notes unchanged. Original source PNG and original
aggregate-gallery matrix section remain unchanged. No image copying/moving/
regeneration, recalculation, reconstruction/refit, code/card/build/install change.

Live Nov15page HTTP200 contains all33old previews plus the requested matrix
exactly once, with the correct description and root daycount34. All121daily
preview endpoints return200; other day pages retain13/44and12/43. The matrix
download hash matches both original source and existing aggregate-gallery copy.
Root validates2465PNGs/11sections and the catalog3days/36entries/121refs. Server
PID1404151 stays unchanged; no restart. Main independently compares the old
Nov15ten entries and both other day objects to the pre-edit catalog: identical.
Caption/source_date/review_date retain the explicit placement exception.

Metadata inventory/README/live verification are updated in the existing
uncommitted daily-review artifact directory. Pre-edit snapshots are at
`/tmp/gallery_daily_review_matrix_nov15_before/`. Original aggregate matrix
section is preserved. This is presentation of an existing result, not a new
population calculation or1%threshold study.

## Follow-up: recoil first, population matrix second

User next requests moving "Recoil mass from the completed BP H0_F/theta25
production" from November14toNovember15, with that study first and the
population matrix second. Move the existing BP row and its6images (do not
duplicate); retain original source_date2026-11-14, review_date2026-11-15 and
all original description/notes. Keep matrix source12/review15 provenance.
All other Nov15entries retain their relative order; remaining Nov14entries
and all Nov13entries are identical to their pre-edit objects.

New day counts: Nov15=12studies/40previews; Nov14=12/38; Nov13=12/43.
Overall36studies/121image paths and7historical notes are unchanged. Main
compares against the valid snapshot
`/tmp/gallery_daily_review_move_bp_h0f_before/daily_review.json`: requested top2
order, one BP move, all121image paths, BP title/6images/README/full old
description and all non-target rows/orders pass. Independent result is saved
as `main_review_top2.json` in the uncommitted daily-review artifact directory.
The main session's own initial snapshot raced the edit and captured the new
state; it is not used as the before-state comparison.

Final live checks confirm the requested two top-level study headings in order,
all3daypages and their121preview links HTTP200, and the6moved BP downloads
hash-match their local source files. The matrix and aggregate copy still match.
The original standalone BP study and aggregate page remain accessible. Root
validation stays2465PNGs/11sections and catalog36studies/121refs; server
PID1404151 is unchanged without restart. No image writes or fit/analysis runs.
README, inventory and live_verification.json reflect the new layout.
