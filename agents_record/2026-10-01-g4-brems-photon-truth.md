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
