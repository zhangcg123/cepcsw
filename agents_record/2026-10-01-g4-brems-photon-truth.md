# Geant4 bremsstrahlung photon creation truth

The user requested a background sub-agent to inspect simulation-level photon
truth and implement missing information, followed by main-agent review.

Accepted source change: `Edm4hepWriterAnaElemTool` now writes
`GsfG4BremsPhotons` alongside its existing truth collections when
`WriteGsfTruthEventData` is enabled. For each selected electron/positron G4
step it records explicit photon secondaries whose creator is
`fBremsstrahlung`. Each stores creation position [mm], momentum/energy [GeV],
time [ns], creator subtype, secondary-list index, parent track/step identifiers,
and a direct PODIO relation to the recorded `GsfG4MaterialSteps` object.

This captures photons below the generic MCParticle secondary threshold.
It does not invent photons for locally deposited energy or for secondaries
suppressed by Geant4 production cuts. Parent selection and tracker-envelope
selection follow the existing truth-step configuration.

Main review requested replacing the parent step-limiter subtype gate with an
explicit e-/e+ parent check; the photon's own creator process identifies its
origin. This is a clearer collection contract and avoids depending on which
process limits a step. No missing explicit photons were demonstrated in the
ordinary eBrem smoke: Geant4's standard eBrem is not an ionisation along-step
producer. The revised source uses the named Geant4 subtype constant.

Builds of GsfTruthEventData, its dictionary and DetSimAna passed. A new 20-event
electron test recorded 44 photons, including 31 below 10 MeV. A five-event
positron test recorded five photons, including three below 10 MeV. The earlier
10-event test had nine photons. Checks resolved actual PODIO parent relations,
verified parent identifiers/PDG, creator subtype, unique step/secondary keys,
finite kinematics, E=|p|, and emission-time bounds.

One new electron test step had subtype 3 but no explicit photon: event 18,
step 1611. Its kinetic-energy decrease and local deposition were both
1.497863 MeV. A step subtype alone therefore does not establish an explicitly
created photon in the saved secondary list.

Main-agent acceptance is based on reviewing the final source/schema diff and
the focused validation code/results. Temporary reproduction inputs and checks:
`/tmp/gsf_photon_review_smoke.py`, `/tmp/gsf_photon_review_check.py`, and
`/tmp/gsf_photon_review_{smoke,positron}.root`. Run through the configured build
environment after `source setup.sh`; use PODIO Frame reading.

No run-card, GSF or breakpoint changes were included. No runtime installation
has been performed; existing simulation files do not acquire the new collection.

## Same-seed preservation check

The old installed `DetSimAna`/`GsfTruthEventData` runtime and the new uninstalled
build were run with the same maintained `DumpGsfTrks/sim.py.bk` settings: seed
12345, primary e-, 2 GeV, theta 85 degrees, 20 events. The paired ROOT files
are `/tmp/gsf_photon_old_installed.root` and
`/tmp/gsf_photon_review_smoke.root`. This comparison uses the active local card,
including its pre-existing user edits; it does not compare different cards.

The old event tree had 29 collections and 452 branches. The new event tree had
the same 29 collections plus `GsfG4BremsPhotons`, represented by 16 new branches.
No old branch was removed. For every old branch, type, event count, basket
count, basket event boundaries, uncompressed length, and stored payload agreed
exactly across the 20 paired events: 825 old baskets compared, zero mismatches.
This covers `GsfG4MaterialSteps`, `GsfSimTrackerHitG4StepLinks`, MCParticles,
tracker hits, calorimeter hits/contributions, and all other original event
collections. The new tuple recorded 44 photon entries in this sample.

Thus, no pre-existing **event collection content** changed in these paired
events. File metadata necessarily gains the photon collection, and this check
does not claim an all-seed/all-geometry guarantee. The direct PODIO check of
every generic getter crashed inside the ROOT binding, so the final broad check
compared each ROOT branch's stored baskets and event boundaries instead.
