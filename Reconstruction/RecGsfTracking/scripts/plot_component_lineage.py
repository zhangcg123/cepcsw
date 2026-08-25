#!/usr/bin/env python3
"""Plot one reverse component-lineage graph and its ordinal weight maps.

The script consumes the automatic ``lineage_node_*`` and ``lineage_edge_*``
branches written by RecGsfFlatTuple.  When the passive ``truth_material_*``
vectors are present and valid, it marks the reverse measurement column that
consumes every truth interval with positive Geant4 eBrem loss.  It is
intentionally diagnostic: it does not rerun, steer, or reinterpret the GSF.
"""

from __future__ import annotations

import argparse
import copy
from collections import Counter, defaultdict
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection
from matplotlib.lines import Line2D
import numpy as np
import ROOT


REVERSE_SOURCE = 2
SEED_OPERATION = 1
BH_SPLIT_OPERATION = 2
MEASUREMENT_OPERATION = 3
KL_MERGE_OPERATION = 4

ACTIVE_FATE = 0
ADVANCED_FATE = 1
MEASUREMENT_REJECTED_FATE = 2
WEIGHT_CUTOFF_FATE = 3
KL_MERGED_FATE = 4
FINAL_SURVIVOR_FATE = 5
ABANDONED_FATE = 6

NODE_FIELDS = (
    "input_track_index",
    "id",
    "source",
    "operation",
    "hit_index",
    "fate",
    "best_branch",
    "weight",
    "normalized_posterior",
    "filtered_pT",
)
EDGE_FIELDS = (
    "input_track_index",
    "from_node_id",
    "to_node_id",
    "operation",
)
TRUTH_MATERIAL_FIELDS = (
    "input_track_index",
    "hit_from_index",
    "hit_to_index",
    "ebrem_loss",
    "p_before",
)

TRUTH_EBREM_COLOR = "#00c9e8"


def truth_marker_size(fractional_loss: float) -> float:
    """Return a visible, bounded marker area for one truth interval."""
    fraction = max(0.0, fractional_loss) if np.isfinite(fractional_loss) else 0.0
    return 38.0 + 142.0 * min(fraction / 0.20, 1.0)


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Plot one reverse GSF component-lineage graph")
    parser.add_argument("--input", type=Path, required=True,
                        help="RecGsfFlatTuple ROOT file")
    parser.add_argument("--entry", type=int, required=True,
                        help="zero-based gsf_tuple entry")
    parser.add_argument("--input-track", type=int, default=0,
                        help="CompleteTracks input index (default: 0)")
    parser.add_argument("--tree", default="gsf_tuple",
                        help="flat tuple tree name (default: gsf_tuple)")
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--stem", default="",
                        help="output stem; derived from entry/track when empty")
    parser.add_argument("--dpi", type=int, default=220,
                        help="PNG resolution (default: 220)")
    return parser.parse_args()


def branch_values(tree, name: str) -> list:
    branch = tree.GetBranch(name)
    if not branch:
        raise RuntimeError(f"required branch is absent: {name}")
    return list(getattr(tree, name))


def load_graph(args: argparse.Namespace):
    root_file = ROOT.TFile.Open(str(args.input))
    if not root_file or root_file.IsZombie():
        raise RuntimeError(f"cannot open {args.input}")
    tree = root_file.Get(args.tree)
    if not tree:
        raise RuntimeError(f"tree {args.tree!r} is absent in {args.input}")
    if args.entry < 0 or args.entry >= tree.GetEntries():
        raise RuntimeError(
            f"entry {args.entry} outside [0,{tree.GetEntries()})")
    if tree.GetEntry(args.entry) <= 0:
        raise RuntimeError(f"cannot read entry {args.entry}")

    node_columns = {
        name: branch_values(tree, "lineage_node_" + name)
        for name in NODE_FIELDS
    }
    node_lengths = {len(values) for values in node_columns.values()}
    if len(node_lengths) != 1:
        raise RuntimeError(f"inconsistent node-vector lengths: {node_lengths}")

    nodes = {}
    for index, (track, source) in enumerate(zip(
            node_columns["input_track_index"], node_columns["source"])):
        if (int(track) != args.input_track or
                int(source) != REVERSE_SOURCE):
            continue
        node_id = int(node_columns["id"][index])
        if node_id in nodes:
            raise RuntimeError(
                f"duplicate node ID {node_id} for input track "
                f"{args.input_track}")
        nodes[node_id] = {
            field: values[index] for field, values in node_columns.items()
        }
    if not nodes:
        raise RuntimeError(
            f"no reverse-source nodes for input track {args.input_track}")

    edge_columns = {
        name: branch_values(tree, "lineage_edge_" + name)
        for name in EDGE_FIELDS
    }
    edge_lengths = {len(values) for values in edge_columns.values()}
    if len(edge_lengths) != 1:
        raise RuntimeError(f"inconsistent edge-vector lengths: {edge_lengths}")

    edges = []
    for track, from_id, to_id, operation in zip(
            edge_columns["input_track_index"],
            edge_columns["from_node_id"],
            edge_columns["to_node_id"],
            edge_columns["operation"]):
        from_id = int(from_id)
        to_id = int(to_id)
        if int(track) != args.input_track:
            continue
        # Requiring both endpoints removes the forward-to-reverse seed edge
        # and therefore makes this a strictly reverse-only graph.
        if from_id in nodes and to_id in nodes:
            edges.append((from_id, to_id, int(operation)))

    truth_ebrem = []
    required_truth_branches = [
        "truth_material_" + field for field in TRUTH_MATERIAL_FIELDS
    ]
    if (tree.GetBranch("truth_material_scope_valid") and
            all(tree.GetBranch(name) for name in required_truth_branches) and
            bool(getattr(tree, "truth_material_scope_valid"))):
        truth_columns = {
            field: branch_values(tree, "truth_material_" + field)
            for field in TRUTH_MATERIAL_FIELDS
        }
        truth_lengths = {len(values) for values in truth_columns.values()}
        if len(truth_lengths) != 1:
            raise RuntimeError(
                f"inconsistent truth-material-vector lengths: {truth_lengths}")
        for index, track in enumerate(truth_columns["input_track_index"]):
            if int(track) != args.input_track:
                continue
            loss = float(truth_columns["ebrem_loss"][index])
            if not np.isfinite(loss) or loss <= 0.0:
                continue
            momentum_before = float(truth_columns["p_before"][index])
            fractional_loss = (
                loss / momentum_before
                if np.isfinite(momentum_before) and momentum_before > 0.0
                else float("nan")
            )
            truth_ebrem.append({
                "hit_from": int(truth_columns["hit_from_index"][index]),
                "hit_to": int(truth_columns["hit_to_index"][index]),
                "loss": loss,
                "fractional_loss": fractional_loss,
            })

    return root_file, nodes, edges, truth_ebrem


def classify(nodes):
    posterior_by_hit = defaultdict(list)
    reduced_by_hit = defaultdict(list)
    max_hit = max(int(node["hit_index"]) for node in nodes.values())
    min_hit = min(int(node["hit_index"]) for node in nodes.values())

    for node_id, node in nodes.items():
        operation = int(node["operation"])
        hit = int(node["hit_index"])
        fate = int(node["fate"])
        if operation == MEASUREMENT_OPERATION:
            posterior_by_hit[hit].append(node_id)
        if (operation in (MEASUREMENT_OPERATION, KL_MERGE_OPERATION) and
                fate in (ADVANCED_FATE, FINAL_SURVIVOR_FATE)):
            reduced_by_hit[hit].append(node_id)

    # The outer reverse seeds are the initial reduced layer.
    reduced_by_hit[max_hit] = [
        node_id for node_id, node in nodes.items()
        if (int(node["operation"]) == SEED_OPERATION and
            int(node["hit_index"]) == max_hit)
    ]

    for groups in (posterior_by_hit, reduced_by_hit):
        for node_ids in groups.values():
            node_ids.sort(key=lambda node_id: (
                float(nodes[node_id]["filtered_pT"]), node_id))

    if not posterior_by_hit:
        raise RuntimeError("the selected reverse graph has no measurement nodes")
    if not reduced_by_hit[max_hit]:
        raise RuntimeError("the selected reverse graph has no reverse seeds")
    return posterior_by_hit, reduced_by_hit, min_hit, max_hit


def graph_connectivity(edges):
    parents_by_to = defaultdict(list)
    for from_id, to_id, operation in edges:
        parents_by_to[to_id].append((from_id, operation))
    return parents_by_to


def save_figure(fig, stem: Path, dpi: int) -> None:
    fig.savefig(stem.with_suffix(".png"), dpi=dpi)
    fig.savefig(stem.with_suffix(".pdf"))
    plt.close(fig)


def plot_posterior_reduced_lineage(
        nodes, edges, posterior_by_hit, reduced_by_hit,
        truth_ebrem, min_hit: int, max_hit: int, stem: Path,
        dpi: int) -> dict:
    parents_by_to = graph_connectivity(edges)
    max_posterior_count = max(len(ids) for ids in posterior_by_hit.values())
    max_y = max_posterior_count - 1

    posterior_position = {}
    reduced_position = {}
    for hit, node_ids in posterior_by_hit.items():
        depth = max_hit - hit
        for rank, node_id in enumerate(node_ids):
            posterior_position[node_id] = (float(depth), float(rank))
    for hit, node_ids in reduced_by_hit.items():
        depth = max_hit - hit
        x = 0.0 if hit == max_hit else float(depth) + 0.36
        if len(node_ids) == 1:
            y_values = [0.5 * max_y]
        else:
            y_values = [
                rank * max_y / (len(node_ids) - 1)
                for rank in range(len(node_ids))
            ]
        for node_id, y in zip(node_ids, y_values):
            reduced_position[node_id] = (x, y)

    # Collapse parent -> BH child -> measurement result into one edge.  If no
    # BH split happened, the measurement edge can start directly at the
    # previous reduced state.
    propagation_segments = set()
    for posterior_id, posterior_pos in posterior_position.items():
        for measurement_parent, operation in parents_by_to.get(
                posterior_id, []):
            if operation != 2:  # lineage edge operation: measurement
                continue
            if measurement_parent in reduced_position:
                propagation_segments.add(
                    (reduced_position[measurement_parent], posterior_pos))
            for parent_id, parent_operation in parents_by_to.get(
                    measurement_parent, []):
                if parent_operation == 1 and parent_id in reduced_position:
                    propagation_segments.add(
                        (reduced_position[parent_id], posterior_pos))

    ancestor_cache = {}

    def base_measurement_ancestors(node_id: int, hit: int) -> set[int]:
        key = (node_id, hit)
        if key in ancestor_cache:
            return ancestor_cache[key]
        node = nodes[node_id]
        if int(node["hit_index"]) != hit:
            result = set()
        elif int(node["operation"]) == MEASUREMENT_OPERATION:
            result = {node_id}
        elif int(node["operation"]) == KL_MERGE_OPERATION:
            result = set()
            for parent_id, operation in parents_by_to.get(node_id, []):
                if operation == 3:  # lineage edge operation: KL merge
                    result.update(base_measurement_ancestors(parent_id, hit))
        else:
            result = set()
        ancestor_cache[key] = result
        return result

    # Flatten intermediate KL nodes onto the sparse final survivor layer.
    reduction_segments = set()
    for hit, reduced_ids in reduced_by_hit.items():
        if hit == max_hit:
            continue
        for reduced_id in reduced_ids:
            target = reduced_position[reduced_id]
            for posterior_id in base_measurement_ancestors(reduced_id, hit):
                if posterior_id in posterior_position:
                    reduction_segments.add(
                        (posterior_position[posterior_id], target))

    fig, ax = plt.subplots(figsize=(15, 7.5))
    ax.add_collection(LineCollection(
        list(propagation_segments), colors="black", linewidths=0.48,
        linestyles="-", alpha=1.0, zorder=1))
    ax.add_collection(LineCollection(
        list(reduction_segments), colors="black", linewidths=0.48,
        linestyles=":", alpha=1.0, zorder=1))

    posterior_ids = sorted(posterior_position)
    ax.scatter(
        [posterior_position[node_id][0] for node_id in posterior_ids],
        [posterior_position[node_id][1] for node_id in posterior_ids],
        s=15, color="#d62728", edgecolors="#8c1717", linewidths=0.35,
        alpha=1.0, zorder=3)
    reduced_ids = sorted(reduced_position)
    ax.scatter(
        [reduced_position[node_id][0] for node_id in reduced_ids],
        [reduced_position[node_id][1] for node_id in reduced_ids],
        s=25, color="#7b3294", edgecolors="#3f1550", marker="D",
        linewidths=0.5, alpha=1.0, zorder=4)

    # Fate 4 is not a cutoff: it entered KL and contributed to its merge
    # output.  Never put the cutoff X on a KL-consumed node.
    cutoff_ids = [
        node_id for node_id in posterior_ids
        if int(nodes[node_id]["fate"]) in (
            MEASUREMENT_REJECTED_FATE, WEIGHT_CUTOFF_FATE, ABANDONED_FATE)
    ]
    ax.scatter(
        [posterior_position[node_id][0] for node_id in cutoff_ids],
        [posterior_position[node_id][1] for node_id in cutoff_ids],
        s=34, color="black", marker="x", linewidths=0.8,
        alpha=1.0, zorder=5)

    final_ids = [
        node_id for node_id in reduced_ids
        if int(nodes[node_id]["fate"]) == FINAL_SURVIVOR_FATE
    ]
    ax.scatter(
        [reduced_position[node_id][0] for node_id in final_ids],
        [reduced_position[node_id][1] for node_id in final_ids],
        s=65, facecolors="none", edgecolors="#2ca02c", marker="o",
        linewidths=1.4, alpha=1.0, zorder=6)
    best_ids = [
        node_id for node_id in final_ids if bool(nodes[node_id]["best_branch"])
    ]
    ax.scatter(
        [reduced_position[node_id][0] for node_id in best_ids],
        [reduced_position[node_id][1] for node_id in best_ids],
        s=135, color="#ffbf00", edgecolors="#5b4300", marker="*",
        linewidths=0.8, alpha=1.0, zorder=7)

    matched_truth_ebrem = [
        interval for interval in truth_ebrem
        if min_hit <= interval["hit_from"] <= max_hit
    ]
    if matched_truth_ebrem:
        ax.scatter(
            [max_hit - interval["hit_from"]
             for interval in matched_truth_ebrem],
            [1.012] * len(matched_truth_ebrem),
            s=[truth_marker_size(interval["fractional_loss"])
               for interval in matched_truth_ebrem],
            transform=ax.get_xaxis_transform(), clip_on=False,
            color=TRUTH_EBREM_COLOR, edgecolors="black", marker="o",
            linewidths=0.8, alpha=1.0, zorder=9)

    depths = list(range(max_hit - min_hit + 1))
    ax.set_xticks(depths)
    ax.set_xticklabels([
        f"initial reduced\n(hit {max_hit})" if depth == 0
        else f"posterior\n(hit {max_hit - depth})"
        for depth in depths
    ])
    ax.set_xlabel("inward propagation")
    ax.set_ylabel("ordinal $p_T$ position  (equal posterior spacing; low → high)")
    ax.set_yticks([])
    ax.grid(axis="x", color="#d8d8d8", linewidth=0.7)
    ax.margins(x=0.025, y=0.04)
    legend_handles = [
        Line2D([0], [0], color="black", ls="-", lw=1.2,
               label="previous reduced state → next posterior"),
        Line2D([0], [0], color="black", ls=":", lw=1.2,
               label="posterior → reduced survivor"),
        Line2D([0], [0], color="#d62728", marker="o", lw=0,
               markersize=5, label="measurement posterior"),
        Line2D([0], [0], color="#7b3294", marker="D", lw=0,
               markersize=5, label="post-reduction survivor"),
        Line2D([0], [0], color="black", marker="x", lw=0,
               markersize=6, label="cutoff/rejected posterior"),
        Line2D([0], [0], color="#2ca02c", marker="o",
               markerfacecolor="none", lw=0, markersize=7,
               label="final component"),
        Line2D([0], [0], color="#ffbf00", markeredgecolor="#5b4300",
               lw=0, marker="*", markersize=10,
               label="published BestBranch"),
    ]
    if matched_truth_ebrem:
        legend_handles.append(Line2D(
            [0], [0], color=TRUTH_EBREM_COLOR, markeredgecolor="black",
            lw=0, marker="o", markersize=7,
            label="truth eBrem interval (size ∝ fractional loss)"))
    ax.legend(handles=legend_handles, loc="upper left", ncol=2, fontsize=9,
       frameon=True, framealpha=1.0)
    fig.suptitle(
        "Reverse posterior lineage with sparse reduction layers",
        fontsize=15, y=0.975)
    fig.text(
        0.5, 0.925,
        f"{len(posterior_ids)} exact measurement posteriors; "
        f"{len(reduced_ids)} displayed reduction survivors; "
        f"{len(cutoff_ids)} cutoff/rejected posteriors",
        ha="center", fontsize=10)
    fig.text(
        0.5, 0.015,
        "Black × marks only genuine pre-KL removal. KL-consumed posteriors "
        "remain red and retain dotted ancestry into the reduced survivor.",
        ha="center", fontsize=8.8)
    fig.tight_layout(rect=(0.03, 0.055, 0.995, 0.90))
    save_figure(fig, stem, dpi)
    return {
        "posteriors": len(posterior_ids),
        "reduced": len(reduced_ids),
        "cutoff_or_rejected": len(cutoff_ids),
        "propagation_edges": len(propagation_segments),
        "reduction_edges": len(reduction_segments),
        "truth_ebrem_markers": len(matched_truth_ebrem),
    }


def plot_weight_cells(
        nodes, groups, max_hit: int, weight_field: str, cmap_name: str,
        title: str, color_label: str, caption: str, stem: Path, dpi: int,
        posterior: bool, truth_ebrem) -> dict:
    hits = sorted(groups, reverse=True)
    x_values = [max_hit - hit for hit in hits]
    max_count = max(len(groups[hit]) for hit in hits)
    matrix = np.full((max_count, len(hits)), np.nan, dtype=float)
    for column, hit in enumerate(hits):
        for rank, node_id in enumerate(groups[hit]):
            matrix[rank, column] = float(nodes[node_id][weight_field])

    cmap = copy.copy(plt.get_cmap(cmap_name))
    cmap.set_bad("white")
    fig, ax = plt.subplots(figsize=(15, 7.5))
    image = ax.pcolormesh(
        np.arange(len(hits) + 1) - 0.5,
        np.arange(max_count + 1) - 0.5,
        np.ma.masked_invalid(matrix),
        cmap=cmap, vmin=0.0, vmax=float(np.nanmax(matrix)),
        edgecolors="black", linewidth=0.22, antialiased=True,
        shading="flat")

    if posterior:
        cutoff_x = []
        cutoff_y = []
        for column, hit in enumerate(hits):
            for rank, node_id in enumerate(groups[hit]):
                if int(nodes[node_id]["fate"]) in (
                        MEASUREMENT_REJECTED_FATE,
                        WEIGHT_CUTOFF_FATE,
                        ABANDONED_FATE):
                    cutoff_x.append(column)
                    cutoff_y.append(rank)
        ax.scatter(
            cutoff_x, cutoff_y, color="black", marker="x", s=24,
            linewidths=0.7, alpha=1.0, zorder=4,
            label="cutoff/rejected posterior")
    else:
        final_column = len(hits) - 1
        final_ids = [
            node_id for node_id in groups[hits[final_column]]
            if int(nodes[node_id]["fate"]) == FINAL_SURVIVOR_FATE
        ]
        final_ranks = [groups[hits[final_column]].index(node_id)
                       for node_id in final_ids]
        ax.scatter(
            [final_column] * len(final_ids), final_ranks,
            facecolors="none", edgecolors="#2ca02c", marker="o", s=70,
            linewidths=1.5, alpha=1.0, zorder=4,
            label="final component")
        best_ids = [
            node_id for node_id in final_ids
            if bool(nodes[node_id]["best_branch"])
        ]
        best_ranks = [groups[hits[final_column]].index(node_id)
                      for node_id in best_ids]
        ax.scatter(
            [final_column] * len(best_ids), best_ranks,
            color="#ffbf00", edgecolors="#5b4300", marker="*", s=145,
            linewidths=0.8, alpha=1.0, zorder=5,
            label="published BestBranch")

    hit_to_column = {hit: column for column, hit in enumerate(hits)}
    matched_truth_ebrem = [
        interval for interval in truth_ebrem
        if interval["hit_from"] in hit_to_column
    ]
    if matched_truth_ebrem:
        ax.scatter(
            [hit_to_column[interval["hit_from"]]
             for interval in matched_truth_ebrem],
            [max_count + 0.35] * len(matched_truth_ebrem),
            s=[truth_marker_size(interval["fractional_loss"])
               for interval in matched_truth_ebrem],
            color=TRUTH_EBREM_COLOR, edgecolors="black", marker="o",
            linewidths=0.8, alpha=1.0, clip_on=False, zorder=7,
            label="truth eBrem interval (size ∝ fractional loss)")

    ax.set_xticks(range(len(hits)))
    ax.set_xticklabels([
        f"{depth}\n(hit {hit})" for depth, hit in zip(x_values, hits)
    ], fontsize=9)
    ax.set_yticks([0, max_count - 1])
    ax.set_yticklabels(["low", "high"])
    ax.set_xlabel("inward propagation step")
    ax.set_ylabel("ordinal $p_T$ rank  (equal spacing; low → high)")
    ax.set_title(title, fontsize=15, pad=12)
    ax.set_xlim(-0.5, len(hits) - 0.5)
    ax.set_ylim(-0.5, max_count + (1.25 if matched_truth_ebrem else -0.5))
    ax.legend(loc="upper right", frameon=True, framealpha=1.0)
    colorbar = fig.colorbar(image, ax=ax, pad=0.02)
    colorbar.set_label(color_label)
    fig.text(0.5, 0.018, caption, ha="center", fontsize=9)
    fig.tight_layout(rect=(0.03, 0.05, 0.98, 0.96))
    save_figure(fig, stem, dpi)
    return {
        "layers": len(hits),
        "max_rows": max_count,
        "nodes": sum(len(groups[hit]) for hit in hits),
        "truth_ebrem_markers": len(matched_truth_ebrem),
    }


def main() -> None:
    args = arguments()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    stem = args.stem or (
        f"reverse-lineage-entry{args.entry}-track{args.input_track}")
    _, nodes, edges, truth_ebrem = load_graph(args)
    posterior_by_hit, reduced_by_hit, min_hit, max_hit = classify(nodes)

    lineage_summary = plot_posterior_reduced_lineage(
        nodes, edges, posterior_by_hit, reduced_by_hit, truth_ebrem,
        min_hit, max_hit, args.output_dir / f"{stem}-posterior-reduced",
        args.dpi)
    posterior_summary = plot_weight_cells(
        nodes, posterior_by_hit, max_hit, "normalized_posterior", "Reds",
        "Reverse posterior weight cells before cutoff/KL",
        "normalized posterior weight",
        "Each column is independently ordered from low to high pT; rows are "
        "ordinal and do not imply lineage continuity.",
        args.output_dir / f"{stem}-posterior-weight-cells", args.dpi,
        posterior=True, truth_ebrem=truth_ebrem)
    reduced_summary = plot_weight_cells(
        nodes, reduced_by_hit, max_hit, "weight", "Purples",
        "Reverse reduced-mixture weight cells after cutoff/KL",
        "normalized reduced weight",
        "Each column is independently ordered from low to high pT; rows are "
        "ordinal and do not imply lineage continuity.",
        args.output_dir / f"{stem}-reduced-weight-cells", args.dpi,
        posterior=False, truth_ebrem=truth_ebrem)

    operation_counts = Counter(int(node["operation"])
                               for node in nodes.values())
    fate_counts = Counter(int(node["fate"]) for node in nodes.values())
    best_nodes = [node for node in nodes.values()
                  if bool(node["best_branch"])]
    print(f"input={args.input} entry={args.entry} "
          f"input_track={args.input_track}")
    print(f"reverse_nodes={len(nodes)} reverse_edges={len(edges)}")
    print(f"operation_counts={dict(sorted(operation_counts.items()))}")
    print(f"fate_counts={dict(sorted(fate_counts.items()))}")
    print(f"lineage_plot={lineage_summary}")
    print(f"posterior_cells={posterior_summary}")
    print(f"reduced_cells={reduced_summary}")
    print("truth_ebrem_intervals=" + repr([
        {
            "hit_from": interval["hit_from"],
            "hit_to": interval["hit_to"],
            "fractional_loss_pct": 100.0 * interval["fractional_loss"],
        }
        for interval in truth_ebrem
    ]))
    if len(best_nodes) == 1:
        print("bestbranch_filtered_pT="
              f"{float(best_nodes[0]['filtered_pT']):.12g} GeV "
              f"weight={float(best_nodes[0]['weight']):.12g}")
    for suffix in (
            "posterior-reduced", "posterior-weight-cells",
            "reduced-weight-cells"):
        print(args.output_dir / f"{stem}-{suffix}.png")
        print(args.output_dir / f"{stem}-{suffix}.pdf")


if __name__ == "__main__":
    main()
