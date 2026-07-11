#!/usr/bin/env python3
"""Plot transition loss spectra with conditioned BH components overlaid."""

import argparse
import glob
import json
import math
import os

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scipy.special import ndtr


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", help="Transition CSVs or globs")
    parser.add_argument("--artifact", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    paths = []
    for pattern in args.inputs:
        matches = sorted(glob.glob(pattern))
        if not matches:
            parser.error("no input matches %s" % pattern)
        paths.extend(matches)
    paths = list(dict.fromkeys(paths))

    with open(args.artifact) as stream:
        artifact = json.load(stream)
    knots = artifact["knots"]
    tx0_edges = np.array([knots[0]["tx0_low"]] +
                         [knot["tx0_high"] for knot in knots])
    loss_edges = np.logspace(-8, 0, 121)
    counts = np.zeros((len(knots), len(loss_edges) - 1), dtype=np.int64)
    totals = np.zeros(len(knots), dtype=np.int64)
    below = np.zeros(len(knots), dtype=np.int64)

    for path in paths:
        for data in pd.read_csv(path, usecols=["g4_t_over_x0", "z"],
                                chunksize=200000):
            tx0 = data["g4_t_over_x0"].to_numpy()
            loss = 1.0 - data["z"].to_numpy()
            indices = np.searchsorted(tx0_edges, tx0, side="right") - 1
            valid = ((indices >= 0) & (indices < len(knots)) &
                     (loss >= 0.0) & (loss <= 1.0))
            for index in range(len(knots)):
                selected = loss[valid & (indices == index)]
                totals[index] += len(selected)
                below[index] += np.count_nonzero(selected < loss_edges[0])
                counts[index] += np.histogram(selected, bins=loss_edges)[0]

    centers = np.sqrt(loss_edges[:-1] * loss_edges[1:])
    colors = plt.cm.tab10(np.arange(5))
    figure, axes = plt.subplots(4, 2, figsize=(12, 14), sharex=True)
    for index, (axis, knot) in enumerate(zip(axes.flat, knots)):
        denominator = max(1, totals[index])
        observed = counts[index] / denominator
        axis.step(centers, observed, where="mid", color="black", linewidth=1.4,
                  label="G4 transitions")
        mixture = np.zeros_like(centers)
        for component in knot["components"]:
            weight = component["weight"]
            loss_mean = 1.0 - component["mean_z"]
            sigma = math.sqrt(component["variance_z"])
            probability = weight * (
                ndtr((loss_edges[1:] - loss_mean) / sigma) -
                ndtr((loss_edges[:-1] - loss_mean) / sigma))
            mixture += probability
            axis.plot(centers, probability, color=colors[component["component"]],
                      linewidth=1.0, label=component["loss_class"])
        axis.plot(centers, mixture, color="tab:red", linestyle="--",
                  linewidth=1.4, label="five-component sum")
        for boundary in (1e-4, 1e-2, 5e-2, 2e-1):
            axis.axvline(boundary, color="0.75", linestyle=":", linewidth=0.7)
        axis.set_xscale("log")
        axis.set_yscale("log")
        axis.set_ylim(1e-8, 1.0)
        axis.grid(alpha=0.2, which="both")
        axis.set_ylabel("probability / log bin")
        axis.set_title(r"$%.4g \leq t/X_0 < %.4g$  (%s transitions)" %
                       (knot["tx0_low"], knot["tx0_high"],
                        format(totals[index], ",")))
        axis.text(0.02, 0.04, "loss < 1e-8: %.2f%%" %
                  (100.0 * below[index] / denominator),
                  transform=axis.transAxes, fontsize=8)
    for axis in axes[-1]:
        axis.set_xlabel(r"fractional momentum loss $1-z$")
    handles, labels = axes[0, 0].get_legend_handles_labels()
    figure.legend(handles, labels, loc="upper center", ncol=4, fontsize=9,
                  bbox_to_anchor=(0.5, 0.944))
    figure.suptitle("CEPC 2 GeV, 85° conditioned transition spectra and BH components\n"
                   "Gaussian curves use extracted conditional means and variances",
                   fontsize=14)
    figure.tight_layout(rect=(0.0, 0.0, 1.0, 0.89))
    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
    figure.savefig(args.output, dpi=170)
    plt.close(figure)
    print("wrote %s from %d transitions" % (args.output, int(totals.sum())))


if __name__ == "__main__":
    main()
