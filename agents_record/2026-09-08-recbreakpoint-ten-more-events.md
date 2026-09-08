# RecBreakpoint: ten additional truth-location tests — 2026-09-08

## Scope and pairing

The user requested a few more events and a comparison with GSF. All runs use
the unchanged RecBreakpoint implementation at 7d278e3, on test_breakpoint.
No fitter, shared KF, GSF source, or maintained run card was edited. The new
cards/scripts and outputs are isolated, uncommitted analysis artifacts.

Input: gsf_doublebhoff_freshseed_diagnostic/trk-e--2.0-85-12.root, the same
seed-12 tracker file as the preceding two breakpoint records.

New zero-based event indices: 0,1,2,3,6,8,12,13,14,15. They were selected from
the first seed-12 events using truth-loss categories and the existing topology
census, not the new breakpoint or GSF performance. There are three no-loss
controls, three events with 0.1--1% loss between tracker hits, and four with
at least 1% loss between hits. This is a diagnostic sample, not a population.

Raw embedded-data auditing then confirmed zero non-primary SimTrackerHits in
VXD, ITK barrel/endcap, TPC and OTK barrel/endcap for every selected event,
matching the established MCParticle-index>0 topology convention and prior
census. The earlier event 17 has one such secondary hit and is not included
in this ten-event single-track comparison; its old and new GSF values are
checked only as a separate regression control.

Reconstructed-hit truth associations and complete Geant4 hook links validate
all 2,327 hits and 2,317 adjacent intervals. The ordered endpoint cell IDs,
hook step numbers and losses agree with the previous passive GSF records.
All new GSF and breakpoint outputs are then paired by event, generator pT,
CompleteTracks pT, and ordered interval endpoint cell IDs.

The first extended Python truth reader failed while advancing past the final
requested event. It produced no completed JSON. Keeping selected PODIO Frames
alive while Python handles reference their collections, and stopping directly
after the last selected event, allowed the complete audit to finish normally.
The successful audit was repeated with raw topology counts before comparison.
This was analysis-reader work, not a fitter/shared-source change.

## Settings

Breakpoint: all positive truth-eBrem intervals selected, not just the dominant
one. MeanLogLoss=0, SigmaLogLoss=.05, SeedScale=1, MS=true, Eloss=false.
Every loss is fitted: truth provides interval indices, not loss magnitudes or
prior means/widths. No beam spot. Each job also performs an independent native
MarlinTrk no-breakpoint fit with matching seed and material settings.

GSF: fresh reverse FullMixtureMode, CEPCRuntimeCategoryAligned9Clear,
DD4hepBetweenSurfaces, MaxComponents=10, cutoff=1e-4, SymmetricKL, identity
protection on, both directional BH gates on, ForwardSeed=BackwardSeed=1,
fresh inward seed (scale=-1), LocalMeasurement, look-ahead depth=0,
beam spot off, MS=true, Eloss=false, truth oracle off and ECAL off.
An isolated wrapper loads the maintained card then explicitly overrides beam
and look-ahead settings and all input/output paths. It does not edit the card.

All ten breakpoint fits succeeded. All selected GSF FullMixtureMode outputs
were available with status=1. Full verbose component/state/covariance logs are
saved. GSF also reran old indices 11,16,17; its pT agrees with the earlier
record within 1e-6 GeV (largest difference 9.78e-7 GeV at event 16).

## Results

Residuals below are 100*(pT_reco-pT_truth)/pT_truth. KF means the stored
CompleteTracks reference. GSF and breakpoint are fresh same-code outputs.
The loss column includes only selected hit-to-hit intervals and combines
multiple interval fractions as 100*(1-product(1-fraction)).

| Event | Interval indices | Truth interval loss % | KF residual % | GSF residual % | Breakpoint residual % |
|---:|---|---:|---:|---:|---:|
| 0 | none | 0 | +0.086157 | +0.067366 | +0.053595 |
| 1* | 5 | 0.149687 | -0.553703 | -0.547980 | -0.680272 |
| 2 | 6 | 2.019346 | -2.030544 | -2.059961 | -1.624363 |
| 3 | 4 | 0.363305 | -0.373885 | -0.337096 | -11.968825 |
| 6 | none | 0 | -0.024182 | -0.039748 | -0.045962 |
| 8 | 231 | 3.514426 | -0.286467 | -0.311948 | -0.318531 |
| 12 | 8 | 7.164912 | -6.753988 | -0.347776 | -0.216455 |
| 13* | 231 | 0.308807 | -2.298512 | -2.304665 | -2.324117 |
| 14 | 0,7,230 | 26.276076 | -5.566795 | -5.593213 | -4.411410 |
| 15 | none | 0 | +0.087718 | +0.069455 | +0.065090 |

*Events 1 and 13 additionally lose 0.502245% and 2.294819%, respectively,
before the first tracker hit (Geant4 post-step radii 10.49175 and 9.99819 mm).
The current breakpoint implementation has no configurable IP-to-hit-0 edge.
They are retained and separately flagged as boundary-limited controls, not
treated as fully covered truth-location tests.

Across all ten rows, breakpoint is closer to generator pT than GSF in 5/10.
Of the eight fully covered rows, three are no-eBrem controls with no breakpoint;
their differences mostly reflect ordinary KF/refit details. Among the five
fully covered eBrem events, breakpoint is closer than GSF in 3/5 (2,12,14),
worse in 2/5 (3,8). Counting wins hides event 3's very large new tail.
Native-KF and no-breakpoint numerical differences near 1e-6 GeV should not be
interpreted as physical improvements.

## Concrete limitation exposed by event 3

Event 12 is encouraging: truth pT=19.19856 GeV, GSF=19.13180 GeV and
breakpoint=19.15701 GeV. But event 3, with only 0.363% truth interval loss,
moves from GSF=41.94469 GeV to breakpoint=37.04930 GeV, versus truth=42.08657.

Its verbose breakpoint log shows at source hit 4:

- Forward-filtered kappa approximately -0.159029 GeV^-1 (pT about 6.29 GeV).
- Smoothed source kappa approximately -0.026991 GeV^-1.
- Smoothed next-hit kappa approximately -0.0238487 GeV^-1.
- Fitted b=-0.0197586744, i.e. an allowed negative-loss estimate.

The loss Jacobian d(kappa_after)/db is evaluated once using the original
forward source curvature, which is far from the subsequently smoothed value.
The fitted mean retains that linearization. Numerically the curvature jump
is consistent with old-source-kappa times fitted b. This identifies a concrete
single-linearization consistency concern, not proof that all poor cases have
the same cause. Relinarization or a constrained/nonlinear fit was NOT tried
and the source was NOT modified in this comparison.

Conclusion: truth interval knowledge can help, but the first working
breakpoint fit is not reliably better than GSF. Investigate this large light-
loss regression before interpreting win counts as improvement or expanding
to a production claim.

## Artifacts

TrackingPerformanceStudies/recbreakpoint_more_events_2026-09-08/ contains:

- truth_interval_audit_topology.json and its successful log;
- breakpoint_seed12_event*.root/.log;
- gsf_reverse_seed12.root/.log;
- comparison.csv (full absolute pT and residuals for every method);
- comparison_summary.log (pairing, status, topology and regression checks);
- run_comparison.py, run_gsf_comparison.py and summarize.py.

The reusable truth-audit script remains under
TrackingPerformanceStudies/recbreakpoint_first_working_2026-09-08/.
All generated outputs and analysis/run scripts remain uncommitted. Only this
durable knowledge record is checkpointed; no remote operations were performed.
