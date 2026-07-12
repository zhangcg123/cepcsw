#!/usr/bin/env python3
"""Extract a fit-ready, step-t/X0-conditioned retained-momentum mixture.

This closure-stage extractor uses five fixed loss strata and records the
conditional mean and variance of z=p_after/p_before within each t/X0 bin.
The strata prevent the dominant negligible-loss peak from absorbing the rare
eBrem tail.  The output is a tabulated provisional model, not a smooth or
independently validated final Bethe-Heitler parameterization.
"""

import argparse
import csv
import glob
import json
import math
import os
import sys


DEFAULT_TX0_EDGES = (0.0, 1e-4, 5e-4, 2e-3, 5e-3, 1e-2, 1.5e-2,
                     2e-2, 3e-2)
LOSS_EDGES = (0.0, 1e-4, 1e-2, 5e-2, 2e-1, 1.0 + 1e-12)
LOSS_LABELS = ("negligible", "small", "moderate", "large", "extreme")
EBREM_LOSS_LABELS = ("no_ebrem", "small", "moderate", "large", "extreme")
OUTPUT_FIELDS = (
    "tx0_low", "tx0_high", "tx0_center", "component", "loss_class",
    "count", "weight", "mean_z", "variance_z", "sigma_z",
    "ebrem_count", "ebrem_fraction", "mean_minus_log_z",
)


def expand_inputs(patterns):
    result = []
    for pattern in patterns:
        matches = sorted(glob.glob(pattern))
        if not matches:
            raise FileNotFoundError("no input matches: %s" % pattern)
        result.extend(matches)
    return list(dict.fromkeys(os.path.abspath(path) for path in result))


def find_bin(value, edges):
    for index in range(len(edges) - 1):
        if edges[index] <= value < edges[index + 1]:
            return index
    return None


def new_accumulator(n_tx0, n_loss):
    return [[{"count": 0, "sum_z": 0.0, "sum_z2": 0.0,
              "sum_y": 0.0, "ebrem": 0}
             for _ in range(n_loss)] for _ in range(n_tx0)]


def extract(paths, tx0_edges, loss_source):
    accum = new_accumulator(len(tx0_edges) - 1, len(LOSS_LABELS))
    audit = {
        "input_files": len(paths), "input_rows": 0, "accepted_rows": 0,
        "outside_tx0_range": 0, "invalid_rows": 0, "ebrem_rows": 0,
        "tx0_edges": list(tx0_edges), "loss_edges": list(LOSS_EDGES),
        "loss_source": loss_source,
        "loss_variable": "1-z", "mixture_variable": "z",
    }
    required = {"g4_t_over_x0", "z", "minus_log_z", "n_ebrem_steps",
                "p_before_GeV", "ebrem_step_loss_sum_GeV"}

    for path in paths:
        with open(path, newline="") as stream:
            reader = csv.DictReader(stream)
            missing = required - set(reader.fieldnames or ())
            if missing:
                raise RuntimeError("%s lacks columns: %s" %
                                   (path, ", ".join(sorted(missing))))
            for row in reader:
                audit["input_rows"] += 1
                try:
                    tx0 = float(row["g4_t_over_x0"])
                    ebrem_steps = int(row["n_ebrem_steps"])
                    ebrem = ebrem_steps > 0
                    if loss_source == "ebrem":
                        p_before = float(row["p_before_GeV"])
                        ebrem_loss = float(row["ebrem_step_loss_sum_GeV"])
                        if p_before <= 0.0:
                            raise ValueError("non-positive p_before")
                        fraction_loss = min(1.0, max(0.0,
                            ebrem_loss / p_before)) if ebrem else 0.0
                        z = 1.0 - fraction_loss
                        y = -math.log(max(z, 1e-15))
                    else:
                        z = float(row["z"])
                        y = float(row["minus_log_z"])
                except (TypeError, ValueError):
                    audit["invalid_rows"] += 1
                    continue
                if not (math.isfinite(tx0) and math.isfinite(z) and
                        math.isfinite(y) and tx0 >= 0.0 and 0.0 <= z <= 1.0):
                    audit["invalid_rows"] += 1
                    continue
                tx0_bin = find_bin(tx0, tx0_edges)
                if tx0_bin is None:
                    audit["outside_tx0_range"] += 1
                    continue
                if loss_source == "ebrem" and not ebrem:
                    loss_bin = 0
                elif loss_source == "ebrem":
                    # Reserve component zero as an exact no-radiation atom.
                    # All non-zero radiative losses occupy four tail strata.
                    tail_bin = find_bin(1.0 - z,
                                        (0.0,) + LOSS_EDGES[2:])
                    loss_bin = None if tail_bin is None else 1 + tail_bin
                else:
                    loss_bin = find_bin(1.0 - z, LOSS_EDGES)
                if loss_bin is None:
                    audit["invalid_rows"] += 1
                    continue
                item = accum[tx0_bin][loss_bin]
                item["count"] += 1
                item["sum_z"] += z
                item["sum_z2"] += z * z
                item["sum_y"] += y
                item["ebrem"] += int(ebrem)
                audit["accepted_rows"] += 1
                audit["ebrem_rows"] += int(ebrem)

    rows = []
    for tx0_bin, components in enumerate(accum):
        total = sum(item["count"] for item in components)
        low, high = tx0_edges[tx0_bin:tx0_bin + 2]
        labels = EBREM_LOSS_LABELS if loss_source == "ebrem" else LOSS_LABELS
        for component, (label, item) in enumerate(zip(labels, components)):
            count = item["count"]
            mean = item["sum_z"] / count if count else 1.0
            variance = max(0.0, item["sum_z2"] / count - mean * mean) \
                if count else 0.0
            rows.append({
                "tx0_low": low, "tx0_high": high,
                "tx0_center": math.sqrt(low * high) if low > 0.0 else high / 2.0,
                "component": component, "loss_class": label,
                "count": count, "weight": count / total if total else 0.0,
                "mean_z": mean, "variance_z": variance,
                "sigma_z": math.sqrt(variance), "ebrem_count": item["ebrem"],
                "ebrem_fraction": item["ebrem"] / count if count else 0.0,
                "mean_minus_log_z": item["sum_y"] / count if count else 0.0,
            })
    return rows, audit


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", help="Transition CSVs or globs")
    parser.add_argument("--output", required=True, help="Mixture table CSV")
    parser.add_argument("--audit", help="Audit JSON (default: OUTPUT.audit.json)")
    parser.add_argument("--max-tx0", type=float, default=3e-2,
                        help="Upper reconstruction-range edge (default: 0.03)")
    parser.add_argument(
        "--loss-source", choices=("total", "ebrem"), default="total",
        help="Fit total transition loss or eBrem-attributed loss (default: total)")
    args = parser.parse_args()

    try:
        paths = expand_inputs(args.inputs)
        if args.max_tx0 <= DEFAULT_TX0_EDGES[-2]:
            raise ValueError("--max-tx0 must exceed %.6g" %
                             DEFAULT_TX0_EDGES[-2])
        tx0_edges = DEFAULT_TX0_EDGES[:-1] + (args.max_tx0,)
        rows, audit = extract(paths, tx0_edges, args.loss_source)
        os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
        with open(args.output, "w", newline="") as stream:
            writer = csv.DictWriter(stream, fieldnames=OUTPUT_FIELDS)
            writer.writeheader()
            writer.writerows(rows)
        audit_path = args.audit or args.output + ".audit.json"
        with open(audit_path, "w") as stream:
            json.dump(audit, stream, indent=2, sort_keys=True)
            stream.write("\n")
    except Exception as error:
        print("error: %s" % error, file=sys.stderr)
        return 1

    print("wrote %d components from %d/%d transitions to %s" %
          (len(rows), audit["accepted_rows"], audit["input_rows"], args.output))
    print("eBrem rows: %d; outside t/X0 range: %d" %
          (audit["ebrem_rows"], audit["outside_tx0_range"]))
    print("audit: %s" % audit_path)
    return 0


if __name__ == "__main__":
    sys.exit(main())
