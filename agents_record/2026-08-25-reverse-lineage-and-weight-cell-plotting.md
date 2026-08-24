# Reverse lineage and ordinal weight-cell plotting workflow

Date: 2026-08-25

## Purpose and boundary

The automatic component-lineage DAG is now the primary passive diagnostic for
locating the first inward measurement where a promising reverse lineage loses
posterior support, is cut, or is absorbed by KL reduction. This record freezes
the agreed visualization contract and the maintained plotting command so that
future event studies use the same node/fate interpretation.

The maintained tool is:

```text
Reconstruction/RecGsfTracking/scripts/plot_component_lineage.py
```

This analysis script is a specific user-authorized exception to the normal law
that generated analysis helpers remain uncommitted. Generated ROOT files,
PNGs, PDFs, logs, and per-campaign output directories remain untracked. The
tool only reads flat-tuple branches; it does not steer or rerun the GSF.

The graph schema and mechanical persistence gates remain authoritative in
`agents_record/2026-08-25-component-lineage-dag-flat-tuple.md` and
`Reconstruction/RecGsfTracking/README.md`. This plotting record does not
replace either contract.

## Reproduction command

Run from the repository root in the normal CEPCSW environment:

```bash
source setup.sh
MPLCONFIGDIR=/tmp/mpl-lineage \
python3 Reconstruction/RecGsfTracking/scripts/plot_component_lineage.py \
  --input /path/to/gsf-flat.root \
  --entry 16 \
  --input-track 1 \
  --output-dir /tmp/lineage-entry16-track1
```

The entry is the zero-based `gsf_tuple` entry. `--input-track` is the
`CompleteTracks` input index, not the row-aligned output index. The script
selects `lineage_node_source=2`, retains edges whose two endpoints belong to
that reverse graph, and thereby omits the forward graph and the
forward-to-reverse seed edges. It fails explicitly if the selected input track
does not have a reverse lineage record.

The command writes PNG and PDF pairs for:

```text
<stem>-posterior-reduced
<stem>-posterior-weight-cells
<stem>-reduced-weight-cells
```

The default PNG geometry is 15 by 7.5 inches at 220 DPI, or 3300 by 1650
pixels. Use the PDF for vector zooming. The output stem defaults to
`reverse-lineage-entry<entry>-track<input-track>`.

## Posterior/reduced lineage view

The accepted lineage view deliberately hides the verbose BH-child and
intermediate KL-output columns while preserving their ancestry:

- Red circles are exact measurement nodes (`operation=3`). Their
  `normalized_posterior` is the normalized weight before cutoff and KL.
- Purple diamonds form the sparse post-reduction layer. At the outermost hit
  they are the reverse seeds. At every later hit they are measurement or KL
  nodes with fate `1` or `5`, meaning an independent continuation survived
  reduction or became a final component.
- A solid black edge collapses the previous reduced state through any BH child
  and its measurement update into the next explicit posterior.
- A dotted black edge flattens the same-hit KL merge tree from an exact
  measurement posterior into the final displayed reduced survivor.
- A black cross is reserved for genuine pre-KL loss: measurement rejection
  (fate `2`), weight cutoff (fate `3`), or an abandoned track (fate `6`).
  Fate `4` never receives a cross in this view.
- Green rings mark every member of the final reduced mixture. The gold star is
  overlaid on the one final reduced component marked BestBranch. It belongs to
  the reduced last-posterior layer, not to the red last-measurement layer and
  not to a later processing layer.

Both posterior and reduced columns are independently sorted by filtered pT.
The vertical coordinate is ordinal with equal spacing. It preserves low-to-
high ordering within one displayed column but is not a physical pT scale.

## Fate 3 versus fate 4

The distinction is essential when interpreting downstream edges:

```text
fate 3: normalized posterior is removed by removeLowWeight()
        -> deleted before GsfMixture::reduce()
        -> no KL contribution and no downstream reduction edge

fate 4: posterior survived cutoff and was selected as one input of a KL merge
        -> its mean, covariance, and weight contribute to the merged state
        -> it then ceases to exist as an independent component
        -> downstream KL ancestry edge is required
```

Thus a fate-4 node is merge-consumed, not killed before KL. The live reducer
performs cutoff first and passes only the retained component vector to KL.
The lineage recorder retrospectively marks both old merge inputs fate `4` and
creates a new merge-output node because the post-merge state is not identical
to either immutable input snapshot.

## Ordinal weight-cell maps

The two color maps retain the original equal-sized cell style requested for
future comparisons:

1. `posterior-weight-cells` uses one row for every exact measurement
   posterior in that hit and colors the cell with
   `lineage_node_normalized_posterior`. This is the pre-cutoff, pre-KL
   normalized weight. A black cross marks only fates `2`, `3`, and `6`.
2. `reduced-weight-cells` uses the sparse post-reduction population and colors
   cells with `lineage_node_weight`, which is normalized after cutoff and KL.
   The final column retains green final-component rings and the gold
   BestBranch star.

Within each column, components are independently sorted from low to high
filtered pT and assigned equal-height ordinal cells. Therefore:

- cell row is not a physical pT coordinate;
- the same row in adjacent columns is not necessarily the same lineage;
- a horizontal band is not evidence that one component persisted;
- use the lineage view and node IDs/edges for ancestry, not the color maps;
- use `lineage_node_filtered_pT` directly when a physical pT axis is required.

Weight is represented by a linear color scale starting at zero. White or very
light cells can still be valid small-weight components. The posterior and
reduced maps have independent maxima and must not be compared by raw color
shade without reading their color bars.

## Reference mechanical gate

The final plotting gate used
`/tmp/gsf-lineage-final-gate-flat.root`, zero-based flat entry 16, input track
1. This is the shortest graph in the focused hard-event file and contains only
nine hit indices, but it is a secondary-tracker-activity/control track rather
than a topology-clear performance representative.

The maintained script reproduced:

| quantity | count |
|---|---:|
| reverse nodes | 1,072 |
| reverse edges after removing forward seed links | 1,160 |
| reverse seeds | 12 |
| BH-child nodes | 480 |
| measurement-posterior nodes | 480 |
| intermediate KL-output nodes | 100 |
| posterior-cutoff nodes | 284 |
| KL-consumed nodes, including intermediate KL inputs | 200 |
| displayed reduced-layer nodes, including initial seeds | 108 |
| final components | 12 |
| BestBranch nodes | 1 |

Among the 480 exact measurement posteriors, the fates are 284 cutoff, 146
KL-consumed, 44 advanced unchanged, and six direct final survivors. Six
additional final components are KL outputs. All 146 KL-consumed measurement
posteriors have normalized pre-cutoff weights above the configured
`ComponentWeightCutoff=1e-4`: their range is
`1.081769097879284e-4` to `0.41515436250980564`. All 284 fate-3 posteriors are
below the threshold: their range is `4.675036078955813e-8` to
`9.931489189440305e-5`. No cutoff posterior reaches the KL input vector in
this gate.

The plotted track maps to output track 1. Direct EDM reading gives:

| endpoint | pT [GeV] |
|---|---:|
| input `CompleteTracks[1]` | 38.6096848083 |
| `GSFTracksBestBranch[1]` | 38.8965630818 |
| `GSFTracksWeightedMean[1]` | 39.0942670598 |
| `GSFTracksFullMixtureMode[1]` | 38.9249002497 |

The primary generator pT is `37.8940162659 GeV`, but this event contains two
reconstructed input tracks. These values are a plotting/mechanical reference,
not a topology-clear truth-performance claim. The legacy scalar flat-tuple
LCIO value `18.2928318629 GeV` describes input track 0 and must not be confused
with the visualized input track 1.

## Use in further studies

For every bad event, select the physically relevant input track explicitly and
produce the same three views. Pair it with a same-code good control. Use the
lineage graph to find the first inward posterior where the truth-compatible
lineage loses rank or is cut/merged; then inspect its saved prior/BH weight,
`dchi2`, and `logDetInnovation`. The ordinal weight cells are an overview of
competition and reduction, not sufficient evidence for the cause of a branch
change.

Generated plots demonstrate recorded mechanics only. They do not validate the
BH model, the selected branch, or the reverse method's population performance.
