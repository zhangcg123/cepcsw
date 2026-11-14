# Charged electron always included in isolation — 2026-11-15

User explicitly states the electron charged cluster should never be excluded
and requests removal of the corresponding plots/curves from the recent studies.
This is now an active project law: all valid charged electron ECALclusters
inside the angularcone stay in the isolation neighbourenergy sum. Exclude
only the neutralcandidate itself. Do not use/display electron-excluded variants.

## Applied plot cleanup

- Remove the dedicated `iso_purity_without_electron_R50` PNG/PDF pair.
- Remove excluded-electron curves and captions from exactly three other pairs:
  `iso_purity_correlations_vs_radius`, `photon_id_isolation_clean_fraction`,
  `photon_id_selection_tradeoff`.
- Keep the charged-electron-inclusive plots/curves in both recent sections.
  The correlation section now has five plots; photon-ID section still six.
  Definitions explicitly state the charged electron cluster is included.
- Update only those two section descriptions/patterns. Preserve all other
  15 section objects and92unaffected image hashes. All previous removals remain
  absent. Gallery changes74->73PNGs,17sections,26->25PDFs.

Archive/verification artifacts are under
`TrackingPerformanceStudies/neutral_iso_include_electron_20261115/`:
pre-change manifest/all100imagehashes, previousversions of8removed/modified
files, oldanalysiscode/summary, removedsourcepair, currentverificationJSON and
galleryverificationJSON. All removals are reversible; no historical data
deletion. Current plotting paths cannot regenerate excluded variants.

## Preserved physics and limits

The same55391candidates/currentREC-derived cache are reused. The entire
inclusivecorrelation statistics dictionary is unchanged exactly, including
R50Pearson.059011/Spearman.175317 andR300.274913/.397462. Selectedphysics
rules already used inclusiveI, so no selection or fittedendpoint changes.
Historical excluded-featurecolumns/frozensearches/datedrecords remain solely
for provenance and must not override the new rule. No newRECread, simulation,
reconstruction,cutsearch,source/card/build/install/default/production change.

Three modifiedrenders reviewed. Self-checks verify no excluded variables in
active plot code or profiles, removedsourceplot absent, inclusivevalues exact,
cacheSHA fixed and22currentplot hashes matching. Threeplotting scripts compile.
Gallery audit checks73PNGs exact-once,removedpairoffpage,threeupdatedPNGHTTP200,
source/local/served hashes and92untouched oldimages. Reuse healthylocalhost8000
server if unchanged. Complete outgoing AGENTS is preserved in
`2026-11-15-agents-before-isolation-include-electron.md`; the added project law
is the only intended change to its global prefix.
