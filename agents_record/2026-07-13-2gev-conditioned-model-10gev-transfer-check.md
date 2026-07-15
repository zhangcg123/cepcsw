# 2 GeV conditioned BH model: 10 GeV transfer check

Date: 2026-07-13

## Provenance correction

The newly generated files at the repository root are named
`*-e--10.0-85-SEED.root`, and the flat tuples store generator momentum
`mc_p ~= 10.04 GeV`. They are therefore a 10 GeV, theta-85-degree sample, not
a 20 GeV sample. One hundred seeds with ten events each are present.

The 100 `gsf_material_steps-e--10.0-85-*.root` files were converted with the
established outgoing-current Geant4 surface-ownership script into
`TrackingPerformanceStudies/lcio_track_resolution_10p0_theta85/`
`material_transitions-e--10.0-85.csv`. The audit records 235,214 transitions
from 1,000 events. Restricting to the model range `t/X0 < 0.03` leaves 235,186
transitions, including 866 with a primary-electron eBrem step.

## Truth-level transfer evidence

The comparison uses the eBrem-attributed fractional loss
`ebrem_step_loss_sum_GeV / p_before_GeV`, matching the conditioned artifact and
avoiding double counting with deterministic `ElossOn=True`.

The global eBrem-transition fraction is 0.3702% in the ten-seed 2 GeV source
and 0.3682% in the 10 GeV sample. In the well-populated material bins, the 2
versus 10 GeV probabilities are:

| `t/X0` | 2 GeV | 10 GeV |
|---:|---:|---:|
| 0.002--0.005 | 4.48% | 5.08% |
| 0.005--0.010 | 8.94% | 9.20% |
| 0.010--0.015 | 14.74% | 14.48% |

No difference is statistically significant in those bins. The thicker 10 GeV
bins contain only 89 and 20 total transitions and cannot support a precise
claim.

Among radiative transitions, the loss-class fractions are:

| eBrem-attributed fractional loss | 2 GeV | 10 GeV |
|---:|---:|---:|
| below 1% | 58.20% | 58.66% |
| 1--5% | 16.47% | 15.94% |
| 5--20% | 12.90% | 13.97% |
| above 20% | 12.44% | 11.43% |

The full conditional loss spectra give a two-sample KS distance 0.0218 with
`p=0.84`; the available sample therefore provides no evidence for an
energy-dependent eBrem shape mismatch between 2 and 10 GeV at fixed angle and
geometry.

The global loss-per-radiation-length comparison gives transition medians
`DeltaE/(t/X0) = 1.709` and `9.263 GeV` at 2 and 10 GeV, reflecting the larger
absolute primary momentum. After momentum normalization the transition
medians are 1.222 and 0.979. At event level, the more stable aggregate
`(sum DeltaE / p0) / sum(t/X0)` medians are 0.19172 and 0.19138. Thus the
absolute loss scale changes as expected while the fractional loss rate is
consistent.

The following products were produced during the transfer study. The user later
explicitly cleared the 10 GeV `plots/` directory to replace it with three
pairwise reconstruction comparisons, so the first two plot products are no
longer retained there; the transition table, numerical evidence in this
record, and reproduction scripts remain:

- `TrackingPerformanceStudies/lcio_track_resolution_10p0_theta85/plots/`
  `cepc2gev85_model_vs_10gev_g4_spectrum.png`;
- `TrackingPerformanceStudies/lcio_track_resolution_10p0_theta85/plots/`
  `ebrem_energy_loss_per_x0_2gev_vs_10gev.{png,pdf}` and its summary CSV;
- `Reconstruction/RecGsfTracking/scripts/compare_energy_loss_per_x0.py`.

## Reconstruction result and conclusion boundary

The 100 GSF flat tuples contain 1,000 valid events. LCIO versus reverse
BestBranch has median pT residual -0.0872% versus -0.0308%, central-68 interval
`[-4.72%, +0.0841%]` versus `[-0.654%, +0.183%]`, and 842 versus 902 events
inside +/-5%. GSF improves the core and negative-loss side, but its full RMS
is worse because significant positive and negative outliers remain.

Conclusion at the time of this check: `CEPC2GeV85StepConditioned` is a
reasonable provisional energy-transferable BH process model from 2 to 10 GeV
at theta 85 degrees in the same detector geometry and ownership convention.
The subsequent 10 GeV-pT, 20-degree Geant4 study extends the evidence across
one large angle change; see
`agents_record/2026-07-13-cepc-conditioned-bh-transferability-universe.md`.
Neither result is general CEPC BH validation or validation of the complete
reverse GSF. Low energies, other detector regions and material configurations,
and broad independent validation remain untested. The active light-eBrem
optimization focus is unchanged.
