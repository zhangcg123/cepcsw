# In-process truth BH-loss G4StepTuple reader

Date: 2026-08-21

## Purpose and boundary

The default-off truth BH-loss oracle originally required a separately joined
CSV containing exact runtime accepted-hit intervals. That preprocessing was
adequate for focused diagnostics but was not a safe batch interface: every GSF
job needed an external join step, and a stale CSV could be paired with the
wrong reconstructed sample.

`RecGsfTracking` now supports a second oracle input source,
`TruthBHLossSource="G4StepTuple"`. It reads the material recorder ROOT tuple
inside the algorithm, derives the same interval retained fractions, and feeds
them into the existing oracle call sites. `TruthBHLossOverride` remains false
by default. This is mechanism-diagnostic plumbing only; it is neither
production truth steering nor a proposed BH model.

Implementation remains inside `Reconstruction/RecGsfTracking`. The reader is
in `src/TruthBHLossTupleReader.{h,cpp}` and is linked into the existing plugin.
No shared tracking package was changed.

## Interface

Three properties extend the previous 43-property surface to 46:

- `TruthBHLossSource`, compiled default `CSV`, accepts `CSV` or
  `G4StepTuple`;
- `TruthBHLossInputTrackIndex`, compiled default `0`, selects the zero-based
  `CompleteTracks` input receiving the unique primary-electron truth in tuple
  mode;
- `TruthBHLossMaxEndpointDistance`, compiled default `5.0` mm, is the strict
  hit-to-truth-anchor distance guard in tuple mode.

`TruthBHLossInput` is reused as either the strict CSV path or the material
recorder ROOT path. The original CSV contract and whole-track all-or-nothing
coverage behavior remain unchanged.

The dedicated option audit confirmed all 46 properties have an authoritative
row in `Reconstruction/RecGsfTracking/README.md` and an explicit assignment in
`DumpGsfTrks/gsf.py.bk`. The maintained card intentionally differs from the
compiled/reverse-template oracle input while the override remains off: it
preconfigures `G4StepTuple`, the per-job `gsf_material_steps.root` placeholder,
track index 0, and the 5 mm guard. `dump_gsftrk.sh` substitutes the same
sample-qualified material filename used by simulation.

## Tuple matching contract

The reader opens `g4step_tuple`, checks every required pre/post-step branch,
and builds a ROOT index on `event_id`. It loads only the current event. Each
processed event must contain exactly one primary electron or positron Geant4
track. Steps are ordered by Geant4 track-step number and recorder index.

Sensitive midpoint anchors use the material recorder convention, including
collapsing the TPC lower/upper sensitive half-volume pair into one pad-row
anchor. Every radius-ordered accepted runtime hit is nearest-matched to an
anchor. The match must be strictly increasing and every distance must be no
larger than the configured guard. Geant4 process subtype 3 (`eBrem`) losses are
summed over every truth interval spanned by a runtime hit pair. The resulting
retained fraction is keyed by the existing exact runtime interval key and is
consumed by the unchanged seed, forward, and reverse oracle sites.

Other input tracks use the configured BH model. A missing event, malformed
branch vector, ambiguous/missing primary electron, nonphysical loss,
nonmonotonic mapping, incomplete runtime interval, or excessive endpoint
distance fails the job instead of silently mixing truth and model responses.
The material tuple and reconstructed input must therefore come from the same
simulation event stream.

## Mechanical validation

The plugin built and installed successfully with the EL9/LCG 105 focused
commands.

A deliberate mismatched-input test paired the current 10--50 GeV reconstructed
sample with an archived fixed-2 GeV material tuple. It failed the endpoint
guard with a maximum mismatch of about 1690.69 mm. This establishes the guard,
not physics correctness.

Seed 1 was then deterministically replayed through event 17 with the current
simulation/material recorder. Event 11 matched 233 runtime intervals with a
maximum endpoint distance of 1.72256 mm. Events 11, 16, and 17 all completed
the required verbose hard-event gate; the maximum distance across the three
selected tracks was 1.93021 mm. Forty-two executed BH responses used the
oracle. Event 16's second reconstructed input track remained an explicit
ordinary-BH passthrough.

The new tuple-source audits were compared by full record identity against the
previous CSV-source audits:

| Event | Audit records | Maximum retained-fraction difference | New tuple-source GSF pT |
|---:|---:|---:|---:|
| 11 | 488 | `1.04e-11` | `40.913856 GeV` |
| 16 | 1,474 | `0` | `18.318808 GeV` |
| 17 | 480 | `1.42e-9` | `18.725061 GeV` |

All audit keys and truth-override flags matched. The three output pT values are
identical to the earlier CSV-oracle outputs at stored tuple precision. The tiny
fraction differences come from the float-valued replayed Geant4 tuple and are
far below the configured or algorithmic resolution of this diagnostic.

A separate same-code event-11 run left the override off and selected the
compiled `CSV`/empty-input defaults. It reproduced the stored ordinary GSF
output exactly at tuple precision, `40.935105 GeV`, confirming that constructing
and linking the reader does not alter default-off behavior.

This is source-equivalence and batch-mechanism validation. It does not extend
the prior selected oracle evidence into a held-out population result.

## Batch steering and outputs

`dump_gsftrk.sh` retains its six legacy arguments and accepts an optional
seventh truth-oracle boolean. False/omitted preserves ordinary behavior. True
requires the paired nonempty `gsf_material_steps-<sample>.root`, sets
`TruthBHLossOverride=True` in the generated card, and leaves the strict C++
reader responsible for eventwise closure. The generated truth card and its
GSF EDM, flat tuple, material/BH audit, and batch logs use a `truth-bh` suffix
so the ordinary A/B member is not overwritten.

`subtrkjobs.sh` passes the control through; the truth diagnostic can be
submitted with:

```bash
TRUTH_BH_OVERRIDE=true bash subtrkjobs.sh
```

The current worker executes only its final GSF stage. It does not synthesize a
missing truth tuple. The matching simulation/material-recorder job must run
first, and the separate material ROOT file must be retained. The endpoint
guard catches gross mismatches but is not a substitute for preserving a clear
per-seed provenance contract.

## Next physics step

Keep this diagnostic default-off and return to the existing ordered material/
BH investigation. Use the tuple source for future response-only population
A/B jobs, classify below-threshold truth losses separately, and do not infer a
BH default change from successful oracle closure.
