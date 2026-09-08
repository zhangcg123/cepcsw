# Matched-control no-breakpoint KF comparison

User requested the same KF setup to check reproduction of CompleteTracks.
An isolated wrapper ran seed12 events 0,3,6,11,15,16,17, all with
BreakpointIntervals=[], FirstMiddleLast, SeedScale=1, MSOn=true,
ElossOn=true and MaxChi2PerHit=200. This matches the inspected stored tracker
card's principal material/cut controls; it does NOT reproduce the full
CompleteTracks publication workflow. Sources and maintained cards were unchanged.
All seven fit rows succeeded, with verbose state/covariance dumps and an
independent native-KF reference. Events 0,6,15,16 are no-eBrem controls;
event17 is a secondary-activity control.

| Event | CompleteTracks pT | Native KF pT | No-breakpoint pT | Relative BP/Complete difference (%) |
|---|---:|---:|---:|---:|
| 0 | 9.288425568 | 9.287065437 | 9.287066135 | -0.014636 |
| 3 | 41.929211405 | 42.001194890 | 42.000634738 | +0.170343 |
| 6 | 21.651941801 | 21.653887601 | 21.653883808 | +0.008969 |
| 11 | 9.097590152 | 9.097261402 | 9.097258723 | -0.003643 |
| 15 | 18.499556413 | 18.498311980 | 18.498320285 | -0.006682 |
| 16 | 38.261774981 | 38.244074247 | 38.244074247 | -0.046262 |
| 17 | 18.220514852 | 18.220610198 | 18.220608855 | +0.000516 |

Units are GeV. Last column is 100*(BP/CompleteTracks-1), NOT truth residual.
No-breakpoint and native KF differ by at most 4.49e-5 percent for the four
no-eBrem controls; event16 agrees at the stored double precision. Event3's
BP/native difference is larger, -0.001334 percent; numerical closure is not
uniform across all events.

## Structural mismatch discovered

RecTrkGlobal/src/FitterTool/KalTestTool.cpp finaliseLCIOTrack explicitly smooths
to lastHit, obtains the last-hit state, initializes a temporary track, revisits
all remaining accepted hits inward using addAndFit, then calls propagate to IP
(approximately lines 337--420). It does not publish the same endpoint as
the breakpoint RTS smoother. RecBreakpoint's native reference uses smooth()
then extrapolate from the first hit; its own endpoint uses RTS and a geometric
MoveTo(IP). Material-aware propagate versus geometric extrapolation is also
a publication difference when deterministic loss is enabled.

Thus the earlier explanation focusing on ElossOn and MaxChi2PerHit was
incomplete. Matching these controls is insufficient to establish exact
CompleteTracks reproduction. This test confirms close agreement with the
matching native outward-KF/smoother reference, not with the full standard
producer. Quantifying each publication effect separately remains pending;
do not attribute the whole remaining shift to a single cause from this test.

Artifacts and isolated card:
TrackingPerformanceStudies/recbreakpoint_matched_kf_2026-09-08/
Input: gsf_doublebhoff_freshseed_diagnostic/trk-e--2.0-85-12.root.
