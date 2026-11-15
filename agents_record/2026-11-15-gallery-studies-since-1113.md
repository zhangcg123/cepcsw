# Apply the study-page layout to plotting directories since November13

User asks to apply descriptions+previews on one study page to all plotting
directories made since1113. Scope is inclusive2026-11-13through2026-11-15,
using the recorded study date in directory names rather than mutable mtimes.
There are35plot-bearing study roots and304PNGfiles including historical copies
under the existing served TrackingPerformanceStudies root. Reports without PNGs
are not plotting directories (6dated no-PNG directories recorded). Four other
plot-directory README date checks establish August/September provenance and
exclude them from the November scope. The older aggregate BP/event-display pages remain
accessible and retain their existing descriptions/sections.

Reuse the shared inline-`plots/` renderer from the preceding task; no file move,
plot regeneration, analysis rerun, reconstruction, fit or CEPCSW installation change.
Current direct-PNG and `plots/` studies render descriptions and images together.
Genuine category/substudy navigation and original PNG URLs remain available.

Historical output is distinct from current figures. In particular,
neutral_purity_cluster_mean_20261114 retains obsolete electron-excluded isolation
figures in its subdirectories; neutral_purity_gallery_cleanup_20261114 contains
copies explicitly removed from the main gallery; neutral_iso_include_electron_
20261115 is an audit/archive whose corrected outputs were published elsewhere.
Do not resurface those figures as current plots. Notes/files are retained.
The previously description-free fsr_qed_diagrams_20261113 page needs a caption
from its existing draw script and gallery provenance, without new theory claims.

Concrete obsolete-isolation definitions are in the original source READMEs:
neutral_purity_species_20261114:30, neutral_isolation_selection_20261114:19 and
neutral_purity_metrics_20261113:40. These exclude selected main-electron charged
clusters. Targeted grep initially also found neutral_isolation_radius_20261114:45
and neutral_photon_id_20261114:30/101, but review caught the distinction: radius
already uses charged-inclusive isolation and merely compares an old control;
photon_id explicitly documents a November15 correction removing excluded curves.
Do not call those corrected figures electron-excluded. The user previously
retired the radius/held-out gallery section and all but the first photon-ID
figure; any historical labels must cite retirement rather than a wrong current
isolation definition. Preserve the corrected current figure where published.
Conversely,
neutral_iso_purity_correlation_20261114 starts with a November15 correction;
keep its current electron-inclusive figures visible and distinguish its
previous_formula_plots archive. The single_photon_purity_20261114 current rule
also explicitly uses inclusive isolation and is not suppressed on suspicion.

The narrow generic presentation control is per-directory `gallery.json`
`show_previews=false`, defaulttrue, inherited by descendant directory pages.
Any false ancestor suppresses automatic previews even if a child says true.
It does not alter data, direct PNG routes, root study navigation or category
coverage. A boolean is required by the renderer and launcher validator. This
allows historical notes/status to remain visible without redisplaying retired
electron-excluded plots. No physics-directory names are embedded in the server.
Personal skill instructions describe the same contract.

Uncommitted audit artifacts live in
`TrackingPerformanceStudies/gallery_since_20261113/`.
`inventory_geometry.json` records all35directories, dates,304relative PNG paths,
and direct/plots/other-subdirectory count partitions. `png_hashes_before.json`
captures every scoped PNG before presentation edits. No full-root image hash
scan is needed for this bounded task. The dedicated plot-gallery agent owns
metadata changes, live page checks and any necessary personal-renderer change;
main owns review, project documentation and its focused Git checkpoint.

## Completed verification

All35study pages return HTTP200, retain their README descriptions and genuine
child-folder links, and have the expected preview count with no redundant
`plots/` folder card. There are28preview-enabled study roots (120automatic
previews on their parent pages) and7historical roots with notes/status but no
automatic previews. The historical roots are cluster_mean, gallery_cleanup,
iso_include_electron, purity_species, isolation_selection, purity_metrics and
isolation_radius in the dated directories above. The corrected correlation
parent keeps5previews, its previous_formula_plots child has0, and corrected
photon_id retains6. Existing main-gallery sections are not reintroduced.

Root directory navigation still lists every scoped study without expanding
their images on the index. The server safely restarts as PID1332798 on
ailogin002, serving TrackingPerformanceStudies at http://127.0.0.1:8000/.
Root validation remains2465PNGs/11sections with exact-once category coverage;
the8new archive manifests also pass exact-once checks. Ten representative
direct PNG routes (including hidden archived files) return200 and hash-match.
All304scoped PNG hashes independently match before/after, not just samples.

The focused renderer fixture passes default visibility, unchanged root index,
inherited false despite childtrue, visible suppression notice on descendants,
and rejection of non-boolean values. Main session reviews the personal renderer,
launcher validation, metadata/reasons and every row of the live35-page result.
No physical-purity or tracking validation claim is implied by these UI checks.

Artifacts now also include before_pages.json, verification.json with all35
before/after page counts, png_hashes_after.json, image_preservation.json,
metadata_review.json and the audit README. All are uncommitted analysis artifacts.
The QED study gains its missing description; archival source READMEs gain status
notes while retaining their original text. Personal gallery.py, plotweb.py and
SKILL.md implement/document the generic switch; no CEPCSW algorithm/card changes.
