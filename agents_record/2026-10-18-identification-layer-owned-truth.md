# Identification migration to shared layer-owned truth

## Change

User switched focus from breakpoint recovery to eBrem identification and asked
its downstream applications to cooperate with the new layer-owned workflow.
RecBreakpointIdentification now calls TruthBHLossEventData::matchBreakpoints,
not a separate hook-endpoint assignment loop. No new truth mapping class.
It reuses TruthBreakpointTuple for the exact same per-emission records.

All eligible layer groups populate the N-1 candidate label arrays. Index i
means the loss at hit[i], as in the refitter; identification does not select
only the largest group. Both halves of a sensor share its owner. Physical
hook t/X0/bounds remain separate and unchanged. The obsolete assignment-status
branch is removed, interval=-1 means excluded (inspect ownership/eligibility),
and deprecated boundary_ambiguity is zero for compatibility with old analysis
metadata layouts. No KF feature or fit algorithm was changed.

Schema=3, truth_label_schema=3. truth_label_valid=1 requires successful untied
truth association and no ineligible emissions. This is deliberately conservative:
any unowned/shared/missing-hit/ambiguous/outermost/invalid loss excludes that
track from supervised training, even if other groups are known. It remains
saved for coverage analysis. This avoids silently labelling a missing loss
negative. No-loss tracks can be valid. Original topology/purity cuts still
apply downstream. This policy may exclude many tracks and is not an efficiency
claim on the full population.

The local analysis readers in TrackingPerformanceStudies/interval_bdt_20261011
now require schema3, count excluded rows, and use separate layer_owned_v1/
outputs. Updated: train_interval_bdt.py, neighborhood_bdt.py,
hierarchical_sequence_model.py, full_context_sequence_model.py,
track_first_pipeline.py, learning_curve.py. Cached manifests and the full-context
checkpoint carry the schema version. The base/sequence loaders reject stale
manifests; old caches/checkpoints/reports were not overwritten. Historical
report.json supplies ONLY the existing seed split to neighborhood extraction.
Full-context training no longer requires the obsolete CNN test-score assertion.
The historical scripts remain uncommitted analysis artifacts per project law.

The existing local batch helper now checks schema3 and ownership branches
before marking output complete. The maintained run card documents the policy;
no new control is needed. It uses the build-tree plugin, not the installed one.
Existing outputs/frozen jobs are NOT overwritten or resubmitted. No batch jobs
were submitted and no installation was performed.

## Checks

Temporary evidence: /tmp/bpid-layer-ZGuFGf/.
Input: sim_large_barrel_20261001/rec-barrel-1.root, events0--19, including
11/16/17. on.root uses rebuilt plugin with verbose diagnostics; off.root sets
BPID_TRUTH=0. before.root uses the pre-change identification object in the
source-matched temporary library from the preceding breakpoint validation.

- 20 events produce22 track rows; all forward/backward passes succeed.
- All non-truth branches exactly match both before.root and off.root.
- All22 rows' hits, emission steps, owner status/system/layer/key, breakpoint
  index/eligibility and loss match the prior RecBreakpoint final-50.root.
- Schema checker validates group sums/counts, index bounds, eligibility,
  covariance/innovation consistency and material coverage.
- 12 rows pass truth_label_valid;10 are retained but excluded for incomplete
  ownership. Both topology-multiple events2/13 are outside training.
- Actual extraction smoke: neighborhood plus sequence caches made from these
  rows under /tmp only.12 training tracks,1 with a >1% eligible owner loss.
  Full-context conversion yields N x24 hit inputs and (N-1) x5 interval inputs.
  Feature extraction is unchanged when truth fields are stripped. All model
  labels agree with shared grouped loss / primary momentum >0.01.
- Missing/schema2 cache guards reject stale data.
- Six existing CTests and36 batch-helper unittest checks passed; Python compile
  checks passed for modified cards/checker/analysis scripts.

Examples of intended changes: event7 cooling loss moves from old interval5 to
owner hit6. Event3 owner labels [4,8,229] become [5,-1,230]. Event17 non-sensitive
TPC gas loss is unassigned, rather than forced onto interval8. These are label
changes, not evidence of improved identification performance.

Commands (after setup.sh; build-tree configuration/library paths required):

```
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target RecBreakpoint -j4
python Reconstruction/RecBreakpoint/tests/check_interval_identification.py /tmp/bpid-layer-ZGuFGf/on.root /tmp/bpid-layer-ZGuFGf/off.root
python Reconstruction/RecBreakpoint/tests/check_interval_identification.py /tmp/bpid-layer-ZGuFGf/on.root /tmp/bpid-layer-ZGuFGf/before.root
python /tmp/bpid-layer-ZGuFGf/verify_consumers.py
```

The final command writes fresh temporary caches and creates a symlink; use a
fresh temporary path when replaying. It does not train a model. Runtime startup
needed the package directory containing RecBreakpoint.components in the library
search path, as already specified by the maintained batch helper/card runbook.

## Next

Generate a separate new tuple campaign; old schema2 outputs cannot be reused.
Set BPID_INPUT_DIR to that directory for local analysis. Then run
neighborhood_bdt.py extract, hierarchical_sequence_model.py extract, and
full_context_sequence_model.py train. Keep fixed seed split; assess ownership
coverage as well as conditional ML performance. Endcap passive ownership and
endcap ordering remain unvalidated. No claim of reaching 99% efficiency.

The dirty pre-existing additions to interval-identification-tuple.md were
preserved; only this turn's schema changes belong to the new checkpoint.
Outgoing AGENTS focus was preserved in full before replacing it.
