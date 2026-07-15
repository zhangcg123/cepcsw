#!/usr/bin/env python3
"""Compare KL-smoother tuples with recorded LCIO and reverse-GSF values."""

from __future__ import annotations

import argparse
import csv
import json
import math
from pathlib import Path
from statistics import median

import uproot


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--events", type=Path, required=True)
    parser.add_argument("--smoother-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--summary", type=Path, required=True)
    args = parser.parse_args()

    with args.events.open(newline="") as stream:
        source_rows = list(csv.DictReader(stream))

    tuple_rows: dict[tuple[int, int], float] = {}
    for seed in sorted({int(row["seed"]) for row in source_rows}):
        tree = uproot.open(args.smoother_dir / f"gsf-flat-{seed}.root")["gsf_tuple"]
        arrays = tree.arrays(["iev", "gsf_pT"], library="np")
        # RecGsfFlatTuple stores the one-based processed-event count, whereas
        # SelectedEventIndices and the durable event tables are zero-based.
        for event_count, pt in zip(arrays["iev"], arrays["gsf_pT"]):
            tuple_rows[(seed, int(event_count) - 1)] = float(pt)

    rows = []
    for source in source_rows:
        key = (int(source["seed"]), int(source["entry"]))
        truth = float(source["truth_pt_GeV"])
        lcio = float(source["lcio_pt_GeV"])
        reverse = float(source["old_gsf_pt_GeV"])
        smoother = tuple_rows[key]
        residual = lambda pt: 100.0 * (pt - truth) / truth
        lcio_residual = residual(lcio)
        reverse_residual = residual(reverse)
        smoother_residual = residual(smoother)
        rows.append({
            "selection_rank": int(source["selection_rank"]),
            "seed": key[0], "entry": key[1],
            "dominant_transition_index": int(source["dominant_transition_index"]),
            "dominant_transition_loss_pct": float(
                source["dominant_transition_loss_pct"]),
            "truth_pt_GeV": truth, "lcio_pt_GeV": lcio,
            "reverse_pt_GeV": reverse, "smoother_pt_GeV": smoother,
            "lcio_residual_pct": lcio_residual,
            "reverse_residual_pct": reverse_residual,
            "smoother_residual_pct": smoother_residual,
            "smoother_minus_reverse_abs_residual_pct_point":
                abs(smoother_residual) - abs(reverse_residual),
            "smoother_minus_lcio_abs_residual_pct_point":
                abs(smoother_residual) - abs(lcio_residual),
        })

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)

    def metrics(field: str) -> dict[str, float | int]:
        residuals = [row[field] for row in rows]
        absolute = [abs(value) for value in residuals]
        return {
            "mean_residual_pct": sum(residuals) / len(residuals),
            "mean_abs_residual_pct": sum(absolute) / len(absolute),
            "median_abs_residual_pct": median(absolute),
            "rms_residual_pct": math.sqrt(
                sum(value * value for value in residuals) / len(residuals)),
            "inside_1pct": sum(value <= 1.0 for value in absolute),
            "inside_2pct": sum(value <= 2.0 for value in absolute),
        }

    reverse_changes = [
        row["smoother_minus_reverse_abs_residual_pct_point"] for row in rows]
    lcio_changes = [
        row["smoother_minus_lcio_abs_residual_pct_point"] for row in rows]
    summary = {
        "events": len(rows),
        "lcio": metrics("lcio_residual_pct"),
        "reverse": metrics("reverse_residual_pct"),
        "smoother": metrics("smoother_residual_pct"),
        "smoother_vs_reverse": {
            "improved": sum(value < -1.0e-9 for value in reverse_changes),
            "worsened": sum(value > 1.0e-9 for value in reverse_changes),
            "unchanged": sum(abs(value) <= 1.0e-9 for value in reverse_changes),
        },
        "smoother_vs_lcio": {
            "improved": sum(value < -1.0e-9 for value in lcio_changes),
            "worsened": sum(value > 1.0e-9 for value in lcio_changes),
            "unchanged": sum(abs(value) <= 1.0e-9 for value in lcio_changes),
        },
        "largest_smoother_improvement_vs_reverse": min(
            rows, key=lambda row:
            row["smoother_minus_reverse_abs_residual_pct_point"]),
        "largest_smoother_regression_vs_reverse": max(
            rows, key=lambda row:
            row["smoother_minus_reverse_abs_residual_pct_point"]),
    }
    with args.summary.open("w") as stream:
        json.dump(summary, stream, indent=2)
        stream.write("\n")
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
