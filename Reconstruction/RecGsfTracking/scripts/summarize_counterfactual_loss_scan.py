#!/usr/bin/env python3
"""Summarize population counterfactual truth-versus-inward loss scans."""

from __future__ import annotations

import argparse
import csv
from collections import Counter
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


def read(path: Path) -> list[dict]:
    with path.open(newline="") as stream:
        return list(csv.DictReader(stream))


def write(path: Path, rows: list[dict]) -> None:
    with path.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input-dir", type=Path, required=True)
    args = parser.parse_args()
    samples = {name: read(args.input_dir / f"{name}_events.csv")
               for name in ("overshoot", "control")}
    summary = []
    for name, rows in samples.items():
        delta = np.asarray([
            float(row["delta_log_likelihood_truth_minus_inward"])
            for row in rows])
        truth_losses = np.asarray([
            float(row["best_truth_loss_fraction"]) for row in rows])
        inward_losses = np.asarray([
            float(row["best_inward_loss_fraction"]) for row in rows])
        summary.append({
            "sample": name, "count": len(rows),
            "truth_surface_wins": np.count_nonzero(delta > 0),
            "inward_surface_wins_or_ties": np.count_nonzero(delta <= 0),
            "median_delta_logL_truth_minus_inward": np.median(delta),
            "q16_delta_logL": np.quantile(delta, 0.16),
            "q84_delta_logL": np.quantile(delta, 0.84),
            "delta_logL_below_minus2": np.count_nonzero(delta < -2),
            "delta_logL_above_plus2": np.count_nonzero(delta > 2),
            "best_truth_loss_at_least_5pct":
                np.count_nonzero(truth_losses >= 0.05),
            "median_best_truth_loss_pct": 100 * np.median(truth_losses),
            "median_best_inward_loss_pct": 100 * np.median(inward_losses),
        })
    write(args.input_dir / "population_summary.csv", summary)

    fig, axes = plt.subplots(1, 2, figsize=(11.5, 4.8))
    colors = {"overshoot": "#D1495B", "control": "#4C78A8"}
    bins = np.linspace(-20, 20, 81)
    for name, rows in samples.items():
        delta = np.asarray([
            float(row["delta_log_likelihood_truth_minus_inward"])
            for row in rows])
        axes[0].hist(delta, bins=bins, histtype="step", linewidth=2,
                     color=colors[name], label=f"{name.capitalize()} (N={len(rows)})")
    axes[0].axvline(0, color="black", linewidth=1)
    axes[0].set(xlabel=r"$\Delta\log L$ (best truth surface − best inward)",
                ylabel="Events / bin", title="Surface preference")
    axes[0].legend(frameon=False)
    axes[0].grid(axis="y", alpha=0.2)

    losses = sorted({float(row["best_truth_loss_fraction"])
                     for rows in samples.values() for row in rows})
    x = np.arange(len(losses))
    width = 0.38
    for offset, name in zip((-0.5, 0.5), ("overshoot", "control")):
        counts = Counter(float(row["best_truth_loss_fraction"])
                         for row in samples[name])
        axes[1].bar(x + offset * width, [counts[loss] for loss in losses],
                    width, color=colors[name], label=name.capitalize())
    axes[1].set_xticks(x, [f"{100 * loss:g}" for loss in losses], rotation=45)
    axes[1].set(xlabel="Best truth-surface trial loss [%]", ylabel="Events",
                title="Preferred truth-surface magnitude")
    axes[1].legend(frameon=False)
    axes[1].grid(axis="y", alpha=0.2)
    fig.suptitle("MaxComp=12 transition 7–8 counterfactual loss scan")
    fig.tight_layout()
    fig.savefig(args.input_dir / "counterfactual_loss_scan_summary.png", dpi=180)
    fig.savefig(args.input_dir / "counterfactual_loss_scan_summary.pdf")
    plt.close(fig)
    for row in summary:
        print(row)


if __name__ == "__main__":
    main()
