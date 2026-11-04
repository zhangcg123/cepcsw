# Shared default breakpoint steering

The user explicitly requested the same five defaults for the standard card
and every batch helper. Align the compiled package as well, rather than leave
a contradictory configurable default:

```python
FreeLossFit = True
BeamSpotConstraint = True
ChargedEcalMeasurement = True
NeutralEcalMeasurement = False
LocationScanSetups = [110]
```

All enabled truth-location free-loss families remain preserved; only110 scans
by default. No fitter, loss map, material ownership, objective, optimizer bound
or acceptance, PFA/shared KF, collection or tuple schema change. Other options
(including each context's existing TruthOverride, diffuse and charged-energy
mode steering) are deliberately unchanged.

## Reach and compatibility

Changed RecBreakpoint.h defaults, run_breakpoint.py fallbacks, and both root
helpers. batch_breakpoint.py explicitly freezes all five defaults even when
none of their environment controls were supplied. Both shell wrappers preserve
explicit overrides, including an empty BP_LOCATION_SCAN_SETUPS. New tests
compare compiled property declarations, unoverridden card evaluation, direct
batch planning and both wrappers, including explicit all-off/no-scan overrides.
Existing ordinary/legacy-mode tests now explicitly disable dependent scans.

LocationScanSetups=[] is still the opt-out. Nonempty scans require Truth and
FreeLossFit plus their prerequisites. Therefore Manual, HitScan/HitScanAll or
FreeLossFit=False must explicitly clear the list. This remains a fail-fast
dependency check, not a silent downgrade. Existing frozen cards and outputs
are not rewritten. Cards predating the new property may require regeneration
or explicit empty steering for legacy modes under the new compiled default.
The dedicated Higgs launcher now defaults to110, not0/100/110. Input event
counts, output path, stage chain, group and job sizing were not changed.

## Verification/deployment evidence

Evidence directory: /tmp/bp-unified-defaults-po03l2.
62 Python batch tests passed;15 CTests passed (6.34s).
Focused verbose runs use barrel seed1 events11/16/17, leaving all five new
controls unset. The driver asserts the effective card values AND compiled
getDefaultProperties match the requested profile. Auxiliary controls are kept
equal to the previous explicit110 reference for a meaningful direct comparison.
No ROOT outputs are written outside /tmp by these checks. All947 branches per
event exactly reproduce the previous explicit110 configuration (2,841
branch-row values, including full vectors, scores and normal/scan endpoints).
The additional scans contain only110, with10/11/11 entries respectively.

Previous module SHA256:
cb2300011261d42eb5b4b5cc299bca4aa3c3f1f3113e40b5220546e2334992e4.
New built/staged/installed module SHA256:
a1c71b9529f7cf3ace728d93beaa8e5d0c7d9e61235085513bc5def87d15b18e.
The user confirmed the quiet window/installation after validation during this
same controller development. Only the RecBreakpoint package is installed after
verification; no full-project or shared-package installation, batch submission,
new production-card preparation, or remote/branch operation.
The project-default direct gaudirun probe passed initialization/finalization,
checking all five compiled and effective defaults. It used a one-event dry-run
planning card with those controls omitted from the environment, then overrode
EvtMax to0 and output to /tmp. The card explicitly freezes all five requested
defaults, retains its checksum, and has no submitted/started marker. Output
schema is booked with zero rows as intended; no event fit is claimed from it.

Standard card, property table and free-loss documentation are synchronized.
The outgoing AGENTS is fully preserved in
2026-11-04-agents-before-unified-breakpoint-defaults.md; introduction, active
laws and essential commands are unchanged. Only the same current focus's
default values and compatibility note are updated. No history files moved or
deleted; no history-move manifest required. Unrelated dirty batch/workflow
changes remain unstaged. Generated fixtures/logs/ROOTs remain outputs.

These changes do not resolve the10-hour batch runtime limit. Default scans add
work to formerly scan-disabled ordinary jobs; job sizing needs a separate
decision before a large resubmission. No physics-performance claim is made.
