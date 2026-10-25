# Breakpoint independent audit and tomorrow's continuation

## Scope and unchanged runtime

User requested an independent read-only audit, then authorized documentation
correction only (point 4), and asked to record the issues for tomorrow.
Three independent reviewers covered KF/RTS, external constraints, and likelihood
optimization. Numerical implementation audited at c355cd8 on breakpoint.
Documentation/comments corrected in 3ca66e1; no algorithm, card, test, or installed
runtime changed. The initial sandbox push failed because SSH rejected permissions
on /etc/ssh/ssh_config.d/50-redhat.conf. A subsequent authorized host-context push
succeeded, including 3ca66e1 and handoff checkpoint b9d4d6d on origin/breakpoint.
All 11 existing C++ tests and 44 batch-helper tests passed. They do not cover the
demonstrated failures. No batch jobs or detector reruns were launched for this audit.
Outgoing full project status is preserved in
2026-10-26-agents-before-breakpoint-audit-handoff.md.

## Point 1: invalid energy linearization accepted (unfixed)

Actual rec_bp_barrel_20261026/breakpoint_flat-barrel-156.root,
event_index=109, input_track_index=0 (row 116):

| Quantity | Value |
|---|---:|
| Truth pT | 40.061218 GeV |
| Tracker-only free-loss pT | 44.392604 GeV |
| Charged-stage free-loss pT | 46.499305 GeV |
| Neutral-stage free-loss pT | 6094.499552 GeV |
| Neutral observation | 4.248148 +/- 0.039665 GeV |
| Smoothed affine photon energy | 4.377681 GeV |
| Smoothed physical photon energy | -1643.936503 GeV |
| Optimized prior center | 0.1433607643 |
| Posterior log loss | -0.1019351147 |
| Neutral constraint/optimizer status | successful (2), applied mask 7 |

The neutral-site curvature was +0.000459421 with sigma 0.029905 GeV^-1;
after its update sigma became 3.4404e-6 and rho(kappa,b)=0.999832.
The frozen energy derivative is unreliable near the zero-curvature pole, but
finite-state/covariance and local sign checks accept it. Full track/loss
correlations ARE retained. The final endpoint flips charge relative to the
preceding charged stage. This is demonstrated physical-validity/acceptance
failure, not proof of an incorrect analytic derivative or missing covariance.
Negative b alone is not a new bug: unrestricted Gaussian posteriors are documented.
Six of 15 accepted neutral fits in this one 214-row file have affine/physical
energy disagreement exceeding five observation sigmas. No population count
of point 1 has yet been made.

Relevant code: ScalarMeasurementSite.h, NeutralEnergyMeasurement.cpp,
BreakpointFitter.cpp terminal charged update, FreeLossFitter.cpp acceptance.
Potential containment: reject grossly invalid energy linearizations and retain
the preceding accepted stage. Thresholds and a proper recovery strategy remain
to be discussed; do not silently clip momentum, b, or correlations.

## Point 2: accepted result worse than known evaluated candidates (unfixed)

Same seed/event, neutral-stage NLL2 (lower is better):

| Candidate | Prior center | NLL2 |
|---|---:|---:|
| Published | 0.143360764321 | 2098879.706384 |
| Valid Minuit-phase evaluation | 0.142915760503 | 2098485.365114 |
| Valid post-Minuit local scan | 0.141360764503 | 2097116.269007 |

The published point has Minuit status 0 and EDM about 2e-9. The wrapper selects
only final points from successful Minuit runs. Coarse-scan best initializes a
run but is cleared as an acceptance candidate. Intermediate evaluations are
recorded, not used to challenge acceptance. Post-Minuit local scan is diagnostic
only. The repeat check establishes determinism, not optimality.

Do NOT say Minuit found a global optimum and discarded it. Trials from three
starts share one phase label; a better intermediate point may belong to a run
that failed convergence. The exact stopping mechanism is not yet diagnosed.
Potential repair: restart from materially better known valid candidates and
require successful reconciliation, otherwise fallback. Do not promote an
unconverged scan point as a successful minimization.

### Full current-batch frequency

376 files, 80,360 track rows; no read errors or missing trial histories among
accepted optimizations. Includes topology/control tracks, NOT topology-clear
only. Denominator is optimizer status=2 with finite published objective.
Compare published objective against minimum finite trial objective marked valid.
Thresholds below are absolute decreases in NLL2, not statistical significances.

| Stage | Accepted | Gap >0.001 | Rate | >0.1 | >1 | >10 | >100 |
|---|---:|---:|---:|---:|---:|---:|---:|
| Tracker only | 30170 | 17 | 0.05635% | 5 | 1 | 0 | 0 |
| Beam | 30038 | 13 | 0.04328% | 7 | 3 | 2 | 1 |
| Charged | 28986 | 8 | 0.02760% | 8 | 6 | 5 | 2 |
| Neutral | 7163 | 303 | 4.23007% | 290 | 287 | 255 | 215 |

341 affected stage results correspond to 333 distinct track rows. Among neutral
fits, 162 have a better coarse-scan point, 288 a better Minuit-phase point, and
138 a better final local-scan point (overlapping sets). No repeat-phase gap.
Largest neutral gap: seed128/event153/track0, 176918.448093 vs 9067.139363.
Frequency script and complete case list currently reside in
/tmp/bp-minimum-frequency-RDCEwT/count.py and result.log; they are temporary.
Reproduce by reading breakpoint tree branches for each free-loss prefix:
status, objective_nll2, trial_objective_nll2, trial_valid, trial_phase;
use uproot MemmapSource on AIFS, excluding copied/failed fits.

Neutral constraints are strongly associated with point 2, but causal linkage
to point 1 is not established. Known-better-candidate frequency is NOT a global
minimum failure rate or proof of improved pT if those candidates are selected.

## Other audit conclusions

- Point 3: recreated native 1D strip sites have an origin pivot; adapter scoring
  compares it with the propagated pivot and rejects the model. Demonstrated by
  a synthetic native strip-hit probe, NOT a production event. Current cards
  configure U/V measurements. Current campaign impact is unestablished; do not
  present strips as observed in the batch. No fix made.
- Point 4: docs/comments now explain evaluateFitLikelihood's dispatch: direct
  complete RTS quadratic without terminal charged ECAL; captured marginal plus
  terminal charged innovation with it; neutral inside captured model and beam
  factor counted once. No numerical changes; suggested integration test deferred.
- Beam handling rechecked: loose first/middle/last tracker prefit repivoted to
  beam; native scalar transverse drho update; native material-aware beam-to-hit
  transport; RTS reaches beam; IP publication is coordinate repivot, no repeated
  material traversal. sigmaX=.0145 mm, sigmaY=3.6e-5 mm, no z constraint.
  Beam CTest rerun passed. Vertex-distribution suitability not revalidated.
- No missing track/loss correlations, doubled beam likelihood, or omitted
  determinant found. Ordinary and free loss share the same KF/RTS core.

## Tomorrow: start with point 2 and its neutral-stage connection

1. Choose affected neutral events and unaffected controls from the same batch.
2. For each, scan the same objective versus prior center with/without neutral;
   record per-start Minuit outcomes, valid/invalid regions, and physical versus
   affine photon energy. Keep fixed observation sets within each scan.
3. Determine whether competing minima, failed starts, or unstable linearizations
   explain the overlooked candidates. Do not infer causality from rates alone.
4. Discuss acceptance repair and physical-validity containment with the user
   before changing the numerical implementation. No repairs authorized yet.

## Plot handoff

TrackingPerformanceStudies/breakpoint_prereco111_pt_resolution_20261026 remains
the analysis directory; server root is TrackingPerformanceStudies on port8000.
Inclusive hybrid now has only +/-2% views, one for free000 and one for free111,
each compared with baseline KF. No truth-override histogram, no fit, linear y.
Histograms are normalized over all 61,420 paired topology-clear tracks, not
renormalized within the display window; bins are .08 percentage points wide.
The truth gate uses KF below1% and breakpoint at/above1% (42986/18434 tracks).
Other analysis plots unchanged. Old four hybrid PNGs moved recoverably outside
the gallery to /tmp/bp-hybrid-old-views-sqARLs; plotting script reproduces new views.
Gallery verified: 86 PNGs in six sections, every PNG matched exactly once;
leaf page and new PNG return HTTP200. Existing server PID902008 on
ailogin001.ihep.ac.cn still serves the parent TrackingPerformanceStudies root.
