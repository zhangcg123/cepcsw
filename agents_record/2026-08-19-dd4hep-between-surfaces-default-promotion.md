# DD4hepBetweenSurfaces default promotion

Date: 2026-08-19

## Decision and outgoing status

Through commit `f7969f7`, the compiled and committed production default was
`MaterialPathMode=CurrentSurface`. `DD4hepBetweenSurfaces` remained an explicit
control because its earlier performance was worse and its target endpoint
could be rejected by a redundant bounded-surface intersection. The complete
pre-promotion evidence and caution are preserved in:

- `2026-08-18-runtime-material-path-and-bh-input-consistency.md`;
- `2026-08-18-material-path-mode-direction-symmetry.md`;
- `2026-08-19-dd4hep-matched-hit-material-endpoint-and-ownership-validation.md`.

After the direction-dispatch and matched-hit endpoint defects were corrected,
the user explicitly selected `DD4hepBetweenSurfaces` as the project default.
This is a steering decision. It does not erase the earlier performance result
or establish that one collapsed surface-to-surface BH convolution reproduces
the corresponding Geant4 energy-loss distribution.

## Synchronized option surface

The same change updates:

- the compiled `MaterialPathMode` default in `GsfAlgorithm.h`;
- the no-environment-override fallback in
  `options/run_gsf_reverse_template.py`;
- the explicit maintained production steering in `DumpGsfTrks/gsf.py.bk`;
- the complete property reference in `Reconstruction/RecGsfTracking/README.md`;
- the historical-card interpretation in `DumpGsfTrks/README.md`;
- the live project baseline and current focus in `AGENTS.md`.

`CurrentSurface` remains an allowed, explicitly steered comparison mode.
Generated `DumpGsfTrks/rungsf-*` cards are historical batch artifacts and are
not rewritten; their explicit value records the configuration used to create
them.

No other property default changes with this decision. In particular, the
committed production contract remains `BHSplitThreshold=1e-4`,
`ComponentWeightCutoff=1e-4`, and `EcalComponentConstraint=false`. A dirty
working copy can steer experimental alternatives, but those are not silently
promoted with the material mode.

## Validation boundary

The promotion reuses the completed endpoint/ownership validation: zero invalid
displayed paths in the selected 30-event rerun and sub-per-mille closure to
representative Geant4-owned inner-VXD intervals.

The default-change checkpoint built and installed `RecGsfTracking`
successfully and syntax-checked both maintained Python cards. A focused seed-
107 entry-0 card removed the historical explicit material assignment, so the
algorithm had to use its compiled default. It finalized one fitted track with:

- 105/105 valid displayed forward paths;
- 2,146/2,146 valid displayed reverse paths;
- all 2,146 reverse lines labeled `mode=DD4hepBetweenSurfaces`;
- reverse output `pT=45.0768 GeV`.

An otherwise identical explicit `CurrentSurface` control also finalized. It
selected `mode=CurrentSurface` on all 1,452 displayed reverse paths and
retained the earlier bounded-crossing behavior: 48 reverse evaluations were
invalid in this event. The control is available, but is no longer the fallback.

This validates the default dispatch mechanically. The active physics task
remains the branch-local comparison of exact runtime path and BH response to
Geant4 loss at the first wrong lineage decision, followed by held-out
population checks.
