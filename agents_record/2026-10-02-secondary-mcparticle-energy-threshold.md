# Ordinary secondary MCParticle threshold lowered to 10 MeV

The user requested lowering the ordinary `SecTrackEk=100 MeV` secondary-
recording threshold after reviewing primary-electron bremsstrahlung photons.
This threshold belongs to the generic `MCParticle` writer, not to
`GsfG4BremsPhotons`. A zero-threshold control showed a substantial increase
in records, so the final `Edm4hepWriterAnaElemTool` compiled default is
`SecTrackEk=10 MeV`; the local maintained `DumpGsfTrks/sim.py.bk` also sets
`10.0` explicitly. That run-card edit remains uncommitted under the project
law for non-GSF cards. The property remains available for comparisons.
Spatial bounds, process exceptions, and Geant4 production cuts are unchanged.

The `DetSimAna` target built successfully in the EL9/LCG105 configuration.
A same-seed 20-event electron-gun smoke at 2 GeV and 85 degrees completed
at each of the 100, 0, and final 10 MeV settings using the uninstalled build
library. The 100 MeV baseline was the earlier photon-path smoke:

| Quantity | 100 MeV | 10 MeV | 0 MeV |
|---|---:|---:|---:|
| Ordinary `MCParticle` entries | 32 | 59 | 766 |
| Ordinary electrons | 24 | 33 | 669 |
| Ordinary photons | 4 | 13 | 76 |
| Ordinary positrons | 4 | 13 | 21 |
| Dedicated `GsfG4BremsPhotons` | 44 | 44 | 44 |
| Dedicated `GsfG4BremsPhotonSteps` | 11,532 | 11,532 | 11,532 |
| Compressed ROOT bytes | 6,081,796 | 6,083,821 | 6,132,056 |

At 10 MeV the file grew by 2,025 bytes (0.033%) versus the 100 MeV control
in this sample; zero threshold grew by 50,260 bytes (0.83%). These are not
general batch-size estimates. Smoke tuples are temporary
`/tmp/gsf_photon_{path_smoke,sectrk10_test,sectrk0_test}_20261002.root`, not
project outputs. No install was performed during these checks.

In the zero-cut control, the 734 additional ordinary entries were 645
electrons, 72 photons, and 17 positrons. Of the 746 simulated secondary
entries, 736 had kinetic energy below 100 MeV. The secondary-electron median
was about 2 keV; 531 of 649 secondary electrons were below 10 keV. Event
index 18 alone grew from 7 to 241 ordinary entries. The final 10 MeV run
retained 59 ordinary entries, including 28 in event 18. These are different
*records* of already simulated Geant4 secondaries, not particles created or
removed by the writer threshold. The stored MCParticle collection does not
retain enough creator-process detail for a reliable per-process breakdown.
