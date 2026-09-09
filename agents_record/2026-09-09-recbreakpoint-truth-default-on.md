# RecBreakpoint truth-override default on

The user requested TruthOverride=True by default and clarification of its
interaction with interval selection. This supersedes the earlier default-off
parallel-oracle setting; it does not change the GSF truth override.

## Contract

- Compiled RecBreakpoint.TruthOverride is true; the dedicated local card uses
  BP_TRUTH_OVERRIDE default 1. Explicit 0 retains copied ordinary endpoints.
- IntervalSelectionMode chooses WHERE: Truth selects accepted-hit intervals
  with positive associated Geant4 eBrem loss; Manual uses the supplied list.
  Auto remains reserved and fails initialization.
- Both endpoint pairs use the same effective interval list. Ordinary RTS and
  backward fits use the configured loss priors. The additional oracle pair
  fixes each selected loss to its associated Geant4 value, with zero added
  loss variance and one pass. It does not modify the ordinary fits.
- Empty effective intervals produce copies (truth_override_result_status=1),
  even with True. Successful nonempty oracle fits have result status 2.
  Existing invalid-truth error handling is unchanged.
- Already prepared batch cards are frozen copies. Prepare new cards in a new
  output directory to adopt the changed card default.

## Verification

Rebuilt and installed RecBreakpoint successfully. Generated Gaudi configuration
reports TruthOverride=True. Build/install library SHA256 agrees:
afec47c8b89afeecd5bb9b35b9139982653d31c2eb00a343db27d228c45421b3.

Focused jobs clear ambient BP_* controls and exercise the actual card defaults,
with verbose state/covariance dumps and VerifyKFReference=True. Tests are under
TrackingPerformanceStudies/breakpoint_truth_default_on_20260909 (uncommitted).
Every flat branch, including vectors, exactly matches preceding explicit-on
references; seed2:68 also has a same-build explicit-on rerun.

| Seed:entry | Selected intervals | Result status | Ordinary RTS pT | Oracle RTS pT |
|---|---|---|---|---|
| 2:68 | 7 | 2 | 44.97146297895819 | 44.676959069487786 |
| 12:11 | 10 | 2 | 9.125044417245304 | 9.139602411882846 |
| 12:16 | empty | 1 | 38.250515169205485 | 38.250515169205485 |
| 12:17 | 2 | 2 | 17.466338203037605 | 31.713790395231793 |

Momentum units are GeV. Seed12:17 remains a secondary-activity control, not a
clean-track performance case. These tests verify default steering and exact
regression only, not population physics performance. All 13 batch helper unit
tests pass; no Condor submission was performed for this default change.

GSF/shared sources and user-owned shell/card edits are untouched. No remote
operations. The dedicated local breakpoint card remains untracked under the
run-card policy. Outgoing project status is preserved in
2026-09-09-agents-before-breakpoint-truth-default-on.md.
