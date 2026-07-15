#!/usr/bin/env python3
"""Compare two current-code GSF tuple directories on an explicit event list."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path

import numpy as np
import ROOT


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--events", type=Path, required=True)
    parser.add_argument("--filter-column")
    parser.add_argument("--baseline-dir", type=Path, required=True)
    parser.add_argument("--candidate-dir", type=Path, required=True)
    parser.add_argument("--baseline-label", default="Baseline")
    parser.add_argument("--candidate-label", default="Candidate")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--summary", type=Path)
    return parser.parse_args()


def main() -> None:
    args = arguments()
    with args.events.open(newline="") as stream:
        events = [row for row in csv.DictReader(stream)
                  if not args.filter_column or int(row[args.filter_column])]
    files = {}
    rows = []
    try:
        for event in events:
            seed, entry = int(event["seed"]), int(event["entry"])
            if seed not in files:
                pair = []
                for directory in (args.baseline_dir, args.candidate_dir):
                    source = ROOT.TFile.Open(str(directory / f"gsf-flat-{seed}.root"))
                    tree = source.Get("gsf_tuple") if source else None
                    if not tree:
                        raise RuntimeError(f"missing tuple for seed {seed} in {directory}")
                    pair.append((source, tree))
                files[seed] = pair
            (_, baseline), (_, candidate) = files[seed]
            baseline.GetEntry(entry)
            candidate.GetEntry(entry)
            truth = float(candidate.mc_pT)
            baseline_pt, candidate_pt = float(baseline.gsf_pT), float(candidate.gsf_pT)
            baseline_residual = 100.0 * (baseline_pt / truth - 1.0)
            candidate_residual = 100.0 * (candidate_pt / truth - 1.0)
            rows.append({
                "seed": seed, "entry": entry,
                "outcome": event.get("outcome", ""),
                "owned_loss_pct": event.get("owned_loss_pct", ""),
                "baseline_pt": baseline_pt, "candidate_pt": candidate_pt,
                "baseline_residual_pct": baseline_residual,
                "candidate_residual_pct": candidate_residual,
                "candidate_minus_baseline_pct": candidate_residual - baseline_residual,
                "absolute_error_improvement_pct": (
                    abs(baseline_residual) - abs(candidate_residual)),
                "changed": int(abs(candidate_residual - baseline_residual) > 1.0e-8),
                "material_change": int(
                    abs(candidate_residual - baseline_residual) > 0.1),
                "improved": int(abs(baseline_residual) - abs(candidate_residual) > 0.1),
                "worsened": int(abs(candidate_residual) - abs(baseline_residual) > 0.1),
                "baseline_nhits": int(baseline.gsf_nhits),
                "candidate_nhits": int(candidate.gsf_nhits),
            })
    finally:
        for pair in files.values():
            for source, _ in pair:
                source.Close()

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)
    changed = [row for row in rows if row["changed"]]
    material = [row for row in rows if row["material_change"]]
    print(f"direct A/B events={len(rows)} changed={len(changed)} "
          f"materially_changed={len(material)} "
          f"improved={sum(row['improved'] for row in rows)} "
          f"worsened={sum(row['worsened'] for row in rows)}")
    for row in material:
        print(f"  {row['seed']}/{row['entry']}: {row['baseline_residual_pct']:.6g}% "
              f"-> {row['candidate_residual_pct']:.6g}%")
    if args.summary:
        def metrics(label: str, values: np.ndarray) -> dict:
            q16, median, q84 = np.quantile(values, [0.16, 0.5, 0.84])
            return {
                "sample": label, "count": len(values), "median_pct": median,
                "q16_pct": q16, "q84_pct": q84,
                "width68_pct": 0.5 * (q84 - q16),
                "mean_abs_pct": np.mean(np.abs(values)),
                "rms_pct": np.sqrt(np.mean(values * values)),
                "inside_1pct": np.count_nonzero(np.abs(values) <= 1.0),
                "inside_2pct": np.count_nonzero(np.abs(values) <= 2.0),
                "inside_5pct": np.count_nonzero(np.abs(values) <= 5.0),
                "inside_10pct": np.count_nonzero(np.abs(values) <= 10.0),
                "beyond_10pct": np.count_nonzero(np.abs(values) > 10.0),
            }
        baseline_values = np.asarray(
            [row["baseline_residual_pct"] for row in rows])
        candidate_values = np.asarray(
            [row["candidate_residual_pct"] for row in rows])
        summaries = [metrics(args.baseline_label, baseline_values),
                     metrics(args.candidate_label, candidate_values)]
        args.summary.parent.mkdir(parents=True, exist_ok=True)
        with args.summary.open("w", newline="") as stream:
            writer = csv.DictWriter(stream, fieldnames=summaries[0].keys())
            writer.writeheader()
            writer.writerows(summaries)
        print(args.summary.read_text(), end="")
    print(args.output)


if __name__ == "__main__":
    main()
