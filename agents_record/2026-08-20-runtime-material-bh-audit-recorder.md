# Runtime material/BH audit recorder

Date: 2026-08-20

## Purpose and boundary

`RecGsfTracking` now has a default-off `MaterialBHAuditCSV` property for the
branch-local material/BH consistency investigation. The output is deliberately
separate from `RecGsfFlatTuple`: a downstream tuple writer cannot reconstruct
the component state, exact material path, threshold decision, or returned BH
mixture after filtering. It is also separate from the legacy
`MaterialTransitionCSV`, whose forward, non-seed comparison schema remains
unchanged for existing analysis scripts.

The recorder is diagnostic only. Enabling it writes a generated CSV and does
not change the flat-tuple schema, the GSF output collection, a compiled physics
default, or a selection decision.

## Runtime interval authority

One candidate record corresponds to a path actually evaluated for a GSF
component between consecutive accepted/matched hits. The DD4hep seed path from
hit 0 to hit 1 is recorded and flagged separately. Forward records use the
inner-to-outer hit order. Reverse records retain the same canonical
inner-to-outer bounds while `direction=reverse` and `bh_reverse=1` state that
the material response was applied during inward propagation.

These reconstructed-hit bounds are authoritative for the runtime population.
The Geant4 sensitive-midpoint interval table is a truth-side reference that
must be spatially matched event by event; it must not be used to synthesize a
runtime interval.

## CSV contract

Every candidate has one stable `call_id` and a `record_kind=candidate` row.
The row groups are:

- event, input/output track, direction, seed flag, accepted-hit indices,
  runtime surface indices and cell IDs, and both endpoint coordinates;
- parent component/debug-parent identity, generation, weight, dominant-lineage
  fraction, no-radiation flag, kappa/momentum, and forward/reverse signatures;
- material path mode, validity, normal t/X0, incidence, path t/X0, segment
  count, and material composition;
- split threshold, above-threshold flag, electron hypothesis, BH model,
  execution flag, and reverse-response flag.

An executed BH call adds one `record_kind=child` row per returned component
under the same `call_id`. Child fields contain its index and debug identity,
actual child weight, conditional BH mixture weight, retained-momentum mean and
variance, and resulting kappa/momentum. A below-threshold or invalid candidate
has no child row. The complete schema has 54 columns.

The reverse template exposes the property as
`GSF_MATERIAL_BH_AUDIT_CSV`. The maintained historical card explicitly sets
`MaterialBHAuditCSV=""`; batch jobs must assign unique output paths when it is
enabled.

Later on 2026-08-20, the maintained campaign card was intentionally returned
to the production split/cutoff `1e-4` settings with ECAL off and assigned an
input-sample/method-specific audit filename under `tuplepath`. With the empty
tuple path used by `dump_gsftrk.sh`, that file is written under the project
root. The algorithm and reverse-template property defaults remain empty/off;
this is explicit campaign steering, not a default change.

## Mechanical validation

The EL9/LCG 105 `RecGsfTracking` target built and installed successfully. A
focused verbose production-baseline run used file/seed 27 entries 0, 11, 16,
and 17 with `DD4hepBetweenSurfaces`, split/cutoff `1e-4`, the five-component
`CEPC2GeV85StepConditioned` model, reverse filtering, aggregate selection, and
ECAL off.

The resulting audit contained 16,796 candidate calls: 7,754 forward (including
four seed paths) and 9,042 reverse. All were valid. There were 390 executed
forward calls and 479 executed reverse calls. Every one of the 869 calls
returned five child rows, for 4,345 child rows. All candidate/child groups had
consecutive accepted-hit bounds and internally consistent threshold and
direction flags. Conditional BH weights and child-to-parent weight sums closed
within `4.44e-16` and `3.33e-16`, respectively; all audited mixture and child
state values were finite.

An otherwise identical audit-disabled run completed successfully. All 79
branches and 18 entries in the paired flat tuples were bitwise equal at the
array-buffer level, including the four selected fitted events and the skipped
entries. This establishes mechanical non-interference for the focused sample,
not population physics validation.

## Next use

Run the recorder on an unbiased sample with an output path unique to every
job. First map its exact accepted-hit pairs to the truth-side interval table,
keeping seed/forward/reverse populations separate. Then use the same `call_id`
groups at the first wrong branch crossover to test path/ownership, collapsed
interval response, BH retained-energy closure, and only then downstream
measurement/selection effects.
