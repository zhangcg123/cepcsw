# LCIO Track Resolution: 2 GeV, theta=85 deg

This study compares the standard LCIO `CompleteTracks` reconstruction performance for the 2 GeV, theta=85 deg electron and muon samples.

Input files in the CEPCSW top directory:

```text
trk-e--2.0-85-{1..10}.root
trk-mu--2.0-85-{1..10}.root
```

The input ROOT files are generated tracking outputs and are not copied into this analysis directory.

## Method

The analyzer reads the PODIO `events` tree with uproot.

Track selection:

- Reconstructed track collection: `CompleteTracks`
- Matching: use `CompleteTracksParticleAssociation` to select the reconstructed track associated with primary `MCParticle` index 0 when available
- Fallback: if no valid association is found, select the `CompleteTracks` entry with the largest `ndf`
- Track state: LCIO `TrackState` with `location == 1`, which has reference point `(0,0,0)` in these files

Truth definition:

- Primary generated particle: `MCParticleGen[0]`
- `D0` truth and `Z0` truth are taken as zero for these particle-gun IP samples
- `phi` and `tan(lambda)` truth are computed from generated momentum
- `omega` truth uses `omega = q * 0.0003 * B / pT`, with `B = 3.0 T` and `omega` in `1/mm`

The output rows are stored in `track_resolution_rows.csv`; summary statistics are in `track_resolution_summary.csv` and `summary.txt`. Tail fractions are also written to `track_resolution_tail_summary.csv`.

## Reproduction

Run from the CEPCSW top directory:

```bash
TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/scripts/analyze_lcio_track_resolution.py
```

## Plots

Each plot overlays electron in black and muon in blue, normalized to unit area.

![D0 resolution](plots/d0_mm_resolution_comparison.png)

![Z0 resolution](plots/z0_mm_resolution_comparison.png)

![phi resolution](plots/phi_mrad_resolution_comparison.png)

![tan lambda resolution](plots/tanlambda_resolution_comparison.png)

![omega resolution](plots/omega_rel_pct_resolution_comparison.png)

![pT resolution](plots/pt_rel_pct_resolution_comparison.png)

![chi2 over ndf](plots/chi2_ndf_resolution_comparison.png)

## Current Results

Both samples have 1000 matched tracks.

Central 68% interval, from q16 to q84:

```text
electron D0        -0.0055 to 0.0172 mm
muon     D0        -0.0064 to 0.0072 mm

electron Z0        -0.0076 to 0.0084 mm
muon     Z0        -0.0071 to 0.0070 mm

electron phi       -1.1860 to 0.3641 mrad
muon     phi       -0.4857 to 0.4207 mrad

electron tanLambda -4.739e-4 to 5.140e-4
muon     tanLambda -4.844e-4 to 4.768e-4

electron omega     -0.115% to 5.332%
muon     omega     -0.184% to 0.064%

electron pT        -5.062% to 0.115%
muon     pT        -0.064% to 0.184%
```

The muon sample has narrow, nearly Gaussian residuals. The electron sample has a comparable central core in impact parameters and `tan(lambda)`, but much larger non-Gaussian tails, especially in curvature/momentum:

```text
electron |pT residual| > 10%: 124 / 1000
electron |pT residual| > 50%:  31 / 1000
muon     |pT residual| > 10%:   0 / 1000
muon     |pT residual| > 50%:   0 / 1000
```

The electron mean/RMS values are therefore dominated by tail failures from bremsstrahlung and difficult pattern/fit cases. For tuning or monitoring the standard LCIO track performance, use both the robust q16/q50/q84 core metrics and the explicit tail fractions.
