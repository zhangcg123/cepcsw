# 2 GeV, theta=85 deg Material-Step Study

This directory repeats the 1 GeV, theta=85 deg material-step procedure for the 2 GeV electron and muon tuples.

Input tuples in the CEPCSW top directory:

```text
gsf_material_steps-e--2.0-85-{1..10}.root
gsf_material_steps-mu--2.0-85-{1..10}.root
```

The ROOT tuples are not copied here. This directory contains only analysis scripts, generated summaries, and plots.

## Directory Contents

- `scripts/`: ROOT macros used for this study point.
- `plots/`: generated PNG/PDF figures.
- `summary.txt`: electron eBrem track-id and radius summary.
- `category_energy_loss_summary.txt`: primary eBrem energy-loss summaries for BGO crystal bars, tungsten beampipe shell, and other tracker volumes.
- `tracker_process_loss_summary.txt`: primary electron tracker-process loss spectra.
- `muon_tracker_process_loss_summary.txt`: primary muon tracker-process loss spectra.
- `tracker_electron_muon_ioni_summary.txt`: direct eIoni vs muIoni tracker comparison.

## Step-Wise Definitions

All selections are step-wise: a `g4step_tuple` entry is one event, and vector branch index `i` is one Geant4 step. The category is assigned from the values at the same step index `i`; it is not inferred from the event or whole track.

Primary selections:

- Electron: `track_id == 1 && parent_id == 0 && pdg == 11`
- Muon: `track_id == 1 && parent_id == 0 && pdg == 13`

Process selections:

- Electron bremsstrahlung: `process_subtype == 3` in the electron sample (`eBrem`).
- Electron ionization: `process_subtype == 2` in the electron sample (`eIoni`).
- Muon ionization: `process_subtype == 2` in the muon sample (`muIoni`).

Tracker-volume selections use `pre_volume` names containing one of `VXD`, `ITK`, `TPC`, `OTK`, `SIT`, or `SET`. Energy-loss spectra use the tuple `loss` branch in GeV.

The mutually exclusive primary eBrem material categories are:

- `crystal_bar`: `material == G4_BGO` and `pre_volume` starts with `bar_s` or contains `crystal_s`.
- `w_beampipe_shell`: `material == G4_W` and `pre_volume` contains `BeamPipe_BeforeCryoW`.
- `other_tracker`: tracker-named `pre_volume`, excluding the previous categories.

## Reproduction

Run from the CEPCSW top directory:

```bash
root -l -b -q 'G4MaterialStepComparison/studies/e2p0_theta85/scripts/analyze_ebrem_trackid_r.C("G4MaterialStepComparison/studies/e2p0_theta85")'
root -l -b -q 'G4MaterialStepComparison/studies/e2p0_theta85/scripts/plot_ebrem_energy_loss_categories.C("G4MaterialStepComparison/studies/e2p0_theta85")'
root -l -b -q 'G4MaterialStepComparison/studies/e2p0_theta85/scripts/plot_tracker_process_loss_spectra.C("G4MaterialStepComparison/studies/e2p0_theta85")'
root -l -b -q 'G4MaterialStepComparison/studies/e2p0_theta85/scripts/plot_muon_tracker_process_loss_spectra.C("G4MaterialStepComparison/studies/e2p0_theta85")'
root -l -b -q 'G4MaterialStepComparison/studies/e2p0_theta85/scripts/plot_electron_muon_ioni_tracker_loss.C("G4MaterialStepComparison/studies/e2p0_theta85")'
```

## Current Results

Electron eBrem track-id continuity:

```text
all_eBrem_steps                  850404
all_eBrem_same_track_after       848017
all_eBrem_no_same_track_after      2387
primary_eBrem_steps               36525
primary_eBrem_same_track_after    36518
primary_eBrem_no_same_track_after     7
```

The primary eBrem `track_id` therefore almost always continues after the eBrem step. The few no-same-track cases are edge cases where no later same-track step is recorded.

Primary eBrem radius distribution:

```text
primary_eBrem_R_mid_mm mean 1807.16
q01/q05/q10/q50/q90/q99 = 142.73 / 1831.09 / 1836.09 / 1852.65 / 1879.40 / 1914.18 mm
R~100 primary eBrem count = 160
R~100 midR q10/q50/q90 = 70.14 / 90.02 / 110.00 mm
R~100 midZ q10/q50/q90 = 6.14 / 715.89 / 849.18 mm
```

The dominant primary eBrem population is still near `R = 1.8-1.9 m`. The small-radius population is much smaller than in the 1 GeV point and is dominated by forward/MDI material:

```text
stainless_steel 70
G4_W            45
CFRP_M55J       26
CrZrCu18150     16
Air              3
```

Primary eBrem category loss summaries:

```text
crystal_bar       count 34269  frac_loss_mean 0.117973  abs_loss_GeV_mean 0.049002
w_beampipe_shell  count    45  frac_loss_mean 0.188707  abs_loss_GeV_mean 0.026412
other_tracker     count  1107  frac_loss_mean 0.074683  abs_loss_GeV_mean 0.116773
```

Tracker-only primary electron process loss summaries:

```text
eBrem           count   1107  mean 0.13005871 GeV  q50 0.00654459  q90 0.42895171
eIoni           count  18871  mean 0.00005924 GeV  q50 0.00000262  q90 0.00001323
msc             count    132  mean 0.00003134 GeV  q50 0.00001458  q90 0.00005900
StepLimiter     count  11335  mean 0.00000395 GeV  q50 0.00000238  q90 0.00000376
Transportation  count 647347  mean 0.00000525 GeV  q50 0.00000072  q90 0.00001121
```

Tracker-only primary muon process loss summaries:

```text
muIoni          count  16192  mean 0.00006383 GeV  q50 0.00000238  q90 0.00001359
StepLimiter     count   6577  mean 0.00000187 GeV  q50 0.00000167  q90 0.00000286
Transportation  count 594676  mean 0.00000483 GeV  q50 0.00000048  q90 0.00001073
other           count      1  mean 0.00002575 GeV
```

Direct tracker ionization comparison:

```text
eIoni  count 18871  mean 0.00005924 GeV  q50 0.00000262  q90 0.00001323  q99 0.00104129
muIoni count 16192  mean 0.00006383 GeV  q50 0.00000238  q90 0.00001359  q99 0.00103476
```

At 2 GeV, the electron and muon tracker ionization spectra are very similar in the central and high-percentile regions. The electron sample still has more ionization steps, but the mean `loss` is not larger than the muon mean for this 2 GeV tracker-only selection.
