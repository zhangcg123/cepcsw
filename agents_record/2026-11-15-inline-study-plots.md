# Show study descriptions and plots on one page

User requests descriptions and previews together under each study subdirectory,
without another `plots/` directory click. The personal plot-gallery renderer
now inlines safe PNG descendants of an exact `plots` child on non-root pages.
It retains the original image URLs and omits only that redundant folder card.
The root study index and genuine child-study/category links are unchanged.
No image, analysis output, physics source, run card or installation is changed.

The parent README and manifest descriptions render with the previews. When no
parent manifest exists, an available `plots/gallery.json` supplies its section
descriptions. No physics interpretation changes. The managed restart requires
one narrow top-level `TrackingPerformanceStudies/gallery.json` section for the
three new `converted_photon_recoil_20261115/plots/` PNGs, previously absent from
that root manifest. Existing sections and image files are preserved.
The shared card helper keeps its existing callers and supports the shortened
display label without changing the actual `plots/...png` image address.

Maintained files are personal
`~/.agents/skills/plot-gallery/scripts/gallery.py` and its `SKILL.md` navigation
contract. The historical project server copy is not changed. A dedicated
plot-gallery agent owns the focused assertions, managed restart and live HTTP
verification; main session reviews the renderer and final evidence. Reuse the
existing safe PNG list rather than scanning the plots subtree a second time.

The converted-photon study now opens directly at
`http://127.0.0.1:8000/converted_photon_recoil_20261115/`.
This presentation change does not rerun or alter the conversion/recoil study.

Focused fixtures verify recursive inline previews, parent descriptions,
child-manifest fallback, unchanged root indexing and genuine child links,
and original image URLs. Syntax checks use a temporary bytecode cache rather
than writing in the protected personal skill directory. Root and both existing
BP/event-display manifests validate; all10previous top-level sections are intact.
The personal launcher safely replaces PID640260 with PID1193516 on ailogin002,
still serving TrackingPerformanceStudies at localhost8000. Live converted-study
HTML has its README above all3previews and no `plots/` folder card. The old
BP gallery and event-display child navigation remain accessible.
Final validation counts: root2465PNGs/11sections; BP gallery91/22; event
displays12/5. Original `/converted_photon_recoil_20261115/plots/` still responds
200 with3previews; new, legacy-BP and event-display PNG downloads respond200
and hash-match local files. Existing BP/child manifests and all10old root
sections are unchanged. Parent-manifest precedence also passes the fixture.
Personal-file backups are in `/tmp/plot-gallery-presentation-snapshot/`.
