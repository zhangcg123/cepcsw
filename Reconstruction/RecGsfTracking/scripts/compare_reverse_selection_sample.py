#!/usr/bin/env python3
"""Compare a rerun reverse-selection sample with the baseline outcome table."""

from __future__ import annotations

import argparse
import csv
from collections import Counter, defaultdict
from pathlib import Path

import numpy as np
import ROOT


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--category", required=True,
                        choices=("no_ebrem", "light_ebrem", "hard_ebrem"))
    parser.add_argument("--input-dir", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--candidate-label", default="SurfaceConsistency")
    parser.add_argument(
        "--outcomes", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/topology_clean_event_outcomes.csv"))
    return parser.parse_args()


def metrics(values: np.ndarray) -> dict[str, float | int]:
    q16, median, q84 = np.quantile(values, [0.16, 0.5, 0.84])
    return {
        "count": len(values), "median_pct": median, "q16_pct": q16,
        "q84_pct": q84, "width68_pct": 0.5 * (q84 - q16),
        "rms_pct": np.sqrt(np.mean(values * values)),
        "inside_1pct": np.count_nonzero(np.abs(values) <= 1),
        "inside_2pct": np.count_nonzero(np.abs(values) <= 2),
        "inside_5pct": np.count_nonzero(np.abs(values) <= 5),
        "inside_10pct": np.count_nonzero(np.abs(values) <= 10),
        "beyond_10pct": np.count_nonzero(np.abs(values) > 10),
    }


def main() -> None:
    args = arguments()
    baseline = []
    with args.outcomes.open(newline="") as stream:
        for row in csv.DictReader(stream):
            if (row["category"] == args.category
                    and not int(row["excluded_secondary_topology"])):
                baseline.append(row)

    eventwise = []
    files: dict[int, tuple[ROOT.TFile, object]] = {}
    try:
        for row in baseline:
            seed, entry = int(row["seed"]), int(row["entry"])
            if seed not in files:
                source = ROOT.TFile.Open(str(args.input_dir / f"gsf-flat-{seed}.root"))
                tree = source.Get("gsf_tuple") if source else None
                if not tree:
                    raise RuntimeError(f"Missing tuple for seed {seed}")
                files[seed] = (source, tree)
            tree = files[seed][1]
            tree.GetEntry(entry)
            truth, candidate_pt = float(tree.mc_pT), float(tree.gsf_pT)
            candidate = 100 * (candidate_pt - truth) / truth
            base = float(row["gsf_residual_pct"])
            eventwise.append({
                "seed": seed, "entry": entry, "baseline_outcome": row["outcome"],
                "owned_loss_pct": row["owned_loss_pct"],
                "lcio_residual_pct": row["lcio_residual_pct"],
                "baseline_gsf_residual_pct": base,
                "candidate_gsf_residual_pct": candidate,
                "candidate_minus_baseline_pct": candidate - base,
                "absolute_error_improvement_pct": abs(base) - abs(candidate),
                "material_change": int(abs(candidate - base) > 0.1),
                "improved": int(abs(base) - abs(candidate) > 0.1),
                "worsened": int(abs(candidate) - abs(base) > 0.1),
                "new_beyond_10pct": int(abs(base) <= 10 and abs(candidate) > 10),
                "fixed_beyond_10pct": int(abs(base) > 10 and abs(candidate) <= 10),
                "gsf_nhits": int(tree.gsf_nhits),
            })
    finally:
        for source, _ in files.values():
            source.Close()

    args.output_dir.mkdir(parents=True, exist_ok=True)
    event_path = args.output_dir / f"{args.category}_eventwise.csv"
    with event_path.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(eventwise[0]))
        writer.writeheader()
        writer.writerows(eventwise)

    base_values = np.asarray([float(row["baseline_gsf_residual_pct"])
                              for row in eventwise])
    candidate_values = np.asarray([float(row["candidate_gsf_residual_pct"])
                                   for row in eventwise])
    metric_rows = []
    for label, values in (("AggregateWeight_baseline", base_values),
                          (f"{args.candidate_label}_candidate", candidate_values)):
        metric_rows.append({"sample": label, **metrics(values)})
    metric_path = args.output_dir / f"{args.category}_summary.csv"
    with metric_path.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(metric_rows[0]))
        writer.writeheader()
        writer.writerows(metric_rows)

    changed = [row for row in eventwise if row["material_change"]]
    transitions = defaultdict(Counter)
    for row in changed:
        direction = "improved" if row["improved"] else (
            "worsened" if row["worsened"] else "changed_equal_abs")
        transitions[row["baseline_outcome"]][direction] += 1
    print(metric_path.read_text(), end="")
    print(f"materially changed={len(changed)} improved={sum(r['improved'] for r in eventwise)} "
          f"worsened={sum(r['worsened'] for r in eventwise)}")
    print(f"new_beyond_10={sum(r['new_beyond_10pct'] for r in eventwise)} "
          f"fixed_beyond_10={sum(r['fixed_beyond_10pct'] for r in eventwise)}")
    for outcome, counts in sorted(transitions.items()):
        print(outcome, dict(counts))
    print(event_path)


if __name__ == "__main__":
    main()
