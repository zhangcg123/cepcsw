# Pre-disconnect integrated understanding and handoff

Date: 2026-07-13

## Stable physics picture

The accumulated checks separate the problem into a provisionally transferable
material-loss model and an imperfect reconstruction selector.

1. `CEPC2GeV85StepConditioned` models eBrem-attributed retained momentum
   fraction versus outgoing-current interval `t/X0`; it does not model absolute
   loss in GeV. Geant4 truth supports fractional-loss transfer across the
   tested 2--10 GeV pT range and theta 85--20 degrees in unchanged geometry.
2. The tested global fractional eBrem loss per accumulated X0 is 0.958, 0.973,
   and 0.987 for 2 GeV-pT/85 degrees, 10 GeV-pT/85 degrees, and 10
   GeV-pT/20 degrees. Positive-eBrem loss shapes are statistically compatible.
3. The 10 GeV-pT/20-degree sample has total momentum about 29.32 GeV because
   the filename quantity is pT. Its much larger absolute loss per X0 is
   expected and disappears in the fractional comparison.
4. Reverse GSF demonstrably recovers genuine electron radiation. At 10
   GeV-pT/85 degrees it improves inclusive central-68 pT half-width from
   2.401% to 0.418% and moves 788 to 857 of 1000 events inside +/-2%.
5. The remaining weakness is a minority selection tail. Forced electron-BH
   GSF on 10 GeV-pT muons leaves most of the LCIO-like core intact but creates
   large false-correction outliers. This disfavors a universal reverse momentum
   inflator and a gross BH scaling failure, while supporting a state-by-state
   identity/radiative hypothesis-selection problem.
6. The exact no-eBrem identity uses `sigma_z=1e-6`; Geant4 no-eBrem central
   spreads near `t/X0~=0.01` are about `5--9e-5`. Together with decisive
   inner-hit likelihood ratios, this makes missing non-radiative process width
   the leading targeted physics hypothesis, but not yet a proven complete fix.

## Final 10 GeV-pT, 20-degree reconstruction result

All production jobs stopped. Ninety-nine of 100 flat tuples contain exactly
ten valid entries; `gsf_flat-e--10.0-20-55.root` is empty/no-key. The 990-event
comparison is:

| reconstruction | median pT residual | q16 | q84 | central-68 half-width | inside +/-2% | RMS |
|---|---:|---:|---:|---:|---:|---:|
| LCIO | -1.063% | -17.347% | +0.0788% | 8.713% | 592/990 | 21.08% |
| reverse GSF BestBranch | -0.293% | -7.252% | +0.541% | 3.897% | 702/990 | 27.18% |

Within +/-10%, LCIO has 789/990 and GSF has 827/990. The final wide-window
plot uses 0.25%-wide bins and separately normalizes each curve inside that
window. The result confirms stronger genuine recovery under the larger
material exposure, but the full RMS worsens because GSF creates extreme
positive and negative outliers.

This is categorized only inclusively; a surface-owned no/light/hard breakdown
at 20 degrees has not yet been produced. Do not interpret the inclusive width
or median as clean-track performance.

## Operational understanding

Several 20-degree GSF jobs previously exceeded memory or remained incomplete
for long periods. The final production reached 99/100, but shallow-angle tracks
traverse more material and can generate more split components, clones, and
retained measurement histories. Component/history memory growth is therefore
a real broad-angle deployment issue. It should be addressed after the physics
selection semantics are understood, so memory control does not silently remove
the branches needed for hard-loss recovery.

## Resume point and unchanged optimization order

The active light-eBrem optimization order remains authoritative:

1. finish decisive-hit prior/innovation/KL audits on ordinary missed, partial,
   good, overshooting, and low-loss false-correction light events;
2. test a mechanism-specific no-eBrem/near-unity process-width change only if
   supported by those audits and the measured straggling scale;
3. validate first on ordinary light representatives, clean 62/9, hard 1/3,
   then 11/16/17 and the full clean/light/hard 2 GeV categories;
4. add forced-BH muons, representative 10 GeV/85-degree electrons, and
   representative 10 GeV-pT/20-degree electrons as transfer/safety controls;
5. only after physics stability, diagnose and bound shallow-angle memory growth.

Do not introduce an ad hoc measurement-evidence gate, truth-dependent
selection, WeightedMean default, global covariance tuning, or premature
component pruning. Success remains simultaneous clean-core preservation,
light-tail improvement, and hard-loss recovery with finite complete tracks.

## Files needed after reconnect

- All 20-degree ROOT products are under `tuples1020/`: 100 each of simulation,
  tracking, GSF EDM, GSF flat, and material-step files.
- Final 20-degree plots and summary are under
  `TrackingPerformanceStudies/material_loss_10p0_theta20/gsf_finished/`.
- The wide plot is `electron_lcio_vs_gsf_finished_zoom_m10_10.{png,pdf}`.
- The concurrency-safe reproduction script is
  `TrackingPerformanceStudies/material_loss_10p0_theta20/`
  `plot_finished_lcio_vs_gsf.py`.
- The material transition table, audit, and three-sample loss-per-X0 products
  are under `TrackingPerformanceStudies/material_loss_10p0_theta20/`.
- Full BH transferability definitions and evidence are in
  `agents_record/2026-07-13-cepc-conditioned-bh-transferability-universe.md`.
- The 10 GeV/85-degree electron/muon controls are in
  `agents_record/2026-07-13-10gev-electron-muon-lcio-gsf-controls.md`.

No Git operation was performed. The active branch restriction and all project
laws remain unchanged.
