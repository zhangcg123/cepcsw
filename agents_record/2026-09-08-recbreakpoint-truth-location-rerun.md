# Breakpoint rerun at truth eBrem intervals — 2026-09-08

User requested checking truth eBrem locations and retrying the breakpoint fit
on the same seed-12 events. This diagnostic uses truth ONLY to select interval
indices. The prior mean, prior width and fitted loss are not replaced by truth.
No fitter/GSF/shared-KF source or maintained workflow card was changed.

## Exact matching

Input remains
`gsf_doublebhoff_freshseed_diagnostic/trk-e--2.0-85-12.root` from the preceding
first-working-version record. Event indices 11,16,17 are zero-based; the old
GSF flat tuple labels them with one-based iev 12,17,18 respectively.

The audit follows reconstructed-hit MCRecoTrackerAssociations (including
TPCTrackerHitAss) into SimTrackerHits, then GsfSimTrackerHitG4StepLinks and
GsfG4MaterialSteps. All 699 ordered hits have unique associations, complete
status=31 links and the same primary-electron G4 track ID=1 per event.
Their cell-ID order equals the breakpoint input order. All 696 intervals
agree with the stored passive GSF interval record in endpoint cell IDs,
hook step numbers, momentum-before and eBrem loss.

eBrem process subtype 3 is assigned at the step post-point using the existing
(start hook,end hook] ownership and 1e-6 endpoint tolerance. No nearest-distance
truth-hit matching or continuous spreading of a discrete eBrem loss is used.
All positive eBrem steps on the matched primary are also inspected for losses
outside the hit intervals; none were found for these three events.

| Event index | Selected interval | Detector/radii [mm] | G4 step | eBrem loss [GeV] | Interval loss fraction |
|---:|---|---|---:|---:|---:|
| 11 | h10 -> h11 | TPC, 642.5 -> 647.5 | 151 | 0.063866824 | 0.524946% |
| 16 | none | no recorded primary eBrem step | — | 0 | 0 |
| 17 | h2 -> h3 | VXD, 22.08142 -> 27.58086 | 18 | 19.597587585 | 42.547443% |

Event 11's post-step radius is 644.58924694 mm. Event 17's is
27.57493746 mm: only 0.00592254 mm in radial distance before h3. These are
radial separations, not claimed path lengths. Its loss is in a sensitive
volume near the outer interval endpoint, whereas this first breakpoint model
places the loss at the upstream source surface.

Event 16 is a no-eBrem control, not a hard-eBrem example. The earlier generic
11/16/17 smoke-test naming must not be interpreted as a verified hard-loss
category for every one of these tracks. No population topology audit was run.

## Same-code reruns

Code checkpoint 7d278e3. Fresh individual runs select [10], [], [2] for events
11,16,17 respectively. Settings unchanged: mean b=0, sigma b=.05, SeedScale=1,
MS=true, deterministic Eloss=false, no beam spot and no truth-loss override.
Verbose predicted/filtered/smoothed states and covariances are saved. All three
fits succeeded. Native KF reference was also rerun in each job.

| Event | Truth pT | Earlier GSF FullMixtureMode | Fresh no-breakpoint/native reference pT | Truth-location breakpoint pT | Breakpoint residual |
|---:|---:|---:|---:|---:|---:|
| 11 | 9.15107727 | 9.09424610 | 9.09556848 | 9.12546949 | -0.279833% |
| 16 | 38.36070251 | 38.25728290 | 38.25095017 | 38.25095312 | -0.286098% |
| 17 | 31.75560379 | 18.21669500 | 18.21669917 | 17.48163862 | -44.949437% |

pT is in GeV; residual = 100*(pT_reco-pT_truth)/pT_truth. Earlier GSF values
are the saved reverse, beam-off, depth-zero FullMixtureMode values from
2026-09-07-live-beam-boundary-gsf.md, NOT a fresh GSF rerun in this turn.
Their residuals are -0.621033%, -0.269598%, -42.634707% respectively.

| Event | Truth b | Smoothed fitted b | Sigma(b) | 100*(1-exp(-fitted b)) |
|---:|---:|---:|---:|---:|
| 11 | 0.005263291 | 0.003543713 | 0.005114938 | 0.353744% |
| 17 | 0.554210670 | 0.005795909 | 0.049295684 | 0.577915% |

The last column transforms the Gaussian posterior mean b; it is not the mean
of the transformed loss distribution. Event 17's posterior width remains close
to the prior .05, and its inferred loss remains near zero despite the correct
interval. Its true b is about 11 prior sigmas from zero. This, single
linearization and the source-surface approximation are limitations to examine,
not separately proven causes of the failure in this test.

## Conclusion and outputs

Correct interval selection helps the light-loss event 11 relative to its
no-breakpoint result and the stored GSF result. It does NOT solve event 17;
the hard-loss result worsens. Event 16 correctly keeps the no-breakpoint fit.
The previous arbitrary [5]/[5,7] tests did not select either actual eBrem
interval. These three tests establish neither population improvement nor
production interval-selection ability.

Under TrackingPerformanceStudies/recbreakpoint_first_working_2026-09-08/:

- audit_truth_intervals.py, truth_interval_audit.json and matching log retain
  the exact association/hook/step audit.
- seed12_event11_truth_location_h10.root and .log;
- seed12_event16_truth_location_none.root and .log;
- seed12_event17_truth_location_h2.root and .log.

The analysis script and generated outputs are uncommitted experiment artifacts.
The dedicated run card is unchanged and still locally available pending the
previous Git-law exception question for its build/card files.
