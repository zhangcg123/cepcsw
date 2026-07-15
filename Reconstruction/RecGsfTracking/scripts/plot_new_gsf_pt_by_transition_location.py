#!/usr/bin/env python3
"""Plot matched new-GSF pT resolution by dominant eBrem transition location."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


GROUPS = ("0-2", "3-4", "5-6", "7-8", "9-11", ">11")
COLORS = ("#4c78a8", "#f58518", "#54a24b", "#e45756", "#b279a2",
          "#72b7b2")


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--residuals", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/maxcomp24_new_tuples_2026-07-14/matched_event_residuals.csv"))
    parser.add_argument(
        "--transitions", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/light_outcomes_by_dominant_loss_surface.csv"))
    parser.add_argument(
        "--output-dir", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/maxcomp24_new_tuples_2026-07-14/transition_location"))
    parser.add_argument("--gsf-label", default="GSF MaxComponents=24")
    return parser.parse_args()


def group(source: str) -> str:
    return source if source in GROUPS[:-1] else ">11"


def load(args: argparse.Namespace) -> list[dict]:
    with args.residuals.open(newline="") as stream:
        residuals = {
            (int(row["seed"]), int(row["entry"])): row
            for row in csv.DictReader(stream)
            if row["category"] == "light_ebrem"
            and not int(row["excluded_secondary_topology"])
        }
    rows = []
    with args.transitions.open(newline="") as stream:
        for source in csv.DictReader(stream):
            event_id = (int(source["seed"]), int(source["entry"]))
            if event_id not in residuals:
                continue
            result = residuals[event_id]
            rows.append({
                "seed": event_id[0], "entry": event_id[1],
                "transition_group": group(source["dominant_transition_bin"]),
                "dominant_transition_index": int(source["dominant_transition_index"]),
                "dominant_transition_loss_pct": float(source["dominant_transition_loss_pct"]),
                "owned_loss_pct": float(source["owned_loss_pct"]),
                "lcio_residual_pct": float(result["lcio_residual_pct"]),
                "gsf_residual_pct": float(result["gsf_residual_pct"]),
            })
    return rows


def stats(values: np.ndarray) -> dict[str, float | int]:
    q16, median, q84 = np.quantile(values, [0.16, 0.5, 0.84])
    return {
        "count": values.size, "median_pct": median, "q16_pct": q16,
        "q84_pct": q84, "width68_pct": 0.5 * (q84 - q16),
        "rms_pct": np.sqrt(np.mean(values * values)),
        "inside_1pct": np.count_nonzero(np.abs(values) <= 1),
        "inside_2pct": np.count_nonzero(np.abs(values) <= 2),
        "inside_5pct": np.count_nonzero(np.abs(values) <= 5),
        "inside_10pct": np.count_nonzero(np.abs(values) <= 10),
    }


def write_csv(path: Path, rows: list[dict]) -> None:
    with path.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def main() -> None:
    args = arguments()
    rows = load(args)
    if not rows:
        raise RuntimeError("No matched topology-clean light-eBrem events")
    args.output_dir.mkdir(parents=True, exist_ok=True)
    write_csv(args.output_dir / "transition_location_event_residuals.csv", rows)

    summary = []
    for transition_group in GROUPS:
        selected = [row for row in rows if row["transition_group"] == transition_group]
        for algorithm, field in (("LCIO", "lcio_residual_pct"),
                                 (args.gsf_label, "gsf_residual_pct")):
            values = np.asarray([row[field] for row in selected])
            summary.append({
                "transition_group": transition_group, "algorithm": algorithm,
                **stats(values),
            })
    write_csv(args.output_dir / "transition_location_summary.csv", summary)

    for zoom in (False, True):
        fig, axes = plt.subplots(3, 2, figsize=(11.5, 10.2), sharex=zoom)
        for ax, transition_group, color in zip(axes.flat, GROUPS, COLORS):
            selected = [row for row in rows if row["transition_group"] == transition_group]
            lcio = np.asarray([row["lcio_residual_pct"] for row in selected])
            gsf = np.asarray([row["gsf_residual_pct"] for row in selected])
            if zoom:
                bins = np.linspace(-5, 5, 101)
            else:
                low, high = np.quantile(np.concatenate((lcio, gsf)), [0.005, 0.995])
                padding = max(0.25, 0.05 * (high - low))
                bins = np.linspace(low - padding, high + padding, 81)
            for label, values, linestyle in (("LCIO", lcio, "--"),
                                             (args.gsf_label, gsf, "-")):
                result = stats(values)
                ax.hist(values, bins=bins, histtype="step", linewidth=1.8,
                        color="#555555" if label == "LCIO" else color,
                        linestyle=linestyle,
                        label=(f"{label}: med {result['median_pct']:.3g}%, "
                               f"w68 {result['width68_pct']:.3g}%"))
            ax.axvline(0, color="black", linewidth=0.9, alpha=0.6)
            if zoom:
                ax.set_xlim(-5, 5)
            ax.set_title(f"Dominant transition {transition_group} (N={len(selected)})")
            ax.set_ylabel("Events / bin")
            ax.grid(axis="y", alpha=0.2)
            ax.legend(frameon=False, fontsize=8)
        for ax in axes[-1]:
            ax.set_xlabel(r"$(p_T^{reco}-p_T^{truth})/p_T^{truth}$ [%]")
        suffix = " (core zoom)" if zoom else ""
        fig.suptitle(
            f"Topology-clean light-eBrem: new {args.gsf_label} tuples\n"
            f"pT resolution by dominant surface-owned loss transition{suffix}")
        fig.tight_layout()
        stem = args.output_dir / "pt_resolution_by_transition_location"
        if zoom:
            stem = stem.with_name(stem.name + "_zoom_m5_5")
        fig.savefig(stem.with_suffix(".png"), dpi=180)
        fig.savefig(stem.with_suffix(".pdf"))
        plt.close(fig)

    print(f"Matched topology-clean light-eBrem events: {len(rows)}")
    print("Counts:", {g: sum(r["transition_group"] == g for r in rows) for g in GROUPS})
    print(args.output_dir / "transition_location_summary.csv")


if __name__ == "__main__":
    main()
