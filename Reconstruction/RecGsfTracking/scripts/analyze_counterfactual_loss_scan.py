#!/usr/bin/env python3
"""Extract truth-surface versus inward counterfactual loss-scan evidence."""

from __future__ import annotations

import argparse
import csv
import re
from pathlib import Path


EVENT = re.compile(r"GSF event index (\d+)")
RESULT = re.compile(
    r"COUNTERFACTUAL LOSS SCAN result truthHit=(\d+) processHit=(-?\d+) "
    r"loss=([0-9.eE+-]+) valid=(\d+) acceptedHits=(\d+) "
    r"cumulativeLogL=([0-9.eE+-]+) finalPt=([0-9.eE+-]+)")


def parse_event(path: Path, entry: int) -> list[dict]:
    rows = []
    active = False
    with path.open(errors="replace") as stream:
        for line in stream:
            event = EVENT.search(line)
            if event:
                active = int(event.group(1)) == entry
                continue
            if not active:
                continue
            result = RESULT.search(line)
            if result:
                rows.append({
                    "truth_transition": int(result.group(1)),
                    "process_transition": int(result.group(2)),
                    "loss_fraction": float(result.group(3)),
                    "valid": int(result.group(4)),
                    "accepted_hits": int(result.group(5)),
                    "cumulative_log_likelihood": float(result.group(6)),
                    "final_pT_GeV": float(result.group(7)),
                })
    if not rows:
        raise RuntimeError(f"No scan results for event {entry} in {path}")
    return rows


def write(path: Path, rows: list[dict]) -> None:
    with path.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--events", type=Path, required=True)
    parser.add_argument("--log-dir", type=Path, required=True)
    parser.add_argument("--additional-log-dir", type=Path, action="append",
                        default=[])
    parser.add_argument("--sample", required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    args = parser.parse_args()
    branch_rows = []
    event_rows = []
    with args.events.open(newline="") as stream:
        for source in csv.DictReader(stream):
            seed, entry = int(source["seed"]), int(source["entry"])
            scans_by_key = {}
            for log_dir in [args.log_dir, *args.additional_log_dir]:
                for scan in parse_event(log_dir / f"seed-{seed}.log", entry):
                    scans_by_key[(scan["process_transition"],
                                  scan["loss_fraction"])] = scan
            scans = list(scans_by_key.values())
            truth = int(source["dominant_transition_index"])
            for scan in scans:
                branch_rows.append({"sample": args.sample, "seed": seed,
                                    "entry": entry, **scan})
            truth_scans = [scan for scan in scans
                           if scan["valid"] and
                           scan["process_transition"] == truth]
            inward_scans = [scan for scan in scans
                            if scan["valid"] and
                            scan["process_transition"] == truth - 1]
            baseline = next(scan for scan in scans
                            if scan["process_transition"] == -1)
            best_truth = max(truth_scans,
                             key=lambda row: row["cumulative_log_likelihood"])
            best_inward = max(inward_scans,
                              key=lambda row: row["cumulative_log_likelihood"])
            event_rows.append({
                "sample": args.sample, "seed": seed, "entry": entry,
                "truth_transition": truth,
                "owned_loss_pct": source.get("owned_loss_pct", ""),
                "stored_gsf_residual_pct": source.get(
                    "gsf_maxcomp12_residual_pct",
                    source.get("gsf_residual_pct", "")),
                "baseline_log_likelihood": baseline["cumulative_log_likelihood"],
                "best_truth_loss_fraction": best_truth["loss_fraction"],
                "best_truth_log_likelihood":
                    best_truth["cumulative_log_likelihood"],
                "best_truth_final_pT_GeV": best_truth["final_pT_GeV"],
                "best_inward_loss_fraction": best_inward["loss_fraction"],
                "best_inward_log_likelihood":
                    best_inward["cumulative_log_likelihood"],
                "best_inward_final_pT_GeV": best_inward["final_pT_GeV"],
                "delta_log_likelihood_truth_minus_inward": (
                    best_truth["cumulative_log_likelihood"] -
                    best_inward["cumulative_log_likelihood"]),
                "truth_surface_wins": int(
                    best_truth["cumulative_log_likelihood"] >
                    best_inward["cumulative_log_likelihood"]),
            })
    args.output_dir.mkdir(parents=True, exist_ok=True)
    write(args.output_dir / f"{args.sample}_branches.csv", branch_rows)
    write(args.output_dir / f"{args.sample}_events.csv", event_rows)
    print(f"Wrote {len(event_rows)} events and {len(branch_rows)} branches")


if __name__ == "__main__":
    main()
