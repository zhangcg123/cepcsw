# Truth FSR/eBrem to main-electron ECAL entry angles

The user requested angular distances between FSR/main electron and material
eBrem/main electron at the ECAL surface. This is read-only truth analysis on
the same498 completed BP files x200=99,600 events as the preceding comparison.
No simulation, fit, PFA replay, fitter/source/library/card change, submission
or job interruption. The complete outgoing AGENTS is preserved in
[the snapshot](2026-11-11-agents-before-ecal-photon-angles.md).

## Definitions and associations

Consider all197,637 selected reconstructed production-electron tracks, before
BP/GSF fit-success selection. FSR is identified by saved generator origin
status1/final-electron MC index. A known parent G4 ID must agree with that
selected electron. Material eBrem uses the exact saved parent G4 track ID.
No angular matching defines either photon type or owner.

Use breakpoint.truth_ecal_entry_x/y/z for the electron and
neutral_pfos.{fsr,ebrem}_photon_ecal_entry_x/y/z for the original photon.
Both entry statuses must be1, with finite nonzero position rays. The common
origin is the main electron's saved truth_vertex_x/y/z. Define3D opening angle
atan2(norm(r_e cross r_gamma),dot(r_e,r_gamma)), mrad; delta phi is wrapped
photon minus electron, and delta theta is photon minus electron. These are
**ECAL entry-position angles**, not momentum-direction or emission angles.
The entries come from the first saved G4 step entering an ECAL-named volume
ancestry, not reconstructed cluster centroids or an analytic cylinder.

Missing original-photon entries are retained as status/NaN rows in the CSV,
not extrapolated or replaced by conversion daughters. This conditions the
plots on both original photon and electron having entries. Geometry does not
establish photon-to-PFO/cluster truth membership. No direct shower attribution
is added to the tuples.197,289 selected electrons have entry status1;348 have0.
There are no unresolved selected truth/G4 IDs, ambiguous FSR origin records,
invalid-position pairs or undefined delta-phi cases in this sample.

## Coverage and numerical results

| Source | Photons of selected owners | Main entry missing | Photon entry missing with main valid | Valid pairs |
| --- | ---: | ---: | ---: | ---: |
| Generator FSR | 179,355 | 390 | 55,384 | 123,581 |
| Material eBrem | 346,179 | 9,059 | 19,258 | 317,862 |

Missing-entry reasons are exclusive in this table, checking main entry first.
Absence alone does not distinguish conversion, absorption, escape or a photon
never transported; no such causal classification is inferred here.

| Birth-energy selection | FSR pairs | Median opening [mrad] | eBrem pairs | Median opening [mrad] |
| --- | ---: | ---: | ---: | ---: |
| All | 123,581 | 36.71592 | 317,862 | 3.32458 |
| >=1GeV | 35,304 | 29.71450 | 104,496 | 4.39911 |

Opening<100mrad fractions: all energies FSR65.00%, eBrem97.93%; >=1GeV
FSR74.23%, eBrem98.18%. They are conditional geometric fractions, not PFO
collection efficiencies. FSR already had broad generator-level tails in the
older fsr_angles_summary.json study; no causal separation of initial emission,
magnetic bending and material effects is attempted or claimed.

## Figures and checks

Two new PNG/PDF figures are added to the existing served BP directory:
truth_photon_electron_ecal_opening_angle and
truth_photon_electron_ecal_delta_angles. Each has all-energy and>=1GeV birth-
energy rows. Opening panels show0--100mrad core and log-x/log-y from0.001mrad
to pi*1000. Coordinate panels show delta phi/theta in[-100,100]mrad. Each source
is normalized by its own full valid pair count. Out-of-view/zero/sub-log-range
entries remain in denominators. No extra cuts affect the all-energy plots.
All16 prior BP comparison PNG/PDF files remain byte-identical by SHA256.

All498 source identities,200-event order, photon-array alignment, per-event
photon-key uniqueness, selected owner identities/G4 agreement and population
sums pass. Synthetic parallel/antiparallel, wrap and zero-vector tests pass.
The compressed CSV reads back with525,534 selected-owner photon rows and
identical statistics. Independent arccos normalized-dot angles on441 samples
(112FSR/329eBrem) agree with the cross/atan2 formula within7.84e-6mrad;
independently calculated phi/theta also agree. Both final layouts are inspected;
legends are shortened/placed to leave the sharp peaks visible.

Artifacts (uncommitted):
TrackingPerformanceStudies/fsr_ebrem_ecal_angles_20261111/ with analyze.py,
input_snapshot.json, photon_electron_entry_pairs.csv.gz, summary.json,
coverage_by_job.json, validation.json, README, log, two PNG/PDF figures and the
hash manifest of preserved BP figures. Figure copies and a README addition
are in TrackingPerformanceStudies/bp_matched_recoil_pt_20261110/.
The personal plot-gallery agent updates the combined10-PNG manifest and
verifies the live gallery separately; verified state follows below.

Previous BP plots/counts/bins remain in
[their record](2026-11-11-bp-plots-498-completed-jobs.md). Photon-to-cluster MC
membership definitions/replay limits remain in
[the FSR survey](2026-11-10-fsr-cluster-membership.md). No new recorder or fit
steering is developed here; ongoing production dependencies remain frozen.

## Verified gallery

The personal plot-gallery agent added the truth-geometry section first and
preserved the existing no-FSR and inclusive BP sections in order. All10 PNGs
are covered exactly once across3 sections. The main agent reviewed the new
manifest against the study README/summary and rechecked old/new plot hashes.
The healthy existing server remains on ailogin002.ihep.ac.cn, PID2122174,
http://127.0.0.1:8000/. Live homepage and served opening-angle PNG SHA256
agree with the updated disk artifacts.

## Follow-up: exact distance in millimetres

The user requested physical distance inmm. A new first geometry figure uses
`d = hypot(x_gamma-x_e, y_gamma-y_e, z_gamma-z_e)` from the saved truth
coordinates. This is3D point-to-point separation, not an ECAL surface geodesic
or an angle-times-assumed-radius approximation. Same123,581 FSR/317,862 eBrem
pairs, same all-energy and>=1GeV birth-energy populations, same no-fit-success
selection and original-photon/missing-entry limitations. No ROOT reread or fit.

| Birth energy | FSR median [mm] | eBrem median [mm] |
| --- | ---: | ---: |
| All | 90.95325 | 8.79999 |
| >=1GeV | 71.36600 | 11.37115 |

The core view is0--200mm, with0.001--10000mm log-wide view; full pair-count
normalization is retained. The maximum observed all-energy distance is
6708.13mm (FSR)/6616.80mm (eBrem). Synthetic3-4-5, identical-point and unequal-
radius examples pass.441 independent law-of-cosines checks agree within
2.49e-5mm; populations and source CSV identities are unchanged. Final layout
is inspected; log-view legends are placed below the spectra.

Uncommitted study artifacts: distance_mm.py/.log, summary_distance_mm.json,
validation_distance_mm.json and truth_photon_electron_ecal_distance_mm.png/pdf,
plus the preserved-before-distance SHA256 manifest. Copies are added to the
existing gallery first; all20 preceding PNG/PDF files remain byte-identical.
Study/gallery READMEs and current focus are synchronized. The personal gallery
agent verified11 PNGs across3 sections, each exactly once; the parent reviewed
the manifest and rechecked all20 old hashes plus both new plot hashes. The healthy
server remains on ailogin002.ihep.ac.cn, PID2122174, http://127.0.0.1:8000/.
The live page shows the mm results and the served distance PNG hash matches disk.
