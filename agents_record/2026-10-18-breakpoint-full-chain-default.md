# Breakpoint full-chain submission default

The user requested extending subbreakpointjobs.sh to the complete simulation
through breakpoint chain. The existing batch helper and worker already support
all five stages, so no additional execution path was introduced.

- subbreakpointjobs.sh now defaults STAGES to
  sim,trk,calodigi,rec,breakpoint, replacing breakpoint-only.
- Explicit STAGES overrides remain supported: existing SIM inputs can use
  trk,calodigi,rec,breakpoint; existing REC inputs can use breakpoint.
- The shared DumpGsfTrks/sim.py.bk and downstream templates are frozen when
  the campaign is prepared, not when workers start.
- Existing overwrite guards and cleanup remain unchanged. SIM and REC are
  retained; owned TRK/calodigi files are removed only after downstream
  validation. Input and output directories must still differ, even when the
  full chain does not consume an external input.
- No fitter, simulation physics setting, shared card, or installed binary was
  changed. Existing prepared manifests are unaffected.

Validation: bash syntax checks passed; all 37 test_batch.py tests passed.
The added shell-wrapper dry-run test prepares the complete chain with an absent
input directory, checks every predecessor filename, and verifies the explicit
REC-only override. Worker ordering, cleanup and failure-retention tests remain
passing. No Gaudi physics jobs or Condor jobs were run for this change.

Use a fresh output directory, or new seed indices with no existing output/card
collision. DRY_RUN=1 prepares cards without submission; submit those frozen
cards afterward with ./subbreakpointjobs.sh submit OUTPUT_DIRECTORY.
