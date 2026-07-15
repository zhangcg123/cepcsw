#!/usr/bin/env python3
"""Screen bounded surface-lineage selection on extracted final components."""

from __future__ import annotations

import argparse
import csv
from collections import defaultdict
from pathlib import Path
import statistics


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--scores", type=Path, required=True)
    parser.add_argument("--outcomes", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument(
        "--uninformative-floor", type=float, default=0.05,
        help=("Lower bound on the consistency likelihood. The default caps "
              "the consistency Bayes factor at 20 and prevents a component "
              "below 5%% of the default winner's weight from winning."))
    return parser.parse_args()


def main() -> None:
    args = arguments()
    floor = args.uninformative_floor
    if not 0.0 < floor <= 1.0:
        raise ValueError("--uninformative-floor must be in (0, 1]")

    outcomes = {}
    with args.outcomes.open(newline="") as stream:
        for row in csv.DictReader(stream):
            outcomes[(int(row["seed"]), int(row["entry"]))] = row

    components = defaultdict(list)
    with args.scores.open(newline="") as stream:
        for row in csv.DictReader(stream):
            key = (int(row["seed"]), int(row["entry"]), row["population"])
            coincidence = float(row["forward_reverse_coincidence_noisy_or"])
            likelihood = floor + (1.0 - floor) * coincidence
            weight = float(row["component_weight"])
            components[key].append({
                "rank": int(row["component_rank"]),
                "id": int(row["component_id"]),
                "weight": weight,
                "pt": float(row["component_pt"]),
                "coincidence": coincidence,
                "likelihood": likelihood,
                "score": weight * likelihood,
            })

    rows = []
    for (seed, entry, population), candidates in sorted(components.items()):
        default = next(item for item in candidates if item["rank"] == 0)
        selected = max(candidates, key=lambda item: item["score"])
        outcome = outcomes[(seed, entry)]
        default_residual = float(outcome["gsf_residual_pct"])
        truth_pt = default["pt"] / (1.0 + default_residual / 100.0)
        selected_residual = 100.0 * (selected["pt"] / truth_pt - 1.0)
        rows.append({
            "seed": seed,
            "entry": entry,
            "population": population,
            "uninformative_floor": floor,
            "max_consistency_bayes_factor": 1.0 / floor,
            "default_id": default["id"],
            "default_weight": default["weight"],
            "default_pt": default["pt"],
            "default_coincidence": default["coincidence"],
            "default_likelihood": default["likelihood"],
            "default_score": default["score"],
            "selected_id": selected["id"],
            "selected_rank": selected["rank"],
            "selected_weight": selected["weight"],
            "selected_pt": selected["pt"],
            "selected_coincidence": selected["coincidence"],
            "selected_likelihood": selected["likelihood"],
            "selected_score": selected["score"],
            "changed": int(selected["rank"] != 0),
            "default_residual_pct": default_residual,
            "selected_residual_pct": selected_residual,
            "absolute_residual_change_pct": (
                abs(selected_residual) - abs(default_residual)),
        })

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)

    for population in ("overshoot", "control"):
        group = [row for row in rows if row["population"] == population]
        changed = [row for row in group if row["changed"]]
        improvements = [row["absolute_residual_change_pct"] for row in changed]
        summary = (
            f"{population}: changed={len(changed)}/{len(group)}"
            f", improve/worsen={sum(value < 0 for value in improvements)}/"
            f"{sum(value > 0 for value in improvements)}")
        if improvements:
            summary += f", median |residual| change={statistics.median(improvements):.6g} pp"
        print(summary)
        for row in changed:
            print(
                f"  {row['seed']}/{row['entry']}: id {row['default_id']} -> "
                f"{row['selected_id']}, pT {row['default_pt']:.6g} -> "
                f"{row['selected_pt']:.6g}, residual "
                f"{row['default_residual_pct']:.6g}% -> "
                f"{row['selected_residual_pct']:.6g}%")
    print(f"wrote {len(rows)} event selections to {args.output}")


if __name__ == "__main__":
    main()
