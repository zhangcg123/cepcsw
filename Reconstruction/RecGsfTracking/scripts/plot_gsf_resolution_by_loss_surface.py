#!/usr/bin/env python3
"""Plot default reverse-GSF pT residuals by dominant loss transition."""

import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


GROUPS = ("0-2", "3-4", "5-6", "7-8", "9-11", ">11")
COLORS = ("#4c78a8", "#f58518", "#54a24b", "#e45756", "#b279a2",
          "#72b7b2")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--input", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/light_outcomes_by_dominant_loss_surface.csv"))
    parser.add_argument(
        "--output-dir", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/loss_surface_dependency_2026-07-13"))
    parser.add_argument("--xmin", type=float, default=-15.0)
    parser.add_argument("--xmax", type=float, default=15.0)
    parser.add_argument("--bin-width", type=float, default=0.5)
    args = parser.parse_args()

    values = {group: {"gsf": [], "lcio": []} for group in GROUPS}
    with args.input.open(newline="") as stream:
        for row in csv.DictReader(stream):
            source_group = row["dominant_transition_bin"]
            group = ">11" if source_group in {
                "12-49", "50-99", "100-149", "150-199", "200-9999"
            } else source_group
            if group in values:
                values[group]["gsf"].append(float(row["gsf_residual_pct"]))
                values[group]["lcio"].append(float(row["lcio_residual_pct"]))

    edges = np.arange(args.xmin, args.xmax + args.bin_width * 0.5,
                      args.bin_width)
    args.output_dir.mkdir(parents=True, exist_ok=True)

    fig, ax = plt.subplots(figsize=(9.2, 6.2))
    for group, color in zip(GROUPS, COLORS):
        data = np.asarray(values[group]["gsf"])
        ax.hist(data, bins=edges, density=True, histtype="step", linewidth=1.8,
                color=color, label=f"transitions {group} (N={data.size})")
    ax.axvline(0, color="black", linewidth=1, alpha=0.6)
    ax.set(xlabel=r"GSF $p_T$ residual $(p_T^{GSF}/p_T^{truth}-1)$ [%]",
           ylabel="Normalized density",
           title="Topology-clean light-eBrem: default GSF by loss surface")
    ax.legend(frameon=False, fontsize=9)
    ax.grid(alpha=0.2)
    fig.tight_layout()
    fig.savefig(args.output_dir / "gsf_pt_resolution_by_loss_surface_overlay.png",
                dpi=180)
    fig.savefig(args.output_dir / "gsf_pt_resolution_by_loss_surface_overlay.pdf")
    plt.close(fig)

    fig, axes = plt.subplots(3, 2, figsize=(11, 10), sharex=True)
    for ax, group, color in zip(axes.flat, GROUPS, COLORS):
        data = np.asarray(values[group]["gsf"])
        lcio = np.asarray(values[group]["lcio"])
        inside = data[(data >= args.xmin) & (data <= args.xmax)]
        ax.hist(lcio, bins=edges, histtype="step", linewidth=1.6,
                color="#555555", linestyle="--", label="LCIO")
        ax.hist(data, bins=edges, histtype="stepfilled", alpha=0.45,
                color=color, edgecolor=color, linewidth=1.4, label="GSF")
        ax.axvline(0, color="black", linewidth=1, alpha=0.6)
        ax.set_title(f"Transitions {group}: N={data.size}, in range={inside.size}")
        ax.set_ylabel("Events / 0.5%")
        ax.grid(alpha=0.2)
        ax.legend(frameon=False, fontsize=9)
    for ax in axes[-1, :]:
        ax.set_xlabel(r"$p_T$ residual [%]")
    fig.suptitle("Topology-clean light-eBrem: default GSF by loss surface")
    fig.tight_layout()
    fig.savefig(args.output_dir / "gsf_pt_resolution_by_loss_surface_panels.png",
                dpi=180)
    fig.savefig(args.output_dir / "gsf_pt_resolution_by_loss_surface_panels.pdf")
    plt.close(fig)

    summary = args.output_dir / "gsf_pt_resolution_by_loss_surface_summary.csv"
    with summary.open("w", newline="") as stream:
        fields = ("transition_bin", "count", "gsf_median_pct", "gsf_q16_pct",
                  "gsf_q84_pct", "gsf_inside_1pct", "gsf_inside_2pct",
                  "gsf_inside_5pct", "lcio_median_pct", "lcio_q16_pct",
                  "lcio_q84_pct", "lcio_inside_1pct", "lcio_inside_2pct",
                  "lcio_inside_5pct", "gsf_outside_plot_range",
                  "lcio_outside_plot_range")
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        for group in GROUPS:
            data = np.asarray(values[group]["gsf"])
            lcio = np.asarray(values[group]["lcio"])
            writer.writerow({
                "transition_bin": group, "count": data.size,
                "gsf_median_pct": np.median(data),
                "gsf_q16_pct": np.quantile(data, 0.16),
                "gsf_q84_pct": np.quantile(data, 0.84),
                "gsf_inside_1pct": np.count_nonzero(np.abs(data) < 1),
                "gsf_inside_2pct": np.count_nonzero(np.abs(data) < 2),
                "gsf_inside_5pct": np.count_nonzero(np.abs(data) < 5),
                "lcio_median_pct": np.median(lcio),
                "lcio_q16_pct": np.quantile(lcio, 0.16),
                "lcio_q84_pct": np.quantile(lcio, 0.84),
                "lcio_inside_1pct": np.count_nonzero(np.abs(lcio) < 1),
                "lcio_inside_2pct": np.count_nonzero(np.abs(lcio) < 2),
                "lcio_inside_5pct": np.count_nonzero(np.abs(lcio) < 5),
                "gsf_outside_plot_range": np.count_nonzero(
                    (data < args.xmin) | (data > args.xmax)),
                "lcio_outside_plot_range": np.count_nonzero(
                    (lcio < args.xmin) | (lcio > args.xmax)),
            })
    print(summary.read_text(), end="")


if __name__ == "__main__":
    main()
