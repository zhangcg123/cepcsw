# FSR and eBrem separately versus reconstructed metrics — 2026-11-14

Same100000REC inputs and EXACT SAME cached neutral candidates as
neutral_purity_metrics_20261113:55391baseline clusters from38749events,
91482wide clusters from48721events,198081truth-covered selected production
electron tracks. No BP flat input, new reconstruction, ROOT reread or new
truth association in this extension. Parent metrics SHA256 is in summary.json.
The parent already verified all500currentREC identities/geometry/window masks
and freshly reclassified native MC fractions for source1. Native FSR roots
remain generator-tagged, eBrem roots use saved Geant4/MC provenance.

## Definitions

FSR purity = sum(Eneutral*FSR_native_fraction)/sum(FULL neutral energy).
eBrem purity = sum(Eneutral*eBrem_native_fraction)/sum(FULL neutral energy).
Both numerators use the SAME denominator/sample in every comparable bin/cell.
The two fractions therefore ADD BACK to the original combined photon purity.
There is NO FSR-tagged/eBrem-tagged cluster requirement, event-category split,
positive-signal filter, dominance cut or truth-angle selection. Mixed clusters
stay in both component calculations; their energy is partitioned by MC weights,
not duplicated. These ratios measure composition of the full selected neutral
sample, not collection efficiency relative to generator photon energy or a
conditional purity within only truth-tagged clusters. Other radiation species,
main-electron energy and Others remain in each denominator.

All original geometry/isolation definitions unchanged: baseline theta/phi
half-windows25/200mrad; fixed neutral55.757GeV cap; distances in3Dmm to the input
track ECAL reference or its uniquely associated charged-PFO ECAL cluster;
isolation I=Eneutral/(Eneutral+other valid ECAL E in100mrad OPENING cone),
excluding the candidate and both selected main charged-cluster sets. Missing
charged references kept as separate control. Charged-distance/cut comparisons
use the common48321available-reference cluster cohort;7070missing stay in all
other inclusive plots. No MC enters metric values or angular assignment.

## Figures and uncertainties

Four profile plots overlay separate orangeFSR/blueeBrem curves versus track
ECAL distance, main charged-cluster distance, local isolation and neutral E.
Separate FSR/eBrem angular maps (wide200/200diagnostic with baseline marked),
separate distance/isolation maps, separate target-energy retention tradeoffs,
and one grouped reference-availability control:11PNG/PDF pairs. All previous
combined figures are retained. Common0–100%map scales avoid visual rescaling.
Profiles display0–500mm, same bins as parent; all bins/tails saved in CSV.
Atleast30clusters per shown bin/cell/control-curve point. Same300Poisson
resamplings of500source blocks, pointwise16/84percentile errorbars per species.
Component errorbars are correlated and should not simply be added.

Target-retention x-axis in each tradeoff divides the retained target's collected
energy by its baseline collected energy in the SAME reference-available cohort.
It does not use photon birth energy, nor a common denominator between retention
fractions of different species. The y-axis still uses full surviving neutral E.
No cut/selection/hypothesis choice is applied to reconstruction.

## Validation

All sample counts and masks unchanged. FSR+eBrem profile and map-cell means
reproduce the combined parent ratios at1e-12; counts match exactly. Every cut
count and joint purity matches its parent; individual target retentions match
the prior FSR/eBrem retention columns, and their baseline-energy-weighted sum
reproduces combined retention. All parent plot hashes remain unchanged.
Code/data/PNG/PDF hashes and numerical validation are in summary.json and
validation.json. These are MC response-attribution fractions, not a calibrated
physical posterior or proof of improved recoil mass. No new fit/physics test.

## Results: shares of full neutral energy

| Reconstructed condition | FSR | eBrem |
|---|---:|---:|
|Inclusive baseline|22.97%|40.34%|
|Charged distance <50mm|14.16%|33.29%|
|Charged distance50–150mm|29.93%|41.09%|
|Charged distance >=150mm|37.24%|50.30%|
|Isolation <0.5|10.46%|47.67%|
|Isolation >=0.9|29.35%|34.16%|
|Charged reference unavailable|4.53%|37.13%|

Isolation separates the two origins in opposite directions; the combined curve hid this compositional change. This does not mean eBrem recovery efficiency falls by the same amount: the table gives fractions of the full measured neutral energy.

For common charged-reference candidates, distance>=50mm keeps89.36%FSR and82.99%eBrem energy, with respective fractions31.71/43.33%. Isolation>=0.9 keeps84.65%FSR and65.13%eBrem energy, with fractions32.89/37.23%. Denominators/cohort differ from the inclusive isolation groups above; do not mix them.

## Reproduction artifacts

Directory: `TrackingPerformanceStudies/neutral_purity_species_20261114/`.
All code, tables, PNG/PDFs and gallery manifests remain uncommitted analysis
artifacts. This record and the outgoing AGENTS snapshot preserve the definitions
and complete prior project focus. No shared GSF/BP source, card, installation,
ROOT input, production job or prior plot was modified. The extension is a
post-processing split of the existing validated REC-based metric cache.

Implementation SHA256: `3aad322b8df7b3aa50d4723038887d6eea013884a1de2e164f81537fd4dfcf11`.

Parent metrics SHA256: `1caab6eec6f563c0c1cb0ee87d7e1a8783c4d0150d43b155c2bd8a86ac0b22ab`.

## Gallery publication

Appended one section, "FSR and eBrem separately versus reconstructed metrics",
with11newPNG/PDF pairs. Final75PNGs/16sections/exact-once,27PDFs. All80prior
gallery image hashes unchanged, including the combined-purity parents. Delegated
gallery workflow verified page/all11newPNG HTTP200 and source/local/served SHA
equality. Same ailogin002.ihep.ac.cn PID443262, localhost8000, no restart. Main
independently reviewed renders, all80old hashes, all22new copies and manifest
coverage. Evidence: gallery_verification.json/before_gallery.json in artifacts.

New filenames: neutral_species_vs_{track_distance_mm,charged_distance_mm,
isolation,energy_GeV}; neutral_{fsr,ebrem}_{angular_map,distance_isolation_map,
selection_tradeoff}; neutral_species_charged_reference_control. Each has PNG/PDF.

Global AGENTS introduction/status/laws/commands remain byte-identical. Complete
outgoing focus is saved in2026-11-14-agents-before-neutral-purity-species.md.
