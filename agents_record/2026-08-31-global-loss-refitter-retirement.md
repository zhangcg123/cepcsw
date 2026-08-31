# Standalone global-loss refitter retirement

Date: 2026-08-31

The user retired the separate experimental `RecGsfGlobalLossRefitter` to keep
the active GSF package and maintained workflow focused on the ordinary
`RecGsfTracking` smoother and reverse methods.

Removed active implementation and configuration:

- the `RecGsfGlobalLossRefitter` Gaudi component and its
  `GlobalLossRefitter.h` declaration;
- the all-hit profile/refit implementation, fixed-loss helper, and
  `GlobalLossTracks` plus `GlobalLoss*` diagnostic outputs;
- `RecGsfFlatTuple.UseGlobalLossTracks` and the corresponding
  `GlobalLossTracks` input adapter;
- the `method="global-loss"` branch and its 14 explicit properties in
  `DumpGsfTrks/gsf.py.bk`;
- active package/workflow documentation presenting global loss as a supported
  third method.

The maintained card now accepts only `method="smoother"` and
`method="reverse"`. Generated cards that import
`RecGsfGlobalLossRefitter`, select `method="global-loss"`, or assign
`RecGsfFlatTuple.UseGlobalLossTracks` are stale artifacts and must not be run
with the current build. Existing ROOT outputs remain readable as historical
data, although the current flat-tuple producer no longer adapts
`GlobalLossTracks` into the generic `gsf_*` schema.

No historical evidence was deleted. The implementation contract, focused
tests, and observed limitations remain in:

- `agents_record/2026-08-22-global-one-loss-evidence-refitter.md`;
- `agents_record/2026-08-22-global-one-loss-maintained-card-integration.md`;
- `agents_record/2026-08-22-global-one-loss-expanded-event-gate.md`;
- `agents_record/2026-08-22-test-new-session-recovery-handoff.md`.

This retirement does not reject the general idea of using all remaining hits
to judge a radiative hypothesis. The active design discussion instead concerns
a bounded identity-backbone bank inside the reverse GSF: only the identity
history emits one-loss BH children, every child survives to hit 0 without
surface-local pruning or KL merging, and complete-history scores are compared
there. That proposed method is separate and not yet implemented.

Validation after retirement:

- `RecGsfTracking` and `RecGsfFlatTuple` built and installed successfully in
  the EL9/LCG-105 development build.
- The generated target component catalogs expose only `RecGsfTracking` and
  `RecGsfFlatTuple`; `RecGsfGlobalLossRefitter` is no longer constructible, and
  `RecGsfFlatTuple` no longer exposes `UseGlobalLossTracks`.
- The active reverse template fitted hard events 11, 16, and 17 successfully
  (four tracks total). Its 18-row flat tuple has the same 243 branches and zero
  exact branch-value mismatches against the pre-retirement tuple produced with
  the same reverse card/configuration from the immediately preceding installed
  source checkpoint.
- A comprehensive event-11 component dump completed with 10 final components,
  2,189 accepted updates, zero rejected updates, 11 splits, and 11 reductions.
