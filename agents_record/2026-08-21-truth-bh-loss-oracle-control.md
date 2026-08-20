# Truth BH-loss oracle control

Date: 2026-08-21

## Purpose and boundary

This change adds a default-off, truth-dependent mechanism control requested to
separate the Bethe-Heitler (BH) response from the downstream GSF workflow. It
is not a reconstruction candidate and must never be production steering.

At each BH call that is already eligible under the ordinary material-path,
electron-hypothesis, and `BHSplitThreshold` logic, the control replaces the
configured BH model's complete returned mixture with one deterministic child:

```text
conditional weight = 1
retained mean z     = 1 - truth_ebrem_loss_GeV / truth_p_before_GeV
retained variance   = 1e-12
```

The input is Geant4 eBrem-attributed energy loss, not total pre/post momentum
loss. `ElossOn` independently handles the baseline deterministic energy-loss
treatment, so using total loss here would overlap other processes. After this
single process hypothesis is installed, the ordinary continuation,
measurement update, reduction, reverse filter, and publication paths run
unchanged.

The control does not force a split below `BHSplitThreshold`. It therefore
tests the BH returned response at existing calls, not material gating. A truth
loss located in a below-threshold runtime interval remains uninjected and is a
gating/granularity finding rather than an oracle response failure.

## Configuration contract

Two Gaudi properties were added:

- `TruthBHLossOverride=false`
- `TruthBHLossInput=""`

Enabling the control requires `ElectronHypothesis=true`,
`MaterialPathMode=DD4hepBetweenSurfaces`, and a readable nonempty input. The
CSV header is exact:

```text
event_index,input_track_index,hit_from_index,hit_to_index,cell_from,cell_to,truth_p_before_GeV,truth_ebrem_loss_GeV
```

CSV presence selects a complete `(event_index,input_track_index)` scope. Once
any row selects a track, every consecutive accepted-hit interval must be
present with `hit_to_index=hit_from_index+1` and the exact bounding cell IDs.
This includes hit 0 to hit 1: the seed already contains filtered hit 0, so the
special seed-material call owns that ordinary first interval and the normal
forward loop begins at hit 1. A malformed, duplicate, nonphysical, or missing
selected-track row fails instead of falling back.

A track with zero CSV rows is explicitly outside the oracle scope. It is
logged and counted, and uses the configured BH model for its whole workflow.
This all-or-nothing track boundary is needed for selected events such as event
16, which contains a second reconstructed track while the authoritative
surface-interval truth recorder covers the primary electron. No secondary
truth value is invented and there is no partial truth/BH hybrid within a
selected track.

The maintained `DumpGsfTrks/gsf.py.bk` explicitly sets the two properties to
off/empty. The authoritative package README now covers 43 Gaudi properties,
and a dedicated sub-agent verified that all 43 are documented and assigned
exactly once in the maintained card. `DumpGsfTrks/README.md` records the same
count and default-off boundary.

## Implementation and audit

`BetheHeitlerSplitter::splitWithRetainedFraction` reuses the exact normal
mixture-application machinery with the one-component truth mixture. The
ordinary `split` path was factored through that same helper without changing
its model selection or process arithmetic.

`GsfAlgorithm` parses and validates the truth map during initialization,
prechecks all exact intervals before allocating the selected track's GSF
mixture, and dispatches the truth response at all three process locations:

1. seed material, hit 0 to hit 1;
2. forward material, hit `ih` to `ih+1`;
3. reverse material, canonical hit `reverseHit` to `reverseHit+1`.

`MaterialBHAuditCSV` gained `truth_bh_loss_override` and
`truth_retained_fraction`. Candidate rows expose the per-track scope and input
fraction; executed oracle child rows expose conditional weight one, the same
mean, and variance `1e-12`. Passthrough tracks retain flag zero and ordinary BH
child values.

## Focused validation

The EL9/LCG-105 `RecGsfTracking` target built and installed successfully.
Same-code default and oracle reruns used `rec-e--2.0-85-1.root`, reverse
filtering, the frozen production settings, verbose component dumps, and event
indices 11, 13, 16, 17, and 41. The oracle map contained 1,150 exact primary
track intervals. Runtime endpoints matched the truth sensitive-midpoint
anchors monotonically; the maximum nearest-endpoint distances were 1.57--1.97
mm, consistent with the established truth-midpoint versus digitized-hit
boundary.

The successful oracle audit contained 3,406 rows. It recorded 80 oracle child
calls across seed, forward, and reverse processing and one explicitly logged
whole-track passthrough (event 16 input track 1). Every oracle child had
conditional weight one, retained variance `1e-12`, and a retained mean equal
to the input fraction. The application finalized successfully. Removing the
event-11 hit-0-to-hit-1 row caused the intended explicit missing-key failure
and nonzero job status.

Focused primary-track interaction-point pT results were:

| event index | truth pT | default GSF pT | truth-oracle pT | mechanism note |
|---:|---:|---:|---:|---|
| 11 | 40.7316 | 40.9351 | 40.9139 | only tiny truth eBrem loss; small change |
| 13 | 25.5197 | 24.4866 | 21.9509 | 6.2999 GeV truth loss is at hit 39--40, but runtime `pathTX0=5.8e-5`, below threshold; all executed oracle calls were identity |
| 16 | 37.8940 | 18.3188 | 18.3188 | 23.8095 GeV truth loss is at hit 5--6, but runtime `pathTX0=5.2e-5`, below threshold; primary oracle cannot inject it |
| 17 | 18.7970 | 18.6248 | 18.7251 | two truth-loss intervals were above threshold; residual improved from -0.916% to -0.383% |
| 41 | 12.4253 | 22.2932 | 12.3851 | no truth eBrem; forcing identity at executed calls removed the default false overshoot |

This is mechanism evidence only. It shows that exact response substitution can
both suppress a false radiative branch and improve a genuine radiative case,
while also exposing important truth-loss intervals that never call the BH
splitter because their collapsed runtime `pathTX0` is below the production
threshold. It is not an unbiased rate, clean-core validation, or permission to
tune the threshold. The next branch-local work must distinguish why those
low-`pathTX0` intervals contain large Geant4 eBrem losses before proposing a
material, granularity, threshold, or response change.

Generated ROOT files, CSV maps/audits, and logs were kept under `/tmp` and are
not project status artifacts.
