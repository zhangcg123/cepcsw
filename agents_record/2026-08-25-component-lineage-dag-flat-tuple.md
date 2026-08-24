# Complete component-lineage DAG in the flat tuple

Date: 2026-08-25

## Objective and boundary

The reverse-posterior investigation requires more than the final twelve
components. It must retain every component state that was actually evaluated,
including a state later deleted after a failed measurement, removed by the
posterior cutoff, or consumed as either input of a KL moment merge. Otherwise
the first surface where a truth-compatible branch loses posterior support
cannot be reconstructed after the job.

This change adds passive, automatic lineage recording inside
`Reconstruction/RecGsfTracking`. It does not add a configurable property and
does not change a component mean, covariance, weight, lifetime decision,
reduction pair, final selector, or published track. The requested BH-component
variance scale remains deliberately unimplemented until the unscaled
posterior evolution is diagnosed.

## Graph model

Each input track owns one directed acyclic graph. A node is an immutable
snapshot created by one of four operations:

1. seed;
2. Bethe-Heitler split child;
3. measurement evaluation result;
4. KL merge output.

Node IDs start at zero for each input track and are never reused. The unique
event-local key is `(input_track_index,node_id)`, not a bare node ID. Each edge
is tagged as BH split, measurement, KL merge, or forward-final to reverse-seed.
Every KL output has two incoming merge edges. Thus a split followed by a merge
draws as a diverging/reconverging diamond—the requested visual loop—without
introducing a directed cycle.

The graph keeps explicit fates: active, advanced, measurement rejected,
posterior-cutoff removal, consumed by KL merge, final survivor, and abandoned
after a failed reverse endpoint or complete output-track failure. The final
survivors additionally identify final-mixture membership and the selected
BestBranch.

## Persisted decision information

The node record contains input/output track mapping; workflow source;
operation, hit, surface, component, and generation identifiers; exact
identity-lineage status; BH mode weight/mean/variance and `pathTX0`; local
measurement status; prior weight; exact `dchi2`, `logDetInnovation`,
unnormalized log posterior, and normalized pre-cutoff posterior; predicted and
filtered kappa/variance; dominant-lineage fraction; merge cost; and the final
fate flags. The flat tuple derives predicted and filtered pT as
`1/abs(kappa)`.

The corresponding EDM collections use `GSFLineageNode*` and
`GSFLineageEdge*` names. `RecGsfFlatTuple` always creates the
`lineage_graph_available`, `lineage_node_n`, `lineage_node_*`,
`lineage_edge_n`, and `lineage_edge_*` branches. Smoother and ordinary reverse
jobs populate them automatically. Forward-only, CMS-like, global-loss,
unprocessed rows, and older EDM inputs leave them present but empty. No graph
is silently truncated.

The graph is also retained if an evaluated smoother/reverse input track cannot
publish an endpoint. Such nodes and edges use `output_track_index=-1`; any
still-live terminal components are marked with fate `6` (track abandoned).
This prevents the most diagnostically important total-fit failures from
disappearing merely because the row-aligned GSF output is absent.

The smoother record contains the forward component construction subsequently
used by the KL reduction-aware smoother. The reverse record additionally
connects every surviving forward component to its reverse seed, then records
the complete inward evolution. The existing `final_mixture_component_*`
vectors remain the authoritative IP marginal parameters; the lineage graph is
the surface-by-surface decision history rather than a fourth endpoint output.

## Implementation

- `GsfComponent` carries only the current passive node ID, and cloning copies
  that ID.
- `GsfMixture::removeLowWeight` accepts an optional observer invoked before a
  removed component is deleted.
- `GsfMixture::reduce` accepts an optional observer after the unchanged moment
  merge and before the dropped component is deleted. It supplies both source
  node IDs and the selected merge cost.
- `RecGsfTracking` creates immutable node/edge records at every lifecycle
  transition and writes them as parallel PODIO user-data collections.
- `RecGsfFlatTuple` validates the parallel collection lengths, copies the
  complete vectors, and derives node pT values.
- `DumpGsfTrks/gsf.py.bk` requires no steering or explicit collection list
  because the record is automatic and the maintained output uses `keep *`.

An initial forward-only smoke exposed a passive-recorder bug: disabled graph
mode returned node ID `-1`, after which split metadata tried to index that
node. Guarding disabled split/measurement/merge decoration fixed the crash.
The corrected forward-only run completes with graph availability/counts
`0/0/0`.

## Mechanical validation

Both `RecGsfTracking` and `RecGsfFlatTuple` build and install in the maintained
EL9/LCG 105 tree.

A comprehensive verbose reverse run of file 1, event 11 completed with all
234 hits. Its graph has 7,048 nodes and 7,281 edges: 1,185 split children,
5,616 measurement nodes, and 234 merge outputs. It retains 703 posterior-
cutoff nodes and both 468 merge inputs, then marks 12 final survivors and one
BestBranch.

The required final-code hard gate used file 1 events 11, 16, and 17. Event 16
contains two reconstructed tracks, so graph invariants were checked per input
track. Across the three tuple rows:

| event | nodes | edges | cutoff nodes | merge outputs | final nodes | best nodes |
|---:|---:|---:|---:|---:|---:|---:|
| 11 | 7,048 | 7,281 | 703 | 234 | 12 | 1 |
| 16 | 4,259 | 4,421 | 525 | 164 | 21 | 2 |
| 17 | 5,413 | 5,655 | 482 | 243 | 12 | 1 |

For every graph, `(input_track_index,node_id)` is unique; every edge endpoint
exists in the same track; every edge points to a later-created node; every KL
output has exactly two incoming merge edges; and every finite measurement-
posterior group sums to one within numerical precision before pruning. A
same-input repeat after the disabled-recorder fix produced zero differing flat
branches, including the full graph and all pre-existing track/result fields.

The smoother event-0 smoke produced 2,761 nodes and 2,836 edges with 289
cutoff nodes, 76 two-parent merge outputs, 12 final nodes, and one BestBranch;
all structural and posterior-normalization invariants passed. The ordinary
forward event-0 control completed with all lineage vectors empty, as required.

These are mechanical persistence and non-interference gates. They do not show
that a selected branch is physically correct or validate any BH model.

## Next diagnostic

Use the graph on bad events and matched good controls to locate the first
inward measurement where the truth-compatible lineage loses posterior rank.
At that measurement, decompose the change into prior/BH weight,
`dchi2`, and `logDetInnovation`, and follow whether the state is later cut or
merged. Only after this unscaled baseline is understood should the reviewed
control scale the newly added within-BH-component variance term. Scale one
must reproduce this recorded baseline exactly.
