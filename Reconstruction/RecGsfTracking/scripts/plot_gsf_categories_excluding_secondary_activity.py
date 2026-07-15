#!/usr/bin/env python3
"""Plot LCIO/GSF pT resolution by eBrem category after topology exclusion."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


CATEGORIES = [
    ("no_ebrem", "No owned eBrem"),
    ("light_ebrem", "Light owned eBrem (<10%)"),
    ("hard_ebrem", "Hard owned eBrem (≥10%)"),
]


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--events", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/full_500_sample_2026-07-12/surface_owned_ebrem_event_categories.csv"))
    parser.add_argument(
        "--topology", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/secondary_tracker_activity_2026-07-13/secondary_tracker_activity_event_ids.csv"))
    parser.add_argument(
        "--output-dir", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/secondary_tracker_activity_excluded_2026-07-13"))
    return parser.parse_args()


def load(args: argparse.Namespace):
    with args.topology.open(newline="") as stream:
        secondary_ids = {
            (int(row["seed"]), int(row["entry"]))
            for row in csv.DictReader(stream)
            if int(row["has_secondary_tracker_activity"])
        }
    retained = {key: [] for key, _ in CATEGORIES}
    excluded = {key: 0 for key, _ in CATEGORIES}
    with args.events.open(newline="") as stream:
        for row in csv.DictReader(stream):
            category = row["category"]
            if category not in retained:
                continue
            event_id = (int(row["seed"]), int(row["entry"]))
            if event_id in secondary_ids:
                excluded[category] += 1
                continue
            lcio = float(row["lcio_residual_pct"])
            gsf = float(row["gsf_residual_pct"])
            if np.isfinite(lcio) and np.isfinite(gsf):
                retained[category].append((lcio, gsf))
    return retained, excluded, secondary_ids


def statistics(values: np.ndarray):
    q16, median, q84 = np.quantile(values, [0.16, 0.5, 0.84])
    return {
        "count": len(values), "median": median, "q16": q16, "q84": q84,
        "width68": 0.5 * (q84 - q16),
        "rms": np.sqrt(np.mean(values * values)),
        "inside1": np.count_nonzero(np.abs(values) <= 1),
        "inside2": np.count_nonzero(np.abs(values) <= 2),
        "inside5": np.count_nonzero(np.abs(values) <= 5),
        "inside10": np.count_nonzero(np.abs(values) <= 10),
    }


def make_plot(output: Path, retained, excluded, zoom: bool) -> list[str]:
    fig, axes = plt.subplots(1, 3, figsize=(16.5, 5.3))
    summary = [
        "category,excluded_secondary_events,algorithm,count,median_pct,q16_pct,"
        "q84_pct,width68_pct,rms_pct,inside_1pct,inside_2pct,inside_5pct,inside_10pct"
    ]
    for ax, (category, title) in zip(axes, CATEGORIES):
        values = np.asarray(retained[category], dtype=float)
        series = [("LCIO", values[:, 0], "#276FBF", "--"),
                  ("GSF", values[:, 1], "#D1495B", "-")]
        if zoom:
            bins = np.linspace(-5, 5, 101)
        else:
            combined = values.ravel()
            low, high = np.quantile(combined, [0.001, 0.999])
            padding = max(0.25, 0.04 * (high - low))
            bins = np.linspace(low - padding, high + padding, 101)
        for algorithm, residuals, color, linestyle in series:
            stats = statistics(residuals)
            ax.hist(residuals, bins=bins, histtype="step", linewidth=2,
                    color=color, linestyle=linestyle,
                    label=(f"{algorithm}: median {stats['median']:.3g}%, "
                           f"width₆₈ {stats['width68']:.3g}%"))
            if not zoom:
                summary.append(
                    f"{category},{excluded[category]},{algorithm},{stats['count']},"
                    f"{stats['median']:.9g},{stats['q16']:.9g},{stats['q84']:.9g},"
                    f"{stats['width68']:.9g},{stats['rms']:.9g},{stats['inside1']},"
                    f"{stats['inside2']},{stats['inside5']},{stats['inside10']}")
        ax.axvline(0, color="black", linewidth=1, alpha=0.6)
        if zoom:
            ax.set_xlim(-5, 5)
        ax.set_title(f"{title}\nN = {len(values)}; excluded = {excluded[category]}")
        ax.set_xlabel(r"$(p_T^{\mathrm{reco}}-p_T^{\mathrm{truth}})/p_T^{\mathrm{truth}}$ [%]")
        ax.grid(axis="y", alpha=0.2)
        ax.legend(frameon=False, fontsize=8.5)
    axes[0].set_ylabel("Events / bin")
    suffix = " (core zoom)" if zoom else ""
    fig.suptitle(
        r"2 GeV $p_T$ electrons, $\theta=85^\circ$: surface-owned Geant4 eBrem categories"
        + "\nEvents with non-primary tracker SimHits excluded" + suffix)
    fig.tight_layout()
    fig.savefig(output.with_suffix(".png"), dpi=180)
    fig.savefig(output.with_suffix(".pdf"))
    plt.close(fig)
    return summary


def main() -> None:
    args = arguments()
    retained, excluded, secondary_ids = load(args)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    stem = args.output_dir / "gsf_vs_lcio_pt_resolution_by_surface_owned_ebrem"
    summary = make_plot(stem, retained, excluded, zoom=False)
    make_plot(stem.with_name(stem.name + "_zoom_m5_5"), retained, excluded, zoom=True)
    stem.with_name(stem.name + "_summary.csv").write_text("\n".join(summary) + "\n")
    matched_excluded = sum(excluded.values())
    print(f"Topology IDs: {len(secondary_ids)}; excluded matched events: {matched_excluded}")
    for category, _ in CATEGORIES:
        print(f"{category}: retained={len(retained[category])} excluded={excluded[category]}")
    print(stem.with_suffix(".png"))


if __name__ == "__main__":
    main()
