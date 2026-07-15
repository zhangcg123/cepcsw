#!/usr/bin/env python3
"""Compare selected radiative surfaces in overshoots and matched controls."""

from __future__ import annotations

import argparse
import csv
import re
from collections import Counter
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


MODE = re.compile(r"(\d+):g(\d+):")


def read(path: Path, sample: str) -> list[dict]:
    rows = []
    with path.open(newline="") as stream:
        for source in csv.DictReader(stream):
            modes = [(int(surface), int(mode))
                     for surface, mode in MODE.findall(
                         source["selected_bh_surface_modes"])
                     if int(mode)]
            truth = int(source["dominant_transition_index"])
            surfaces = [surface for surface, _ in modes]
            if not surfaces:
                relation = "identity"
            elif truth in surfaces:
                relation = "includes_truth"
            elif min(surfaces) < truth:
                relation = "inward"
            else:
                relation = "outward"
            rows.append({
                "sample": sample, "seed": source["seed"],
                "entry": source["entry"], "truth_transition": truth,
                "selected_surface_modes": source["selected_bh_surface_modes"],
                "first_selected_surface": surfaces[0] if surfaces else -1,
                "all_selected_surfaces": ";".join(map(str, surfaces)),
                "surface_relation": relation,
                "current_lcio_residual_pct": source["current_lcio_residual_pct"],
                "current_gsf_residual_pct": source["current_gsf_residual_pct"],
                "decisive_stage": source["decisive_stage"],
                "decisive_hit": source["decisive_hit"],
                "selected_weight": source["selected_weight"],
                "final_identity_weight": source["final_identity_weight"],
            })
    return rows


def write(path: Path, rows: list[dict]) -> None:
    with path.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    base = Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13")
    parser.add_argument("--overshoots", type=Path, default=base /
                        "maxcomp12_transition_7_8_overshoot_all57_diagnostics.csv")
    parser.add_argument("--controls", type=Path, default=base /
                        "maxcomp12_transition_7_8_controls57_diagnostics.csv")
    parser.add_argument("--output-dir", type=Path, default=base /
                        "maxcomp12_transition_7_8_surface_selection")
    args = parser.parse_args()
    rows = read(args.overshoots, "overshoot") + read(args.controls, "control")
    args.output_dir.mkdir(parents=True, exist_ok=True)
    write(args.output_dir / "eventwise_surface_selection.csv", rows)

    summary = []
    for sample in ("overshoot", "control"):
        selected = [row for row in rows if row["sample"] == sample]
        relations = Counter(row["surface_relation"] for row in selected)
        first = Counter(int(row["first_selected_surface"]) for row in selected)
        any_surface = Counter()
        for row in selected:
            any_surface.update(set(int(value) for value in
                                   row["all_selected_surfaces"].split(";") if value))
        for category, counts in (("relation", relations),
                                 ("first_surface", first),
                                 ("any_surface", any_surface)):
            for value, count in sorted(counts.items(), key=lambda item: str(item[0])):
                summary.append({"sample": sample, "metric": category,
                                "value": value, "count": count,
                                "fraction": count / len(selected)})
    write(args.output_dir / "surface_selection_summary.csv", summary)
    fig, axes = plt.subplots(1, 2, figsize=(11.5, 4.8))
    samples = ("overshoot", "control")
    colors = ("#D1495B", "#4C78A8")
    surface_values = (-1, 5, 6, 7, 8, 9)
    x = np.arange(len(surface_values))
    width = 0.36
    for offset, sample, color in zip((-0.5, 0.5), samples, colors):
        selected = [row for row in rows if row["sample"] == sample]
        counts = Counter(int(row["first_selected_surface"]) for row in selected)
        axes[0].bar(x + offset * width,
                    [counts[value] for value in surface_values], width,
                    color=color, label=sample.capitalize())
    axes[0].set_xticks(x, ["identity", "5", "6", "7", "8", "9"])
    axes[0].set(xlabel="First selected radiative surface",
                ylabel="Events", title="Selected surface")
    axes[0].legend(frameon=False)
    axes[0].grid(axis="y", alpha=0.2)

    relations = ("identity", "inward", "includes_truth", "outward")
    x = np.arange(len(relations))
    for offset, sample, color in zip((-0.5, 0.5), samples, colors):
        selected = [row for row in rows if row["sample"] == sample]
        counts = Counter(row["surface_relation"] for row in selected)
        axes[1].bar(x + offset * width,
                    [counts[value] for value in relations], width,
                    color=color, label=sample.capitalize())
    axes[1].set_xticks(x, ["identity", "inward", "truth", "outward"])
    axes[1].set(ylabel="Events", title="Selected surface relative to truth")
    axes[1].grid(axis="y", alpha=0.2)
    fig.suptitle("MaxComp=12 transition 7–8: 57 overshoots vs 57 loss-matched controls")
    fig.tight_layout()
    fig.savefig(args.output_dir / "surface_selection_overshoot_vs_control.png",
                dpi=180)
    fig.savefig(args.output_dir / "surface_selection_overshoot_vs_control.pdf")
    plt.close(fig)
    for row in summary:
        print(row)


if __name__ == "__main__":
    main()
