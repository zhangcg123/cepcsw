#!/usr/bin/env python3
"""Build a constrained interpolated CEPC step-conditioned BH artifact.

The input is a fixed-stratum table emitted by
extract_cepc_step_conditioned_mixture.py.  The artifact stores one knot per
t/X0 bin and a zero-material anchor.  Evaluation is linear from zero to the
first knot, then linear in log(t/X0), using:

  * additive-log-ratio coordinates for normalized nonnegative weights;
  * logits for component retained-fraction means; and
  * logarithms for positive conditional variances.

This is deliberately an inspectable execution model, not a general or
independently validated Bethe-Heitler parameterization.
"""

import argparse
import csv
import json
import math
import os
import sys

import matplotlib.pyplot as plt
import numpy as np


MODEL_NAME = "CEPC2GeV85StepConditioned"
WEIGHT_FLOOR = 1e-12
MEAN_EPSILON = 1e-9
VARIANCE_FLOOR = 1e-12


def logit(value):
    value = min(1.0 - MEAN_EPSILON, max(MEAN_EPSILON, value))
    return math.log(value / (1.0 - value))


def inv_logit(value):
    if value >= 0.0:
        e = math.exp(-value)
        return 1.0 / (1.0 + e)
    e = math.exp(value)
    return e / (1.0 + e)


def normalize(weights):
    values = np.maximum(np.asarray(weights, dtype=float), 0.0)
    total = float(values.sum())
    if not math.isfinite(total) or total <= 0.0:
        raise ValueError("component weights have no finite positive mass")
    return values / total


def read_table(path):
    by_center = {}
    with open(path, newline="") as stream:
        for row in csv.DictReader(stream):
            center = float(row["tx0_center"])
            by_center.setdefault(center, []).append({
                "component": int(row["component"]),
                "loss_class": row["loss_class"],
                "is_identity": row["loss_class"] == "no_ebrem",
                "count": int(row["count"]),
                "weight": float(row["weight"]),
                "mean_z": float(row["mean_z"]),
                "variance_z": float(row["variance_z"]),
                "tx0_low": float(row["tx0_low"]),
                "tx0_high": float(row["tx0_high"]),
            })
    knots = []
    for center, rows in sorted(by_center.items()):
        rows.sort(key=lambda item: item["component"])
        if [item["component"] for item in rows] != list(range(len(rows))):
            raise ValueError("components are incomplete at t/X0=%g" % center)
        weights = normalize([item["weight"] for item in rows])
        knots.append({
            "tx0": center,
            "tx0_low": rows[0]["tx0_low"],
            "tx0_high": rows[0]["tx0_high"],
            "total_count": sum(item["count"] for item in rows),
            "components": [{
                "component": item["component"],
                "loss_class": item["loss_class"],
                "is_identity": item["is_identity"],
                "count": item["count"],
                "weight": float(weights[index]),
                "mean_z": min(1.0, max(0.0, item["mean_z"])),
                "variance_z": max(VARIANCE_FLOOR, item["variance_z"]),
            } for index, item in enumerate(rows)],
        })
    if not knots:
        raise ValueError("empty mixture table")
    return knots


def interpolate_components(knots, tx0):
    n_components = len(knots[0]["components"])
    if tx0 <= 0.0:
        return [{"weight": 1.0 if i == 0 else 0.0,
                 "mean_z": 1.0, "variance_z": VARIANCE_FLOOR}
                for i in range(n_components)]

    first = knots[0]
    if tx0 < first["tx0"]:
        fraction = tx0 / first["tx0"]
        first_weights = np.array([c["weight"] for c in first["components"]])
        weights = normalize((1.0 - fraction) * np.array([1.0] +
                            [0.0] * (n_components - 1)) +
                            fraction * first_weights)
        result = []
        for i, component in enumerate(first["components"]):
            if component.get("is_identity", False):
                mean = 1.0
                variance = VARIANCE_FLOOR
            else:
                mean = 1.0 - fraction * (1.0 - component["mean_z"])
                variance = VARIANCE_FLOOR * math.exp(
                    fraction * math.log(component["variance_z"] /
                                        VARIANCE_FLOOR))
            result.append({"weight": float(weights[i]), "mean_z": mean,
                           "variance_z": variance})
        return result

    if tx0 >= knots[-1]["tx0"]:
        lower = upper = knots[-1]
        fraction = 0.0
    else:
        upper_index = next(i for i, knot in enumerate(knots)
                           if knot["tx0"] >= tx0)
        lower, upper = knots[upper_index - 1], knots[upper_index]
        fraction = ((math.log(tx0) - math.log(lower["tx0"])) /
                    (math.log(upper["tx0"]) - math.log(lower["tx0"])))

    if lower is upper:
        return [{"weight": c["weight"], "mean_z": c["mean_z"],
                 "variance_z": c["variance_z"]}
                for c in lower["components"]]

    # Additive log ratios relative to component zero preserve normalization.
    def weight_coordinates(knot):
        weights = np.maximum([c["weight"] for c in knot["components"]],
                             WEIGHT_FLOOR)
        return np.log(weights[1:] / weights[0])

    coordinates = ((1.0 - fraction) * weight_coordinates(lower) +
                   fraction * weight_coordinates(upper))
    unnormalized = np.concatenate(([1.0], np.exp(coordinates)))
    weights = normalize(unnormalized)
    result = []
    for i, (left, right) in enumerate(zip(lower["components"],
                                          upper["components"])):
        if left.get("is_identity", False) and right.get("is_identity", False):
            mean = 1.0
            variance = VARIANCE_FLOOR
        else:
            mean = inv_logit((1.0 - fraction) * logit(left["mean_z"]) +
                             fraction * logit(right["mean_z"]))
            variance = math.exp(
                (1.0 - fraction) * math.log(left["variance_z"]) +
                fraction * math.log(right["variance_z"]))
        result.append({"weight": float(weights[i]), "mean_z": mean,
                       "variance_z": variance})
    return result


def write_artifact(path, source, knots, model_name):
    artifact = {
        "model": model_name,
        "scope": {"particle": "electron", "pt_GeV": 2.0,
                  "theta_deg": 85.0, "max_tx0": 0.03},
        "variable": "z=1-ebrem_step_loss_sum/p_before",
        "status": "same-sample execution artifact; not physics validated",
        "source_table": os.path.abspath(source),
        "interpolation": {
            "zero_to_first_knot": "linear in physical parameters",
            "above_first_knot": "linear in log(t/X0)",
            "weights": "additive log ratio relative to component 0",
            "means": "logit(mean_z)",
            "variances": "log(variance_z)",
            "above_last_knot": "constant extrapolation",
            "weight_floor_for_transform": WEIGHT_FLOOR,
            "mean_epsilon_for_transform": MEAN_EPSILON,
            "variance_floor": VARIANCE_FLOOR,
        },
        "zero_material_components": [
            {"component": i, "weight": 1.0 if i == 0 else 0.0,
             "mean_z": 1.0, "variance_z": VARIANCE_FLOOR}
            for i in range(len(knots[0]["components"]))],
        "knots": knots,
    }
    with open(path, "w") as stream:
        json.dump(artifact, stream, indent=2, sort_keys=True)
        stream.write("\n")


def write_dense_table(path, knots, points=240):
    positive = np.geomspace(knots[0]["tx0"] / 100.0,
                            knots[-1]["tx0"], points)
    grid = np.concatenate(([0.0], positive))
    fields = ["tx0", "component", "weight", "mean_z", "variance_z",
              "sigma_z"]
    with open(path, "w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        for tx0 in grid:
            for component, item in enumerate(interpolate_components(knots, tx0)):
                writer.writerow({"tx0": tx0, "component": component,
                                 **item,
                                 "sigma_z": math.sqrt(item["variance_z"])})


def write_plot(path, knots, model_name):
    grid = np.concatenate(([0.0], np.geomspace(knots[0]["tx0"] / 100.0,
                                               knots[-1]["tx0"], 400)))
    evaluated = [interpolate_components(knots, value) for value in grid]
    n_components = len(knots[0]["components"])
    colors = plt.cm.tab10(np.arange(n_components))
    figure, axes = plt.subplots(3, 1, figsize=(10, 11), sharex=True)
    labels = [component["loss_class"] for component in knots[0]["components"]]
    for component in range(n_components):
        axes[0].plot(grid[1:], [row[component]["weight"] for row in evaluated[1:]],
                     color=colors[component], label=labels[component])
        axes[1].plot(grid[1:], [row[component]["mean_z"] for row in evaluated[1:]],
                     color=colors[component])
        axes[2].plot(grid[1:], [math.sqrt(row[component]["variance_z"])
                                for row in evaluated[1:]], color=colors[component])
        centers = [knot["tx0"] for knot in knots]
        components = [knot["components"][component] for knot in knots]
        axes[0].scatter(centers, [item["weight"] for item in components],
                        color=colors[component], s=22)
        axes[1].scatter(centers, [item["mean_z"] for item in components],
                        color=colors[component], s=22)
        axes[2].scatter(centers, [math.sqrt(item["variance_z"])
                                 for item in components],
                        color=colors[component], s=22)
    for axis in axes:
        axis.set_xscale("log")
        axis.grid(alpha=0.25, which="both")
    axes[0].set_ylabel("component weight")
    axes[0].set_ylim(-0.02, 1.02)
    axes[0].legend(ncol=3, fontsize=9)
    axes[1].set_ylabel("mean retained fraction z")
    axes[1].set_ylim(0.25, 1.02)
    axes[2].set_ylabel("conditional sigma(z)")
    axes[2].set_yscale("log")
    axes[2].set_xlabel("transition t/X0")
    figure.suptitle(model_name + " constrained interpolation\n"
                    "points are extracted strata; lines are artifact evaluation")
    figure.tight_layout(rect=(0.0, 0.0, 1.0, 0.96))
    figure.savefig(path, dpi=170)
    plt.close(figure)


def validate(knots):
    grid = np.concatenate(([0.0], np.geomspace(1e-10, 0.03, 2000)))
    maximum_weight_error = 0.0
    for tx0 in grid:
        components = interpolate_components(knots, tx0)
        weights = np.array([item["weight"] for item in components])
        means = np.array([item["mean_z"] for item in components])
        variances = np.array([item["variance_z"] for item in components])
        maximum_weight_error = max(maximum_weight_error,
                                   abs(float(weights.sum()) - 1.0))
        if (not np.all(np.isfinite(weights)) or np.any(weights < 0.0) or
                not np.all(np.isfinite(means)) or np.any(means <= 0.0) or
                np.any(means > 1.0) or not np.all(np.isfinite(variances)) or
                np.any(variances <= 0.0)):
            raise ValueError("constraint failure at t/X0=%g" % tx0)
    near_zero = interpolate_components(knots, 1e-12)
    tail_probability = sum(item["weight"] for item in near_zero[1:])
    if tail_probability > 1e-6:
        raise ValueError("tail does not vanish near zero: %g" % tail_probability)
    return maximum_weight_error, tail_probability


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", help="Fixed-stratum mixture CSV")
    parser.add_argument("--output-dir", required=True)
    parser.add_argument("--model-name", default=MODEL_NAME)
    parser.add_argument("--output-stem", default="cepc2gev85_step_conditioned")
    args = parser.parse_args()
    try:
        knots = read_table(args.input)
        os.makedirs(args.output_dir, exist_ok=True)
        artifact = os.path.join(args.output_dir, args.output_stem + ".json")
        dense = os.path.join(args.output_dir, args.output_stem + "_dense.csv")
        plot = os.path.join(args.output_dir, args.output_stem + ".png")
        write_artifact(artifact, args.input, knots, args.model_name)
        write_dense_table(dense, knots)
        write_plot(plot, knots, args.model_name)
        weight_error, tail_probability = validate(knots)
    except Exception as error:
        print("error: %s" % error, file=sys.stderr)
        return 1
    print("model: %s" % args.model_name)
    print("knots: %d; components: %d" %
          (len(knots), len(knots[0]["components"])))
    print("maximum normalization error: %.3g" % weight_error)
    print("tail probability at t/X0=1e-12: %.3g" % tail_probability)
    print("artifact: %s" % artifact)
    print("dense table: %s" % dense)
    print("plot: %s" % plot)
    return 0


if __name__ == "__main__":
    sys.exit(main())
