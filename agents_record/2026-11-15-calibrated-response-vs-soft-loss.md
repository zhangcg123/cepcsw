# Corrected ECAL response is not soft-photon retention — 2026-11-15

The user asks why full-angle photon collection can approach100% when PFA removes
soft radiation response. Read-only source trace establishes a relevant distinction:
our numerator is already ENERGY-CORRECTED pre-reclustering cluster response.
“Raw” in the earlier full-angle table means before our study energy caps,
not before detector reconstruction/correction. No algorithm was changed.

## Verified source path

1. Actual current REC job-10 card orders EnergyTimeMatchingAlg, then
   TrackClusterConnectingAlg, then PFOReclusteringAlg. Other jobs were not
   newly rechecked for card identity during this read-only clarification.
2. EnergyTimeMatchingAlg.cpp:64–76 filters half-cluster inputs below5%of their
   tower energy in towers with totalenergy<1GeV. Thus soft-response filtering
   exists BEFORE our saved pre-reclustering snapshot. This is a reconstruction
   component cut, not a direct truth-photon birth-energy threshold. No number
   of removed photons/energy was measured in this turn.
3. TrackClusterConnectingAlg.cpp:327–337 applies
   EnergyCorrSvc->energyCorrection(E,phi,theta)/E as cluster Escale.
4. Service/CrystalEcalSvc/src/CrystalEcalEnergyCorrectionSvc.cpp:91 implements
   energy-dependent MPV and angular/crack corrections in the barrel, and angular
   corrections in the endcap. These can increase reconstructed response; we
   have not quantified their compensation of soft losses on these events.
5. Calo3DCluster.cc:231–243 getLongiE() returns summed U/V shower energy times
   Escale. OutputCreator.cpp:30 saves this corrected value into the snapshot.
6. CyberPFAlg.cpp:758–765 requests the snapshot after TrackClusterConnectingAlg;
   include/Tools/AlgorithmManager.h:51–55 invokes the callback after RunAlgorithm
   and before ClearAlgorithm. Snapshot therefore follows energy correction and
   precedes PFOReclusteringAlg. It is not final-PFA photon retention.

## Interpretation

R_E = sum_clusters[E_corrected(cluster) * native_origin_fraction(cluster)] /
      sum_tagged_photons[E_birth(photon)].
This is an energy-weighted response ratio. It is neither the number fraction of
retained photons nor exact deposited-energy closure. A large number of missing
soft photons CAN carry little energy; that statement does not measure the soft
energy fraction in this sample. Corrected response, native attribution and
large response tails can compensate missing contributions in an aggregate sum.
The eBrem value above100% itself excludes interpreting this as a probability.

Published full-angle raw response98.17%FSR/100.79%eBrem includes other charged
clusters AND the study-cap-rejected clusters. It is not the operational25/200
BP collection, whose retainedneutral+matchedcharged response is59.06%/79.73%.
With full-angle neutrals and both selected/othercharged under frozen caps,
response is91.67%/92.68%. The raw near-unity numbers should be called corrected
response accounting, not complete photon recovery. Independently validated
spatial response/purity and exact physical soft losses remain unmeasured.
To measure actual soft retention one needs response/identification versus
truth-photon birth energy and a consistent comparison across reconstruction
stages or original simulation contributions. That analysis was not performed.
No code/build/install/card/default/reconstruction/simulation changes occurred.

## Requested gallery merge

The sole distance plot is merged into the immediately preceding current section:
“New BP recoil: theta25mradwindow andH0+FSR”. Append its existing PNG as seventh
pattern and its diagnostic description, preserving all oldsixpatterns and all
oldprecedingdescriptiontext. Remove only the nowempty standalone photon-ID
section. ALL102PNG/PDFhashes stay unchanged;20sections become19. Other18section
objects/order remain identical. The rawcount/logy/3Dmm content and five prior
figure removals stay unchanged. No new figure or numerical analysis is needed.
Premerge snapshots are before_section_merge_gallery*.json under
TrackingPerformanceStudies/neutral_photon_opening_counts_20261115/;
section_merge_gallery_verification.json records local and HTTP checks.
Full outgoing AGENTS.md is retained in
2026-11-15-agents-before-section-merge-response-clarification.md. The global
prefix changes only by adding this corrected-response interpretation; active
laws and compile/run instructions remain intact. Analysis artifacts uncommitted.

Merge completed and independently reviewed:75PNGs/19sections/27PDFs/102images.
Distanceplot is seventh in the precedingBPsection. The oldsixpatterns/title/
description and EXACToriginaldistanceparagraph (includingwindow/cap) remain.
All102imagehashes unchanged; other18sectionobjects/order fixed. Page andPNG
HTTP200/source-local-servedhashmatch; fivepriorremovedURLs404. Sameailogin002
localhost8000PID443262, no restart. Finalverificationfile is referenced above.

## Corrected distance-plot destination — 2026-11-15

User explicitly names “Photon-energy locations: matched charged clusters and
full-angle accounting (1)” as the destination. The previous placement in the
BP recoil section was incorrect. Move photon_id_track_opening.png into that
energy-accounting section as its SECOND pattern, after
photon_energy_location_table_bars.png. Restore the BP section's exact original
six patterns/title/description from the premerge snapshot. Preserve both full
energy-section and distance descriptions, including window/cap/metric caveats.
No standalone photon-ID section is recreated. No image, curve, value, label,
normalization or dataset change. All102imagehashes remain fixed; other17section
objects/order intact,75PNGs19sections27PDFs. FiveoldremovedplotURLsstay404.
Snapshots before_distance_migration_gallery*.json and final
distance_migration_gallery_verification.json live in the distance-study folder.
The source-backed corrected-response/soft-loss interpretation above is unchanged.
Full outgoing AGENTS.md retained in
2026-11-15-agents-before-distance-correct-section.md; global prefix unchanged.

Main local review confirms the corrected destination has exactly two patterns
in the requested order, the BPsection is byte-equivalent to its original
six-plot manifest object, other17sectionobjects/order andall102imagehashes are
unchanged. No plots/data regenerated; same75PNGs19sections27PDFs.
