# CMS-like component-lineage persistence

Date: 2026-08-27

## Scope and outgoing contract

Before this change, the automatic component-lineage EDM and flat-tuple vectors
were populated only by smoother and ordinary reverse runs. CMS-like executed
the same forward and backward BH split, measurement, cutoff, and KL mechanics,
but its `lineage_graph_available`, `lineage_node_n`, and `lineage_edge_n`
fields were `0,0,0`. It persisted only the final component mixture. Therefore
an existing CMS-like tuple could not reconstruct the per-surface identity and
truth-like BH weights, exact local `dchi2`, innovation log determinant, or
normalized measurement posterior.

This record supersedes that CMS-like-empty part of the historical contract in
`2026-08-25-component-lineage-dag-flat-tuple.md`. The historical record remains
unchanged evidence of the behavior at that checkpoint.

## Implemented passive record

CMS-like now enables the existing `LineageGraphRecorder` automatically. No
configurable property, default, run-card assignment, fit state, component
weight, covariance, cutoff, reduction, or publication rule changed.

The existing node sources remain:

- `1`: forward filtering;
- `2`: reverse/CMS-backward filtering.

CMS-like adds node source `3` for the forward×backward Gaussian-product
mixture. Node operation `5` identifies one unreduced product candidate. Edge
operation `5` connects each product candidate to exactly one source-1 forward
updated state and one source-2 backward measurement node. The product uses
the backward node's persisted predicted state, not its filtered posterior
state. CMS product cutoff and KL reduction use the existing observer hooks, so
cut candidates and both inputs to every product-mixture KL merge remain in the
DAG. The retained product mixture is marked as the final mixture and its
selected member as BestBranch.

Fate `7` identifies a terminal forward or backward CMS message that was fully
evaluated but was not the published product-mixture endpoint. This prevents
such internal messages from being left ambiguously active. Product formation
and KL reconvergence remain a directed acyclic graph and never steer the fit.

Forward-only and global-loss jobs retain empty lineage collections. Ordinary
reverse and smoother behavior is unchanged.

## Focused verbose gate

The focused final-code gate used file 66 entry 15 from the non-production
BH15 diagnostic campaign:

```text
BHModel=CEPCRuntimeCategoryAligned15Clear
MaxComponents=10
ComponentWeightCutoff=1e-4
CmsErrorRescaling=1
TruthBHLossOverride=false
```

The verbose run completed successfully and formed 1,500/1,500 valid
forward×backward pairs at hit 1, retaining ten endpoint components. The flat
tuple contains 10,108 nodes and 12,144 edges:

```text
source counts:    1: 4,135; 2: 4,470; 3: 1,503
operation counts: 1: 46; 2: 2,955; 3: 5,070; 4: 537; 5: 1,500
fate counts:      1: 5,267; 3: 3,737; 4: 1,074; 5: 10; 7: 20
```

All 1,500 product nodes have exactly two incoming operation-5 edges with
parent sources `{1,2}`. All three source-3 KL outputs have exactly two
incoming merge edges. All 2,541 backward measurement nodes have finite exact
`dchi2`, `logDetInnovation`, and normalized posterior. Every edge resolves,
all edges follow increasing node IDs, the final mixture contains ten source-3
nodes, and exactly one is BestBranch.

A direct comparison with the immediately preceding same-code CMS tuple found
exact equality in all 64 compared BestBranch, WeightedMean, FullMixtureMode,
status, and final-component fields. The focused flat file grew from 217,713 to
903,089 bytes because the complete graph is now present; this is recording
overhead, not a fit change.

## Required hard-event gates

Events 11, 16, and 17 all completed with comprehensive verbose dumps. Event
16 has two input tracks, checked independently.

| Event/track | Nodes | Edges | Product nodes | Finite backward measurement nodes | Final components | BestBranch | Endpoint-field differences |
|---|---:|---:|---:|---:|---:|---:|---:|
| 11/0 | 10,367 | 12,409 | 1,500/1,500 valid ancestry | 2,688/2,688 | 10 | 1 | 0/64 |
| 16/0 | 1,431 | 1,523 | 2/2 valid ancestry | 920/920 | 2 | 1 | 0/64 |
| 16/1 | 6,710 | 8,588 | 1,500/1,500 valid ancestry | 1,350/1,350 | 10 | 1 | 0/64 |
| 17/0 | 8,354 | 10,167 | 1,350/1,350 valid ancestry | 2,122/2,122 | 10 | 1 | 0/64 |

Event 17 attempted 1,500 product pairs; 150 failed the pre-existing finite
Gaussian-product test, and all 1,350 successfully formed candidates have valid
two-parent ancestry. This is a mechanical persistence gate, not physics
validation.

## Eight-event low-loss diagnostic availability

The eight previously studied topology-clear low-loss events were rerun with
the final recorder. The table below compares exact pre-KL CMS-backward
measurement children at the primary truth-loss surface. BH weights are
unchanged from reverse because the model and path thickness are the same;
local likelihoods and normalized posteriors differ because CMS uses its own
backward seed/covariance.

| Event | Identity BH weight | Truth-like BH weight | Identity local chi2 | Truth-like local chi2 | Identity posterior | Truth-like posterior |
|---|---:|---:|---:|---:|---:|---:|
| 34/12 | 90.2948% | 0.8069% | 5.259 | 1.052 | 20.6739% | 0.2282% |
| 55/79 | 88.5695% | 0.9585% | 1.958 | 1.956 | 67.0477% | 0.7264% |
| 66/15 | 91.5541% | 0.7130% | 0.760 | 1.020 | 62.3369% | 0.4263% |
| 89/16 | 91.6099% | 0.7088% | 0.953 | 0.978 | 54.1463% | 0.4139% |
| 75/93 | 90.4138% | 0.7982% | 2.439 | 2.481 | 48.5017% | 0.4192% |
| 52/46 | 92.6216% | 0.6297% | 0.278 | 0.256 | 65.4693% | 0.4502% |
| 46/75 | 99.2821% | 0.0448% | 2.607 | 2.607 | 52.5582% | 0.0237% |
| 10/5 | 99.1377% | 0.0486% | 1.469 | 1.468 | 49.4595% | 0.0242% |

These selected events show the same mechanism-level problem as reverse: the
truth-like child's local chi2 is usually comparable to the identity child's,
but the measurement is not informative enough to overcome the much larger
identity BH prior. This table is a selected diagnostic and does not establish
population behavior or validate CMS-like.
