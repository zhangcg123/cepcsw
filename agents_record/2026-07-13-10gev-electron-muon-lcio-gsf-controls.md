# 10 GeV electron/muon LCIO and GSF controls

Date: 2026-07-13

## Samples and configuration

The comparison uses 100 ten-event electron flat tuples and 100 nominal
ten-event muon flat tuples at generated `pT ~= 10.002 GeV`, theta 85 degrees.
All 1,000 electron entries are valid. Muon seed 89 is a 955-byte ROOT file
without `gsf_tuple`, leaving 990 valid muon entries.

The muon GSF production is an intentional stress/control configuration, not a
physical muon refit: its cards set `ElectronHypothesis=True`, use
`BHModel="CEPC2GeV85StepConditioned"`, run reverse filtering, and publish
`BestBranch`. Therefore “muon GSF” below means the electron BH and electron
process hypothesis forced onto muons.

All zoomed histograms use 0.05%-wide bins on `[-2%, +2%]` and normalize each
curve to unit area using only entries inside that displayed window. Full-range
histograms normalize each complete sample to unit area.

## Results

| reconstruction | count | median pT residual | q16 | q84 | central-68 half-width | inside +/-2% |
|---|---:|---:|---:|---:|---:|---:|
| muon LCIO | 990 | +0.00442% | -0.1290% | +0.1349% | 0.1320% | 989 |
| muon GSF with electron BH | 990 | +0.01296% | -0.1255% | +0.1698% | 0.1476% | 966 |
| electron LCIO | 1000 | -0.08720% | -4.7174% | +0.0841% | 2.4008% | 788 |
| electron reverse GSF BestBranch | 1000 | -0.03083% | -0.6535% | +0.1829% | 0.4182% | 857 |

Muon LCIO has RMS 0.208%. Forcing the electron BH workflow onto muons leaves
most of the core close to LCIO but gives RMS 7.81%, loses 23 events from the
`+/-2%` window, and creates large positive outliers. This demonstrates that
reverse electron-radiative selection can create a minority false-correction
tail even when genuine electron bremsstrahlung is absent. It is consistent
with, but does not by itself localize, the clean-electron over-selection
mechanism.

Electron LCIO has a near-zero peak but a long negative radiative tail. Its
inclusive central-68 width is about eighteen times the muon LCIO width, and it
loses 201 more events outside `+/-2%`. This establishes that the dominant
inclusive electron-versus-muon difference is the unrecovered material-loss
population, not a broad symmetric failure of the baseline tracker fit. A
surface-owned no-eBrem electron comparison is still needed to quantify the
intrinsic electron core against muons.

Electron reverse GSF moves the median toward zero, reduces the central-68
half-width from 2.401% to 0.418%, and raises the `+/-2%` population from 788 to
857. It therefore recovers a substantial part of the genuine electron
negative-loss population at 10 GeV. Its core remains broader than muon LCIO,
and its full RMS is 21.92% versus electron LCIO's 14.97% because it also
creates large positive and negative tails.

## Interpretation and conclusion boundary

Together with the independent Geant4 result that the fractional eBrem spectrum
is compatible between 2 and 10 GeV, these controls favor a reconstruction
hypothesis-selection limitation over a gross BH energy-scaling failure:

```text
electron LCIO negative tail
  -> reverse GSF recovers many genuine radiative losses into the core
  -> occasional false or excessive radiative selection creates new tails
```

The stress-test muons show the same qualitative minority over-selection in the
absence of electron radiation. This strengthens the existing priority to
understand identity/near-unity process covariance and decisive inner-hit odds,
while preserving demonstrated hard-loss recovery. It does not justify an ad
hoc measurement-evidence gate, a truth-dependent selection rule, or a claim
that the full GSF is validated at 10 GeV.

The user explicitly requested that these findings enhance understanding before
optimization; they do not change or reorder the active light-eBrem TODOs.

After the plot directory was deliberately cleared, the durable regenerated
products are exactly three pairwise comparisons under
`TrackingPerformanceStudies/lcio_track_resolution_10p0_theta85/plots/`:

1. `01_muon_gsf_vs_lcio{,_zoom_m2_2}.{png,pdf}`;
2. `02_lcio_muon_vs_electron{,_zoom_m2_2}.{png,pdf}`;
3. `03_electron_lcio_vs_gsf{,_zoom_m2_2}.{png,pdf}`;
4. `three_comparisons_summary.csv`.

They are reproduced by
`TrackingPerformanceStudies/lcio_track_resolution_10p0_theta85/`
`make_three_pt_comparisons.py`.
