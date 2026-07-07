# Material-Step Study Points

This directory contains the focused `g4step_tuple` studies grouped by beam momentum and polar angle. Each study point is self-contained so plots, summaries, and C++ ROOT macros do not overwrite another point.

## Layout

- `e1p0_theta85/`: 1 GeV, theta=85 deg electron and muon tuples. This is the completed reference study.
- `e2p0_theta85/`: 2 GeV, theta=85 deg electron and muon tuples. This repeats the same procedure and output structure as the 1 GeV study.

Inside each point:

- `scripts/`: ROOT macros for the step-wise selections and plots.
- `plots/`: generated PNG/PDF figures.
- `README.md`: point-specific selection definitions, reproduction commands, and conclusions.
- `summary.txt` and `*_summary.txt`: text outputs from the macros.

The raw ROOT tuples stay in the CEPCSW top directory. They are generated analysis products and should not be committed with the documentation or macros.

## Shared Step-Wise Selections

Selections are applied per Geant4 step vector index inside `g4step_tuple`, not per event or per whole track.

Primary particle selections:

- Electron: `track_id == 1 && parent_id == 0 && pdg == 11`
- Muon: `track_id == 1 && parent_id == 0 && pdg == 13`

Process selections:

- Electron bremsstrahlung: `process_subtype == 3` in the electron sample (`eBrem`).
- Electron ionization: `process_subtype == 2` in the electron sample (`eIoni`).
- Muon ionization: `process_subtype == 2` in the muon sample (`muIoni`).

Tracker-volume selections use `pre_volume` names containing one of `VXD`, `ITK`, `TPC`, `OTK`, `SIT`, or `SET`. Energy-loss spectra use the tuple `loss` branch in GeV.
