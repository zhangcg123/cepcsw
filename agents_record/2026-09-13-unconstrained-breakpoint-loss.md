# Unconstrained loss KF/RTS: implementation and focused checks (2026-09-13)

## Request and scope

The user requested a no-constraint version of the breakpoint KF/RTS, then
tests of whether weak early-loss determination prevents outer-hit information
from constraining the pre-loss track. Interpreted explicitly as **no Gaussian
prior on b**, not b fixed to zero and not SigmaLogLoss=0 as a physical fit.
Only RecBreakpoint sources, its dedicated card/batch helper/tests and project
documentation changed. Shared KF, GSF and user-owned edits were preserved.
No branch or remote operation is authorized or performed.

LossPriorMode=Unconstrained is opt-in (Gaussian remains the compiled/card
default). It uses native conditional KF updates plus zero-prior-precision
scalar regression and the existing RTS response. The final state covariance
includes Var(b) and state/b correlations; it is not the conditional error of
the separate Minuit result. The first implementation supports one
LocalMarginal interval. Negative b is permitted: no positivity bound is hidden.
An explicit Fixed control holds MeanLogLoss with zero variance and is used
only for matched truth-loss diagnostics here. The extra truth-prior pair
remains Gaussian; the optional Minuit pair remains independent.

The derivation, controls, masks for diffuse/unrepresentable prefixes, and
schema are in Reconstruction/RecBreakpoint/docs/unconstrained-loss.md.
A globally proper track seed and the native noise remain: no prior applies
only to b. Backward still uses the copied/scaled forward endpoint and is not
an independent Bayesian smoother. No reference iterations were added.

## Deployment and preservation

Built plugin SHA256:
6c8575ee43a86a0d135deaf870f04393b6db9b9092d6dd96bf44bdb111fedb3e

The new package was tested from the build tree, NOT installed. The installed
plugin remains the previously validated normalized-RTS-objective version:
7ed0302ff8f490f6e59c452ac3e12fda21615556bb9f4c230977b3463f574540

An asynchronous question about active batch jobs was sent. No answer was
available while preparing this record. Do not assume the installed schema
already knows LossPriorMode: the new card needs the build-tree configuration
and library, or a later package-only install after deployment is agreed.
Old prepared cards and their old installed workflow remain unchanged.
The current source mode is not promoted as a production candidate.

Unrelated tracked edits remain in DumpGsfTrks/gsf.py.bk,
DumpGsfTrks/sim.py.bk and subtrkjobs.sh. Generated ROOT files, logs, cards and
analysis outputs are not staged/committed.

## Mechanical gates

- Three compiled numerical tests pass. The new test compares the no-prior
  solution to an independent dense joint least-squares system: b, Var(b),
  every smoothed mean/covariance and Cov(state,b), with Q=0 and Q>0,
  and changed affine reference coordinates. Zero-information b is rejected.
- 25 batch/card tests pass; all 30 properties are explicitly steered.
- Gaussian mode reruns 2:68 and 12:11,16,17 against the previous installed
  outputs: all 160 old tuple fields and 11,172 verbose mean/covariance
  records agree exactly. The old numerical path is preserved.
- A first diffuse smoke run exposed an unrepresentable intermediate
  covariance. Such prefix states now receive explicit invalid masks/NaN
  covariance, without jitter or an artificial b prior. Final RTS states and
  both endpoints must still pass validation.
- The final 11 rows have status=1 and valid complete smoothed chi2. The
  maximum absolute smoothed-minus-profiled-forward chi2 closure is
  1.232224e-6.
  Numerical status is NOT a physics-quality gate; notably 4:11 backward is
  essentially unconstrained and gives a pathological momentum.

## Exact sample and settings

Inputs: gsf_doublebhoff_freshseed_diagnostic/trk-e--2.0-85-{2,3,4,5,6,12}.root.
Identity checked by seed, zero-based event_index, input_track_index, hit cell
IDs and selected interval. Truth selection uses the largest matched G4 loss
interval. That does not fix unselected losses or the within-interval emission
position. SeedScale=1, BackwardSeedScale=100, FirstMiddleLast, MSOn=true,
ElossOn=false, LocalMarginal. Gaussian prior b=0, sigma=0.001; no-prior
reference b=0 with no prior precision. FreeLossFit=false for the comparisons.
The regression gate additionally uses the original FreeLossFit=true settings.

There are 11 input-track-0 rows: nine selected-loss examples, one no-selected-
interval control (12:16) and one secondary-activity control (12:17).
12:17 is excluded from clean single-track interpretations and shown separately.
The three soft h4->h5 rows 3:10,3:33,5:62 come from the recorded single-eBrem,
zero-secondary-hit selection in recbreakpoint_single_ebrem_2026-09-08.

Residual = 100*(pT_reco-pT_truth)/pT_truth, in percent. Fixed-truth is a separate
native rerun with exact matched b held fixed, not the Gaussian truth-prior pair.

| Seed:entry | Interval | Truth loss % | KF | Gaussian RTS | No-prior RTS | No-prior backward | Fixed-truth RTS | Fixed-truth backward |
|---|---|---:|---:|---:|---:|---:|---:|---:|
| 2:68 | h7->h8 | 3.158228 | -2.519150 | -2.586328 | 0.788773 | 0.752194 | -0.014498 | 0.101270 |
| 3:10 | h4->h5 | 0.716130 | -0.708109 | -0.727532 | -29.018931 | -29.434987 | -0.011621 | 0.031074 |
| 3:33 | h4->h5 | 0.935474 | -1.115832 | -1.091382 | -3.204486 | 1.077818 | -0.157487 | -0.288414 |
| 3:36 | h4->h5 | 20.452285 | -20.444274 | -20.467908 | 51.110885 | 49.919710 | -0.028149 | 0.049985 |
| 4:11 | h0->h1 | 0.009618 | 0.127005 | 0.085801 | -0.006581 | -100.000000 | 0.095432 | 0.102456 |
| 5:62 | h4->h5 | 0.555308 | -0.130488 | -0.133079 | 60.328798 | 76.063486 | 0.423662 | 0.383774 |
| 6:37 | h34->h35 | 0.489021 | -0.271920 | -0.265113 | -0.099223 | 0.174978 | 0.120417 | -0.045552 |
| 6:38 | h7->h8 | 27.352344 | -26.497885 | -26.267249 | 25.749545 | 21.539331 | 0.461916 | 0.479737 |
| 12:11 | h10->h11 | 0.524946 | -0.584490 | -0.601689 | -0.280739 | -0.292781 | -0.125394 | -0.095410 |
| 12:16 | none | — | -0.257888 | -0.287240 | -0.287240 | -0.191094 | -0.287240 | -0.191094 |
| 12:17 (secondary control) | h2->h3 | 42.547443 | -42.622679 | -42.637323 | -77.115399 | -77.255394 | -0.131672 | -0.111779 |

## Loss and inner-state dependence

b is log(p_before/p_after), not percent. The covariance/correlation below is
at the first hit, before the selected loss, and comes from the captured affine
model. It is not an independently calibrated experimental uncertainty.

| Seed:entry | Truth b | Fitted b | sigma_b | Corr(inner kappa,b) | Marginal inner sigma_pT/pT % | Conditional inner sigma_pT/pT % |
|---|---:|---:|---:|---:|---:|---:|
| 2:68 | 0.032092 | 0.042845 | 0.010335 | 0.977738 | 0.862803 | 0.181042 |
| 3:10 | 0.007187 | -0.382393 | 0.468259 | 0.999995 | 34.898472 | 0.112574 |
| 3:33 | 0.009399 | -0.050389 | 0.647319 | 0.999987 | 27.445669 | 0.138127 |
| 3:36 | 0.228813 | -0.462179 | 0.261559 | -0.999986 | 50.934593 | 0.265312 |
| 4:11 | 0.000096 | -0.000923 | 132.505253 | 1.000000 | 13251.029686 | 0.135278 |
| 5:62 | 0.005569 | 0.192561 | 0.266816 | 0.999994 | 83.890399 | 0.285036 |
| 6:37 | 0.004902 | 0.002172 | 0.005400 | 0.955476 | 0.447684 | 0.132098 |
| 6:38 | 0.319549 | 0.566032 | 0.013528 | 0.992382 | 1.708322 | 0.210460 |
| 12:11 | 0.005263 | 0.003611 | 0.005138 | 0.971514 | 0.490836 | 0.116320 |
| 12:17 | 0.554211 | 0.208053 | 0.291940 | -1.000000 | 84.340756 | 0.059532 |

The three soft h4->h5 examples have |correlation|>0.99998 and sigma_b
0.267--0.647. Their data cannot tightly separate b and inner curvature WITHIN
THIS MODEL. The conditional uncertainty (b held fixed) can be small while
the marginal uncertainty is enormous. This supports the user's concern,
but does not prove that the detector itself has an irreducible limit here.

## Additional response discrepancy: do not blame only sparse hits

A small native fixed-b rerun, b=0 to 1e-4, was compared to the conditional
inner-curvature response recovered from Cov(kappa,b)/Var(b). Subtracting this
response times fitted b reconstructs the b=0 native RTS reference exactly.

| Seed:entry | Captured d(kappa_inner)/db | Full native finite difference | Native / captured |
|---|---:|---:|---:|
| 2:68 | 0.018124096066933472 | 0.018789955890038956 | 1.0367389259385087 |
| 3:10 | 0.033171456889167186 | 0.031808709831659954 | 0.9589180824327235 |
| 3:36 | -0.048718801211766236 | 0.04751385501544636 | -0.975267326651115 |
| 5:62 | 0.05648310165945943 | 0.028819700186033526 | 0.5102357933491244 |

The opposite sign for 3:36 and roughly factor-two difference for 5:62 require
explanation. This is a frozen-model response versus a full native rerun:
the latter also changes propagation/measurement expansions and covariance
evolution. The test ALONE does not identify a particular bad local Jacobian.
Do not call it proof of an implementation bug in a specific shared class.
Nor does the dense linear-model unit test validate agreement with the native
nonlinear rerun. The event results therefore do not justify attributing all
failure to missing inner information.

Repeating the two discrepant cases with a ten-times-smaller step, 1e-5,
preserves the discrepancy: native derivatives 0.04751600315641701 (3:36)
and 0.02882102229685712 (5:62), native/captured ratios -0.9753114192994812
and 0.5102592005414482. Reference reconstruction remains exactly zero-difference.
These reruns and checks are retained in probe_small/ and response_probe_small.json.

A native fixed-truth rerun is also not the same model as holding truth b in
the b=0 affine capture. Both comparisons are retained in results.json; for
example 3:36 has fixed-native-truth residual -0.028%, but the fixed-truth
conditional mean of the b=0 capture has first-hit residual -35.576%.
These endpoints/contexts must not be silently equated.

## Current conclusion and next direction

The no-prior implementation is mechanically tested but is NOT a performance
solution: large tails and unphysical negative losses remain. Preserve the
Gaussian default. First trace the loss-to-inner-state response discrepancy
surface by surface, separating mean transport, measurement expansion and
covariance dependence. The strong inner-curvature/b correlation is real in
the captured model but is not yet a sufficient detector-level explanation.
Do not add a new prior, iteration or shared-KF modification without discussing
the next change with the user.

Generated evidence is under
TrackingPerformanceStudies/breakpoint_unconstrained_20260913/:
run.py, analyze.py, regression/, gaussian/, unconstrained/, fixed/,
probe_zero/, probe_epsilon/, results.csv, results.json,
regression_checks.json and response_probe.json.
The first failed smoke and the subsequent passing smoke2 are retained.
Build/numerical/batch logs are /tmp/recbreakpoint_unconstrained_*.
The outgoing full project status is preserved in
agents_record/2026-09-13-agents-before-unconstrained-loss.md.
