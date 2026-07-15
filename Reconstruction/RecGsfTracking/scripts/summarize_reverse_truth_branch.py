#!/usr/bin/env python3
"""Summarize best and truth-pT-compatible components in verbose reverse logs."""

import argparse
import csv
import re


MIX = re.compile(r"MIX reverse-after-hit\s+hit=\s*(-?\d+)")
COMP = re.compile(
    r"top\d+\s+comp\[\d+\] id=(\d+).*?noRad=(\d+) w=([0-9.eE+-]+)"
    r".*?pT=([0-9.eE+-]+)")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("logs", nargs="+")
    parser.add_argument("--truth-pt", type=float, default=2.0)
    parser.add_argument("--max-hit", type=int, default=12)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()
    rows = []
    for path in args.logs:
        hit = None
        components = []

        def emit():
            if hit is None or hit > args.max_hit or not components:
                return
            best = max(components, key=lambda item: item["weight"])
            truth = min(components,
                        key=lambda item: abs(item["pt"] - args.truth_pt))
            compatible = [item for item in components
                          if abs(item["pt"] / args.truth_pt - 1.0) <= 0.01]
            compatible_weight = sum(item["weight"] for item in compatible)
            rows.append({
                "log": path, "hit": hit, "component_count": len(components),
                "best_id": best["id"], "best_weight": best["weight"],
                "best_pt": best["pt"], "best_no_rad": best["no_rad"],
                "truth_id": truth["id"], "truth_weight": truth["weight"],
                "truth_pt": truth["pt"], "truth_no_rad": truth["no_rad"],
                "truth_to_best_weight": truth["weight"] / best["weight"],
                "compatible_1pct_count": len(compatible),
                "compatible_1pct_weight": compatible_weight,
                "compatible_to_best_weight": compatible_weight / best["weight"],
            })

        with open(path, errors="replace") as stream:
            for line in stream:
                match = MIX.search(line)
                if match:
                    emit()
                    hit = int(match.group(1))
                    components = []
                    continue
                match = COMP.search(line)
                if hit is not None and match:
                    components.append({
                        "id": int(match.group(1)), "no_rad": int(match.group(2)),
                        "weight": float(match.group(3)),
                        "pt": float(match.group(4)),
                    })
            emit()
    rows.sort(key=lambda row: (row["log"], -row["hit"]))
    with open(args.output, "w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    for row in rows:
        print("%s hit=%2d best %.6f@%.4f truth %.6g@%.4f ratio %.6g compat %.6g" %
              (row["log"], row["hit"], row["best_weight"], row["best_pt"],
               row["truth_weight"], row["truth_pt"],
               row["truth_to_best_weight"], row["compatible_1pct_weight"]))


if __name__ == "__main__":
    main()
