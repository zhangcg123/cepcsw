# Neutral event displays by distance to charged cluster — 2026-11-15

User accepts the accessible style, asks to retain it, create a dedicated
subdirectory and show more distinct events spanning far/middle/near neutral
separation from the charged cluster. Six new current-REC examples were created,
plus unchanged copies of the two earlier examples for reference.

No reconstruction/simulation source, tracking/PFA algorithm, card, build, install,
default, fit or production job changed. All work is read-only native-data
analysis and plotting. The existing analysis reader, original-photon identity
resolver and accessible renderer are reused. The renderer gains passive focus,
caption and label placement support, distinct hatch choices for additional roots,
and correct radiation-origin/missing-entry notes. These analysis scripts stay
uncommitted; only project memory is checkpointed.

## Definition and selected cases

Distance is the minimum3D Euclidean distance between the selected neutral's
reconstructed pre-reclustering ECAL cluster centre and a valid ECAL cluster on
the PFO matched to that same selected CompleteTracks electron. In each selected
example there is exactly one matched charged cluster. This is not a cone angle,
track-entrance distance, projected separation or gap between shower edges.

Near: d<100mm. Middle:100<=d<250mm. Far:d>=250mm. These are illustration groups,
not proposed reconstruction cuts. Each new image labels the selected neutral,
charged partner and exact3D distance; a black ring identifies the focus neutral.

| Group | Job/event index/track | Focus / charged | d mm | Native FSR% | Native eBrem% |
|---|---|---|---:|---:|---:|
| Near |181/115/11|N25 / C2|63.1135|45.8634|51.3203|
| Near |141/7/9|N41 / C0|64.1545|95.8734|1.4422|
| Middle |258/153/27|N29 / C4|179.1864|60.1988|37.7702|
| Middle |377/149/19|N27 / C1|176.3505|97.7174|1.0464|
| Far |472/121/20|N19 / C7|389.8249|25.6866|74.2903|
| Far |202/89/15|N34 / C1|380.8397|99.4781|0.1574|

Each pair deliberately contrasts a mixed and an FSR-dominated neutral at similar
distance. This is a truth-selected illustration, not an unbiased population,
reconstructed photon selector, proof that a distance cut works, or a performance
claim. All tagged photon components on the six focus neutrals were checked to
belong to the corresponding same selected Z electron. Both owner FSR and eBrem
birth sums exceed1GeV in each chosen event. Multiple roots of one radiation kind
remain separate: e.g.472/121's74.3%eBrem is two roots, not one photon purity.

Candidates came from the existing55391-current-neutral feature cache, requiring
valid charged reference,1–45GeV measured neutral energy, native FSR+eBrem>97%,
main-electron fraction<5%, no more than3 photon roots. Mixed candidates require
each radiation fraction>20%; FSR-dominated examples have>95% individual-root FSR.
Search ranges were20–100/100–250/250–650mm, ranked toward60/175/380mm and6GeV.
No cut efficiency is inferred from this selection.

Eight distinct current-REC entries were freshly inspected; six were retained.
Two unselected cases remain as audit artifacts:354/75 is at the barrel/endcap
edge with large matched charged energy;492/114 is an endcap case. The retained
six all use the established barrel-reference cylinder projection. No endcap
projection convention is introduced in this task.

## Truth and image meaning

The source campaign is the existing
`higgs_eeHX_ISRFSRON_REC_ECALTRUTH_500jobs_200evt_20261112`.
Sources, sizes, mtime_ns, owners, cluster IDs and distances are captured in
`TrackingPerformanceStudies/neutral_event_distance_examples_20261115/selected_events.json`
and the fresh `event_JOB_EVENT.json` payloads.

FSR identity comes from saved generator links. eBrem identity uses the unique
exact G4/MC birth momentum+position tuple from the selected electron before its
first ECAL entry. Descendants are grouped only by unique photon root. There is
no angular truth matching, fabricated entry or reconstructed per-hit photon tag.

The retained charcoal/gold/grey palette uses circles/triangles for charged/neutral
hits, gold stars/grey diamonds for FSR/eBrem original ECAL entrances, black x for
electron truth and black + for Baseline KF. Distinct individual-root bar hatches
and direct black labels encode meaning without colour dependence. The measured
cluster energies and individual photon birth energies remain separately labelled.

All left-panel positions are IP-ray projected onto a cylinder at the selected
Baseline KF ECAL-reference radius: x=r_ref*DeltaPhi,y=r_ref*z/rho-z_reference,
inmm. This suppresses shower depth and differs from the3D categorization distance.
Axis ranges vary per local view; do not compare separation by pixel size.
Finite positive hit energies are retained, with the previous display-only marker
size scaling/cap. The current25/200mrad theta/phi half-window is unchanged.

Only actual saved original-photon entrances receive stars/diamonds. In202/89,
FSR gamma14 converted before ECAL (saved conversion process/status and e-/e+
daughters checked), so the figure explicitly notes conversion and draws no FSR
entrance. Missing-entry notes now identify FSR versus eBrem correctly and only
claim conversion when the saved status supports it. Root entries outside a local
viewport are noted when relevant. Soft roots can still appear when their native
component weight warrants an individual segment; legend energies are birth energy.

Right panels normalize to each cluster's full native MC weight, keep each original
photon separate, and retain direct selected-electron contribution as Main electron.
Others includes other/ambiguous response plus roots below the existing0.1% legend
inclusion threshold in every displayed cluster. The native map inherits U/V bar
projection fractions and is not validated spatial3D deposit purity. These figures
show the saved map and geometry; they do not locate proven physical mixing.

## Checks and artifacts

Study: `TrackingPerformanceStudies/neutral_event_distance_examples_20261115/`.
`prepare.py` reuses the existing native reader; `plot.py` reuses the accessible
renderer; `verify.py` independently checks the six cases. The old reader's
collection/index/hit uniqueness, fraction closure, measured-energy and cached
FSR/eBrem/max-photon/isolation checks pass. All isolation includes charged clusters.

Independent verification passes16 individual-photon-fraction comparisons,
6 cached3D charged distances,6 same-owner checks, current source size/mtime,
birth sums, conversion daughters and2/2/2 distinct-event grouping. Original-photon
projection assertions and stack closure pass, and all changed scripts compile.
All six complete images were inspected; three had overlapping labels repositioned
and their final renders re-reviewed. `verification.json`, `plot_summary.json`,
`reproducibility.json` and logs retain checks, exact ranges/labels and hashes.
Two original PNG/PDF pairs are byte-identical copies from the earlier study.

The outgoing full AGENTS is preserved verbatim in
`2026-11-15-agents-before-distance-event-examples.md`. Only current focus changes;
global status, accessibility preference, scope laws and commands remain identical.
The earlier event/accessibility records remain authoritative historical evidence.

## Gallery

The dedicated plot-gallery workflow publishes all8PNG/8PDF pairs in the child
`TrackingPerformanceStudies/bp_matched_recoil_pt_20261110/neutral_event_displays/`,
with ordered Near/Middle/Far/Earlier reference sections. Existing104 root images
and20 existing root section objects/order/descriptions must remain unchanged;
old original-example URLs stay valid. The root may append one category for
recursive manifest validation and exposes a navigable child-directory link.
`gallery_before/` and `gallery_verification.json` retain publication evidence.


Publication verified: localhost8000/neutral_event_displays/ on ailogin002,
existing server PID3994352. The child validates8PNG/8PDF/4sections; root recursive
counts84PNG/36PDF/120images, with one appended root category for child PNGs
(21 root sections). All104 original root image hashes and20 original section
objects/order/descriptions are unchanged; original URLs still work. All16 copied
files match source hashes. Root and child pages returnHTTP200, the root exposes
the child link, and two new PNG responses return200 with matching hashes.
