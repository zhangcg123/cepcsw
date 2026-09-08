# RecBreakpoint: twenty additional single-eBrem negative-residual events

## Scope and reproducibility

User requested more events restricted to single eBrem. Used unchanged source
at checkpoint 4b20778 on local test_breakpoint. No source, maintained card,
branch or remote modifications were made for this experiment. This extends
the six-event study, not a population validation or optimization.

Generated scripts, detailed tables, tuples and verbose logs are under
TrackingPerformanceStudies/recbreakpoint_single_ebrem_2026-09-08/.
The main numerical artifact is comparison.csv; summary.json and
summary_completed.log contain aggregate metrics and verification results.

## Selection and truth audit

Candidates came from the historical barrel event_residuals.csv in
TrackingPerformanceStudies/newbh_barrel_endcap_pt_resolution_2026-09-06/.
Required seeds 2--6, zero-based entry below 100 (the available tracker-input
range), zero secondary tracker hits, valid truth-material scope, one recorded
eBrem interval, cumulative loss in [0.2,2)% and historical FullMixtureMode
residual below -0.1%. This is deliberately a negative-residual selection.

Fresh exact reconstructed-hit association -> SimTrackerHit/G4-step link
audits then required exactly ONE positive-loss primary Geant4 eBrem step
across all recorded primary steps, inside the fitted hit-to-hit range, and no
secondary tracker hits in the full detector SimTrackerHit collections.
This is stricter than merely requiring one interval containing eBrem.
All ordered endpoint cell IDs, hook steps, initial momenta and interval losses
were checked against the existing passive GSF interval record.

Of 31 candidates, 22 passed; the first 20 in seed/entry order were fixed before
examining the new fit results. Seven have losses 0.2--0.5%, seven 0.5--1%, and
six 1--2%. All twenty are new relative to the preceding seed-1 study.
Rejected multi-step candidates (seed:entry): 3:18, 3:81, 3:82, 4:1, 4:29,
5:23, 6:2, 6:22, 6:39. Five of these also contain an outside-interval loss.
Details are retained in selected.json, rejected.json and truth_audit_seed*.json.

Truth selects the single breakpoint interval ONLY. It does not set the fitted
loss or its prior. Interval i means ordered hit[i] -> hit[i+1], not a fixed
detector layer number. The experiment does not move the loss mapping to the
exact within-interval Geant4 position.

## Settings and execution

RecBreakpoint: both RTS and BackwardFilter, FirstMiddleLast, SeedScale=1,
MeanLogLoss=0, SigmaLogLoss=0.05, MSOn=true, ElossOn=false,
MaxChi2PerHit=1e100. Each mode also runs all twenty events with an empty
breakpoint list. Native mode-matched reference and verbose diagnostics on.
There are 80 successful fit rows across 42 breakpoint jobs.

Fresh GSF reference: reverse FullMixtureMode, CEPCRuntimeCategoryAligned9Clear,
DD4hepBetweenSurfaces, MaxComponents=10, cutoff=1e-4, SymmetricKL,
identity protection on, both BH splitting gates on, ForwardSeed=BackwardSeed=1,
InwardSeedCovarianceScale=-1, LocalMeasurement, look-ahead depth0, beam spot
off, MS=true, Eloss=false, ECAL off, truth override off. All three verbose
component/splitting controls remained on. These are the same explicit settings
as the preceding six-event comparison, not necessarily the historical
negative-residual campaign settings or current maintained-card values.

The original grouped seed-5 GSF job exited 139 after printing the entry-62 fit
summary. Its partial ROOT output was NOT used. Entry62 was rerun alone, then
entries13,49,58,78,84,92 were rerun together. Both retries terminated normally
with identical source, physics settings and verbose diagnostics. The crash's
cause is unresolved; successful retries are not a source-level repair claim.
Original failed log/output are retained. Other four seed reference jobs
terminated normally. Final analysis uses twenty successful fresh GSF endpoints.

All selected output statuses and FullMixtureMode availability/status passed.
Pairing checks passed for event indices, generator and stored CompleteTracks
pT, every ordered endpoint cell pair, selected intervals and mode fields.
The RTS and BackwardFilter runs have identical outward filtered curvature and
curvature-variance sequences. Maximum covariance-transport closure is
3.32931e-16; maximum no-breakpoint/native-reference relative pT difference is
2.96004e-6. These are mechanical checks, not physics validation.

## Results

Residual (%) = 100*(pT_reco/pT_truth-1). KF is stored CompleteTracks. GSF is
the fresh FullMixtureMode reference. RTS/Bwd both include the truth-selected
single breakpoint with an unfitted-prior mean of zero. Entries are zero-based.

| Seed:entry | Interval | Truth loss % | KF % | GSF % | RTS % | Bwd % |
|---|---:|---:|---:|---:|---:|---:|
| 2:16 | 6 | 1.14889 | -1.43393 | -1.42973 | -1.21268 | -1.17299 |
| 2:86 | 7 | 0.49054 | -0.54979 | -0.56375 | +0.20869 | +0.03087 |
| 3:10 | 4 | 0.71613 | -0.70811 | -0.71862 | -1.17142 | -1.15723 |
| 3:12 | 5 | 1.93335 | -1.57601 | -0.30769 | -0.54183 | -0.71519 |
| 3:19 | 6 | 1.36630 | -1.24935 | -1.26541 | +0.37010 | +0.65764 |
| 3:33 | 4 | 0.93547 | -1.11583 | -1.12874 | -1.10417 | -1.10683 |
| 3:37 | 7 | 0.20028 | -0.12642 | -0.14158 | -0.13659 | -0.05526 |
| 3:79 | 6 | 0.31334 | -0.27462 | -0.24839 | +0.49500 | +0.60159 |
| 4:4 | 8 | 1.05645 | -1.15849 | -1.19171 | -0.08246 | -0.14714 |
| 5:13 | 9 | 0.24497 | -0.19446 | -0.22332 | +0.34841 | +0.34335 |
| 5:49 | 7 | 0.27884 | -0.19071 | -0.17641 | +0.19293 | +0.27965 |
| 5:58 | 6 | 0.56369 | -0.43866 | -0.47484 | +1.27548 | +1.28658 |
| 5:62 | 4 | 0.55531 | -0.13049 | -0.12606 | +1.16073 | +0.28251 |
| 5:78 | 8 | 1.56399 | -1.45950 | -1.43828 | -0.56363 | -0.60558 |
| 5:84 | 5 | 0.77899 | -0.72615 | -0.73199 | -2.51972 | -1.15340 |
| 5:92 | 6 | 0.89879 | -0.74148 | -0.72356 | -2.22323 | -2.09560 |
| 6:4 | 7 | 0.34434 | -0.37039 | -0.36213 | +1.21021 | +1.17012 |
| 6:17 | 13 | 0.91266 | -0.81052 | -0.81960 | -0.43437 | -0.52482 |
| 6:37 | 34 | 0.48902 | -0.27192 | -0.26463 | -0.10103 | +0.04495 |
| 6:50 | 7 | 1.11899 | -0.94797 | -0.94440 | +0.16706 | +0.12650 |

| Method | Mean residual % | Mean absolute residual % | RMS % | Within +/-0.5% | Within +/-1% |
|---|---:|---:|---:|---:|---:|
| Stored KF | -0.72374 | 0.72374 | 0.86141 | 8/20 | 14/20 |
| Fresh GSF | -0.66404 | 0.66404 | 0.79264 | 9/20 | 15/20 |
| RTS empty | -0.72774 | 0.72774 | 0.86867 | 8/20 | 14/20 |
| RTS breakpoint | -0.23313 | 0.77599 | 1.03054 | 10/20 | 12/20 |
| Backward empty | -0.73605 | 0.73605 | 0.87159 | 8/20 | 14/20 |
| Backward breakpoint | -0.19551 | 0.67789 | 0.86470 | 8/20 | 13/20 |

RTS improves absolute residual versus KF, GSF and its own empty control in
10/20 cases each. Backward improves versus KF and its own empty control in
11/20, and versus GSF in 10/20. These selected-sample RMS values are NOT fitted
core resolution or unbiased population estimates. Smaller mean bias alone is
not improved precision: the new positive and negative tails must count.

## Findings

RTS has negative fitted b in four events: 3:10, 3:33, 5:84, 5:92. Backward
has negative fitted b in three: 3:10, 5:84, 5:92. Thus wrong-sign fitted loss
persists with exactly one true eBrem; extra tiny losses cannot explain it away.

Examples of fitted b (log momentum ratio, not percent):

- 4:4: truth0.0106207, RTS0.0120874, backward0.0113459. Both recover most of
  the negative residual; GSF -1.19171% becomes -0.08246%/-0.14714%.
- 5:92: truth0.00902855, RTS-0.0159522, backward-0.0140939. GSF -0.72356%
  becomes -2.22323%/-2.09560%. Fitted b uncertainties are still large,
  0.0197361/0.0192494; this is not a highly significant negative-loss detection.
- 5:58: truth0.00565281, RTS0.0185722, backward0.0175980. Overestimated loss
  turns GSF -0.47484% into +1.27548%/+1.28658%.
- 5:84: truth0.00782039, RTS-0.00752647, backward-0.00423452. RTS forward
  source kappa=-0.0672634 versus smoothed -0.0279604, repeating the large
  linearization-point shift seen in earlier failures. Its exact loss is near
  the downstream endpoint (r235.889mm within r45.349 ->236.082mm); location and
  linearization are possible contributors, not isolated causal conclusions.

The single-Gaussian unconstrained-sign breakpoint is not a demonstrated
replacement for GSF. Known interval placement helps some tracks but does not
guarantee the correct loss sign or magnitude. No positivity constraint,
relinearization or altered interval convention was tested/tuned in this run.
