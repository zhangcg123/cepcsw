#!/usr/bin/env python3
"""Plot the ActsAtlas BH implementation directly from its C++ coefficients."""

from __future__ import annotations

import argparse
import glob
import math
from pathlib import Path
import re

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scipy.special import ndtr


def coefficient_array(source: str, name: str) -> np.ndarray:
    match = re.search(
        rf"static const double {name}\[6\]\[3\]\[6\]\s*=\s*\{{(.*?)\n\}};",
        source, re.S)
    if not match:
        raise RuntimeError(f"Cannot find {name} in BetheHeitlerSplitter.cpp")
    numbers = [float(value) for value in re.findall(
        r"[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?", match.group(1))]
    if len(numbers) != 6 * 3 * 6:
        raise RuntimeError(f"Expected 108 values for {name}, found {len(numbers)}")
    return np.asarray(numbers).reshape(6, 3, 6)


def poly(x: float, coefficients: np.ndarray) -> float:
    value = 0.0
    for coefficient in coefficients:
        value = x * value + coefficient
    return value


def inv_logit(value: float) -> float:
    if value >= 0.0:
        exponential = math.exp(-value)
        return 1.0 / (1.0 + exponential)
    exponential = math.exp(value)
    return exponential / (1.0 + exponential)


def mixture(x: float, data: np.ndarray) -> np.ndarray:
    if x < 1.0e-4:
        return np.asarray([[1.0, 1.0, 0.0]])
    if x < 2.0e-3:
        mean = math.exp(-x)
        second = math.exp(-x * math.log(3.0) / math.log(2.0))
        return np.asarray([[1.0, mean, max(0.0, second - mean * mean)]])
    xx = min(x, 0.2)
    result = np.empty((6, 3))
    for component in range(6):
        result[component, 0] = inv_logit(poly(xx, data[component, 0]))
        result[component, 1] = inv_logit(poly(xx, data[component, 1]))
        result[component, 2] = math.exp(poly(xx, data[component, 2]))
    result[:, 0] /= np.sum(result[:, 0])
    return result


def plot_parameters(data: np.ndarray, output: Path) -> None:
    grid = np.geomspace(1.0e-6, 0.25, 800)
    weights = np.zeros((len(grid), 6))
    means = np.full((len(grid), 6), np.nan)
    sigmas = np.full((len(grid), 6), np.nan)
    for row, x in enumerate(grid):
        values = mixture(float(x), data)
        weights[row, :len(values)] = values[:, 0]
        means[row, :len(values)] = values[:, 1]
        sigmas[row, :len(values)] = np.sqrt(np.maximum(values[:, 2], 0.0))

    figure, axes = plt.subplots(3, 1, figsize=(10, 11), sharex=True)
    colors = plt.cm.tab10(np.arange(6))
    for component in range(6):
        label = f"component {component}"
        axes[0].plot(grid, weights[:, component], color=colors[component],
                     label=label)
        axes[1].plot(grid, means[:, component], color=colors[component])
        positive = np.where(sigmas[:, component] > 0.0,
                            sigmas[:, component], np.nan)
        axes[2].plot(grid, positive, color=colors[component])
    for axis in axes:
        axis.set_xscale("log")
        axis.grid(alpha=0.25, which="both")
        for boundary in (1.0e-4, 2.0e-3, 0.2):
            axis.axvline(boundary, color="0.45", linestyle=":", linewidth=0.9)
    axes[0].set_ylabel("component weight")
    axes[0].set_ylim(-0.03, 1.05)
    axes[0].legend(ncol=3, fontsize=8)
    axes[1].set_ylabel("mean retained fraction z")
    axes[2].set_ylabel("conditional sigma(z)")
    axes[2].set_yscale("log")
    axes[2].set_xlabel(r"transition $t/X_0$")
    figure.suptitle(
        "ActsAtlas Bethe-Heitler approximation from BetheHeitlerSplitter.cpp\n"
        "dotted lines: no-change, analytic/polynomial, and cap boundaries")
    figure.tight_layout(rect=(0, 0, 1, 0.95))
    output.parent.mkdir(parents=True, exist_ok=True)
    figure.savefig(output, dpi=180)
    figure.savefig(output.with_suffix(".pdf"))
    plt.close(figure)


def plot_spectra(data: np.ndarray, patterns: list[str], output: Path,
                 linear_x: bool = False) -> None:
    paths = []
    for pattern in patterns:
        matches = sorted(glob.glob(pattern))
        if not matches:
            raise RuntimeError(f"No transition files match {pattern!r}")
        paths.extend(matches)
    paths = list(dict.fromkeys(paths))
    tx0_edges = np.asarray([0.0, 1e-4, 5e-4, 2e-3, 5e-3, 1e-2,
                            1.5e-2, 2e-2, 3e-2])
    loss_edges = (np.linspace(0.0, 1.0, 121) if linear_x else
                  np.logspace(-8, 0, 121))
    counts = np.zeros((8, 120), dtype=np.int64)
    totals = np.zeros(8, dtype=np.int64)
    below = np.zeros(8, dtype=np.int64)
    for path in paths:
        columns = ["g4_t_over_x0", "p_before_GeV",
                   "ebrem_step_loss_sum_GeV", "n_ebrem_steps"]
        for frame in pd.read_csv(path, usecols=columns, chunksize=200000):
            tx0 = frame["g4_t_over_x0"].to_numpy()
            before = frame["p_before_GeV"].to_numpy()
            has_ebrem = frame["n_ebrem_steps"].to_numpy() > 0
            loss = np.where(
                has_ebrem & (before > 0.0),
                np.clip(frame["ebrem_step_loss_sum_GeV"].to_numpy() / before,
                        0.0, 1.0), 0.0)
            indices = np.searchsorted(tx0_edges, tx0, side="right") - 1
            valid = ((indices >= 0) & (indices < 8) &
                     np.isfinite(loss) & (loss >= 0.0) & (loss <= 1.0))
            for index in range(8):
                selected = loss[valid & (indices == index)]
                totals[index] += len(selected)
                below[index] += np.count_nonzero(selected < loss_edges[0])
                counts[index] += np.histogram(selected, bins=loss_edges)[0]

    centers = ((loss_edges[:-1] + loss_edges[1:]) / 2.0 if linear_x else
               np.sqrt(loss_edges[:-1] * loss_edges[1:]))
    colors = plt.cm.tab10(np.arange(6))
    figure, axes = plt.subplots(4, 2, figsize=(12, 14), sharex=True)
    for index, axis in enumerate(axes.flat):
        denominator = max(1, totals[index])
        observed = counts[index] / denominator
        axis.step(centers, observed, where="mid", color="black", linewidth=1.4,
                  label="G4 eBrem-attributed loss")
        low_edge, high_edge = tx0_edges[index:index + 2]
        center = high_edge / 2.0 if low_edge == 0.0 else math.sqrt(
            low_edge * high_edge)
        values = mixture(center, data)
        total_curve = np.zeros_like(centers)
        for component, (weight, mean, variance) in enumerate(values):
            loss_mean = 1.0 - mean
            sigma = math.sqrt(max(variance, 0.0))
            if sigma == 0.0:
                probability = np.zeros_like(centers)
                bin_index = max(0, np.searchsorted(
                    loss_edges, loss_mean, side="right") - 1)
                if 0 <= bin_index < len(probability):
                    probability[bin_index] = weight
            else:
                probability = weight * (
                    ndtr((loss_edges[1:] - loss_mean) / sigma) -
                    ndtr((loss_edges[:-1] - loss_mean) / sigma))
            total_curve += probability
            axis.plot(centers, probability, color=colors[component],
                      linewidth=1.0, label=f"component {component}")
        axis.plot(centers, total_curve, color="tab:red", linestyle="--",
                  linewidth=1.4, label="ActsAtlas sum")
        if not linear_x:
            axis.set_xscale("log")
        axis.set_yscale("log")
        axis.set_ylim(1e-8, 1.0)
        axis.grid(alpha=0.2, which="both")
        axis.set_ylabel("probability / bin" if linear_x else
                        "probability / log bin")
        axis.set_title(r"$%.4g \leq t/X_0 < %.4g$  (%s transitions)" %
                       (low_edge, high_edge, format(totals[index], ",")))
        if not linear_x:
            axis.text(0.02, 0.04, "loss < 1e-8: %.2f%%" %
                      (100.0 * below[index] / denominator),
                      transform=axis.transAxes, fontsize=8)
    for axis in axes[-1]:
        axis.set_xlabel(r"fractional momentum loss $1-z$")
    handles, labels = axes[0, 0].get_legend_handles_labels()
    figure.legend(handles, labels, loc="upper center", ncol=4, fontsize=8,
                  bbox_to_anchor=(0.5, 0.945))
    figure.suptitle(
        "CEPC 2 GeV, 85° Geant4 transitions versus ActsAtlas BH components\n"
        "ACTS parameters evaluated at each t/X0-bin geometric center",
        fontsize=14)
    figure.tight_layout(rect=(0, 0, 1, 0.89))
    output.parent.mkdir(parents=True, exist_ok=True)
    figure.savefig(output, dpi=180)
    figure.savefig(output.with_suffix(".pdf"))
    plt.close(figure)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--source", type=Path,
        default=Path("Reconstruction/RecGsfTracking/src/BetheHeitlerSplitter.cpp"))
    parser.add_argument(
        "--transitions", nargs="*",
        default=["BHModelComparisonStudies/CEPC2GeV85StepConditioned/production/"
                 "gsf_material_transitions-e--2.0-85-*.csv"])
    parser.add_argument(
        "--output-dir", type=Path,
        default=Path("Reconstruction/RecGsfTracking/data/ActsAtlas"))
    parser.add_argument("--linear-x", action="store_true",
                        help="Use uniform linear loss bins and a linear x-axis")
    args = parser.parse_args()
    source = args.source.read_text()
    data = coefficient_array(source, "actsData")
    plot_parameters(data, args.output_dir / "acts_atlas_bh_parameters.png")
    spectrum_name = ("acts_atlas_bh_spectrum_components_linear_x.png"
                     if args.linear_x else
                     "acts_atlas_bh_spectrum_components.png")
    plot_spectra(data, args.transitions, args.output_dir / spectrum_name,
                 args.linear_x)
    print(f"wrote ActsAtlas plots to {args.output_dir}")


if __name__ == "__main__":
    main()
