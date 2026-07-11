#!/usr/bin/env python3
"""Compare LCIO and GSF pT residuals by Geant4 tracker-eBrem category."""

from __future__ import annotations

import argparse
import csv
import math
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import ROOT


TRACKER_TOKENS = ("VXD", "ITK", "TPC", "OTK", "SIT", "SET")
CATEGORIES = ("no_ebrem", "light_ebrem", "hard_ebrem")
LABELS = {
    "no_ebrem": "No tracker eBrem",
    "light_ebrem": "Light tracker eBrem (<10%)",
    "hard_ebrem": "Hard tracker eBrem (>=10%)",
}


def tracker_volume(name: str) -> bool:
    return any(token in name for token in TRACKER_TOKENS)


def classify(tree, threshold: float) -> tuple[str, float, float, int]:
    retained_product = 1.0
    max_fraction = 0.0
    count = 0
    for i, subtype in enumerate(tree.process_subtype):
        if (int(subtype) != 3 or int(tree.track_id[i]) != 1
                or int(tree.parent_id[i]) != 0 or int(tree.pdg[i]) != 11
                or not tracker_volume(str(tree.pre_volume[i]))):
            continue
        pre_p = float(tree.pre_p[i])
        post_p = float(tree.post_p[i])
        if pre_p <= 0.0 or not math.isfinite(pre_p) or not math.isfinite(post_p):
            continue
        retained = max(0.0, min(1.0, post_p / pre_p))
        fraction = 1.0 - retained
        retained_product *= retained
        max_fraction = max(max_fraction, fraction)
        count += 1
    cumulative = 1.0 - retained_product if count else 0.0
    if count == 0:
        category = "no_ebrem"
    elif max_fraction >= threshold or cumulative >= threshold:
        category = "hard_ebrem"
    else:
        category = "light_ebrem"
    return category, max_fraction, cumulative, count


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--first-seed", type=int, default=1)
    parser.add_argument("--last-seed", type=int, default=100)
    parser.add_argument("--hard-threshold", type=float, default=0.10)
    parser.add_argument(
        "--output-dir", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots"))
    args = parser.parse_args()

    rows = []
    for seed in range(args.first_seed, args.last_seed + 1):
        step_path = f"gsf_material_steps-e--2.0-85-{seed}.root"
        flat_path = f"gsf_flat-e--2.0-85-{seed}.root"
        step_file = ROOT.TFile.Open(step_path)
        flat_file = ROOT.TFile.Open(flat_path)
        if not step_file or step_file.IsZombie() or not flat_file or flat_file.IsZombie():
            raise RuntimeError(f"Cannot open matched files for seed {seed}")
        step_tree = step_file.Get("g4step_tuple")
        flat_tree = flat_file.Get("gsf_tuple")
        if step_tree.GetEntries() != flat_tree.GetEntries():
            raise RuntimeError(
                f"Seed {seed}: step/flat entry mismatch "
                f"{step_tree.GetEntries()} != {flat_tree.GetEntries()}")
        for entry in range(step_tree.GetEntries()):
            step_tree.GetEntry(entry)
            flat_tree.GetEntry(entry)
            category, max_frac, cumulative_frac, count = classify(
                step_tree, args.hard_threshold)
            truth = float(flat_tree.mc_pT)
            lcio = float(flat_tree.lcio_pT)
            gsf = float(flat_tree.gsf_pT)
            if not all(math.isfinite(x) and x > 0.0 for x in (truth, lcio, gsf)):
                continue
            rows.append({
                "seed": seed, "entry": entry, "category": category,
                "ebrem_count": count, "max_frac": max_frac,
                "cumulative_frac": cumulative_frac,
                "lcio_residual_pct": 100.0 * (lcio - truth) / truth,
                "gsf_residual_pct": 100.0 * (gsf - truth) / truth,
            })
        step_file.Close()
        flat_file.Close()

    args.output_dir.mkdir(parents=True, exist_ok=True)
    stem = args.output_dir / "gsf_reverse_vs_lcio_pt_resolution_by_tracker_ebrem"
    with stem.with_name(stem.name + "_events.csv").open("w", newline="") as output:
        writer = csv.DictWriter(output, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)

    fig, axes = plt.subplots(1, 3, figsize=(16.5, 5.2))
    summary = []
    for ax, category in zip(axes, CATEGORIES):
        selected = [row for row in rows if row["category"] == category]
        lcio = np.asarray([row["lcio_residual_pct"] for row in selected])
        gsf = np.asarray([row["gsf_residual_pct"] for row in selected])
        combined = np.concatenate((lcio, gsf))
        lo, hi = np.quantile(combined, [0.005, 0.995])
        pad = max(0.1, 0.06 * (hi - lo))
        bins = np.linspace(lo - pad, hi + pad, 81)
        for name, values, color, linestyle in (
                ("LCIO", lcio, "#276FBF", "--"),
                ("GSF reverse mixture", gsf, "#D1495B", "-")):
            median, q16, q84 = np.quantile(values, [0.5, 0.16, 0.84])
            ax.hist(values, bins=bins, histtype="step", linewidth=1.8,
                    color=color, linestyle=linestyle, label=name)
            summary.append({
                "category": category, "sample": name, "count": len(values),
                "mean_pct": np.mean(values), "std_pct": np.std(values),
                "median_pct": median, "q16_pct": q16, "q84_pct": q84,
                "rms_pct": np.sqrt(np.mean(values * values)),
                "within_5pct": np.count_nonzero(np.abs(values) <= 5.0),
            })
        ax.axvline(0.0, color="black", linewidth=0.8, alpha=0.6)
        ax.set_title(f"{LABELS[category]}\nN = {len(selected)}")
        ax.set_xlabel(r"$(p_T^{reco}-p_T^{truth})/p_T^{truth}$ [%]")
        ax.grid(axis="y", alpha=0.2)
    axes[0].set_ylabel("Events / bin")
    axes[0].legend(frameon=False)
    fig.suptitle(r"2 GeV $p_T$ electrons, $\theta=85^\circ$: Geant4 tracker-eBrem categories")
    fig.tight_layout()
    fig.savefig(stem.with_suffix(".png"), dpi=180)
    fig.savefig(stem.with_suffix(".pdf"))

    zoom_fig, zoom_axes = plt.subplots(1, 3, figsize=(16.5, 5.2), sharex=True)
    zoom_bins = np.linspace(-5.0, 5.0, 101)
    for ax, category in zip(zoom_axes, CATEGORIES):
        selected = [row for row in rows if row["category"] == category]
        for name, field, color, linestyle in (
                ("LCIO", "lcio_residual_pct", "#276FBF", "--"),
                ("GSF reverse mixture", "gsf_residual_pct", "#D1495B", "-")):
            values = np.asarray([row[field] for row in selected])
            inside = np.count_nonzero(np.abs(values) <= 5.0)
            ax.hist(values, bins=zoom_bins, histtype="step", linewidth=1.8,
                    color=color, linestyle=linestyle,
                    label=f"{name} ({inside}/{len(values)})")
        ax.axvline(0.0, color="black", linewidth=0.8, alpha=0.6)
        ax.set_xlim(-5.0, 5.0)
        ax.set_title(f"{LABELS[category]}\nN = {len(selected)}")
        ax.set_xlabel(r"$(p_T^{reco}-p_T^{truth})/p_T^{truth}$ [%]")
        ax.grid(axis="y", alpha=0.2)
        ax.legend(frameon=False, fontsize=8)
    zoom_axes[0].set_ylabel("Events / 0.1% bin")
    zoom_fig.suptitle(
        r"2 GeV $p_T$ electrons, $\theta=85^\circ$: tracker-eBrem core zoom")
    zoom_fig.tight_layout()
    zoom_stem = stem.with_name(stem.name + "_zoom_m5_5")
    zoom_fig.savefig(zoom_stem.with_suffix(".png"), dpi=180)
    zoom_fig.savefig(zoom_stem.with_suffix(".pdf"))

    with stem.with_name(stem.name + "_summary.csv").open("w", newline="") as output:
        writer = csv.DictWriter(output, fieldnames=summary[0].keys())
        writer.writeheader()
        writer.writerows(summary)
    print("category counts:", {c: sum(r["category"] == c for r in rows) for c in CATEGORIES})
    print(stem.with_suffix(".png"))


if __name__ == "__main__":
    main()
