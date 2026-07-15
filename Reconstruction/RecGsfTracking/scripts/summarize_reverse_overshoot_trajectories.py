#!/usr/bin/env python3
"""Summarize best-component pT evolution in verbose reverse-GSF logs."""

import argparse
import csv
import math
import re
from pathlib import Path


MIX = re.compile(r"MIX reverse-after-hit\s+hit=\s*(\d+)")
TOP0 = re.compile(r"top0\s+comp\[\d+\] id=(\d+).*?noRad=(\d+) "
                  r"w=([0-9.eE+-]+).*?pT=([0-9.eE+-]+)")
TABLE = re.compile(r"pT\s+([0-9.eE+-]+)\s+([0-9.eE+-]+)\s+([0-9.eE+-]+)")
EVENT = re.compile(r"(\d+)-(\d+)\.log$")


def parse(path):
    states = []
    current_hit = None
    truth = lcio = gsf = None
    with path.open(errors="replace") as stream:
        for line in stream:
            match = MIX.search(line)
            if match:
                current_hit = int(match.group(1))
                continue
            match = TOP0.search(line)
            if match and current_hit is not None:
                states.append({"hit": current_hit, "id": int(match.group(1)),
                               "no_rad": int(match.group(2)),
                               "weight": float(match.group(3)),
                               "pt": float(match.group(4))})
                current_hit = None
                continue
            match = TABLE.search(line)
            if match:
                truth, lcio, gsf = map(float, match.groups())
                # The flat performance tuple and topology-clean survey use the
                # first CompleteTrack. Some events also contain a short second
                # reconstructed track, which must not overwrite this trace.
                break
    if not states or truth is None:
        raise RuntimeError(f"cannot parse {path}")
    states.sort(key=lambda row: -row["hit"])
    changes = []
    for previous, current in zip(states, states[1:]):
        changes.append((abs(current["pt"] - previous["pt"]), previous, current))
    upward_fractional_changes = [current["pt"] / previous["pt"] - 1.0
                                 for previous, current in zip(states, states[1:])
                                 if current["pt"] > previous["pt"]]
    _, before, decisive = max(changes, key=lambda item: item[0])
    first_corrected = next((state for state in states
                            if state["pt"] > lcio * 1.005), states[-1])
    first_above_truth = next((state for state in states
                              if state["pt"] > truth), None)
    event = EVENT.search(path.name)
    return {
        "seed": int(event.group(1)), "entry": int(event.group(2)),
        "truth_pt": truth, "lcio_pt": lcio, "gsf_pt": gsf,
        "first_corrected_hit": first_corrected["hit"],
        "first_corrected_pt": first_corrected["pt"],
        "first_above_truth_hit": "" if first_above_truth is None
            else first_above_truth["hit"],
        "first_above_truth_pt": "" if first_above_truth is None
            else first_above_truth["pt"],
        "largest_jump_hit": decisive["hit"],
        "largest_jump_from_pt": before["pt"],
        "largest_jump_to_pt": decisive["pt"],
        "largest_jump_size": decisive["pt"] - before["pt"],
        "final_best_weight": states[-1]["weight"],
        "final_best_no_rad": states[-1]["no_rad"],
        "upward_jumps_above_0p5pct": sum(
            change > 0.005 for change in upward_fractional_changes),
        "upward_jumps_above_1pct": sum(
            change > 0.01 for change in upward_fractional_changes),
        "cumulative_positive_log_pt_change": sum(
            math.log1p(change)
            for change in upward_fractional_changes),
        "state_count": len(states),
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("logs", nargs="+", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    rows = [parse(path) for path in args.logs]
    rows.sort(key=lambda row: (row["largest_jump_hit"], row["seed"], row["entry"]))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    print(args.output.read_text(), end="")


if __name__ == "__main__":
    main()
