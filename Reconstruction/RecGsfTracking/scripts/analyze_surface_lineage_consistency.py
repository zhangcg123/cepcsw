#!/usr/bin/env python3
"""Build event/component surface-lineage diagnostics from extracted marginals."""

from __future__ import annotations

import argparse
import csv
from collections import defaultdict
import math
from pathlib import Path
import statistics


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--marginals", type=Path, required=True)
    parser.add_argument("--pairs", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--max-surface", type=int, default=11)
    return parser.parse_args()


def main() -> None:
    args = arguments()
    truth = {}
    with args.pairs.open(newline="") as stream:
        for row in csv.DictReader(stream):
            transition = int(row["transition"])
            truth[(int(row["overshoot_seed"]),
                   int(row["overshoot_entry"]))] = transition
            truth[(int(row["control_seed"]),
                   int(row["control_entry"]))] = transition

    components = {}
    with args.marginals.open(newline="") as stream:
        for row in csv.DictReader(stream):
            key = (int(row["seed"]), int(row["entry"]),
                   int(row["component_rank"]))
            component = components.setdefault(key, {
                "seed": key[0], "entry": key[1], "component_rank": key[2],
                "population": row["population"],
                "component_id": int(row["component_id"]),
                "component_weight": float(row["component_weight"]),
                "component_pt": float(row["component_pt"]),
                "forward": {}, "reverse": {},
            })
            component[row["direction"]][int(row["hit"])] = float(
                row["radiative_mass"])

    rows = []
    for key, component in components.items():
        event = key[:2]
        transition = truth[event]
        surfaces = {surface for surface in
                    set(component["forward"]) | set(component["reverse"])
                    if surface <= args.max_surface}
        forward = [component["forward"].get(surface, 0.0)
                   for surface in surfaces]
        reverse = [component["reverse"].get(surface, 0.0)
                   for surface in surfaces]
        dot = sum(left * right for left, right in zip(forward, reverse))
        forward_norm2 = sum(value * value for value in forward)
        reverse_norm2 = sum(value * value for value in reverse)
        cosine = (dot / math.sqrt(forward_norm2 * reverse_norm2)
                  if forward_norm2 > 0.0 and reverse_norm2 > 0.0 else 0.0)
        overlap = sum(min(left, right) for left, right in zip(forward, reverse))
        union = sum(max(left, right) for left, right in zip(forward, reverse))
        # Marginal probability of at least one common radiative surface under
        # the explicit approximation that per-surface coincidence indicators
        # are independent.  Unlike cosine/Jaccard normalization, this retains
        # the absolute scale of both the forward and reverse lineage masses.
        coincidence_noisy_or = 1.0 - math.prod(
            1.0 - left * right for left, right in zip(forward, reverse))
        rows.append({
            key: component[key] for key in
            ("seed", "entry", "population", "component_rank",
             "component_id", "component_weight", "component_pt")
        } | {
            "truth_transition": transition,
            "truth_surface_reverse_mass": component["reverse"].get(
                transition, 0.0),
            "one_hit_inward_reverse_mass": component["reverse"].get(
                transition - 1, 0.0),
            "forward_reverse_radiative_dot": dot,
            "forward_reverse_radiative_cosine": cosine,
            "forward_reverse_radiative_overlap": overlap,
            "forward_reverse_radiative_jaccard": overlap / union if union else 0.0,
            "forward_reverse_coincidence_noisy_or": coincidence_noisy_or,
        })

    rows.sort(key=lambda row: (row["population"], row["seed"], row["entry"],
                               row["component_rank"]))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)

    selected = [row for row in rows if row["component_rank"] == 0]
    for population in ("overshoot", "control"):
        group = [row for row in selected if row["population"] == population]
        truth_mass = [row["truth_surface_reverse_mass"] for row in group]
        inward_mass = [row["one_hit_inward_reverse_mass"] for row in group]
        print(
            f"{population}: n={len(group)} median truth/inward mass="
            f"{statistics.median(truth_mass):.6g}/{statistics.median(inward_mass):.6g}; "
            f"truth>inward {sum(a > b for a, b in zip(truth_mass, inward_mass))}, "
            f"inward>truth {sum(b > a for a, b in zip(truth_mass, inward_mass))}")
    print(f"wrote {len(rows)} final-component consistency rows to {args.output}")


if __name__ == "__main__":
    main()
