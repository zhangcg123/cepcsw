#!/usr/bin/env python3
"""Compare two comprehensive GSF reruns against an event-list truth table."""

import argparse
import csv
import json
import math
from pathlib import Path
import re
from statistics import median


IP_PATTERN = re.compile(r"REVERSE IP output:.*?pT=([0-9.eE+-]+)")
SUMMARY_PATTERN = re.compile(
    r"REVERSE summary: finalComps=(\d+) accepted=(\d+) rejected=(\d+)")


def read_outputs(directory, entries_by_seed):
    result = {}
    for seed, entries in entries_by_seed.items():
        text = (directory / f"seed-{seed}.log").read_text(errors="replace")
        pts = [float(value) for value in IP_PATTERN.findall(text)]
        summaries = [tuple(map(int, values))
                     for values in SUMMARY_PATTERN.findall(text)]
        if len(pts) != len(entries) or len(summaries) != len(entries):
            raise RuntimeError(
                f"seed {seed}: expected {len(entries)} outputs, "
                f"found {len(pts)} pT values and {len(summaries)} summaries")
        for entry, pt, summary in zip(entries, pts, summaries):
            result[(seed, entry)] = {
                "pt": pt, "final_components": summary[0],
                "accepted": summary[1], "rejected": summary[2]}
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--events", type=Path, required=True)
    parser.add_argument("--old-dir", type=Path, required=True)
    parser.add_argument("--new-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--summary", type=Path, required=True)
    args = parser.parse_args()

    with args.events.open(newline="") as stream:
        rows = list(csv.DictReader(stream))
    entries_by_seed = {}
    for row in rows:
        entries_by_seed.setdefault(int(row["seed"]), []).append(int(row["entry"]))
    old = read_outputs(args.old_dir, entries_by_seed)
    new = read_outputs(args.new_dir, entries_by_seed)

    output_rows = []
    for row in rows:
        key = (int(row["seed"]), int(row["entry"]))
        truth = float(row["truth_pT_GeV"])
        lcio = float(row["lcio_pT_GeV"])
        old_item, new_item = old[key], new[key]
        residual = lambda pt: 100.0 * (pt - truth) / truth
        lcio_res = residual(lcio)
        old_res = residual(old_item["pt"])
        new_res = residual(new_item["pt"])
        output_rows.append({
            "selection_rank": int(row["selection_rank"]),
            "seed": key[0], "entry": key[1],
            "dominant_transition_index": int(row["dominant_transition_index"]),
            "dominant_transition_loss_pct": float(row["dominant_transition_loss_pct"]),
            "truth_pt_GeV": truth, "lcio_pt_GeV": lcio,
            "old_gsf_pt_GeV": old_item["pt"], "new_gsf_pt_GeV": new_item["pt"],
            "lcio_residual_pct": lcio_res,
            "old_gsf_residual_pct": old_res,
            "new_gsf_residual_pct": new_res,
            "new_minus_old_abs_residual_pct_point": abs(new_res) - abs(old_res),
            "old_final_components": old_item["final_components"],
            "old_reverse_accepted": old_item["accepted"],
            "old_reverse_rejected": old_item["rejected"],
            "new_final_components": new_item["final_components"],
            "new_reverse_accepted": new_item["accepted"],
            "new_reverse_rejected": new_item["rejected"],
        })

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=output_rows[0].keys())
        writer.writeheader()
        writer.writerows(output_rows)

    changes = [row["new_minus_old_abs_residual_pct_point"]
               for row in output_rows]
    old_abs = [abs(row["old_gsf_residual_pct"]) for row in output_rows]
    new_abs = [abs(row["new_gsf_residual_pct"]) for row in output_rows]
    summary = {
        "events": len(output_rows),
        "improved": sum(change < -1e-9 for change in changes),
        "worsened": sum(change > 1e-9 for change in changes),
        "unchanged": sum(abs(change) <= 1e-9 for change in changes),
        "mean_abs_residual_old_pct": sum(old_abs) / len(old_abs),
        "mean_abs_residual_new_pct": sum(new_abs) / len(new_abs),
        "median_abs_residual_old_pct": median(old_abs),
        "median_abs_residual_new_pct": median(new_abs),
        "rms_residual_old_pct": math.sqrt(sum(value * value for value in old_abs) / len(old_abs)),
        "rms_residual_new_pct": math.sqrt(sum(value * value for value in new_abs) / len(new_abs)),
        "inside_1pct_old": sum(value <= 1.0 for value in old_abs),
        "inside_1pct_new": sum(value <= 1.0 for value in new_abs),
        "total_reverse_rejected_old": sum(row["old_reverse_rejected"] for row in output_rows),
        "total_reverse_rejected_new": sum(row["new_reverse_rejected"] for row in output_rows),
        "maximum_improvement": min(output_rows, key=lambda row: row["new_minus_old_abs_residual_pct_point"]),
        "maximum_worsening": max(output_rows, key=lambda row: row["new_minus_old_abs_residual_pct_point"]),
    }
    with args.summary.open("w") as stream:
        json.dump(summary, stream, indent=2)
        stream.write("\n")
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
