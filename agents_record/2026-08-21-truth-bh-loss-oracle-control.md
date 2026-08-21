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

## Sixty-event topology-clear stress/control A/B

The exact fixed 60-event panel previously used for the three-model BH response
comparison was reproduced with current code. It contains the 20 largest
positive default residuals, 20 most negative default residuals, and 20
smallest-absolute-residual remaining controls from
`rec-e--2.0-85-1.root`. All are topology clear under the established
classification. The selection is deliberately based on the default result;
it is a stress/control panel, not an unbiased or held-out population.

Fresh default and oracle runs used identical frozen production steering and
enabled the same runtime BH audit. The current default reproduced the earlier
60 stored GSF pT values exactly: the maximum absolute drift was zero. The
truth map supplied 13,898 exact primary-track accepted-hit intervals. All
endpoint matches were monotonic; maximum truth-sensitive-midpoint to runtime-
hit distances ranged up to 2.96 mm. Both applications finalized successfully
and all 60 selected primary tracks fitted. Events 35 and 65 each contained an
additional reconstructed input track outside the authoritative primary truth
scope; those two tracks were explicitly logged and used ordinary BH for their
whole workflow.

Absolute pT residual metrics were:

| Group | Mode | Mean | Median | 68% quantile | Maximum | `>1%` | `>3%` |
|---|---|---:|---:|---:|---:|---:|---:|
| Overshoot | default | 5.365% | 0.379% | 0.567% | 79.418% | 5 | 4 |
| Overshoot | truth oracle | 0.321% | 0.224% | 0.321% | 1.354% | 1 | 0 |
| Underestimate | default | 8.538% | 0.926% | 1.661% | 92.974% | 9 | 4 |
| Underestimate | truth oracle | 5.744% | 0.392% | 0.462% | 92.974% | 4 | 2 |
| Good control | default | 0.0330% | 0.0316% | 0.0404% | 0.0744% | 0 | 0 |
| Good control | truth oracle | 0.0912% | 0.0613% | 0.0832% | 0.3429% | 0 | 0 |
| All 60 | default | 4.646% | 0.320% | 0.580% | 92.974% | 14 | 8 |
| All 60 | truth oracle | 2.052% | 0.184% | 0.317% | 92.974% | 5 | 2 |

Excluding the dominant no-eBrem false overshoot at event 41, all-panel mean
absolute residual still improved from 3.378% to 2.081%. Eventwise, 34 tracks
improved, 17 worsened, and nine were unchanged. The group counts were 16/1/3
improved/worsened/unchanged for overshoots, 16/2/2 for underestimates, and
2/14/4 for good controls. The oracle therefore provides strong selected bad-
event evidence while also exposing a real small clean-control broadening.

The largest absolute-residual gains were:

| Event | Group | Default residual | Oracle residual | Absolute gain |
|---:|---|---:|---:|---:|
| 41 | overshoot | +79.418% | -0.323% | 79.095 points |
| 57 | underestimate | -36.463% | -1.695% | 34.769 points |
| 33 | underestimate | -21.618% | -0.953% | 20.664 points |
| 27 | overshoot | +11.958% | +0.106% | 11.852 points |
| 19 | overshoot | +5.424% | -0.580% | 4.843 points |
| 96 | overshoot | +4.018% | -0.259% | 3.759 points |

The main counterexample was event 13: its 6.2999 GeV truth loss belongs to one
valid interval with `pathTX0` below `1e-4`, so no nonidentity truth response
was injected. Replacing all other executed calls by their truth identity
response worsened the residual from -4.048% to -13.984%. Event 35 also had no
reachable material loss and remained at -92.974%. These are not failures of
the supplied retained fraction; they demonstrate that a response-only oracle
cannot repair losses that never reach the splitter or failures downstream of
the response.

The audit contained 1,156 oracle child calls over the 60 selected primaries.
Eighty-six were nonidentity calls: the same 43 nonzero-loss intervals were
used once in the outward and once in the reverse workflow. Across the exact
primary truth map:

| Truth-loss reachability | Intervals | Events | Summed eBrem loss |
|---|---:|---:|---:|
| Reached an executed BH call | 43 | 35 | 75.249 GeV |
| Valid but below `BHSplitThreshold` | 11 | 10 | 8.583 GeV |

No nonzero-loss interval in this selected panel was classified as an invalid
material path. Thirty-five events had at least one injected loss; their mean
absolute residual improved from 2.783% to 0.330%, and their >3% count fell
from five to zero. Five events contained loss only in unreached intervals;
their mean worsened from 19.509% to 21.480%, driven by event 13 while event 35
remained unchanged. Twenty events had no truth eBrem. Their aggregate mean
improved because of event 41; excluding it, mean changed from 0.230% to
0.204%, while the median broadened from 0.058% to 0.136%. This mixed clean
behavior prevents a production-safety claim.

The result supports a precise conclusion: when a genuine loss is presented at
an executed BH call, the ordinary downstream GSF usually preserves enough of
the exact response to improve the selected bad tracks. The remaining failure
classes are below-threshold/gating cases, non-BH failures, and smaller clean-
track changes. The result does not establish that the current material
threshold, ordinary BH prior, or final selection should be changed, and it is
not held-out validation.

Generated ROOT files, CSV maps/audits, and logs were kept under `/tmp` and are
not project status artifacts.
