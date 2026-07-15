#!/usr/bin/env python3
"""Plot pT resolution and paired changes for the random light-eBrem sample."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--output-stem", type=Path, required=True)
    parser.add_argument("--baseline-components", type=int, default=12)
    parser.add_argument("--candidate-components", type=int, default=24)
    args = parser.parse_args()

    with args.input.open(newline="") as stream:
        rows = list(csv.DictReader(stream))
    baseline = np.asarray([float(row["baseline_residual_pct"]) for row in rows])
    candidate = np.asarray([float(row["candidate_residual_pct"]) for row in rows])
    absolute_gain = np.abs(baseline) - np.abs(candidate)
    improved = absolute_gain > 0.1
    worsened = absolute_gain < -0.1
    neutral = ~(improved | worsened)

    def summary(values: np.ndarray) -> tuple[float, float, int]:
        q16, median, q84 = np.quantile(values, [0.16, 0.5, 0.84])
        return median, 0.5 * (q84 - q16), int(np.count_nonzero(np.abs(values) <= 1))

    median12, width12, inside12 = summary(baseline)
    median24, width24, inside24 = summary(candidate)

    fig, (hist_ax, pair_ax) = plt.subplots(
        2, 1, figsize=(9.0, 10.0), gridspec_kw={"height_ratios": [1.05, 1.0]})
    edges = np.arange(-6.5, 4.75, 0.25)
    hist_ax.hist(
        baseline, bins=edges, histtype="step", linewidth=2.2,
        color="#4C78A8",
        label=(f"MaxComponents={args.baseline_components}: median {median12:+.3f}%, "
               f"68% half-width {width12:.3f}%, |r|≤1% {inside12}/100"))
    hist_ax.hist(
        candidate, bins=edges, histtype="step", linewidth=2.2,
        color="#F58518",
        label=(f"MaxComponents={args.candidate_components}: median {median24:+.3f}%, "
               f"68% half-width {width24:.3f}%, |r|≤1% {inside24}/100"))
    hist_ax.axvline(0.0, color="black", linewidth=1.0, alpha=0.65)
    hist_ax.axvspan(-1.0, 1.0, color="#54A24B", alpha=0.08,
                    label="±1% core")
    hist_ax.set(
        xlabel=r"$p_T$ residual $(p_T^{\mathrm{GSF}}/p_T^{\mathrm{truth}}-1)$ [%]",
        ylabel="Events / 0.25%",
        title=("Random topology-clean light-eBrem sample: "
               f"MaxComponents = {args.baseline_components} vs "
               f"{args.candidate_components}"))
    hist_ax.legend(frameon=False, fontsize=9)
    hist_ax.grid(axis="y", alpha=0.22)

    limits = (-6.5, 4.75)
    pair_ax.plot(limits, limits, color="black", linewidth=1.1,
                 linestyle="--", alpha=0.65, label="No change")
    pair_ax.scatter(baseline[neutral], candidate[neutral], s=30,
                    color="#9D9D9D", alpha=0.72,
                    label=f"|error| change ≤0.1 pp ({neutral.sum()})")
    pair_ax.scatter(baseline[improved], candidate[improved], s=48,
                    color="#2CA02C", edgecolor="white", linewidth=0.5,
                    label=f"Improved >0.1 pp ({improved.sum()})")
    pair_ax.scatter(baseline[worsened], candidate[worsened], s=52,
                    marker="X", color="#D62728", edgecolor="white",
                    linewidth=0.5, label=f"Worsened >0.1 pp ({worsened.sum()})")
    pair_ax.axhspan(-1.0, 1.0, color="#54A24B", alpha=0.06)
    pair_ax.axvspan(-1.0, 1.0, color="#54A24B", alpha=0.06)
    pair_ax.set_xlim(limits)
    pair_ax.set_ylim(limits)
    pair_ax.set_aspect("equal", adjustable="box")
    pair_ax.set(
        xlabel=f"Residual with MaxComponents={args.baseline_components} [%]",
        ylabel=f"Residual with MaxComponents={args.candidate_components} [%]",
        title="Paired event comparison")
    pair_ax.legend(frameon=False, fontsize=9, loc="upper left")
    pair_ax.grid(alpha=0.2)

    fig.text(
        0.99, 0.01,
        "N=100, uniform random draw from 2,132 topology-clean light-eBrem events; seed 20260713",
        ha="right", va="bottom", fontsize=8.5, color="#444444")
    fig.tight_layout(rect=(0, 0.025, 1, 1))
    args.output_stem.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(args.output_stem.with_suffix(".png"), dpi=200)
    fig.savefig(args.output_stem.with_suffix(".pdf"))
    plt.close(fig)
    print(args.output_stem.with_suffix(".png"))


if __name__ == "__main__":
    main()
