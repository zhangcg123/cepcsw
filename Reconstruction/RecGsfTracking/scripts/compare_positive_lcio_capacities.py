#!/usr/bin/env python3
"""Compare two positive-LCIO diagnostic tables event by event."""

import argparse
import csv
from pathlib import Path


def load(path):
    with path.open(newline="") as stream:
        return {(int(row["seed"]), int(row["entry"])): row
                for row in csv.DictReader(stream)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--reference-label", default="24")
    parser.add_argument("--candidate-label", default="12")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    reference, candidate = load(args.reference), load(args.candidate)
    if reference.keys() != candidate.keys():
        raise RuntimeError("event sets differ")
    rows = []
    for key in sorted(reference):
        old, new = reference[key], candidate[key]
        old_res = float(old["current_gsf_residual_pct"])
        new_res = float(new["current_gsf_residual_pct"])
        delta_abs = abs(new_res) - abs(old_res)
        rows.append({
            "seed": key[0], "entry": key[1], "category": old["category"],
            "stored_amplification_pct": old["stored_amplification_pct"],
            f"residual_{args.reference_label}_pct": old_res,
            f"residual_{args.candidate_label}_pct": new_res,
            f"amplification_{args.reference_label}_pct": old["current_amplification_pct"],
            f"amplification_{args.candidate_label}_pct": new["current_amplification_pct"],
            f"delta_abs_residual_{args.candidate_label}_minus_{args.reference_label}_pct": delta_abs,
            "outcome": "improved" if delta_abs < -1e-9 else
                       "worsened" if delta_abs > 1e-9 else "unchanged",
            f"selected_signature_{args.reference_label}": old["selected_bh_surface_modes"],
            f"selected_signature_{args.candidate_label}": new["selected_bh_surface_modes"],
            "selection_changed": int(old["selected_bh_surface_modes"] !=
                                     new["selected_bh_surface_modes"]),
        })
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    for category in (None, "no_ebrem", "light_ebrem"):
        selected = [row for row in rows
                    if category is None or row["category"] == category]
        counts = {name: sum(row["outcome"] == name for row in selected)
                  for name in ("improved", "worsened", "unchanged")}
        print(category or "all", len(selected), counts,
              "selection_changed", sum(row["selection_changed"] for row in selected))
    print(f"wrote {len(rows)} events to {args.output}")


if __name__ == "__main__":
    main()
