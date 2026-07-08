# BH Model Comparison Studies

This top-level study area is for direct comparisons between the Bethe-Heitler model used by Reconstruction/RecGsfTracking/src/BetheHeitlerSplitter.cpp and true Geant4 material-step results from g4step_tuple.

Current study points:

- globalBHmodelfromSim@2GeV85Degree/: first simulation-derived global BH retained-fraction study from 2 GeV, theta=85 deg primary tracker eBrem; includes the <0.9/>0.9 split, a smooth beta-mixture fit, and a truncated-Gaussian weighted-sum mimic normalized on [0,1].
- current_bh_vs_g4step_2p0_theta85/: compare the current in-code BH splitter behavior to the 2 GeV, theta=85 deg primary tracker-volume eBrem material-step truth.

Use this area before any GSF tracking validation. The current GSF output is known not to work properly, so the next physics task is to validate or replace the BH input model directly against true G4 material-step truth.
