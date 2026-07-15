#!/usr/bin/env python3
"""Summarize final reverse competitors by forward/reverse process surfaces."""

import argparse
import csv
import re
from pathlib import Path


TOP = re.compile(r"top\s*(\d+).*?\bw=([0-9.eE+-]+).*?\bpT=([0-9.eE+-]+)")
SIG = re.compile(r"signatures forward=(.*?) reverse=(.*)$")


def radiative_surfaces(signature):
    result = set()
    for item in signature.split(";"):
        fields = item.split(":")
        if len(fields) >= 2 and fields[1] != "g0":
            result.add(int(fields[0]))
    return result


def final_components(path):
    lines = path.read_text(errors="replace").splitlines()
    starts = [i for i, line in enumerate(lines)
              if "MIX reverse-after-hit" in line and "hit=  0" in line]
    if not starts:
        raise RuntimeError(f"no final reverse mixture in {path}")
    result = []
    current = None
    # Selected events can still contain additional short reconstructed tracks.
    # The resolution tuple's truth-matched primary is the first GSF track, so
    # do not silently replace it with the last reverse mixture in the log.
    for line in lines[starts[0] + 1:]:
        if "REVERSE summary:" in line:
            break
        top = TOP.search(line)
        if top:
            current = {"rank": int(top.group(1)), "weight": float(top.group(2)),
                       "pt_GeV": float(top.group(3))}
            result.append(current)
            continue
        signature = SIG.search(line)
        if signature and current is not None:
            current["forward_signature"] = signature.group(1)
            current["reverse_signature"] = signature.group(2)
            current["forward_surfaces"] = radiative_surfaces(signature.group(1))
            current["reverse_surfaces"] = radiative_surfaces(signature.group(2))
    if not result or any("reverse_surfaces" not in item for item in result):
        raise RuntimeError(f"incomplete final mixture in {path}")
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--events", type=Path, required=True)
    parser.add_argument("--pairs", type=Path, required=True)
    parser.add_argument("--logs", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    transitions = {}
    with args.pairs.open(newline="") as stream:
        for row in csv.DictReader(stream):
            transitions[(int(row["overshoot_seed"]),
                         int(row["overshoot_entry"]))] = int(row["transition"])
            transitions[(int(row["control_seed"]),
                         int(row["control_entry"]))] = int(row["transition"])

    rows = []
    with args.events.open(newline="") as stream:
        for event in csv.DictReader(stream):
            seed, entry = int(event["seed"]), int(event["entry"])
            transition = transitions[(seed, entry)]
            components = final_components(args.logs / f"seed-{seed}.log")
            winner = min(components, key=lambda item: item["rank"])
            truth = [item for item in components
                     if transition in item["reverse_surfaces"]]
            consistent = [item for item in components
                          if item["forward_surfaces"] & item["reverse_surfaces"]]
            best_truth = max(truth, key=lambda item: item["weight"]) if truth else None
            best_consistent = (max(consistent, key=lambda item: item["weight"])
                               if consistent else None)
            rows.append({
                "seed": seed, "entry": entry,
                "population": event["population"], "transition": transition,
                "final_components": len(components),
                "winner_weight": winner["weight"],
                "winner_reverse_surfaces": ";".join(map(str, sorted(
                    winner["reverse_surfaces"]))),
                "truth_surface_competitor": int(best_truth is not None),
                "truth_competitor_rank": (best_truth["rank"] if best_truth else -1),
                "truth_competitor_weight": (best_truth["weight"] if best_truth else 0),
                "truth_to_winner_weight_ratio": (
                    best_truth["weight"] / winner["weight"] if best_truth else 0),
                "forward_consistent_competitor": int(best_consistent is not None),
                "consistent_competitor_rank": (
                    best_consistent["rank"] if best_consistent else -1),
                "consistent_competitor_weight": (
                    best_consistent["weight"] if best_consistent else 0),
                "consistent_reverse_surfaces": (";".join(map(str, sorted(
                    best_consistent["reverse_surfaces"])))
                    if best_consistent else ""),
            })

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)

    for population in ("overshoot", "control"):
        selected = [row for row in rows if row["population"] == population]
        truth = [row for row in selected if row["truth_surface_competitor"]]
        consistent = [row for row in selected
                      if row["forward_consistent_competitor"]]
        competitive = [row for row in truth
                       if row["truth_to_winner_weight_ratio"] >= 0.1]
        print(f"{population}: truth-surface survives {len(truth)}/{len(selected)}, "
              f"within 10:1 {len(competitive)}/{len(selected)}, "
              f"forward-consistent survives {len(consistent)}/{len(selected)}")


if __name__ == "__main__":
    main()
