#!/usr/bin/env python3
"""Compare two extracted step-conditioned mixture tables statistically."""

import argparse
import csv
import math
from collections import defaultdict


def read_table(path):
    bins = defaultdict(list)
    with open(path, newline="") as stream:
        for row in csv.DictReader(stream):
            item = {key: row[key] for key in row}
            for key in ("tx0_low", "tx0_high", "tx0_center", "weight",
                        "mean_z", "variance_z"):
                item[key] = float(item[key])
            for key in ("component", "count"):
                item[key] = int(item[key])
            bins[(item["tx0_low"], item["tx0_high"])].append(item)
    return {key: sorted(value, key=lambda row: row["component"])
            for key, value in bins.items()}


def ratio(numerator, denominator):
    return numerator / denominator if denominator else float("nan")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reference")
    parser.add_argument("comparison")
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    reference = read_table(args.reference)
    comparison = read_table(args.comparison)
    if reference.keys() != comparison.keys():
        raise RuntimeError("mixture tables have different t/X0 bins")

    fields = ("tx0_low", "tx0_high", "tx0_center", "component",
              "loss_class", "reference_count", "comparison_count",
              "reference_total", "comparison_total", "reference_weight",
              "comparison_weight", "weight_difference", "weight_z_score",
              "reference_mean_z", "comparison_mean_z", "mean_z_difference",
              "mean_z_z_score")
    output = []
    for key in sorted(reference):
        left, right = reference[key], comparison[key]
        if len(left) != len(right):
            raise RuntimeError("component count differs in bin %r" % (key,))
        n_left = sum(row["count"] for row in left)
        n_right = sum(row["count"] for row in right)
        for a, b in zip(left, right):
            if a["component"] != b["component"]:
                raise RuntimeError("component ordering differs in bin %r" %
                                   (key,))
            weight_variance = (a["weight"] * (1.0 - a["weight"]) / n_left +
                               b["weight"] * (1.0 - b["weight"]) / n_right)
            mean_variance = (ratio(a["variance_z"], a["count"]) +
                             ratio(b["variance_z"], b["count"]))
            weight_difference = b["weight"] - a["weight"]
            mean_difference = b["mean_z"] - a["mean_z"]
            output.append({
                "tx0_low": key[0], "tx0_high": key[1],
                "tx0_center": a["tx0_center"],
                "component": a["component"], "loss_class": a["loss_class"],
                "reference_count": a["count"],
                "comparison_count": b["count"],
                "reference_total": n_left, "comparison_total": n_right,
                "reference_weight": a["weight"],
                "comparison_weight": b["weight"],
                "weight_difference": weight_difference,
                "weight_z_score": ratio(weight_difference,
                                         math.sqrt(weight_variance)),
                "reference_mean_z": a["mean_z"],
                "comparison_mean_z": b["mean_z"],
                "mean_z_difference": mean_difference,
                "mean_z_z_score": ratio(mean_difference,
                                         math.sqrt(mean_variance)),
            })

    with open(args.output, "w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        writer.writerows(output)

    print("tx0_center ref_n cmp_n ref_tail cmp_tail delta_tail z_tail max_abs_component_z")
    for key in sorted(reference):
        rows = [row for row in output
                if row["tx0_low"] == key[0] and row["tx0_high"] == key[1]]
        a, b = reference[key], comparison[key]
        n_a, n_b = rows[0]["reference_total"], rows[0]["comparison_total"]
        p_a = sum(row["weight"] for row in a[1:])
        p_b = sum(row["weight"] for row in b[1:])
        variance = p_a * (1.0 - p_a) / n_a + p_b * (1.0 - p_b) / n_b
        z_tail = ratio(p_b - p_a, math.sqrt(variance))
        max_z = max(abs(row["weight_z_score"]) for row in rows)
        print("%.8g %d %d %.8g %.8g %+.8g %+.3f %.3f" %
              (a[0]["tx0_center"], n_a, n_b, p_a, p_b, p_b - p_a,
               z_tail, max_z))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
