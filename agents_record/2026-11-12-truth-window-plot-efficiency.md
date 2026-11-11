# Actual tuple window on the5k truth photon plot, 2026-11-12

User requested removing the zoomed-out figure and bottom text, displaying the
window used in current flat tuples (questioning20mradtheta), and efficiency on
the plot. Only the latest5k truth-photon section is revised; older sections and
physics/collection algorithms stay unchanged.

## Verified current collection window

Current BP_NOSCAN_PREECAL500jobs/200evt campaign: all500manifests leave neutral
window controls at defaults. Frozen card assigns NeutralLossThetaWindowMrad
from BP_NEUTRAL_THETA_MRAD default10, and phi default200. The collector compares
absolute angular differences against these values, so they are HALF widths.
Actual cut is |deltaTheta|<=10mrad, a20mradTOTALtheta width, and
|deltaPhi|<=200mrad. It is not a +/-20mrad cut.

Directly checked seed1's399selected track rows: reconstructed cluster eligibility
and10/200window reproduce saved collected_neutral_ecal_cluster_indices exactly.
Five otherwise eligible cluster/track candidates with10<abs(deltaTheta)<=20mrad
are correctly absent. No reconstruction card, library or flat output changed.
The tuple has no dedicated persisted window-size branch; this verification uses
frozen cards plus actual saved indices. See flat_window_verification.json.

## Figure and efficiency definition

Only the zoom remains active, x=deltaTheta +/-150mrad,y=deltaPhi +/-400mrad,
FSR/eBrem separate, electron at0implicitly. Header remains AGORA simulation.
No bottom footer/caption, figure-level bottom legend or out-of-view text.
Actual10/200mradbox and compact epsilon_geom are shown inside each panel.

Photon geometry is unchanged:5000events,6246FSR and16169eBrem valid original
truth-photon/electron ECAL-entry pairs. Counting ALL those valid pairs, including
out-of-view points, inside |deltaTheta|<=10 and |deltaPhi|<=200:
FSR3069/6246=49.1354467%;eBrem15728/16169=97.2725586%.
Displayed49.1%/97.3%. These are photon-count geometric fractions using truth
entrances, not energy fractions or actual neutral-cluster collection/recovery.
Native collection uses Baseline KF plus reconstructed neutral cluster positions.
No photon energy cut or neutral-only selection is introduced. Previous2%
cluster-energy trimming does not apply to this truth-photon figure.

## Preservation and checks

Artifacts: TrackingPerformanceStudies/truth_photon_ecal_centered_5k_20261112/.
Oldfull/zoom PNG/PDFs, code, summary and README are archived under
before_actual_window_efficiency/. Active plots/ and plot_files.json contain only
the new zoom PNG/PDF. Population/source metadata equal pre-edit values; no REC
reread or native event processing for the redraw. Saved CSV gives exact numerator
counts, denominator and percentages. Source/plot hashes updated; visual layout
checked, figure footer absence asserted. Escaped the mathtext percent character
before successful publication; the intermediate render never reached the page.

A dedicated plot-gallery agent removes the full figure from the active served
root as well as its manifest pattern (so it does not appear uncategorized),
archiving it outside the gallery; replaces zoom; updates definitions; preserves
other14plots/sections. Expected15PNGs/7sections, same localhost8000 server.
Current AGENTS focus is updated without changing any active law. Complete
pre-edit snapshot:2026-11-12-agents-before-truth-window-efficiency.md.

Publication verified by the dedicated agent:15PNGs/7sections, exact-once
coverage, homepage and new zoomHTTP200/hash matching source, removed full-view
endpointHTTP404. Old served full PNG moved to the study archive outside served
root; no served full PDF existed. Main independently checked15files, zoom-only
first-section pattern and matching source/copy hash. Same ailogin002 server
PID2122174, no restart. All other14images/sections preserved.
