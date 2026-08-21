# Truth BH invalid-scope fallback and validity tag

Date: 2026-08-22

## Decision

The default-off truth BH-loss oracle keeps its strict all-or-nothing mapping
validation, but a per-event or per-track validation failure no longer returns
`StatusCode::FAILURE`. The selected track instead runs from the beginning with
the configured ordinary BH model. No accepted truth interval is mixed with an
ordinary BH response within that track.

Source-open and configuration errors remain fatal at initialization: an
enabled oracle still requires a readable nonempty input, a valid source mode,
the exact CSV schema when CSV is selected, `ElectronHypothesis=true`, and
`MaterialPathMode=DD4hepBetweenSurfaces`.

This changes only the default-off truth-oracle diagnostic. It does not change
the production BH model, material mode, split/cutoff values, or reverse
workflow.

## Status contract

`RecGsfTracking` writes a `podio::UserDataCollection<int32_t>` named
`GSFTruthBHLossStatus`. Its indices correspond to the input
`CompleteTracks` indices:

| Code | Meaning |
|---:|---|
| `1` | Complete truth scope validated before filtering. |
| `2` | Track not selected by the configured truth source. |
| `0` | Truth override disabled. |
| `-1` | Invalid or unavailable event-level G4StepTuple truth. |
| `-2` | Runtime-hit/truth-anchor endpoint-distance guard failed. |
| `-3` | Exact consecutive runtime interval mapping failed. |
| `-4` | Selected track/event was not processed. |

`RecGsfFlatTuple` copies the status for `CompleteTracks` index 0 to
`truth_bh_scope_status` and writes `truth_bh_scope_valid=1` only for code 1.
Older producers without the status collection receive `0,0`.

`MaterialBHAuditCSV` adds `truth_bh_scope_status`,
`truth_bh_scope_valid`, and `truth_bh_scope_reason`. Invalid scopes continue to
record ordinary material candidates and BH children, so the rejected truth
reason and fallback mechanics can be audited together. The pre-existing
`truth_bh_loss_override` field now unambiguously means that the particular
track passed validation and its eligible calls use truth.

## Implementation boundary

The complete tuple truth interval map is built in a temporary map and is
committed only after every accepted-hit interval passes. CSV-selected tracks
receive the same pre-filter exact-interval completeness check. A failure flips
the whole track to ordinary BH before seed, forward, or reverse splitting can
start.

The status collection and shared enum live only in
`Reconstruction/RecGsfTracking`. No shared tracking package or external
workflow component was changed. No new Gaudi property was added; the 45
property inventory and maintained-card explicit-property contract are
unchanged.

## Focused verification

The EL9/LCG 105 targets `RecGsfTracking` and `RecGsfFlatTuple` built and
installed successfully.

Temporary cards and all generated outputs were kept under `/tmp`.

- Seed 7, event 0 reproduced the former event-level failure
  `nonphysical Geant4 eBrem interval loss`. The job completed one fitted track,
  applied zero truth responses, wrote the EDM, flat tuple, and audit, and
  recorded flat status `-1,0`. The audit recorded the same reason and ordinary
  BH calls. A same-code override-off rerun recorded status `0,0` and the exact
  same GSF pT (`1.8987577279954331 GeV`, bit-for-bit), closing the fallback
  against the configured-BH control.
- Seed 5, event 53 reproduced the former endpoint failure at `7.48037 mm`
  against the `5 mm` guard. A comprehensive verbose component dump completed,
  the track was fitted with ordinary BH, and flat entry 53 recorded `-2,0`.
  Focus-skipped entries recorded `-4,0`. The audit retained the exact endpoint
  reason on its ordinary-BH rows.
- Seed 1, event 0 was a positive control. It recorded `1,1`, matched with a
  maximum endpoint distance of `1.9101 mm`, and replaced all 20 executed BH
  responses. Its new GSF pT was bit-identical to the stored strict-oracle
  result (`35.609061251515087 GeV`) made with the same maintained-card
  steering. This is a mechanism regression, not a momentum-performance claim.
- The output EDM persisted `GSFTruthBHLossStatus`; the flat tuple persisted the
  two new scalar branches; the audit header and rows persisted all three new
  status fields.

The deleted canonical hard-event 11/16/17 input remains unavailable, as
already recorded in the project status. No population-performance claim or
default change follows from these plumbing checks.

## Operational consequence

New truth-oracle batches should finish through isolated invalid truth mappings
instead of producing different file lengths. Analysis must select
`truth_bh_scope_valid == 1` for a pure truth-oracle population. Rows with a
negative status are valid ordinary-GSF fallback outputs and should be reported
separately rather than silently included as truth-oracle results.
