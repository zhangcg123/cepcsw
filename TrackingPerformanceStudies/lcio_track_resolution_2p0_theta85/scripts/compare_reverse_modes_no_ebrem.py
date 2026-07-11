#!/usr/bin/env python3
"""Compare LCIO, reverse mixture, and reverse best branch by eBrem category."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path
import re

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import ROOT


EVENTS = Path(
    "TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/"
    "gsf_reverse_vs_lcio_pt_resolution_by_tracker_ebrem_events.csv")
def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--category", default="no_ebrem",
        choices=("no_ebrem", "light_ebrem", "hard_ebrem"))
    parser.add_argument("--best-dir", type=Path)
    args = parser.parse_args()
    best_dir = args.best_dir or Path(
        f"/tmp/gsf-bestbranch-{args.category.replace('_', '-')}")
    out = Path(
        "TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/"
        f"{args.category}_reverse_bestbranch_vs_mixture")

    rows = []
    with EVENTS.open() as source:
        for row in csv.DictReader(source):
            if row["category"] == args.category:
                rows.append(row)

    files = {}
    successful_entries = {}
    successful_rows = []
    failed = []
    truth_values = []
    best = []
    for row in rows:
        seed = int(row["seed"])
        entry = int(row["entry"])
        if seed not in files:
            root_file = ROOT.TFile.Open(str(best_dir / f"gsf-flat-best-{seed}.root"))
            if not root_file or root_file.IsZombie():
                raise RuntimeError(f"Cannot open best-branch tuple for seed {seed}")
            files[seed] = (root_file, root_file.Get("gsf_tuple"))
            successful_entries[seed] = set()
            current_entry = None
            for line in (best_dir / f"seed-{seed}.log").read_text().splitlines():
                match = re.search(r"GSF event index (\d+)", line)
                if match:
                    current_entry = int(match.group(1))
                    continue
                match = re.search(r"Fitted: (\d+) /", line)
                if match and current_entry is not None:
                    if int(match.group(1)) > 0:
                        successful_entries[seed].add(current_entry)
                    current_entry = None
        if entry not in successful_entries[seed]:
            failed.append((seed, entry))
            continue
        tree = files[seed][1]
        tree.GetEntry(entry)
        truth = float(tree.mc_pT)
        gsf_pt = float(tree.gsf_pT)
        if truth <= 0.0 or gsf_pt <= 0.0:
            raise RuntimeError(f"Missing best-branch output at seed {seed}, entry {entry}")
        successful_rows.append(row)
        truth_values.append(truth)
        best.append(100.0 * (gsf_pt - truth) / truth)
    for root_file, _ in files.values():
        root_file.Close()

    values = {
        "LCIO": np.asarray(
            [float(row["lcio_residual_pct"]) for row in successful_rows]),
        "Reverse weighted mixture": np.asarray(
            [float(row["gsf_residual_pct"]) for row in successful_rows]),
        "Reverse best branch": np.asarray(best),
    }
    summary = []
    for name, residual in values.items():
        median, q16, q84 = np.quantile(residual, [0.5, 0.16, 0.84])
        summary.append({
            "sample": name, "count": len(residual),
            "mean_pct": np.mean(residual), "std_pct": np.std(residual),
            "median_pct": median, "q16_pct": q16, "q84_pct": q84,
            "central68_width_pct": q84 - q16,
            "rms_pct": np.sqrt(np.mean(residual * residual)),
            "within_1pct": np.count_nonzero(np.abs(residual) <= 1.0),
            "within_5pct": np.count_nonzero(np.abs(residual) <= 5.0),
        })
    with out.with_name(out.name + "_summary.csv").open("w", newline="") as output:
        writer = csv.DictWriter(output, fieldnames=summary[0].keys())
        writer.writeheader()
        writer.writerows(summary)

    fig, axes = plt.subplots(1, 2, figsize=(13.5, 5.2))
    styles = (
        ("LCIO", "#276FBF", "--"),
        ("Reverse weighted mixture", "#D1495B", "-"),
        ("Reverse best branch", "#2A9D8F", "-.")
    )
    combined = np.concatenate(list(values.values()))
    lo, hi = np.quantile(combined, [0.002, 0.998])
    broad_bins = np.linspace(lo, hi, 101)
    zoom_bins = np.linspace(-5.0, 5.0, 101)
    for name, color, linestyle in styles:
        axes[0].hist(values[name], bins=broad_bins, histtype="step",
                     linewidth=1.8, color=color, linestyle=linestyle, label=name)
        axes[1].hist(values[name], bins=zoom_bins, histtype="step",
                     linewidth=1.8, color=color, linestyle=linestyle, label=name)
    for ax in axes:
        ax.axvline(0.0, color="black", linewidth=0.8, alpha=0.6)
        ax.set_xlabel(r"$(p_T^{reco}-p_T^{truth})/p_T^{truth}$ [%]")
        ax.set_ylabel("Events / bin")
        ax.grid(axis="y", alpha=0.2)
    axes[0].set_title("Broad range")
    axes[1].set_title("Core zoom")
    axes[1].set_xlim(-5.0, 5.0)
    axes[1].legend(frameon=False, fontsize=9)
    category_label = args.category.replace("_", " ").title()
    fig.suptitle(
        rf"{category_label}: {len(successful_rows)}/{len(rows)} successful "
        rf"2 GeV $p_T$ electrons")
    fig.tight_layout()
    fig.savefig(out.with_suffix(".png"), dpi=180)
    fig.savefig(out.with_suffix(".pdf"))

    truth_values = np.asarray(truth_values)
    pt_values = {
        name: truth_values * (1.0 + residual / 100.0)
        for name, residual in values.items()
    }
    pt_fig, pt_axes = plt.subplots(1, 2, figsize=(13.5, 5.2))
    broad_pt_bins = np.linspace(0.0, 4.0, 101)
    zoom_pt_bins = np.linspace(1.8, 2.2, 101)
    for name, color, linestyle in styles:
        pt_axes[0].hist(pt_values[name], bins=broad_pt_bins, histtype="step",
                        linewidth=1.8, color=color, linestyle=linestyle,
                        label=name)
        pt_axes[1].hist(pt_values[name], bins=zoom_pt_bins, histtype="step",
                        linewidth=1.8, color=color, linestyle=linestyle,
                        label=name)
    truth_pt = float(np.median(truth_values))
    for ax in pt_axes:
        ax.axvline(truth_pt, color="black", linewidth=1.0,
                   label=f"Truth pT = {truth_pt:.4f} GeV")
        ax.set_xlabel(r"Reconstructed $p_T$ [GeV]")
        ax.set_ylabel("Events / bin")
        ax.grid(axis="y", alpha=0.2)
    pt_axes[0].set_title("Broad range")
    pt_axes[1].set_title("2 GeV zoom")
    pt_axes[1].set_xlim(1.8, 2.2)
    pt_axes[1].legend(frameon=False, fontsize=9)
    pt_fig.suptitle(
        rf"{category_label}: {len(successful_rows)}/{len(rows)} successful events")
    pt_fig.tight_layout()
    pt_out = out.with_name(out.name + "_pt")
    pt_fig.savefig(pt_out.with_suffix(".png"), dpi=180)
    pt_fig.savefig(pt_out.with_suffix(".pdf"))
    for row in summary:
        print(row)
    print("failed seed/entry:", failed)


if __name__ == "__main__":
    main()
