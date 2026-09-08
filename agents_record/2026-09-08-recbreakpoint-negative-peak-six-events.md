# RecBreakpoint on six historical negative-peak events

User requested negative-peak tests with the new breakpoint method. Used code
checkpoint 2d0bd24 without changing source or maintained cards. All event
indices below are zero-based, seed 1, input
gsf_doublebhoff_freshseed_diagnostic/trk-e--2.0-85-1.root.

## Selection and truth ownership

Selected entries 2,7,40,55,72,97 BEFORE inspecting new breakpoint results.
These are established negative-residual cases from the Sept6 delayed-measurement
study and Sept5 negative-peak Eloss comparison. All six have zero secondary
tracker hits in a fresh full-detector SimTrackerHit topology count.
Reused the exact association -> SimTrackerHit/G4 link -> step audit, changing
only seed/input/output/event selection. It checked primary track ID, link
status, ordered endpoint cell IDs, hook steps and interval losses against the
passive GSF record. All selected hit-to-hit intervals are represented.
Truth selects intervals only, never fitted b or its prior.

| Entry | Selected intervals | Radius ranges (mm) | In-tracker eBrem loss % |
|---|---|---|---:|
| 2 | [6] | 235.10 -> 344.86 | 1.092436 |
| 7 | [5,121] | 43.81 -> 235.58; 1197.5 -> 1202.5 | 0.275261 |
| 40 | [7] | 345.68 -> 555.27 | 1.727962 |
| 55 | [9] | 556.12 -> 637.5 | 0.325028 |
| 72 | [3] | 44.05 -> 234.63 | 0.462188 |
| 97 | [9] | 555.17 -> 637.5 | 0.232078 |

Entry7 has a dominant 0.271392% loss and tiny 0.003880% TPC loss. Both were
selected, not only the dominant interval. Entry72 also has 0.022153% eBrem
before the first fitted hit; this is outside the model and is reported, not
silently assigned to a tracker interval. Indices are fitted-hit indices, not
fixed detector layer labels.

## Runs and settings

Breakpoint modes RTS and BackwardFilter: FirstMiddleLast, SeedScale=1,
MeanLogLoss=0, SigmaLogLoss=0.05, MSOn=true, ElossOn=false,
MaxChi2PerHit=1e100. Each mode also ran all six events with an empty interval
list. Total 24 successful breakpoint fit rows across 14 jobs, with verbose
states/covariances and native mode-matched references.

Fresh same-code GSF: reverse FullMixtureMode, CEPCRuntimeCategoryAligned9Clear,
DD4hepBetweenSurfaces, MaxComponents=10, cutoff=1e-4, SymmetricKL, identity
protection on, both directional BH gates on, ForwardSeed=BackwardSeed=1,
fresh inward seed scale=-1, LocalMeasurement, look-ahead depth0, beam spot off,
MS=true, Eloss=false, ECAL off, truth oracle off. Six selected GSF endpoints
were available with FullMixtureMode status1. This explicitly steered setup is
the same reference setup as the preceding RecBreakpoint comparisons; it need
not equal every historical negative-peak campaign setting.

Pairing checked event indices, generator/CompleteTracks pT, all ordered cell
pairs, selected intervals, successful status and covariance closure. The two
breakpoint modes have identical outward filtered curvature sequences. All
pairing checks passed. Generated files/scripts remain uncommitted under
TrackingPerformanceStudies/recbreakpoint_negative_peak_2026-09-08/.

## Results

Residual (%) = 100*(pT_reco/pT_truth-1). KF is stored CompleteTracks; GSF and
both breakpoint modes are fresh reruns. Empty is the same mode/settings with
BreakpointIntervals=[].

| Entry | Truth pT GeV | KF % | GSF % | RTS empty % | RTS breakpoint % | Backward empty % | Backward breakpoint % |
|---|---:|---:|---:|---:|---:|---:|---:|
| 2 | 16.336206 | -0.8603 | -0.8828 | -0.8964 | +0.5360 | -0.8750 | +0.4430 |
| 7 | 33.705505 | -0.2628 | -0.2586 | -0.3277 | -0.6602 | -0.2705 | -0.3395 |
| 40 | 22.987638 | -1.8328 | -1.8255 | -1.8559 | -1.1942 | -1.8434 | -1.1929 |
| 55 | 24.841885 | -0.1960 | -0.1966 | -0.2048 | +0.4890 | -0.2075 | +0.4089 |
| 72 | 30.097853 | -0.3888 | -0.4054 | -0.3982 | -4.3831 | -0.3967 | -1.8069 |
| 97 | 37.815044 | -0.4284 | -0.4577 | -0.5682 | +0.1549 | -0.4360 | +0.3145 |

Both breakpoint modes improve absolute residual versus GSF, KF and their own
empty controls in 3/6 cases (2,40,97); they worsen 7,55,72. Sample mean absolute
residuals are KF0.6615%, GSF0.6711%, RTS1.2362%, backward0.7509%. These are
deliberately selected negative cases, not population-resolution estimates.

## Diagnostic findings

Entry72 repeats the RTS failure signature. At the breakpoint its forward
kappa=-0.05996016, whereas smoothed source kappa=-0.03474801. Its next smoothed
kappa=-0.03335751 and fitted b=-0.02319044, versus truth b=+0.00463260.
The smoothed curvature jump ~+0.0013905 matches the frozen forward coefficient
times fitted b. Backward filtering reduces the endpoint tail but still fits
b=-0.01437355 and remains much worse than KF/GSF. Correct interval knowledge
does not guarantee correct sign or loss magnitude; relinearization alone has
not been tested as a remedy.

Entry7 fits a negative dominant loss at h5 in both modes (RTS b=-0.00316472,
backward b=-0.00226803), while overestimating the tiny TPC loss at h121
(b=0.00456442/0.00520796 versus truth 0.00003880). Entry55 overestimates the
loss (b=0.00754509/0.00687563 versus truth0.00325557), producing a worse
positive residual. Thus the limitation is not only branch selection in GSF:
this single-Gaussian, unbounded breakpoint model also misestimates losses.
The tests identify problems, not a validated solution. No tuning or fixes were
performed during this request.
